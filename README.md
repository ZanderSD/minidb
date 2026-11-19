# MiniDB

A tiny SQL database built from scratch in C++ for our Data Structures & Algorithms class.

MiniDB reads SQL commands from the terminal, breaks them into tokens, parses them, and will run them against tables stored in memory. No external libraries, no existing database engines — everything is written by hand.

## Current Status

The front end is done: MiniDB tokenizes and parses every planned command. The B-Tree that will store each table's rows is also done and tested. What's left is the validator and executor that connect the two.

```
Tokenizer  →  Parser  →  Validator  →  Executor  →  Storage (B-Tree)
   ✅          ✅           ⬜            ⬜             🟡
```

✅ done  🟡 in progress  ⬜ not started

| Stage     | Status                                                     | Tests                     |
| --------- | ---------------------------------------------------------- | ------------------------- |
| Tokenizer | All planned SQL tokens                                     | 46 passing                |
| Parser    | `CREATE`, `INSERT`, `SELECT`, `UPDATE`, `DELETE`, `DROP`, `TRUNCATE` | 151 passing     |
| Validator | Not started                                                | —                         |
| Executor  | Not started                                                | —                         |
| Storage   | B-Tree done (insert, find, update, remove, traverse); table layer not started | 86 passing |

283 tests total. GitHub Actions runs `make test` on every push and PR.

## Features

| Command          | Example                                                          | Status     |
| ---------------- | ---------------------------------------------------------------- | ---------- |
| `CREATE TABLE`   | `CREATE TABLE users (id INT, name TEXT, age INT);`               | 🟡 Parsed  |
| `INSERT`         | `INSERT INTO users VALUES (1, 'bob', 20);`                       | 🟡 Parsed  |
| `SELECT`         | `SELECT * FROM users;` or `SELECT name, age FROM users;`         | 🟡 Parsed  |
| `WHERE`          | `... WHERE age > 18 AND (name = 'bob' OR id = 2)`                | 🟡 Parsed  |
| `ORDER BY`       | `SELECT * FROM users ORDER BY age DESC, name;`                   | 🟡 Parsed  |
| `UPDATE`         | `UPDATE users SET age = 21 WHERE id = 1;`                        | 🟡 Parsed  |
| `DELETE`         | `DELETE FROM users WHERE id = 1;`                                | 🟡 Parsed  |
| `DROP TABLE`     | `DROP TABLE users;`                                              | 🟡 Parsed  |
| `TRUNCATE TABLE` | `TRUNCATE TABLE users;`                                          | 🟡 Parsed  |

`WHERE` supports `=`, `!=`, `<`, `>`, `<=`, `>=`, `AND`, `OR` and parentheses.

The REPL has line editing: arrow keys move the cursor, up/down scroll through command history. Type `.exit` (or Ctrl-D) to quit.

## Build & Run

Requires a C++17 compiler (g++ or clang++) and `make`.

```bash
make        # build ./minidb
./minidb
make test   # build and run all tests
make clean  # remove built binaries
```

Smaller test groups:

```bash
make test-parser   # tokenizer + parser
make test-insert   # B-Tree insert / find / update / traverse
make test-remove   # B-Tree remove (borrow and merge)
make test-full     # randomized insert/remove checked against std::map
make test-asan     # all tests with AddressSanitizer + UBSan
```

## Example Session

The REPL prints what it parsed, because there is no executor yet.

```
db > CREATE TABLE users (id INT, name TEXT, age INT);
Parsed CREATE TABLE users with columns: id(INT) name(TEXT) age(INT)
db > INSERT INTO users VALUES (1, 'bob', 20);
Parsed INSERT INTO users with values: 1 'bob' 20
db > SELECT name, age FROM users WHERE age > 18 AND (name = 'bob' OR id = 2) ORDER BY age DESC;
Parsed SELECT from users, columns: name age, WHERE ((age > 18) AND ((name = 'bob') OR (id = 2))), ORDER BY age(DESC)
db > UPDATE users SET age = 21 WHERE id = 1;
Parsed UPDATE users, set: age = 21, WHERE (id = 1)
db > DELETE FROM users WHERE id = 1;
Parsed DELETE from users, WHERE (id = 1)
db > SELECT FROM users;
Syntax error: Expected column names but found 'FROM'
db > .exit
```

## How It Works

Every command goes through the same pipeline:

