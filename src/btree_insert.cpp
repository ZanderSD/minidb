// B-Tree search, insert (with splitting), update, traversal and cleanup.
#include "btree.h"
#include <memory>

// Walk down from the root until we hit the key or fall off a leaf.
const Row* BTree::find(const Value& key) const {
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

// Splitting a full root is the only way the tree grows taller.
bool BTree::insert(const Value& key, const Row& row) {
    // check for duplicates first so a failed insert doesn't split anything
    if (find(key) != nullptr) return false;
    if (row.empty() || compareKeys(key, row.front()) != 0) return false;

    // copy first, key/row might point into the tree and get moved by a split
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
    ++count;
    return true;
}

// Split the full child parent->children[i] into two nodes of t - 1 keys.
// The middle key moves up into the parent.
void BTree::splitChild(BTreeNode* parent, size_t i) {
    BTreeNode* left = parent->children[i];
    const size_t degree = static_cast<size_t>(t);
    auto right = std::make_unique<BTreeNode>();
    right->leaf = left->leaf;

    Value medianKey = left->keys[degree - 1];
    Row medianRow = left->rows[degree - 1];
    right->keys.assign(left->keys.begin() + degree, left->keys.end());
    right->rows.assign(left->rows.begin() + degree, left->rows.end());
    if (!left->leaf)
        right->children.assign(left->children.begin() + degree, left->children.end());
    parent->keys.reserve(parent->keys.size() + 1);
    parent->rows.reserve(parent->rows.size() + 1);
    parent->children.reserve(parent->children.size() + 1);

    parent->keys.insert(parent->keys.begin() + i, std::move(medianKey));
    parent->rows.insert(parent->rows.begin() + i, std::move(medianRow));
    parent->children.insert(parent->children.begin() + i + 1, right.get());
    left->keys.resize(degree - 1);
    left->rows.resize(degree - 1);
    if (!left->leaf) left->children.resize(degree);
    right.release();
}

// node is never full here, so a leaf always has room.
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

    // split full children on the way down
    if (node->children[i]->keys.size() == 2 * static_cast<size_t>(t) - 1) {
        splitChild(node, i);
        if (compareKeys(key, node->keys[i]) > 0) ++i;
    }
    insertNonFull(node->children[i], key, row);
}

// Same walk as find(), but replaces the row.
bool BTree::update(const Value& key, const Row& newRow) {
    // the first column is the key, so it can't change
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
    return false;
}

// In-order walk: child 0, key 0, child 1, key 1, ..., child k
std::vector<std::pair<Value, Row>> BTree::traverseInOrder() const {
    std::vector<std::pair<Value, Row>> out;
    out.reserve(count);
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

// Free children first, then the node.
void BTree::destroy(BTreeNode* node) {
    if (node == nullptr) return;
    for (BTreeNode* child : node->children) destroy(child);
    delete node;
}
