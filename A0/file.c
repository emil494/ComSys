#include <errno.h>   // errno.
#include <stdio.h>   // fprintf, stdout, stderr.
#include <stdlib.h>  // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h>  // strerror.
#include <stdbool.h>

int main(int argc, char *argv[]) {
  // Check if 0 arguments were given
  if (argc == 1) {
    printf("Usage: file path\n");
    return EXIT_FAILURE;
  }

  // Open file in read mode, and creater ptr
  FILE *file = fopen(argv[1], "r");

  // Check if file can be opened
  if (!file) {
    printf("cant finde file...\n");
    return EXIT_SUCCESS;
  }

  // Check if file is empty
  fseek(file, 0, SEEK_END);  // Move to end of file
  long size = ftell(file);   // Get current file pointer position
  if (size == 0) {
    printf("%s: empty\n", argv[1]);
    return EXIT_SUCCESS;
  }

  // Loop trough the file, and find the char with the highest value
  // fseek(file, 0, SEEK_SET);
  // int higestChar = fgetc(file); // Delete, instead call isAsciiChar for every char
  bool isAscii = true;
  for (int i = 0; i <= size; i++) {
    fseek(file, i, SEEK_SET);
    int currentChar = fgetc(file);
    if (!isAsciiChar(currentChar)) {
      isAscii = false;
    }
  }

  // Check if the file is an ASCII text file
  if (isAscii) {
    printf("%s: ASCII text\n", argv[1]);
    return EXIT_SUCCESS;
  }

  // Check if the file is an ISO-8859-1 text file
  //if (higestChar <= 256) {
  //  printf("%s: ISO-8859-1 text\n", argv[1]);
  //  return EXIT_SUCCESS;
  //}
  // Check

  return EXIT_FAILURE;
}

bool isAsciiChar(int byte) {
  if (byte >= 7 && byte <= 13) {
    return true;
  } else if (byte == 27) {
    return true;
  } else if (byte >= 32 && byte <= 126) {
    return true;
  }
  return false;
}

bool isIsoChar(int byte) {
  if (byte >= 160 && byte <= 255) {
    return true;
  } else if (isAsciiChar(byte)) {
    return true;
  } else {
    return false;
  }
}
