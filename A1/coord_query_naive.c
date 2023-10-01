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

double euclid (double x1, double x2, double y1, double y2 ) {
    return sqrt(pow(x1 - x2,2) + pow(y1 - y2,2));
  }

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  struct record* closest = &data->rs[0];
  double closest_dist = euclid(lon,closest->lon,lat,closest->lat);

  for (int i = 0; i < data->n; i++) {
    double newDist = euclid(lon, data->rs[i].lon,lat, data->rs[i].lat);
    if (newDist < closest_dist ){
      closest = &data->rs[i];
      closest_dist = newDist;
    }  
  }
  return closest;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
