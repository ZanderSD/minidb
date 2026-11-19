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
    BTreeNode* current = node->children[index + 1];
    while (!current->children.empty())
    {
        current = current->children.front();
    }
    return { current->keys.front(), current->rows.front() };
}



/*********************************************************
*       void BTree::fill(BTreeNode* node, size_t index) 
*   accpets a pointer to the subtree and an index value
*   moves keys and rows to maintain t - 1 size
*   borrows from the left if it has >= t keys
*   or borrows from the right if its has >= t keys
*   or merges with right or left
* 
*   -Zander Dennison
*********************************************************/

void BTree::fill(BTreeNode* node, size_t index) 
{
    if (index != 0 && node->children[index - 1]->keys.size() >= static_cast<size_t>(t))
    {
        borrowFromPrev(node, index);
    }
    
    else if (index != node->keys.size() && node->children[index + 1]->keys.size() >= static_cast<size_t>(t))
    {
        borrowFromNext(node, index);
    }
    else
    {
        if (index != node->keys.size())
        {
            merge(node, index);
        }
        else
        {
            merge(node, index - 1);
        }
    }
}



/******************************************************************************
*       void BTree::borrowFromPrev(BTreeNode* node, size_t index)
*   accepts a subtree node and an index
*   "Rotate right through parent"
*   moves the previous key from the parent to the front of children[index]
*       and the left siblings last key to the previous key if they are leafs
*   else it moves the left siblings last key to the front of child[index]
******************************************************************************/

void BTree::borrowFromPrev(BTreeNode* node, size_t index) 
{
    BTreeNode* child = node->children[index];
    BTreeNode* sibling = node->children[index - 1];

    child->keys.insert(child->keys.begin(), node->keys[index - 1]);
    child->rows.insert(child->rows.begin(), node->rows[index - 1]);

     if (!child->children.empty()) {
        child->children.insert(child->children.begin(), sibling->children.back());
        sibling->children.pop_back();
    }

   node->keys[index - 1] = sibling->keys.back();
    node->rows[index - 1] = sibling->rows.back();

    sibling->keys.pop_back();
    sibling->rows.pop_back();
}



/**********************************************************************************
*       void BTree::borrowFromNext(BTreeNode* node, size_t index) 
*   accepts a subtree node and an index
*   "Rotate left through parent"
*   moves the key[index] from the parent to the end of children[index]
*       and the right siblings first key to the parent key[index] if they are leafs
*   else it moves the right siblings first key to the end of child[index]
**********************************************************************************/

void BTree::borrowFromNext(BTreeNode* node, size_t index) 
{
    BTreeNode* child = node->children[index];
    BTreeNode* sibling = node->children[index + 1];

    child->keys.push_back(node->keys[index]);
    child->rows.push_back(node->rows[index]);

    if (!child->children.empty()) {
        child->children.push_back(sibling->children.front());
        sibling->children.erase(sibling->children.begin());
    }

    node->keys[index] = sibling->keys.front();
    node->rows[index] = sibling->rows.front();

    sibling->keys.erase(sibling->keys.begin());
    sibling->rows.erase(sibling->rows.begin());
}



/***************************************************************************
*       void BTree::merge(BTreeNode* node, size_t index)
*   accepts a subtree node and an index
*   moves parent keys[index] to the end of the left child
*   copys all keys rows and child form the right child to the left child
*   removed the copied parent's keys, rows, and children
*   deletes the right child from the parent node
***************************************************************************/

void BTree::merge(BTreeNode* node, size_t index) 
{
    BTreeNode* left = node->children[index];
    BTreeNode* right = node->children[index + 1];

    left->keys.push_back(node->keys[index]);
    left->rows.push_back(node->rows[index]);

    for (size_t i = 0; i <right->keys.size(); ++i)
    {
        left->keys.push_back(right->keys[i]);
        left->rows.push_back(right->rows[i]);
    }

    if (!left->children.empty())
    {
        for (size_t i = 0; i < right->children.size(); ++i)
        {
            left->children.push_back(right->children[i]);
        }
    }
    
    node->keys.erase(node->keys.begin() + index);
    node->rows.erase(node->rows.begin() + index);
    node->children.erase(node->children.begin() + index + 1);

    delete right;
}
