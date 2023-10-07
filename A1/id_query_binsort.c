#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "id_query.h"
#include "id_query_indexed.h"

struct index_ord_record {
  const struct indexed_data *record;
  int n;
};

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

int cmpfunc (const void * a, const void * b) {
   int osm_id_a = ((const struct index_record*)a)->osm_id;
   int osm_id_b = ((const struct index_record*)b)->osm_id;
   
   if (osm_id_a < osm_id_b) return -1;
   if (osm_id_a > osm_id_b) return 1;
   return 0;
}

struct index_ord_record* mk_sorted_index(struct record* rs, int n) {
  struct index_ord_record *data = malloc(sizeof(struct index_ord_record));
  if (data == NULL) {
    perror("Failed to allocate memory for sorted indexed data");
    exit(EXIT_FAILURE);
  }

  struct indexed_data *temp = mk_indexed(rs, n);
  qsort(temp->irs, n, sizeof(struct indexed_data), cmpfunc);

  data->n = n;
  data->record = temp;
  return data;
}

void free_sorted_index(struct index_ord_record* data) {
  free(data->record->irs);
  free(data);
}

const struct record* lookup_sorted_index(struct index_ord_record *data, int64_t needle) {
  int l = 0;
  int r = data->n-1;
  while (l <= r) {
    int m = (int)floor((double)((l+r)/ 2));
    if (data->record->irs[m].osm_id < needle) {
        l = m + 1;
    } else if (data->record->irs[m].osm_id > needle) {
        r = m - 1;
    } else {
        return data->record->irs[m].record;
    }
  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_sorted_index,
                    (free_index_fn)free_sorted_index,
                    (lookup_fn)lookup_sorted_index);
}
