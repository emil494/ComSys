#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./networking.h"
#include "./sha256.h"

char server_ip[IP_LEN];
char server_port[PORT_LEN];
char my_ip[IP_LEN];
char my_port[PORT_LEN];

int network_socket, c;
compsys_helper_state_t state;

/*
 * Gets a sha256 hash of specified data, sourcedata. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_data_sha(const char* sourcedata, hashdata_t hash, uint32_t data_size, 
    int hash_size)
{
  SHA256_CTX shactx;
  unsigned char shabuffer[hash_size];
  sha256_init(&shactx);
  sha256_update(&shactx, sourcedata, data_size);
  sha256_final(&shactx, shabuffer);

  for (int i=0; i<hash_size; i++)
  {
    hash[i] = shabuffer[i];
  }
}

/*
 * Gets a sha256 hash of specified data file, sourcefile. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_file_sha(const char* sourcefile, hashdata_t hash, int size)
{
    int casc_file_size;

    FILE* fp = fopen(sourcefile, "rb");
    if (fp == 0)
    {
        printf("Failed to open source: %s\n", sourcefile);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    casc_file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char buffer[casc_file_size];
    fread(buffer, casc_file_size, 1, fp);
    fclose(fp);

    get_data_sha(buffer, hash, casc_file_size, size);
}

/*
 * Combine a password and salt together and hash the result to form the 
 * 'signature'. The result should be written to the 'hash' variable. Note that 
 * as handed out, this function is never called. You will need to decide where 
 * it is sensible to do so.
 */
void get_signature(char* password, char* salt, hashdata_t* hash)
{
    char to_hash[strlen(password) + strlen(salt) + 1];
    strcpy(to_hash, salt);
    strcat(to_hash, password);

    get_data_sha(to_hash, *hash, strlen(to_hash), SHA256_HASH_SIZE);
}

void build_request(char* username, char* password, char* salt, char* payload, uint32_t len, Request_t* request) {

    hashdata_t hash;
    RequestHeader_t header;

    get_signature(password, salt, &hash);

    memcpy(header.username, username, USERNAME_LEN);
    memcpy(header.salted_and_hashed, hash, SHA256_HASH_SIZE);
    header.length = htonl(len);

    request->header = header;
    if (len > 0) {
        memcpy(request->payload, payload, len);
    }
}

/*
 * Register a new user with a server by sending the username and signature to 
 * the server
 */
void register_user(char* username, char* password, char* salt)
{

    // Check wether salts.csv exists, to either create it or append to it
    FILE *fp;
    struct stat buffer;
    if (stat("salts.csv", &buffer) == 0) {
        fp = fopen("salts.csv", "a");
    } else {
        fp = fopen("salts.csv", "w");
        fprintf(fp, "username,salt\n"); 
    }

    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }
    fprintf(fp, "%s,%s\n", username, salt);
    fclose(fp);

    Request_t request;
    build_request(username, password, salt, NULL, 0, &request);

    if (compsys_helper_writen(network_socket, &request, sizeof(request)) != sizeof(request)) {
        perror("Write");
        exit(1);
    }

    RespHeader_t *respheader = malloc(sizeof(RespHeader_t));
    compsys_helper_readnb(&state, respheader, RESPONSE_HEADER_LEN);
    uint32_t length = ntohl(respheader->length);

    char message[MAX_MSG_LEN];
    compsys_helper_readnb(&state, message, length);
    printf("Got response: %s\n", message);
}

/*
 * Get a file from the server by sending the username and signature, along with
 * a file path. Note that this function should be able to deal with both small 
 * and large files. 
 */
void get_file(char* username, char* password, char* salt, char* to_get, uint32_t len)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code

    Request_t request;
    build_request(username, password, salt, to_get, len, &request);

    if (compsys_helper_writen(network_socket, &request, sizeof(request)) != sizeof(request)) {
        perror("Write");
        exit(1);
    }
    
    RespHeader_t *response_header = malloc(sizeof(RespHeader_t));
    compsys_helper_readnb(&state, response_header, RESPONSE_HEADER_LEN);

    uint32_t code = ntohl(response_header->statusCode);
    uint32_t blockcount = ntohl(response_header->blockCount);

    if (blockcount > 0 && code == 1){

        // Initialize payload array
        int32_t totalLen = 0;
        char *array[blockcount];

        hashdata_t received_hash;

        // Fetch all payloads and index them correctly in the array
        for (uint32_t i = 0; i < blockcount; i++) {
            uint32_t length = ntohl(response_header->length);
            uint32_t num = ntohl(response_header->blockNum);
            char payload[length+1];
            compsys_helper_readnb(&state, payload, length);
            payload[length] = '\0';

            // Check block hash
            get_data_sha(payload, received_hash, length, SHA256_HASH_SIZE);
            if (memcmp(received_hash, response_header->blockHash, SHA256_HASH_SIZE) != 0) {
                printf("Block %d/%d is corrupted\n", num, blockcount);
                exit(1);
            }
            
            array[num] = strdup(payload);
            totalLen += length;

            // Read next header
            compsys_helper_readnb(&state, response_header, RESPONSE_HEADER_LEN);
        }

        // Print all payloads to the file
        FILE *fp = fopen(to_get, "w");
        if (fp == NULL) {
            perror("Error opening file");
            exit(1);
        }

        for (uint32_t i = 0; i < blockcount; i++) {
            fprintf(fp, "%s", array[i]);
            free(array[i]);
        }
        fclose(fp);

        // Check file is valid
        get_file_sha(to_get, received_hash, SHA256_HASH_SIZE);
        if (memcmp(received_hash, response_header->totalHash, SHA256_HASH_SIZE) != 0) {
            perror("File is corrupted");
            exit(1);
        } 
        printf("Received file of %d bytes from %d blocks, saved to %s\n", totalLen, blockcount, to_get);
    } else {
        uint32_t length = ntohl(response_header->length);
        char message[MAX_MSG_LEN];
        compsys_helper_readnb(&state, message, length);
        printf("%s\n", message);
    }

    free(response_header);
}

