// ============================================================================
// Hushay: search, insert (with splitting), update, in-order traversal, cleanup.
// Tests:  make test-insert
//
// Study first: CLRS chapter 18.1-18.2, and the USFCA B-Tree visualizer
//   https://www.cs.usfca.edu/~galles/visualization/BTree.html
//
// Use compareKeys(a, b) to compare keys (it returns <0, 0, >0).
// findIndex(node, key) is already written: the first index i with keys[i] >= key.
// ============================================================================
#include "btree.h"
#include <memory>

// Look for `key`, starting at the root.
// At each node: i = findIndex(node, key).
//   If i < keys.size() and keys[i] equals key -> found: return &node->rows[i].
//   Else if the node is a leaf               -> not in the tree: return nullptr.
//   Else                                     -> go down into children[i] and repeat.
const Row* BTree::find(const Value& key) const {
    // One root-to-leaf path: O(t * log_t(n)) with the linear findIndex helper.
    const BTreeNode* node = root;
    while (node != nullptr) {
        size_t i = findIndex(node, key);
        if (i < node->keys.size() && compareKeys(node->keys[i], key) == 0)
            return &node->rows[i];
        if (node->leaf) return nullptr;
        node = node->children[i];
    }
    return nullptr;
}

// 1. If the key already exists (use find), return false.
// 2. If the tree is empty: make a new leaf root holding just this key and row.
// 3. Else if the root is full (2t - 1 keys):
//        make a new root, put the old root as its children[0],
//        splitChild(newRoot, 0), then insert into the correct child.
//    This is the ONLY way the tree gets taller.
// 4. Call insertNonFull(root, key, row).
// 5. count++ and return true.
bool BTree::insert(const Value& key, const Row& row) {
    // Reject duplicates before splitting so a rejected insert changes nothing.
    if (find(key) != nullptr) return false;
    if (row.empty() || compareKeys(key, row.front()) != 0) return false;

    // Inputs may refer to a stored row. Keep copies before splits move rows.
    Value storedKey = key;
    Row storedRow = row;
    if (root == nullptr) {
        auto newRoot = std::make_unique<BTreeNode>();
        newRoot->keys.push_back(std::move(storedKey));
        newRoot->rows.push_back(std::move(storedRow));
        root = newRoot.release();
    } else {
        if (root->keys.size() == 2 * static_cast<size_t>(t) - 1) {
            auto newRoot = std::make_unique<BTreeNode>();
            newRoot->leaf = false;
            newRoot->children.push_back(root);
            splitChild(newRoot.get(), 0);
            root = newRoot.release();
        }
        insertNonFull(root, storedKey, storedRow);
    }
    // Search plus insertion still costs O(t * log_t(n)), excluding row copies.
    ++count;
    return true;
}

// parent->children[i] is full (2t - 1 keys). Split it into two nodes of t - 1 keys:
//   - the middle key (index t - 1) and its row move UP into parent at position i
//   - the keys/rows after the middle move into a NEW right node
//   - if the full node is not a leaf, its last t children move to the new node too
//   - the new node goes into parent->children at position i + 1
void BTree::splitChild(BTreeNode* parent, size_t i) {
    BTreeNode* left = parent->children[i];
    const size_t degree = static_cast<size_t>(t);
    auto right = std::make_unique<BTreeNode>();
    right->leaf = left->leaf;

    // Prepare copies and capacity before changing either existing node.
    Value medianKey = left->keys[degree - 1];
    Row medianRow = left->rows[degree - 1];
    right->keys.assign(left->keys.begin() + degree, left->keys.end());
    right->rows.assign(left->rows.begin() + degree, left->rows.end());
    if (!left->leaf)
        right->children.assign(left->children.begin() + degree, left->children.end());
    parent->keys.reserve(parent->keys.size() + 1);
    parent->rows.reserve(parent->rows.size() + 1);
    parent->children.reserve(parent->children.size() + 1);

    // Promoting the median separates two sorted halves, each with t-1 keys.
    // Splitting costs O(t); moved subtrees now belong to the right sibling.
    parent->keys.insert(parent->keys.begin() + i, std::move(medianKey));
    parent->rows.insert(parent->rows.begin() + i, std::move(medianRow));
    parent->children.insert(parent->children.begin() + i + 1, right.get());
    left->keys.resize(degree - 1);
    left->rows.resize(degree - 1);
    if (!left->leaf) left->children.resize(degree);
    right.release();
}

// `node` is guaranteed NOT full.
//   If it's a leaf: insert key and row at the sorted position (findIndex helps).
//   If not: find which child to go into (i = findIndex(node, key)).
//           If that child is full: splitChild(node, i), then decide whether the
//           key now goes into children[i] or children[i + 1].
//           Recurse: insertNonFull(child, key, row).
void BTree::insertNonFull(BTreeNode* node, const Value& key, const Row& row) {
    size_t i = findIndex(node, key);
    if (node->leaf) {
        Value storedKey = key;
        Row storedRow = row;
        node->keys.reserve(node->keys.size() + 1);
        node->rows.reserve(node->rows.size() + 1);
        node->keys.insert(node->keys.begin() + i, std::move(storedKey));
        node->rows.insert(node->rows.begin() + i, std::move(storedRow));
        return;
    }

    // Split before descending, guaranteeing space at every level we enter.
    if (node->children[i]->keys.size() == 2 * static_cast<size_t>(t) - 1) {
        splitChild(node, i);
        if (compareKeys(key, node->keys[i]) > 0) ++i;
    }
    insertNonFull(node->children[i], key, row);
}

// Find the key and replace its row. Return false if the key isn't there.
// Hint: the same walk as find(), but you need a non-const pointer to change the row.
bool BTree::update(const Value& key, const Row& newRow) {
    // Column zero is the indexed key. Reject attempts to change or omit it.
    if (newRow.empty() || compareKeys(key, newRow.front()) != 0) return false;
    BTreeNode* node = root;
    while (node != nullptr) {
        size_t i = findIndex(node, key);
        if (i < node->keys.size() && compareKeys(node->keys[i], key) == 0) {
            Row replacement = newRow;
            node->rows[i].swap(replacement);
            return true;
        }
        if (node->leaf) return false;
        node = node->children[i];
    }
    // Same search cost as find, plus the cost of copying the replacement row.
    return false;
}

// Every (key, row) pair in ascending order.
// In-order walk of a B-Tree node with k keys:
//   child 0, key 0, child 1, key 1, ..., key k-1, child k
// (a leaf has no children: just its keys in order)
std::vector<std::pair<Value, Row>> BTree::traverseInOrder() const {
    std::vector<std::pair<Value, Row>> out;
    out.reserve(count);
    // Visit each record once: O(n) plus output copies, O(height) call stack.
    auto visit = [&out](auto&& self, const BTreeNode* node) -> void {
        if (node == nullptr) return;
        for (size_t i = 0; i < node->keys.size(); ++i) {
            if (!node->leaf) self(self, node->children[i]);
            out.emplace_back(node->keys[i], node->rows[i]);
        }
        if (!node->leaf) self(self, node->children.back());
    };
    visit(visit, root);
    return out;
}

// Free `node` and everything below it (children first, then the node itself).
// Must handle node == nullptr. Called by the destructor.
void BTree::destroy(BTreeNode* node) {
    if (node == nullptr) return;
    // Post-order cleanup frees each owned node exactly once, in O(n) time.
    for (BTreeNode* child : node->children) destroy(child);
    delete node;
}
