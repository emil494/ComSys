
#pragma once
#include "common.h"
#include "sha256.h"

// container for hasher
typedef uint8_t hashdata_t[SHA256_HASH_SIZE];

// container for assembling password with salt
typedef struct PasswordAndSalt {
    char password[PASSWORD_LEN];
    char salt[SALT_LEN];
} PasswordAndSalt_t;

// container for assembling request message headers
typedef struct RequestHeader {
    char username[USERNAME_LEN];
    hashdata_t salted_and_hashed;
    uint32_t length;
} RequestHeader_t;

// container for assembling request messages
typedef struct Request {
    RequestHeader_t header;
    char payload[PATH_LEN];
} Request_t;

typedef struct RespHeader {
    uint32_t length;
    uint32_t statusCode;
    uint32_t blockNum;
    uint32_t blockCount;
    char blockHash[SHA256_HASH_SIZE];
    char totalHash[SHA256_HASH_SIZE];
} RespHeader_t;
