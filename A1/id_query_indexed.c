#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"
#include "id_query_indexed.h"

struct indexed_data* mk_indexed(struct record* rs, int n) {
  struct indexed_data *data = malloc(sizeof(struct indexed_data));
  if (data == NULL) {
    perror("Failed to allocate memory for indexed data");
    exit(EXIT_FAILURE);
  }

  data->irs = malloc(n * sizeof(struct indexed_data));
  if(data->irs == NULL) {
    perror("Failed to allocate memory for index record array");
    free(data);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < n; i++) {
    data->irs[i].osm_id = rs[i].osm_id;
    data->irs[i].record = &rs[i];
  }

  data->n = n;

  return data;
}

void free_indexed(struct indexed_data* data) {
  free(data->irs);
  free(data);
}

const struct record* lookup_indexed(struct indexed_data *data, int64_t needle) {
  for (int i = 0; i < data->n; i++) {
    if (data->irs[i].osm_id == needle)
      return data->irs[i].record;
  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexed,
                    (free_index_fn)free_indexed,
                    (lookup_fn)lookup_indexed);
}
