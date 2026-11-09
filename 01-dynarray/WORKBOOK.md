# 01 · Dynamic Array (Workbook)

**Goal:** Build a growable array of directory entries. Then use it to make `ls` sort
its output.

**Time:** about 3 sessions, one per part (Parts 2 and 3 can share a session if they go
quickly).

**Rules:** You write all the code. Claude reviews it and writes the test file once your
header (`dynarray.h`) is settled. The hints at the bottom are folded away, so only open
them when you're stuck.

---

## 0. Before you start

**Read** (Beej's Guide to C):
- *Pointers—Cower In Fear!*
 USER: read and explained in chat
- *Structs*
 USER: my experience with Go was very helpful here...although the pointer relationship and the arrow operator is really interestign and unique.
- *Manual Memory Allocation*
USER: read and discussed last night
- *Multifile Projects* (skim it; you'll need it in Part 4)
USER: Skimmed and comprehended. 

**Man pages you'll use:** `man 3 malloc`, `man 3 realloc`, `man 3 free`,
`man 3 strdup`, `man 3 readdir`, `man 3type stat`, `man 3 strcmp`.

**Finish Phase 0 first:**
- [x ] `getgrgid` bug fixed in `00-ls/ls.c`
- [ ] `00-ls/Makefile` has a `clean` target and `.PHONY` USER: cant find info on this. Explain please
- [x ] the root `Makefile` either does something useful or is deleted. USER: removed
- [x ] a `.gitignore` keeps binaries and `.o` files out of git
- [x] committed

---

## Concepts recap

From your own explanations, tightened up:

| Idea | What it means |
|---|---|
| **len vs cap** | `len` is how many items are stored. `cap` is how many fit before you need to grow. Same as a Go slice. |
| **Invariant** | Slots `[0, len)` hold valid items. Slots `[len, cap)` exist but are **never read**. Every function must keep this true. |
| **Growth** | When `len == cap`, multiply the capacity (for example, double it). Growing by 1 makes n appends cost O(n²); doubling makes them O(n) total, O(1) amortized. |
| **Ownership** | Whoever allocates something has to make sure it gets freed. A `strdup`'d name needs exactly one `free`, and it has to be freed **before** the array that points to it. |
| **Copy, don't borrow** | `readdir()` reuses and later frees its buffer. Never store `entry->d_name` as a pointer. Make your own copy of the name. |

---

## Part 1: Design `struct entry`

One element of the array holds everything about one directory entry, so that sorting
moves whole records around.

Answer these in a comment at the top of `dynarray.h`:

1. **What fields does `ls -l` need to print?** Look at your `print_long`. Do you
   store each piece separately, or store the whole `struct stat`? What's the tradeoff?
2. **What fields do you need to sort by name? By modification time?**
3. **What type is the name?** A fixed-size `char` array inside the struct, or a
   `char *` from `strdup`? Think about: how long can a filename be (`NAME_MAX`), what
   each choice costs in memory, and what each choice means for freeing.
4. **Plain `ls` (without `-l`) doesn't need `stat` data.** Do you `lstat` every entry
   anyway, or only when needed? (Hint: `-t` sorting needs mtime even without `-l`.)

---

## Part 2: Design the dynamic array API, in `dynarray.h`

### The operations you need

| Operation | Behavior |
|---|---|
| **create / init** | Make an empty array (`len == 0`). Decide whether it allocates right away or waits for the first append. |
| **append** | Add one entry at the end. Grow if full. Must report failure if memory runs out, and must leave the array **unchanged and still valid** when that happens. |
| **get** | Access the entry at index `i`. Must not allow `i >= len`. |
| **length** | Report how many entries are stored. |
| **free / destroy** | Release everything the array owns. After this, using the array is a bug. Freeing an empty array must be safe. |

### Decisions to make (write your answer to each as a comment in the header)

1. **Who allocates the array struct itself?** Either the caller declares it on the
   stack and you initialize it (`init(&arr)`), or your function `malloc`s it and
   returns a pointer (`arr = create()`). Which do you prefer, and why?
2. **How does append report failure?** A return code? Something else? What does the
   caller check?
3. **What does get do with a bad index?** Return a pointer that might be `NULL`?
   Use `assert`? Whatever you pick, write it down.
4. **Does append copy the entry in, or store a pointer to it?** If you copy, who owns
   the name string after the copy: the caller or the array?
5. **Does destroy free the names too?** This follows from your answer to 4. The rule
   must be the same everywhere: one owner, one `free`.
6. **What starting capacity and growth factor will you use?** What happens when the
   capacity is 0 and you double it?
7. **Naming:** pick a prefix (like `entries_` or `da_`) and use it on every function.

### Deliverable

`dynarray.h` contains the struct definitions, the function declarations, and your
design notes. **Send it to Claude for review.** Once you've agreed on it, Claude
writes `test_dynarray.c` against it.

---

## Part 3: Implement `dynarray.c`

### Requirements
- Check every allocation (`malloc`, `realloc`, `strdup`) for failure.
- If `realloc` fails, the array must still be intact and the old block not leaked.
  (Read the RETURN VALUE section of `man 3 realloc` before you write the grow logic.)
- Keep the invariant in every function: never read a slot at index `len` or higher.
- No global variables.

### Your Makefile (`01-dynarray/Makefile`)
- Uses the default flags: `-std=gnu17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined`
- `make test` builds and runs `test_dynarray`
- `make clean` removes the build output
- Changing `dynarray.h` causes a rebuild

### Done when
- [ ] `make test` passes
- [ ] zero compiler warnings
- [ ] ASan reports no leaks or errors
- [ ] (optional) `valgrind --leak-check=full ./test_dynarray` is clean. Build a
      separate copy without `-fsanitize`, because valgrind and ASan don't work together.

---

## Part 4: Use it in `ls`

1. **Fill:** while reading the directory, append one entry per name. Close the
   directory only once you've finished reading it.
2. **Sort:** write a simple sort yourself; don't use `qsort` yet (that's Phase 3).
   Insertion sort is a good choice: walk left to right, and slide each element left
   until it's in place. It's O(n²), which is fine for directory sizes.
   - default: by name, using `strcmp` (real `ls` sorts by name too)
   - `-t`: by modification time, newest first
   - `-r`: reverse whichever order is active. (This replaces the `reverse_sort` stub;
     delete the stub.)
3. **Print:** loop over the array and print each entry (short or `-l` format).
4. **Free:** everything, in the right order.
5. **Build:** the `00-ls` Makefile now has to compile `../01-dynarray/dynarray.c`
   along with `ls.c`. Look up `-I` in `man gcc` so `ls.c` can include the header.

### Done when
- [ ] `./ls`, `./ls -l`, `./ls -t`, `./ls -lr`, and `./ls -alt` all work
- [ ] output order matches the real `ls` for the same flags. (Real `ls` takes locale
      into account when sorting names; run `LC_ALL=C ls` to compare against plain
      byte order.)
- [ ] ASan reports no leaks or errors on a large directory (try `/usr/bin`)

---

## Stretch (optional)
- Add `-S` (sort by size).
- Shrink the array to fit once reading is done. Is it worth it here?
- Measure it: time appending 10 million `int`s with doubling vs growing by 1.

---

## Hints (open only when stuck)

<details><summary>The realloc trap</summary>

Writing `p = realloc(p, n)` overwrites `p` before you know whether the call worked.
If it returned `NULL`, what happened to your only pointer to the old block? Store
the result somewhere else first, check it, and only then update the struct.
</details>

<details><summary>Growing from capacity 0</summary>

0 × 2 = 0. Either start at a small nonzero capacity, or special-case 0 when computing
the new capacity.
</details>

<details><summary>Comparing modification times</summary>

`st_mtim` is a `struct timespec` with two fields. Two files can have the same
seconds value. What should you compare when the seconds are equal?
</details>

<details><summary>Freeing order</summary>

If you free the array's storage first, where are the pointers to the names now?
</details>

<details><summary>Sorting with -r</summary>

Two ways to do it: sort, then reverse the array in place, or flip the result of the
comparison. Which one keeps your sort function simpler?
</details>
