#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data *data = malloc(sizeof(struct naive_data));
  assert(data != NULL);

  data->n = n;
  data->rs = rs;

  return data;
}

void free_naive(struct naive_data* data) {
  free(data);
}

double euDist(double lon1, double lat1, double lon2, double lat2) {
  return sqrt(pow(lon1 - lon2, 2) + pow(lat1 - lat2, 2));
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  struct record *closestRec = &data->rs[0];
  double closestDist = euDist(lon, lat, closestRec->lon, closestRec->lat);
  
  for (int i = 1; i < data->n; i++) {
    double newDist = euDist(lon, lat, data->rs[i].lon, data->rs[i].lat);
    if (newDist < closestDist) {
      closestRec = &data->rs[i];
      closestDist = newDist;
    }
  }
  return closestRec;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
