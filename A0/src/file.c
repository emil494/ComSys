#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.

int print_hello_world(void) {
  return fprintf(stdout, "Hello, world!\n");
}

int main(void) {
  int retval = EXIT_SUCCESS;

  if (print_hello_world() <= 0) {
    retval = EXIT_FAILURE;
  }

  return retval;
}
