#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.

int main(int argc, char* argv[]) {
  int retval = EXIT_SUCCESS;
  FILE * f;

  if (argc < 2) {
    int retval = EXIT_FAILURE;
    printf("Usage: file path\n");
    return retval;
  }
  f = fopen(argv[argc-1], "r");
  if (!f){
    retval = EXIT_FAILURE;
    return retval;
  }
  return retval;
}
