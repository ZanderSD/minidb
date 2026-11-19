# Hushay's B-Tree implementation

I implemented `find`, `insert`, `splitChild`, `insertNonFull`, `update`,
`traverseInOrder`, and `destroy` on `feature/btree-insertion-hushay`.
The branch starts from `btree-starter`. The working tree was clean before
I started, and the existing public interface and shared helpers are unchanged.

## Files changed

- `src/btree_insert.cpp`: my B-Tree operations and node cleanup.
- `tests/btree_insert_test.cpp`: 14 additional test cases; all 35 original
  cases are preserved.
- `BTREE_IMPLEMENTATION_HUSHAY.md`: these notes.

## How the algorithms work

**Search:** `findIndex` locates the first key in a node that is at least the
search key. If it matches, I return a pointer to its row. Otherwise I follow
the child at that index. Reaching a leaf without a match, or starting with an
empty tree, returns `nullptr`. All comparisons use `compareKeys`.

**Insertion:** I check for duplicates before changing the tree. The first
record creates a leaf root. For later records, I split a full root if needed,
then use `insertNonFull` to follow the correct child. A full child is split
before entering it, so there is always space when insertion reaches a leaf.
The key and row are inserted at the same index, and `count` increases only
after insertion succeeds.

**Splitting:** a full node has `2t - 1` keys. The key at index `t - 1` and its
row move into the parent. The left node keeps the first `t - 1` records, and
a new right sibling gets the last `t - 1`. For an internal node, its last
`t` child pointers also transfer to the right sibling. This makes room while
preserving key order, subtree boundaries, and equal leaf depth. Only splitting
the root increases the tree's height.

**Updating:** I follow the same search path, then replace the matching row.
Updates do not change the indexed key, tree structure, or record count.
Missing keys return `false`. Both insertion and update reject an empty row
or a first column that does not compare equal to the supplied key. Rejected
operations leave the records unchanged.

**Traversal:** I recursively visit child 0, record 0, child 1, record 1, and
so on, ending with the last child. Leaves simply append their records in
order. This returns every key-row pair in ascending order. An empty tree
returns an empty vector.

**Cleanup:** `destroy` frees children before their parent and accepts
`nullptr`. The existing destructor and `loadTreeForTesting` use it. Temporary
new nodes use `unique_ptr` until they are attached successfully; the tree
continues to own nodes through the existing raw-pointer design.

With `n` records and minimum degree `t`, search, insertion, and update take
`O(t * log_t(n))` tree work because `findIndex` scans each node linearly.
For fixed `t`, this is `O(log n)`. A split takes `O(t)`. Traversal and cleanup
take `O(n)`, with `O(log_t(n))` recursion space; traversal also returns `O(n)`
records. These bounds treat comparisons and record copies as constant cost;
long strings and larger rows add their comparison and copying costs.

## Tests and results

Before implementation, `make -k test-insert test-remove test-full` reported:

| Suite | Passed | Failed |
| --- | ---: | ---: |
| Original insertion tests | 11 | 24 |
| Deletion tests | 3 | 29 |
| Integration tests | 0 | 5 |

After implementation, I ran `make test-insert` and
`make -k minidb test test-full`. The project built successfully with C++17,
`-Wall`, and `-Wextra`, with no compiler warnings.

| Suite | Passed | Failed |
| --- | ---: | ---: |
| Tokenizer | 46 | 0 |
| Parser | 151 | 0 |
| Insertion, search, update, traversal | 49 | 0 |
| Deletion | 3 | 29 |
| Integration | 0 | 5 |

`make test` stops its recipe after the deletion failures, so I also ran the
`test-full` target explicitly. Its four randomized cases fail at removal
operations (steps 18, 24, 47, and 205). The final case successfully inserts
2,000 records, then fails when removal starts. `remove` still returns `false`
unconditionally in Z's starter code.

Coverage includes empty trees, missing and existing keys, duplicates in full
nodes, root and internal splits, ascending/descending/shuffled insertion for
degrees 2, 3, 4, and 5, numeric and alphabetical ordering, matching rows in
traversal, and updates in both leaves and internal nodes. The bulk insertion
tests call `checkValid` after every insert to check occupancy, ordering,
child counts, subtree bounds, equal leaf depth, and record count. Separate
row comparisons verify that payloads stay attached to their keys.

The macOS leak checker, `leaks --atExit -- ./btree_insert_test`, reported
**0 leaks and 0 leaked bytes** after all 49 tests passed.

I also built and ran the focused suite with UndefinedBehaviorSanitizer:

```sh
g++ -std=c++17 -Wall -Wextra -g -fsanitize=undefined \
  -fno-omit-frame-pointer -o /tmp/minidb-insert-ubsan \
  tests/btree_insert_test.cpp src/btree.cpp src/btree_insert.cpp src/btree_remove.cpp
/tmp/minidb-insert-ubsan
```

All **49 tests passed with no UBSan diagnostics**. The same build with
`-fsanitize=address,undefined` compiled, but stalled before `main`, both
inside and outside the sandbox. A process sample showed AddressSanitizer
stuck in `AsanInitInternal` / `InitializeShadowMemory` / `StaticSpinMutex`,
so I stopped those processes. ASan verification remains blocked by runtime
initialization on this host; it is not counted as a passing test.

I reviewed the complete diff and ran `git diff --check` successfully.

## Remaining work and integration notes

- My assigned operations are complete. Deletion, borrowing, and merging are
  still TODOs in `src/btree_remove.cpp`; I did not edit that file. CSV
  persistence is not present in this starter and remains outside my work.
- The SQL tokenizer, parser, shared B-Tree helpers, header, and Makefile are
  unchanged. No existing tests were removed or weakened.
- Callers must supply a row whose first column matches the key. Key changes
  need a separate delete/insert operation once deletion is available.
  Numeric equality follows `compareKeys`, so INT `009` equals INT `9`, while
  TEXT `9` is a different key. Mixed keys retain the shared INT-before-TEXT
  ordering. INT values must contain valid, in-range `long long` text for the
  existing `std::stoll` comparator; SQL validation belongs above this layer.
- A pointer returned by `find` refers to tree-owned storage. Read or copy it
  before modifying the tree; insertions and splits can invalidate pointers.
- When Z transfers children during merging, those pointers must be removed
  from the old node's ownership. Delete the emptied sibling directly rather
  than calling `destroy` on subtrees that were transferred.
- The guide mentions `make test-asan T=test-insert`, but the current Makefile
  does not use `T`; that target always runs the full suite. The focused
  sanitizer command above avoids the unfinished deletion suite.
- Review is against `btree-starter`, as described in the team guide. No
  commit, push, merge, or pull request was made.
