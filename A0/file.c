#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <stdbool.h> // Bool

int print_error(char *path, int errnum) {
  return fprintf(stdout, "%s: cannot determine (%s)\n",
    path, strerror(errnum));
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

  // Check if file is empty
  fseek(file, 0, SEEK_END);  // Move to end of file
  long size = ftell(file);   // Get current file pointer position
  if (size == 0) {
    printf("%s: empty\n", argv[1]);
    return EXIT_SUCCESS;
  }
  
  // Loop trough the file, and find the char with the highest value
  fseek(file, 0, SEEK_SET);
  int higestChar = fgetc(file);
  for (int i = 0; i <= size; i++) {
    fseek(file, i, SEEK_SET);
    int currentChar = fgetc(file);
    if (currentChar > higestChar)
      higestChar = currentChar;
  } 

  // Checker for UTF-8 encryption
  unsigned char read[4];
  unsigned char bytes;
  bool isUTF = true;
  while ((bytes = fread(read, sizeof(unsigned char), sizeof(read), file)) == 1) {
    for (int i = 0; i < bytes; i++) {
      if (bytes > 128) {
        printf("asad");
        continue;
      //Check 2 bytes
      } else if (bytes <= i+1 && (read[i] & 224) == 192 && (read[i + 1] & 192) == 128)  {
        continue;
      
      //Check 3 bytes
      } else if (bytes <= i+2 && (read[i] & 240) == 224 && (read[i+1] & 224) == 192 && (read[i + 2] & 192) == 128) {
        continue;

      //Check 4 bytes
      } else if (bytes <= i+3 && (read[i] & 248) == 240 && (read[i+1] & 240) == 224 && 
      (read[i+2] & 224) == 192 && (read[i + 3] & 192) == 128) {
        break;
      } else {
        isUTF = false;
        break;
      }
    }
    if (isUTF == false){
      break;
    }
  }

  // If checker successful it's a UTF-8 encrypted file
  if (isUTF == true) {
    printf("%s: UTF-8 text\n", argv[1]);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
