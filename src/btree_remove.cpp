// ============================================================================
// Z: removing a key, keeping the tree balanced with borrow and merge.
// Tests:  make test-remove
//
// Your tests build trees BY HAND, so you can start before insert is finished.
//
// Study first: CLRS chapter 18.3, the GeeksforGeeks article "Delete Operation
// in B-Tree", and the USFCA B-Tree visualizer (try deleting keys and watch).
//
// The idea (CLRS): never step down into a child that has only t - 1 keys.
// Before going down, fill() that child so it has at least t keys. Then a key
// can always be removed from a leaf without breaking the minimum.
//
// Use compareKeys(a, b) to compare keys. findIndex(node, key) is already written.
// ============================================================================
#include "btree.h"

// 1. If the tree is empty (root == nullptr), return false.
//    If the key isn't in the tree, return false. Don't use find() for this:
//      it's Hushay's, and your tests must pass even before hers do. Write a
//      small walk of your own (same idea as find: findIndex, then go down).
// 2. removeFromNode(root, key).
// 3. If the root now has 0 keys:
//        if it's a leaf -> the tree is empty: delete root, root = nullptr
//        else           -> the tree shrinks: root = its only child, delete the old root
//    This is the ONLY way the tree gets shorter.
// 4. count-- and return true.


/***************************************************
    bool BTree::remove(const Value& key)

steps
    locate key
    delete (leaf node)
        remove key
    delete from internal node
        replace with in-order predecessor (largest key in its left subtree)
        or successor (smallest key in its right subtree)
        then delete predecessor or successor from orginal position
    If a node has fewer keys than required
        merge with sibling
        or borrow a key from sibling
    if node and sibling have minimum number of keys
        merge into a single node
        pulling a key from their parent to separate the merged keys

    Borrowing a key
        if the sibling has more than the min num of keys borrow a key from it
        and adjust the keys in the parent node accordingly
***************************************************/

bool BTree::remove(const Value& key) {
    // TODO (Z)
    
    //checks to see if a tree exists
    if (root == nullptr)
    {
        return false
    }

    //obtains the location of the node
    // i dont think this actually does something rn
    //BTreeNode* subtree = search(root, key);

    //check to see if is leaf and deletes
    if (subtree->children.empty())
    {
        delete key;
    }
    if(n)
    (void)key;
    return false;
}




//helpers below here

/*************
* find
I want a delete root functions
    it should call the delete function and set root to new value
    new value is initialize and nullptr but can change to its child

I want a find function
    it should locate the key
*************/



/*******************************************************
*       Row* search(BTreeNode* node, const Value& key)
*   looks for the subtree containing the key
*   node is a pointers to the current node
*   key is the value being searched for
* 
* -Zander Dennison
*******************************************************/

Row* search(BTreeNode* node, const Value& key)
{
    if (node == nullptr)
    {
        return nullptr;
    }
    
    size_t i = findIndex(node, key);

    if (i < node->keys.size() && node->keys[i] == key)
    {
        return &node->rows[i];
    }
    else if (node->children.empty())
    {
        return nullptr;
    }
    else
    {
        return search(node->children[i], key);
    }
}



/***********************************************************************
*        void BTree::removeFromNode(BTreeNode* node, const Value& key)
*   removes a key from the subtree node
*   key is the value being removed
*   node is a pointer to the subtree
* 
*   -Zander Dennison
************************************************************************/

void BTree::removeFromNode(BTreeNode* node, const Value& key) {

    ///creates index
    size_t index = findIndex(node, key);

    //checks if key is in node
    if (index < node->keys.size() && node->keys[index] == key)
    {
        //checks if leaf
        if (node->children.empty())
        {
            removeFromLeaf(node, index);
        }
        //interal node delete
        else
        {
            removeFromInternal(node, index);
        }
    }
    //if key is not in node
    else
    {
        //if its a leaf key does not exist
        if (node->children.empty())
        {
            exit;
        }
        //recursive to check all nodes
        else
        {
            //checks to see if node has minimun number of keys and
            //  fills if less
            if (node->children[index]->keys.size == t - 1)
            {
                fill(node->children[index], index);
            }
            removeFromNode(node->children[index], index);
        }
    }

    (void)node;
    (void)key;
}




