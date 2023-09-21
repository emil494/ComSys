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

// Enum for supported file types
enum fileType {
  DATA,
  EMPTY,
  ASCII,
  ISO,
  UTF,
};

// Constant array of strings for each elemnt in the enum
const char * const FILE_TYPE_STRINGS[] = {
  "data",
  "empty",
  "ASCII text",
  "ISO-8859-1 text",
  "UTF-8 text"
};

bool isAsciiChar(unsigned int byte) {
  if (byte >= 7 && byte <= 13) {
    return true;
  } else if (byte == 27) {
    return true;
  } else if (byte >= 32 && byte <= 126) {
    return true;
  }
  return false;
}

bool isIsoChar(unsigned int byte) {
  if (byte >= 160 && byte <= 255) {
    return true;
  } else if (isAsciiChar(byte)) {
    return true;
  } else {
    return false;
  }
}

enum fileType determineFile(FILE *file) {
  bool isAscii = true;
  bool isIso = true;
  bool isEmpty = true;
  enum fileType type;
  unsigned char c;

  while (fread(&c, sizeof(c), 1, file) != 0) {
    isEmpty = false;

    if (isAscii && !isAsciiChar((int)c))
      isAscii = false;
    
    if (isIso && !isIsoChar((int)c))
      isIso = false;
    
    if (!isIso && !isAscii)
      break;
  }

  if (isEmpty) {
    type = EMPTY;
  } else if (isAscii) {
    type = ASCII;
  } else if (isIso) {
    type = ISO;
  } else {
    type = DATA;
  }

  return type;
}

void printFileType(enum fileType type, char *path) {
  int num;
  switch (type) {
  case DATA:
    num = 0;
    break;
  case EMPTY:
    num = 1;
    break;
  case ASCII:
    num = 2;
    break;
  case ISO:
    num = 3;
    break;
  default:
    num = 0;
    break;
  }

  printf("%s: %s\n", path, FILE_TYPE_STRINGS[num]);
}

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

  enum fileType type = determineFile(file);
  printFileType(type, argv[1]);

  fclose(file);
  return EXIT_FAILURE;
}

