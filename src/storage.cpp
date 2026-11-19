// ============================================================================
// Dennison: saving and loading tables as CSV files.
// Tests:  make test-storage
//
// Do them in this order, the tests are in the same order:
//   escapeCsvField -> parseCsv -> saveTable -> loadTable
//
// The file format is explained at the top of storage.h.
// Use tree.traverseInOrder() to get the rows and tree.insert() to put them back.
// For files: <fstream> to read/write, std::rename and std::remove from <cstdio>.
// ============================================================================
#include "storage.h"

// 1. Check if the field has a comma, a quote, \n or \r in it.
//    If not, return it as it is.
// 2. Otherwise build a new string: start with ",
//    copy each character, but write every " twice (""),
//    end with ".
//
//   bob        ->  bob
//   a,b        ->  "a,b"
//   say "hi"   ->  "say ""hi"""
std::string escapeCsvField(const std::string& field) {
    // TODO (Dennison)
    return field;
}

// Go through the text one character at a time, keeping track of:
//   field     - the field you're building
//   record    - the fields of the current line
//   inQuotes  - are you inside a quoted field?
//
// Inside quotes:
//   "  followed by another "  -> add one " to the field, skip both
//   "  on its own             -> the quoted part is over, inQuotes = false
//   anything else (even a newline or a comma) -> add it to the field
// Outside quotes:
//   "        -> inQuotes = true (the field is quoted)
//   ,        -> the field is done: push it onto record, start a new field
//   \n       -> the field AND the record are done: push both, start a new record
//   \r\n     -> same as \n (skip the \r)
//   anything else -> add it to the field
//
// At the end:
//   if inQuotes is still true -> a quote was never closed, return false
//   if there's a line without a newline at the end, push it too
//   (but "a\n" is ONE record, not two: don't add an empty record after the
//   last newline)
//
// Remember to clear `records` at the start.
bool parseCsv(const std::string& content, std::vector<std::vector<std::string>>& records) {
    // TODO (Dennison)
    (void)content;
    records.clear();
    return false;
}

// 1. Build the header line: "name TYPE" for each column, joined with commas.
//    e.g.  id INT,name TEXT
// 2. Open "<path>.tmp" for writing. If it doesn't open, return false.
// 3. Write the header line and a \n.
// 4. For each (key, row) in tree.traverseInOrder():
//      write escapeCsvField() of each value's text, joined with commas, then \n
// 5. Close the file. If anything went wrong while writing, delete the .tmp
//    file and return false.
// 6. std::rename(tmp, path). If that fails, delete the .tmp and return false.
//    The rename is what makes this safe: the old file is only replaced once
//    the new one is completely written.
// 7. Return true.
bool saveTable(const std::string& path, const std::vector<Column>& columns, const BTree& tree) {
    // TODO (Dennison)
    (void)path;
    (void)columns;
    (void)tree;
    return false;
}

// Don't touch `columns` or `tree` until EVERY check has passed.
// Read into local variables first, then copy them over at the very end.
//
// 1. If tree.size() != 0, return false.
// 2. Read the whole file into a string. If it can't be opened, return false.
// 3. parseCsv it. If that fails, or there are no records, return false.
// 4. Header (records[0]): each field must be "name INT" or "name TEXT".
//    Split on the space. The name can't be empty, the type must be exactly
//    INT or TEXT. Build a local std::vector<Column>.
// 5. Every other record is a row:
//      - it must have exactly as many fields as there are columns
//      - make a Value for each field, using the column's type
//      - an INT field must be a whole number: an optional - and then only
//        digits (at least one). It also has to fit in a long long,
//        so std::stoll doesn't throw later. (Try std::stoll in a try/catch.)
//    Build a local std::vector<Row>.
// 6. No duplicate keys (the first value of each row). One easy way: insert
//    every row into a temporary BTree, insert() returns false on a duplicate.
// 7. Everything is fine: set columns = your local columns, and insert every
//    row into `tree` (key = row[0]). Return true.
bool loadTable(const std::string& path, std::vector<Column>& columns, BTree& tree) {
    // TODO (Dennison)
    (void)path;
    (void)columns;
    (void)tree;
    return false;
}
