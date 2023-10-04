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
  UTF_8,
};

// Constant array of strings for each elemnt in the enum
const char * const FILE_TYPE_STRINGS[] = {
  "data",
  "empty",
  "ASCII text",
  "ISO-8859 text",
  "Unicode text, UTF-8 text"
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

// Checker for UTF-8
bool isUFT_8_Form (FILE *file) {
  unsigned char read;
  size_t bytes;
  bool isUTF = true;
  while ((bytes = fread(&read, sizeof(unsigned char), 1,  file)) == 1) {
    // Check if ascii
    if (isAsciiChar(read)) {
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

  // If checker was successful it's a UTF-8 file
  if (isUTF == true) {
    return true;
  } else {
    return false;
  }
}

enum fileType determineFile(FILE *file) {
  bool isAscii = true;
  bool isIso = true;
  bool isEmpty = true;
  bool isUTF8 = true;
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
  
  // Reset pointer to start of file
  fseek(file, 0, SEEK_SET);
  if (!isUFT_8_Form(file)) {
    isUTF8 = false;
  }

  if (isEmpty) {
    type = EMPTY;
  } else if (isAscii) {
    type = ASCII;
  } else if (isIso) {
    type = ISO;
  } else if (isUTF8) {
    type = UTF_8;
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
  case UTF_8:
    num = 4;
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
    print_error(argv[1], errno);
    return EXIT_SUCCESS;
  }

  enum fileType type = determineFile(file);
  printFileType(type, argv[1]);

  fclose(file);
  return EXIT_SUCCESS;
}

