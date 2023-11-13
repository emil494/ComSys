#include "common.h"

/*
 * returns 1 if ip_string is a valid IP address; else 0
 */
int is_valid_ip(char *ip_string) {
  int ip[4];
  int num_parsed = sscanf(ip_string, "%d.%d.%d.%d", ip+0, ip+1, ip+2, ip+3);

  if (num_parsed < 0) {
    fprintf(stderr, "sscanf() error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  } 

  else if (num_parsed != 4)
    return string_equal(ip_string, "localhost");

  for (int i = 0; i < 4; i++)
    if (ip[i] > 255 || ip[i] < 0)
      return 0;

  return 1;
}


/*
 * returns 1 if port_string is a valid port number; else 0
 */
int is_valid_port(char *port_string) {
  int port;
  int num_parsed = sscanf(port_string, "%d", &port);

  if (num_parsed < 0) {
    fprintf(stderr, "sscanf() error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  else if (num_parsed != 1 || port < 0 || port > 65535)
    return 0;

  return 1;
}

/*
 * returns 1 if a starts with b; else 0
 */
int starts_with(const char *a, const char *b)
{
  if(strncmp(a, b, strlen(b)) == 0) return 1;
  return 0;
}
