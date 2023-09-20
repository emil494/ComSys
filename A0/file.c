#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.

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
    print_error(argv[1], );
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

  // Check if the file is an ASCII text file
  if (higestChar <= 127) {
    printf("%s: ASCII text\n", argv[1]);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
