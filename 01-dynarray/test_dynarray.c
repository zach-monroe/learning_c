// test_dynarray.c: tests for dynarray.h
//
// Build and run it with your Makefile (`make test`). Each failed check prints
// the file, line, and the expression that failed. The program exits with status
// 1 if anything failed, and 0 if everything passed.
//
// ASan (from -fsanitize=address) also counts as a test: a leak, out-of-bounds
// access, or use-after-free is a failure even if every CHECK passes.
//
// These tests only use the public API in dynarray.h, plus the len/cap fields
// of struct DynArray, to check the invariant len <= cap.

#include "dynarray.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int checks_run = 0;
static int checks_failed = 0;

#define CHECK(expr)                                                            \
  do {                                                                         \
    checks_run++;                                                              \
    if (!(expr)) {                                                             \
      checks_failed++;                                                         \
      fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);        \
    }                                                                          \
  } while (0)

// Build a test entry with a recognizable name and size.
static struct entry make_entry(int n) {
  struct entry e;
  memset(&e, 0, sizeof e);
  snprintf(e.name, sizeof e.name, "file%d", n);
  e.st.st_size = n;
  return e;
}

// ---------------------------------------------------------------------------

static void test_init_is_empty(void) {
  struct DynArray da;
  CHECK(DynArrayInit(&da) == 0);
  CHECK(DynArrayLen(&da) == 0);
  CHECK(da.len <= da.cap);
  DynArrayFree(&da);
}

static void test_add_one(void) {
  struct DynArray da;
  CHECK(DynArrayInit(&da) == 0);

  struct entry e = make_entry(42);
  CHECK(DynArrayAdd(&da, &e) == 0);
  CHECK(DynArrayLen(&da) == 1);

  struct entry out;
  CHECK(DynArrayGet(&da, 0, &out) == 0);
  CHECK(strcmp(out.name, "file42") == 0);
  CHECK(out.st.st_size == 42);

  DynArrayFree(&da);
}

// Enough adds to force many reallocs. Every entry must survive every resize,
// in the same order.
static void test_add_many_keeps_order(void) {
  enum { N = 1000 };
  struct DynArray da;
  CHECK(DynArrayInit(&da) == 0);

  for (int i = 0; i < N; i++) {
    struct entry e = make_entry(i);
    CHECK(DynArrayAdd(&da, &e) == 0);
    CHECK(DynArrayLen(&da) == (size_t)(i + 1));
    CHECK(da.len <= da.cap);
  }

  int mismatches = 0;
  for (int i = 0; i < N; i++) {
    struct entry out;
    char want[32];
    snprintf(want, sizeof want, "file%d", i);
    if (DynArrayGet(&da, (size_t)i, &out) != 0 || strcmp(out.name, want) != 0 ||
        out.st.st_size != i)
      mismatches++;
  }
  CHECK(mismatches == 0);

  DynArrayFree(&da);
}

static void test_get_out_of_range(void) {
  struct DynArray da;
  CHECK(DynArrayInit(&da) == 0);

  struct entry out = make_entry(-1);

  // Empty array: there's no valid index.
  CHECK(DynArrayGet(&da, 0, &out) == -1);

  for (int i = 0; i < 3; i++) {
    struct entry e = make_entry(i);
    CHECK(DynArrayAdd(&da, &e) == 0);
  }

  CHECK(DynArrayGet(&da, 2, &out) == 0);  // last valid index
  CHECK(DynArrayGet(&da, 3, &out) == -1); // index == len
  CHECK(DynArrayGet(&da, SIZE_MAX, &out) == -1);

  // Slots past len exist in memory (when cap > len) but must not be readable.
  CHECK(DynArrayGet(&da, da.cap, &out) == -1);

  DynArrayFree(&da);
}

// A failed Get must leave *out alone.
static void test_get_failure_leaves_out_untouched(void) {
  struct DynArray da;
  CHECK(DynArrayInit(&da) == 0);

  struct entry out = make_entry(7);
  CHECK(DynArrayGet(&da, 5, &out) == -1);
  CHECK(strcmp(out.name, "file7") == 0);
  CHECK(out.st.st_size == 7);

  DynArrayFree(&da);
}

// Add copies the entry in, so changing the caller's struct afterward must not
// change what's stored.
static void test_add_copies_entry(void) {
  struct DynArray da;
  CHECK(DynArrayInit(&da) == 0);

  struct entry e = make_entry(1);
  CHECK(DynArrayAdd(&da, &e) == 0);

  strcpy(e.name, "changed");
  e.st.st_size = 999;

  struct entry out;
  CHECK(DynArrayGet(&da, 0, &out) == 0);
  CHECK(strcmp(out.name, "file1") == 0);
  CHECK(out.st.st_size == 1);

  DynArrayFree(&da);
}

// Get hands back a copy, so changing it must not change the array.
static void test_get_returns_copy(void) {
  struct DynArray da;
  CHECK(DynArrayInit(&da) == 0);

  struct entry e = make_entry(5);
  CHECK(DynArrayAdd(&da, &e) == 0);

  struct entry out;
  CHECK(DynArrayGet(&da, 0, &out) == 0);
  strcpy(out.name, "changed");

  struct entry again;
  CHECK(DynArrayGet(&da, 0, &again) == 0);
  CHECK(strcmp(again.name, "file5") == 0);

  DynArrayFree(&da);
}

// After Free, the same struct can be initialized and used again.
static void test_reuse_after_free(void) {
  struct DynArray da;
  for (int round = 0; round < 3; round++) {
    CHECK(DynArrayInit(&da) == 0);
    for (int i = 0; i < 10; i++) {
      struct entry e = make_entry(i);
      CHECK(DynArrayAdd(&da, &e) == 0);
    }
    CHECK(DynArrayLen(&da) == 10);
    DynArrayFree(&da);
  }
}

// ---------------------------------------------------------------------------

#define RUN(test)                                                              \
  do {                                                                         \
    int before = checks_failed;                                                \
    test();                                                                    \
    printf("%s %s\n", checks_failed == before ? "PASS" : "FAIL", #test);       \
  } while (0)

int main(void) {
  RUN(test_init_is_empty);
  RUN(test_add_one);
  RUN(test_add_many_keeps_order);
  RUN(test_get_out_of_range);
  RUN(test_get_failure_leaves_out_untouched);
  RUN(test_add_copies_entry);
  RUN(test_get_returns_copy);
  RUN(test_reuse_after_free);

  printf("\n%d checks, %d failed\n", checks_run, checks_failed);
  return checks_failed == 0 ? 0 : 1;
}
