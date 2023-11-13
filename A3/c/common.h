#pragma once
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "compsys_helpers.h"

/*
 * add macros and function declarations which should be 
 * common between the name server and peer programs in this file.
 */

#define USERNAME_LEN                        16
#define PASSWORD_LEN                        16
#define SALT_LEN                            64
#define REQUEST_HEADER_LEN                  USERNAME_LEN+SHA256_HASH_SIZE+4
#define REQUEST_BODY_LEN                    128


#define RESPONSE_HEADER_LEN     4+4+4+4+SHA256_HASH_SIZE+SHA256_HASH_SIZE
#define MAX_MSG_LEN             1024
#define MAX_PAYLOAD             MAX_MSG_LEN-(RESPONSE_HEADER_LEN)

#define IP_LEN             16
#define PORT_LEN           8
#define PATH_LEN           128

#define CLIENT_DIR_KEY     "client_directory: " 
#define CLIENT_PORT        "client_port: " 
#define CLIENT_IP          "client_ip: " 
#define SERVER_DIR_KEY     "server_directory: " 
#define SERVER_PORT        "server_port: " 
#define SERVER_IP          "server_ip: " 

/* slightly less awkward string equality check */
#define string_equal(x, y) (strncmp((x), (y), strlen(y)) == 0)

/*
 * naive validity checks of IP addresses and port numbers given as strings,
 * eg. at command line. both return zero on invalid IP/port, else non-zero.
 */
int is_valid_ip(char *ip_string);
int is_valid_port(char *port_string);
int starts_with(const char *a, const char *b);
