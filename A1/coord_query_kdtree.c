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

struct node {
    struct node *left;
    struct node *right;
    struct record *record;
    double coord[2];
    int axis;
    };


int cmpfuncx (const void * a, const void * b) {
   double lon_a = ((const struct record*)a)->lon;
   double lon_b = ((const struct record*)b)->lon;
   
   if (lon_a < lon_b) return -1;
   if (lon_a > lon_b) return 1;
   return 0;
}

int cmpfuncy (const void * a, const void * b) {
   double lat_a = ((const struct record*)a)->lat;
   double lat_b = ((const struct record*)b)->lat;
   
   if (lat_a < lat_b) return -1;
   if (lat_a > lat_b) return 1;
   return 0;
}

int sort_rec(struct record* rs, int n, int xy) {
    if (xy == 0) {
        qsort(rs, n, sizeof(struct record), cmpfuncx);
    } else {
        qsort(rs, n, sizeof(struct record), cmpfuncy);
    }
    return 0;
}

struct node* rec (struct node* node, struct record* rs, int offset, int n, int depth) {
    int N = n - offset;
    //No records
    if (N < 0) {
        free(node);
        return NULL;
    //If one record
    } else if (N == 0) {
        node->axis = depth % 2;
        node->record = &rs[offset];
        node->coord[0] = rs[offset].lon;
        node->coord[1] = rs[offset].lat;
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    //Multiple records
    node->axis = depth % 2;
    sort_rec(rs + offset, N, node->axis);
    int mid = offset + ceill(N/2);
    node->record = &rs[mid];
    node->coord[0] = rs[mid].lon;
    node->coord[1] = rs[mid].lat;

    struct node *l = malloc(sizeof(struct node));
    struct node *r = malloc(sizeof(struct node));
    if (r == NULL || l == NULL) {
        perror("Failed to allocate memory for kd tree");
        exit(EXIT_FAILURE);
    }
    int next_n = offset + n - mid - 1;
    //next_n can be = mid, must be one lower
    if (next_n == mid) {
        next_n -= 1;
    }
    int next_off = mid + 1;
    node->left = rec(l, rs, offset, next_n, depth + 1);
    node->right = rec(r, rs, next_off, n, depth + 1);
    return node;
}

struct node* mk_kdtree(struct record* rs, int n) {
    struct node *first = malloc(sizeof(struct node));
    if (first == NULL) {
        perror("Failed to allocate memory for kd tree");
        exit(EXIT_FAILURE);
    }
    sort_rec(rs, n, 0);
    int m = floor(n/2);
    first->axis = 0;
    first->record = &rs[m];
    first->coord[0] = rs[m].lon;
    first->coord[1] = rs[m].lat;
    struct node *l = malloc(sizeof(struct node));
    struct node *r = malloc(sizeof(struct node));
    if (l == NULL || r == NULL) {
        perror("Failed to allocate memory for kd tree");
        exit(EXIT_FAILURE);
    }
    first->left = rec(l, rs, 0, m-1, 1);
    first->right = rec(r, rs, m+1, n, 1);
    return first;
}   

void free_kdtree(struct node* data) {
    if (data->left != NULL) {
        free_kdtree(data->left);
    }
    if (data->right != NULL) {
        free_kdtree(data->right);
    }
    free(data);
}

double euclid (double point1[], double point2[]) {
    double x1 = point1[0];
    double x2 = point2[0];
    double y1 = point1[1];
    double y2 = point2[1];
    return sqrt(pow(x1 - x2,2) + pow(y1 - y2,2));
}

struct node* lookup_rec(struct node* closest, double query[], struct node* node) {
    if (node == NULL) {
        return closest;
    }
    
    if (euclid(closest->coord, query) > euclid(node->coord, query)) {
        closest = node;
    }

    int axis = node->axis;
    double diff = node->coord[axis]-query[axis];

    if (diff >= 0) {
        closest = lookup_rec(closest, query, node->left);
    } else {
        closest = lookup_rec(closest, query, node->right);
    }
    return closest;
}

const struct record* lookup_kdtree(struct node *data, double lon, double lat) {
    double query[2];
    query[0] = lon;
    query[1] = lat;
    struct node* closest = data;
    closest = lookup_rec(closest, query, data);
    return closest->record;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                    (mk_index_fn)mk_kdtree,
                    (free_index_fn)free_kdtree,
                    (lookup_fn)lookup_kdtree);
}
