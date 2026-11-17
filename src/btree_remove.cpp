#include "btree.h"

bool value_equal(const Value& key1, const Value& key2);



/********************************************************************************************
*       bool BTree::remove(const Value& key)
*
*   steps --
*       locate key
*       delete (leaf node)
*          remove key
*       delete from internal node
*          replace with in-order predecessor (largest key in its left subtree)
*          or successor (smallest key in its right subtree)
*          then delete predecessor or successor from orginal position
*       If a node has fewer keys than required
*          merge with sibling
*          or borrow a key from sibling    
*             Borrowing a key
*             if the sibling has more than the min num of keys borrow a key from it
*             and adjust the keys in the parent node accordingly
*       if node and sibling have minimum number of keys
*          merge into a single node
*          pulling a key from their parent to separate the merged keys
*       If the root now has 0 keys:
*           if it's a leaf -> the tree is empty: delete root, root = nullptr
*           else           -> the tree shrinks: root = its only child, delete the old root
*       count-- and return true.
*
*   -Zander Dennison
********************************************************************************************/

bool BTree::remove(const Value& key) {
    
    //checks to see if a tree exists
    if (root == nullptr)
    {
        return false;
    }

    bool found = false;
    BTreeNode* curr = root;

    //check to see if value exists
    while (curr != nullptr && !found) 
    {
        size_t i = findIndex(curr, key);
        if (i < curr->keys.size() && value_equal(curr->keys[i], key)) {
            found = true;
        }
        else if (curr->children.empty()) {
            return false;
        }
        else {
            curr = curr->children[i];
        }
    }

    removeFromNode(root, key);

    //shrinks key is possible
    if (root->keys.empty()) {
        BTreeNode* oldRoot = root;
        if (root->children.empty()) {
            root = nullptr;
        }
        else {
            root = root->children[0];
        }
        delete oldRoot;
    }

    //changes size and confirms remove was completed
    count--;
    return true;
}




//helpers below here

/***************************************************************
*       bool value_equal(const Value& key1, const Value& key2)
*   accepts two keys
*   Checks to see if the key values are equal
* 
*   -Zander Dennison
***************************************************************/

bool value_equal(const Value& key1, const Value& key2)
{
    if (key1.text == key2.text && key1.type == key2.type)
    {
        return true;
    }
    else
    {
        return false;
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
    if (index < node->keys.size() && value_equal(node->keys[index], key))
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
            return;
        }
        //recursive to check all nodes
        else
        {
            //checks to see if node has minimun number of keys and
            //  fills if less
            bool isLastChild = (index == node->keys.size());

            if (node->children[index]->keys.size() == static_cast<size_t>(t - 1)) 
            {
                fill(node, index);
            }

            // If the last child was merged with its left sibling, move to the merged child
            if (isLastChild && index > node->keys.size()) 
            {
                removeFromNode(node->children[index - 1], key);
            }
            else 
            {
                removeFromNode(node->children[index], key);
            }

        }
    }

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
    //std::pair<Value, Row> values(0, 0);

    //if the predecessor can have the have keys taken from it and structure is maintained
    if (node->children[index]->keys.size() >= static_cast<size_t>(t))
    {
        //get predecessor and replace values
        auto values = getPredecessor(node, index);
        node->keys[index] = values.first;
        node->rows[index] = values.second;

        //clean duplicate keys
        removeFromNode(node->children[index], values.first);
    }
    //if the successor can have the have keys taken from it and structure is maintained
    else if (node->children[index + 1]->keys.size() >= static_cast<size_t>(t))
    {
        //get successor and replace values
        auto values = getSuccessor(node, index);
        node->keys[index] = values.first;
        node->rows[index] = values.second;

        //clean duplicate keys
        removeFromNode(node->children[index + 1], values.first);
    }
    //merge and clean duplicate nodes to maintain t
    else
    {
        merge(node, index);
        removeFromNode(node->children[index], key);
    }

}



/*********************************************************************************************
*       std::pair<Value, Row> BTree::getPredecessor(BTreeNode* node, size_t index) 
*   returns a two values value and row
*   accepts the node and index 
*   checks to see if the array is empty 
*   gets the last element of the children array
* 
*   -Zander Dennison
********************************************************************************************/

std::pair<Value, Row> BTree::getPredecessor(BTreeNode* node, size_t index) 
{
    BTreeNode* current = node->children[index];
    while (!current->children.empty()) 
    {
        current = current->children.back();
    }
    return { current->keys.back(), current->rows.back() };
}



/*********************************************************************************************
*       std::pair<Value, Row> BTree::getSuccessor(BTreeNode* node, size_t index)
*   returns a two values value and row
*   accepts the node and index
*   checks to see if the array is empty
*   gets the first element of the children array
*
*   -Zander Dennison
********************************************************************************************/

std::pair<Value, Row> BTree::getSuccessor(BTreeNode* node, size_t index) 
{
    // TODO (Z)
    BTreeNode* current = node->children[index + 1];
    while (!current->children.empty())
    {
        current = current->children.front();
    }
    return { current->keys.front(), current->rows.front() };
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
