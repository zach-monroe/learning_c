#ifndef DYNARRAY_H
#define DYNARRAY_H
#include <limits.h>
#include <stddef.h>
#include <sys/stat.h>

struct entry {
  char name[NAME_MAX + 1];
  struct stat st;
};

typedef struct entry entry;

struct DynArray {
  size_t len;
  size_t cap;
  struct entry *dt;
};

int DynArrayInit(struct DynArray *da);
// init an array with no content, a length and cap of 1. will need to add our
// first entry. malloc

int DynArrayAdd(struct DynArray *da, const struct entry *e);
// realloc double size every time we hit the cap. increase the cap and length
// accordingly. NEED TO HANDLE ENOMEM

int DynArrayGet(const struct DynArray *da, size_t i, struct entry *out);
// find an entry by its index. return -1 if index is out of range. use a simple
// if statement to check before retrieval. return entry to user via the pointer.

size_t DynArrayLen(const struct DynArray *da);
// return the length of the array

void DynArrayFree(struct DynArray *da);
// free memory (one for every init)

#endif
