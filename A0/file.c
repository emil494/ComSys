#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <stdbool.h> // Bool

//Handed error printer
int print_error(char *path, int errnum) {
  return fprintf(stdout, "%s: cannot determine (%s)\n",
    path, strerror(errnum));
}

// Checker for UTF-8 encryption
bool isUFT_8 (FILE *file, char *name) {
  unsigned char read;
  size_t bytes;
  bool isUTF = true;
  while ((bytes = fread(&read, sizeof(unsigned char), 1,  file)) == 1 && isUTF) {
    // Check if ascii
    if (read < 128) {
      continue;

    // Check if 2-byte sequence
    } else if ((read & 224) == 192) {
      unsigned char next;
      if (fread(&next, sizeof(unsigned char), 1, file) != 1 || (next & 192) != 128) {
          isUTF = false;
          break;
      }

    // Check if 3-byte sequence
    } else if ((read & 240) == 224) {
      unsigned char next1, next2;
      if (fread(&next1, sizeof(unsigned char), 1, file) != 1 || (next1 & 192) != 128 ||
          fread(&next2, sizeof(unsigned char), 1, file) != 1 || (next2 & 192) != 128) {
          isUTF = false;
          break;
      }
    
    // Check if 4-byte sequence
    } else if ((read & 248) == 240) {
      unsigned char next1, next2, next3;
      if (fread(&next1, sizeof(unsigned char), 1, file) != 1 || (next1 & 192) != 128 ||
          fread(&next2, sizeof(unsigned char), 1, file) != 1 || (next2 & 192) != 128 ||
          fread(&next3, sizeof(unsigned char), 1, file) != 1 || (next3 & 192) != 128) {
          isUTF = false;
          break;
      }
    // If UTF-8 format isn't followed, fail
    } else {
      isUTF = false;
      break;
    }
  }

  // If checker was successful it's a UTF-8 encrypted file
  if (isUTF == true) {
    printf("%s: UTF-8 text\n", name);
    return true;
  } else {
    return false;
  }
}

int main(int argc, char* argv[]) {
  // Check if 0 arguments were given
  if (argc == 1) {
    printf("Usage: file path\n");
    return EXIT_FAILURE;
  }

  // Open file in read mode, and creater ptr
  FILE *file = fopen(argv[1], "r");

  // Check if file can be opened
  if (!file) {
    print_error(argv[1], errno);
    return EXIT_SUCCESS;
  }
  
  if (isUFT_8(file, argv[1])){
    return EXIT_FAILURE;
  }

  fclose(file);
  return EXIT_FAILURE;
}

