#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.

int main(int argc, char* argv[]) {
  // Check if 0 arguments were given
  if (argc == 1) {
    printf("Usage: file path\n");
    return EXIT_FAILURE;
  }

  // Open file in read mode
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

  return EXIT_FAILURE;
}