/*********************************************************************
*       void BTree::removeFromLeaf(BTreeNode* node, size_t index) 
*   Removes key and row from leaf
*   accepts a pointer to the node
*   accepts an index value of the row
* 
*   -Zander Dennison
*********************************************************************/
void BTree::removeFromLeaf(BTreeNode* node, size_t index) 
{
    node->rows.erase(node->rows.begin() + index);
    node->keys.erase(node->keys.begin() + index);
}



/**********************************************************************
*       void BTree::removeFromInternal(BTreeNode* node, size_t index) 
*   identifies and removes the value from an internal node
*   calls functions to maintain the structure of BTree
*   node a pointer to the subtree
*   index is the position of values 
* 
*   -Zander Dennison
*************************************************************************/
void BTree::removeFromInternal(BTreeNode* node, size_t index) 
{
    //get key and create a variable to hold the results of get function
    Value key = node->keys[index];
    std::pair<Value, Row> values(0, 0);

    //if the predecessor can have the have keys taken from it and structure is maintained
    if (node->children[index]->keys.size() >= t)
    {
        //get predecessor and replace values
        values = getPredecessor(node, index);
        node->keys[index] = values.first;
        node->rows[index] = values.second;

        //clean duplicate keys
        remove(node->children[index], values.first);
    }
    //if the successor can have the have keys taken from it and structure is maintained
    else if (node->children[index + 1]->keys.size() >= t)
    {
        //get successor and replace values
        values = getSuccessor(node, index);
        node->keys[index] = values.first;
        node->rows[index] = values.second;

        //clean duplicate keys
        remove(node->children[index + 1], values.first);
    }
    //merge and clean duplicate nodes to maintain t
    else
    {
        merge(node, index);
        remove(node->children[index], key);
    }

}

// The largest key in the left subtree: start at children[idx], keep going to
// the LAST child until you reach a leaf, return its last key and row.
std::pair<Value, Row> BTree::getPredecessor(BTreeNode* node, size_t idx) {
    // TODO (Z)
    return {node->keys[idx], node->rows[idx]};
}

// The smallest key in the right subtree: start at children[idx + 1], keep going
// to the FIRST child until you reach a leaf, return its first key and row.
std::pair<Value, Row> BTree::getSuccessor(BTreeNode* node, size_t idx) {
    // TODO (Z)
    return {node->keys[idx], node->rows[idx]};
}

// children[idx] has only t - 1 keys. Make it have at least t:
//   if the LEFT sibling (idx - 1) has >= t keys  -> borrowFromPrev(node, idx)
//   else if the RIGHT sibling (idx + 1) has >= t -> borrowFromNext(node, idx)
//   else merge with a sibling: merge(node, idx) if there is a right sibling,
//                              otherwise merge(node, idx - 1)
void BTree::fill(BTreeNode* node, size_t idx) {
    // TODO (Z)
    (void)node;
    (void)idx;
}

// Rotate right through the parent:
//   parent's keys[idx - 1] moves DOWN to the front of children[idx]
//   left sibling's LAST key moves UP into parent's keys[idx - 1]
//   if not leaves: left sibling's last child moves to the front of children[idx]
// (Move the rows together with their keys!)
void BTree::borrowFromPrev(BTreeNode* node, size_t idx) {
    // TODO (Z)
    (void)node;
    (void)idx;
}

// Rotate left through the parent:
//   parent's keys[idx] moves DOWN to the end of children[idx]
//   right sibling's FIRST key moves UP into parent's keys[idx]
//   if not leaves: right sibling's first child moves to the end of children[idx]
void BTree::borrowFromNext(BTreeNode* node, size_t idx) {
    // TODO (Z)
    (void)node;
    (void)idx;
}

// Merge children[idx + 1] INTO children[idx]:
//   left = children[idx], right = children[idx + 1]
//   parent's keys[idx] moves DOWN to the end of left
//   all of right's keys, rows (and children, if not leaves) go to the end of left
//   remove keys[idx], rows[idx] and children[idx + 1] from the parent
//   delete right (careful: don't destroy() it, its children now belong to left)
void BTree::merge(BTreeNode* node, size_t idx) {
    // TODO (Z)
    (void)node;
    (void)idx;
}
