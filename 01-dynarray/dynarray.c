#include "dynarray.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// init an array with no content, a length and cap of 1. will need to add our
// first entry. malloc
int DynArrayInit(struct DynArray *da) {
  da->len = 0;
  da->cap = 1;
  da->dt = malloc(sizeof *da->dt);
  if (da->dt == NULL) {
    da->cap = 0;
    return -1;
  }
  return 0;
}

// realloc double size every time we hit the cap. increase the cap and length
// accordingly.
int DynArrayAdd(struct DynArray *da, const struct entry *e) {
  if (da->cap == 0) {
    return -1;
  }
  if (da->len == da->cap) {
    struct entry *tmp = realloc(da->dt, da->cap * 2 * sizeof *da->dt);
    if (tmp == NULL) {
      return -1;
    }
    da->dt = tmp;

    da->cap = da->cap * 2;
  }
  da->dt[da->len] = *e;
  da->len = da->len + 1;
  return 0;
}

// find an entry by its index. return -1 if index is out of range. use a simple
// if statement to check before retrieval. return entry to user via the pointer.
int DynArrayGet(const struct DynArray *da, size_t i, struct entry *out) {
  if (i >= da->len) {
    return -1;
  }
  *out = da->dt[i];
  return 0;
}

// return the length of the array
size_t DynArrayLen(const struct DynArray *da) { return da->len; }

// free memory (one for every init)
void DynArrayFree(struct DynArray *da) {
  free(da->dt);
  da->dt = NULL;
  da->len = 0;
  da->cap = 0;
}
