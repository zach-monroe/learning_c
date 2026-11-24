# 02 · Singly Linked List (Workbook)

**Goal:** Build a singly linked list of `int`s. The data is kept simple on purpose, so
all your attention goes to the pointers. The new C skill here is **pointer-to-pointer
(`**`)**: changing a pointer that belongs to the caller.

**Time:** about 2 sessions. Parts 0–2 in the first, Parts 3–4 in the second.

**Rules:** Same as Phase 1. You write all the code. Claude reviews it and writes
`test_list.c` once your `list.h` is settled. The hints at the bottom are folded away.

---

## 0. Before you start

**Read** (Beej's Guide to C):
- *Structs II: More Fun with Structs*: the section on self-referential structs.
- *Pointers III: Pointers to Pointers and More*: the first section (pointers to
  pointers). This is the important one.
- Review *Manual Memory Allocation* if `malloc`/`free` feel shaky.

**Man pages:** `man 3 malloc`, `man 3 free`, `man 3 assert`.

---

## Concepts

### The shape

```
head
 │
 ▼
┌─────┬───┐   ┌─────┬───┐   ┌─────┬──────┐
│  7  │ ●─┼──▶│  3  │ ●─┼──▶│  9  │ NULL │
└─────┴───┘   └─────┴───┘   └─────┴──────┘
```

- A **node** holds one value plus a pointer to the next node.
- **`head`** points to the first node. It is just a `struct node *` variable.
- **An empty list is `head == NULL`.** There are no nodes and no allocation.
- The last node's `next` is `NULL`. That is how a loop knows it has reached the end.

### Invariant

Starting at `head` and following `next`, you visit every node exactly once and then
reach `NULL`. Every function must keep this true. A missing link leaks nodes; a link
that loops back means traversal never ends.

### Big-O compared with your dynamic array

| Operation | Dynamic array | Singly linked list |
|---|---|---|
| Get item `i` | O(1) | O(n), walk from head |
| Add/remove at the front | O(n), shift everything | **O(1)** |
| Add at the back | O(1) amortized | O(n), walk to the end (unless you keep a tail pointer) |
| Memory | one block, cache-friendly | one `malloc` per node, scattered around |

The list doesn't win at everything. Its strength is cheap insert and remove at a spot
you're already holding. That's why it's the base for stacks and queues (next chunk).

### Why `**`? A worked example on a different type

C passes everything **by value**. A function gets a *copy* of whatever you hand it.

```c
#include <stdio.h>

// Broken: changes its own copy of the pointer. The caller never sees it.
void pick_greeting_broken(const char *out) {
  out = "hello";
}

// Works: gets the ADDRESS of the caller's pointer, and writes through it.
void pick_greeting(const char **out) {
  *out = "hello";
}

int main(void) {
  const char *msg = NULL;

  pick_greeting_broken(msg);
  printf("%s\n", msg ? msg : "(null)");   // prints (null)

  pick_greeting(&msg);
  printf("%s\n", msg);                     // prints hello
}
```

```
main's stack                       pick_greeting's stack
┌──────────────┐                   ┌────────────────────┐
│ msg: ●───────┼──▶ "hello"        │ out: ● (= &msg)    │
└──────────────┘                   └──┼─────────────────┘
       ▲                              │
       └──────────────────────────────┘
          *out = "hello" writes into msg itself
```

Compare with Go: you'd write `func pick(out *string)` and call `pick(&s)`. That's the
same idea, one level of pointer. Here the thing being changed is *itself* a pointer,
so you end up with two stars.

**Try it:** compile that example and run it before Part 2. Then answer: if
`push_front` has to make the caller's `head` point at a new node, what type is its
parameter?

---

## Part 1: The node

Write `struct node` in `list.h`: one `int` and one pointer to the next node.

Answer in a comment:
1. Why does the `next` field have to be a *pointer* to `struct node`, and not a
   `struct node`? (What would `sizeof` be?)

---

## Part 2: Design the API in `list.h`

For this chunk, the list **is** just a `struct node *head` owned by the caller. Don't
put it in a wrapper struct yet. That way you have to practice `**`.

### Operations

| Operation | Behavior |
|---|---|
| **push_front** | Add a value at the front. Must report failure if `malloc` fails, and must leave the list unchanged if it does. |
| **pop_front** | Remove the first node and hand its value back to the caller. Must handle an empty list. |
| **length** | Count the nodes. |
| **print** | Print the values in order, e.g. `[7, 3, 9]`. An empty list prints `[]`. |
| **free** | Free every node. Afterwards the caller's `head` should be `NULL`, not a dangling pointer. |

### Decisions to make (write your answer to each as a comment in the header)

1. **Which functions need `struct node **` and which only need `struct node *`?** Rule
   of thumb: does the function ever need to change *which node `head` points to*?
   Go through all five operations.
2. **Which ones should take `const`?** Which functions only read the list?
3. **How does `pop_front` return the value *and* report "list was empty"?** It has two
   things to tell the caller. (You solved the same problem with `DynArrayGet`.)
4. **How does `push_front` report failure?** Keep it consistent with your dynarray.
5. **Naming:** pick a prefix and use it everywhere.

### Deliverable

`list.h` contains the struct, the declarations, and your design notes. **Send it to
Claude for review.** Once it's agreed, Claude writes `test_list.c`.

---

## Part 3: Implement `list.c`

### Requirements
- Check every `malloc`.
- Never use a node after freeing it. (`free` is where use-after-free bugs come from.
  Think about the order you do things in.)
- No global variables.

### Your Makefile (`02-linkedlist/Makefile`)
- Default flags: `-std=gnu17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined`
- `make test` builds and runs `test_list`
- `make clean` removes build output, and is `.PHONY`
- Changing `list.h` causes a rebuild

### Before you run the tests
For `push_front` and `pop_front`, draw the boxes-and-arrows picture on paper:
before, and after each line of your function. If you can't draw what a line does,
that line is a guess.

### Done when
- [ ] `make test` passes
- [ ] zero compiler warnings
- [ ] ASan reports no leaks or errors

---

## Part 4: Remove by value (the payoff)

Add `remove_first(head, value)`: remove the first node holding `value`, and report
whether one was found.

1. **First, write it the obvious way:** walk with a `prev` and a `cur` pointer. How
   many special cases do you need? (Removing the head? An empty list? Last node?)
2. **Then rewrite it with a `struct node **` that walks the list.** It points at
   "the pointer that points to the current node". At the start that's `head` itself,
   and later it's some node's `next` field. Draw it. How many special cases are
   left?

This is the main reason `**` is worth learning. Once a pointer can point at *either*
the head pointer or a `next` field, the head stops being a special case.

### Done when
- [ ] tests for `remove_first` pass (Claude adds them once your declaration is in
      `list.h`)
- [ ] you can explain, with a drawing, why version 2 needs no `if (cur == head)`

---

## Stretch (optional)
- `push_back` in O(n). Then think about what you'd need to make it O(1).
- `reverse` in place, with no new nodes. Draw it first.
- Time it: push 10 million ints onto the front of your list, versus inserting at
  index 0 of your dynamic array. Which is faster, and by how much? Then compare
  *summing* all the values in each. Why might the list lose that one even though both
  are O(n)?

---

## Hints (open only when stuck)

<details><summary>push_front: which line goes first?</summary>

The new node has to point at the old first node. If you change `head` first, where
did the old first node go?
</details>

<details><summary>pop_front on an empty list</summary>

What is `*head` when the list is empty? What does reading `(*head)->next` do then?
</details>

<details><summary>Freeing the whole list</summary>

After `free(cur)`, reading `cur->next` is a use-after-free. What do you need to save
before you call `free`?
</details>

<details><summary>Precedence: *head->next</summary>

`*head->next` means `*(head->next)`, not `(*head)->next`. When `head` is a
`struct node **`, which one do you want? Check the operator precedence table on
cppreference.com/w/c/language/operator_precedence.
</details>

<details><summary>Remove with a pointer-to-pointer</summary>

Start with `struct node **pp = head;`. Move forward with `pp = &(*pp)->next;`. When
`(*pp)->value` matches, what single assignment unlinks the node? (Save the node first
so you can free it.)
</details>