1. **Tokenizer** — splits the raw string into tokens (`SELECT`, `users`, `>`, `18`, ...).
2. **Parser** — checks the tokens follow SQL grammar and builds a `Statement`. Bad syntax throws a `ParseError`.
3. **Validator** — checks the statement makes sense, like whether the table and columns exist.
4. **Executor** — runs the statement against the stored tables.
5. **Storage** — keeps the rows, indexed with a B-tree.

### Example: `CREATE TABLE`

```sql
CREATE TABLE users (name TEXT, age INT);
```

The tokenizer turns the string into a flat list:

```
CREATE  TABLE  IDENTIFIER(users)  (  IDENTIFIER(name)  TEXT  ,  IDENTIFIER(age)  INT  )  ;  END
```

The parser (recursive descent) turns that list into a `Statement`:

```
       CREATE
      /      \
   users   columns
           /     \
      name TEXT  age INT
```

### The B-Tree

Each table will store its rows in one B-Tree, keyed by the first column:

```
key = the first column's value   (e.g. 5)
row = all of the row's values    (e.g. {5, 'bob', 20})
```

It follows CLRS chapter 18. With minimum degree `t`, every node holds at most `2t - 1` keys and every non-root node at least `t - 1`. Insert splits full nodes on the way down; remove borrows from or merges with a sibling on the way down, so neither has to walk back up. `INT` keys compare as numbers and `TEXT` keys alphabetically.

| Operation         | Cost              |
| ----------------- | ----------------- |
| `find`            | O(log n)          |
| `insert`          | O(log n)          |
| `update`          | O(log n)          |
| `remove`          | O(log n)          |
| `traverseInOrder` | O(n)              |

The tests call `checkValid()` after every operation to make sure none of the rules broke.

## Project Structure

```
minidb/
├── README.md
├── GUIDE.MD                # how the B-Tree work was split up
├── Makefile
├── .github/workflows/      # CI: runs make test
├── src/
│   ├── main.cpp            # REPL: line editing, history, prints parsed statements
│   ├── tokenizer.h/.cpp    # string → tokens
│   ├── parser.h/.cpp       # tokens → Statement (recursive descent)
│   ├── statement.h         # Statement, Column, Value, Expr types
│   ├── btree.h             # B-Tree interface
│   ├── btree.cpp           # key comparison, getters, checkValid
│   ├── btree_insert.cpp    # find, insert, split, update, traverse
│   └── btree_remove.cpp    # remove, borrow, merge
└── tests/
    ├── tokenizer_test.cpp
    ├── parser_test.cpp
    ├── btree_insert_test.cpp
    ├── btree_remove_test.cpp
    └── btree_full_test.cpp
```

## Data Structures Used

- **Vector** — list of tokens, columns and values in a statement
- **Expression tree** — `WHERE` clauses like `age > 18 AND name = 'bob'` (`Expr` nodes with `AND` / `OR` / compare)
- **B-tree** — stores each table's rows, keyed by the first column
- **Hash map** (`unordered_map`) _(planned)_ — table name → table lookup

## Roadmap

- [x] REPL loop with `.exit`
- [x] Line editing and command history
- [x] Tokenizer
- [x] Parse `CREATE TABLE` and `INSERT`
- [x] Parse `SELECT` with column lists and `ORDER BY`
- [x] Parse `WHERE` with comparisons, `AND` / `OR` and parentheses
- [x] Parse `UPDATE`, `DELETE`, `DROP TABLE`, `TRUNCATE TABLE`
- [x] Syntax error messages
- [x] B-Tree: search, insert, update, traversal
- [x] B-Tree: remove with borrow and merge
- [x] CI running all tests
- [ ] Table storage on top of the B-Tree
- [ ] Validator
- [ ] Executor for `CREATE TABLE` and `INSERT`
- [ ] Executor for `SELECT` with `WHERE` and `ORDER BY`
- [ ] Executor for `UPDATE`, `DELETE`, `DROP`, `TRUNCATE`
- [ ] _Stretch:_ save/load tables to a file

## Team

- Rajeev Shrestha — Tokenizer, Parser, REPL, Database Engine
- Hushay Madzingira — B-Tree search, insert, update, traversal
- Zander Dennison — B-Tree remove

## References

- [Crafting Interpreters — Scanning](https://craftinginterpreters.com/scanning.html)
- [Crafting Interpreters — Parsing Expressions](https://craftinginterpreters.com/parsing-expressions.html)
- Cormen et al., _Introduction to Algorithms_ (CLRS), Chapter 18: B-Trees
- [USFCA B-Tree Visualization](https://www.cs.usfca.edu/~galles/visualization/BTree.html)
