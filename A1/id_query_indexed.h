// Similar to id_query.h.  See the comments there.

#ifndef ID_QUERY_INDEXED_H
#define ID_QUERY_INDEXED_H

#include "record.h"

struct index_record {
  int64_t osm_id;
  const struct record *record;
};

struct indexed_data {
  struct index_record *irs;
  int n;
};

#endif