int main(int argc, char **argv)
{
    srand(time(NULL));
        // Users should call this script with a single argument describing what 
    // config to use
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    // Read in configuration options. Should include a client_directory, 
    // client_ip, client_port, server_ip, and server_port
    char buffer[128];
    fprintf(stderr, "Got config path at: %s\n", argv[1]);
    FILE* fp = fopen(argv[1], "r");
    while (fgets(buffer, 128, fp)) {
        if (starts_with(buffer, CLIENT_IP)) {
            memcpy(my_ip, &buffer[strlen(CLIENT_IP)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_IP));
            if (!is_valid_ip(my_ip)) {
                fprintf(stderr, ">> Invalid client IP: %s\n", my_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, CLIENT_PORT)) {
            memcpy(my_port, &buffer[strlen(CLIENT_PORT)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_PORT));
            if (!is_valid_port(my_port)) {
                fprintf(stderr, ">> Invalid client port: %s\n", my_port);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_IP)) {
            memcpy(server_ip, &buffer[strlen(SERVER_IP)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_IP));
            if (    !is_valid_ip(server_ip)) {
                fprintf(stderr, ">> Invalid server IP: %s\n", server_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_PORT)) {
            memcpy(server_port, &buffer[strlen(SERVER_PORT)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_PORT));
            if (!is_valid_port(server_port)) {
                fprintf(stderr, ">> Invalid server port: %s\n", server_port);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(fp);

    fprintf(stdout, "Client at: %s:%s\n", my_ip, my_port);
    fprintf(stdout, "Server at: %s:%s\n", server_ip, server_port);

    network_socket = compsys_helper_open_clientfd(server_ip, server_port);
    compsys_helper_readinitb(&state, network_socket);

    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    char user_salt[SALT_LEN+1];
    
    fprintf(stdout, "Enter a username to proceed: ");
    scanf("%16s", username);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up username string as otherwise some extra chars can sneak in.
    for (int i=strlen(username); i<USERNAME_LEN; i++)
    {
        username[i] = '\0';
    }
    
    // Check if username already exists
    bool existing_user = false;
    struct stat s_buffer;
    if (stat("salts.csv", &s_buffer) == 0) {
        char line[1024];
        FILE *fp = fopen("salts.csv", "r");
        if (fp == NULL) {
            perror("Error opening file");
            exit(1);
        }
        while (fgets(line, sizeof(line), fp)) {
            if (strcmp(username, strtok(line, ",")) == 0) {
                existing_user = true;
                strncpy(user_salt, strtok(NULL, ",\n"), SALT_LEN);
                user_salt[SALT_LEN] = '\0';
                break;
            }
        }
    }

    fprintf(stdout, "Enter your password to proceed: ");
    scanf("%16s", password);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up password string as otherwise some extra chars can sneak in.
    for (int i=strlen(password); i<PASSWORD_LEN; i++)
    {
        password[i] = '\0';
    }
    
    // Generate random user salt
    if (!existing_user) {
        for (int i=0; i<SALT_LEN; i++)
        {
            user_salt[i] = 'a' + (random() % 26);
        }
        user_salt[SALT_LEN] = '\0';
    }

    fprintf(stdout, "Using salt: %s\n", user_salt);

    if (!existing_user) {
        register_user(username, password, user_salt);
    }
    close(network_socket);

    // User-interaction
    uint32_t len;
    char path[PATH_LEN];
    while (1) {
        network_socket = compsys_helper_open_clientfd(server_ip, server_port);
        printf("Enter a file you want to fetch\n");
        scanf("%128s", path);
        for (int i = strlen(path); i < PATH_LEN; i++) {
            path[i] = '\0';
        }
        if (strcmp(path, "quit") == 0) {
            close(network_socket);
            break;
        }
        len = strlen(path);
        get_file(username, password, user_salt, path, len);
        close(network_socket);
    }

    exit(EXIT_SUCCESS);
}