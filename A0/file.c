#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <sys/types.h>
#include <sys/stat.h>

int print_error(char *path, int errnum) {
  return fprintf(stdout, "%s: cannot determine (%s)\n",
    path, strerror(errnum));
}

int main(int argc, char* argv[]) {
  int retval = EXIT_SUCCESS;
  FILE * file;
  struct stat fileInfo;

  if (argc < 2) {
    int retval = EXIT_FAILURE;
    printf("Usage: file path\n");
    return retval;
  }
  
  file = fopen(argv[1], "r");
  stat(argv[1], &fileInfo);
  if (!file){
    retval = EXIT_FAILURE;
    print_error(argv[1], retval);
    return retval;
  }
  return retval;
}
