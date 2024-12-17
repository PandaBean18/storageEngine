#include <iostream>

using namespace std;

template<typename t>
struct LinkedListNode{
    t data; 
    LinkedListNode* next;   
};

/// @brief order m,
/// maximum number of children = m,
/// max keys = m-1
typedef struct BTreeNode {
    int order;
    int isLeaf;
    int countKeys;
    int countChildren;
    LinkedListNode<int>* keys;
    LinkedListNode<BTreeNode*>* children;
} BTreeNode;

// The function takes a node and a value as parameters and inserts the
// value into the node in a sorted manner. It does not split incase
// the root is filled.
void insertSorted(BTreeNode* node, int val) {
    
    LinkedListNode<int>* newVal = new LinkedListNode<int>;
    newVal->data = val;
    newVal->next = NULL;

    if (!(node->keys)) {
        node->keys = newVal;
        return;
    }

    LinkedListNode<int>* current = node->keys;
    int inserted = 0;

    if (newVal->data < current->data) {
        newVal->next = current;
        node->keys = newVal;
        inserted = 1;
    }

    while ((current->next != NULL) && !inserted) {
        if (current->next->data > newVal->data) {
            LinkedListNode<int>* temp = current->next;
            current->next = newVal;
            newVal->next = temp;
            inserted = 1;
        }
        current = current->next;
    }

    if (!inserted) {
        current->next = newVal;
    }

    node->countKeys++;
}

/// @brief Takes a node with `m` children and splits the node on the
/// median element, returns this new element as root.
/// @param node 
/// @return BtreeNode*
BTreeNode* splitNode(BTreeNode* node) {
    int median = node->order / 2;
    LinkedListNode<int>* current = node->keys;
    BTreeNode* root = new BTreeNode;
    BTreeNode* lc = new BTreeNode;
    BTreeNode* rc = new BTreeNode;
    int i = 0;

    lc->order = rc->order = root->order = node->order;
    root->isLeaf = 0;

    while(current) {
        if (i == median) {
            insertSorted(root, current->data);
        } else if (i < median) {
            insertSorted(lc, current->data);
        } else if (i > median) {
            insertSorted(rc, current->data);
        }
        i++;
        current = current->next;
    }

    if (!node->isLeaf) {
        i = 0;
        LinkedListNode<BTreeNode*>* childrenLC;
        LinkedListNode<BTreeNode*>* childrenRC;
        LinkedListNode<BTreeNode*>* current = node->children;
        childrenLC = node->children;

        while (i < median) {
            current = current->next;
            i++;
        }
        
        childrenRC = current->next;
        current->next = NULL;
        lc->children = childrenLC;
        rc->children = childrenRC;
    }

    LinkedListNode<BTreeNode*>* rootChildren = new LinkedListNode<BTreeNode*>;
    rootChildren->data = lc;
    rootChildren->next = new LinkedListNode<BTreeNode*>;
    rootChildren->next->data = rc;
    rootChildren->next->next = NULL;
    rc->isLeaf = lc->isLeaf = 1; 
    root->children = rootChildren;    
    return root;
}

void printTree(BTreeNode* root) {
    LinkedListNode<BTreeNode*>* toBeTraversed = new LinkedListNode<BTreeNode*>;
    toBeTraversed->data = root;
    LinkedListNode<BTreeNode*>* last = new LinkedListNode<BTreeNode*>;
    last->data = nullptr;
    toBeTraversed->next = last;

    while(toBeTraversed != last) {
        if (toBeTraversed->data == nullptr) {
            cout << endl;
        } else {
            LinkedListNode<int>* current = toBeTraversed->data->keys;
            while (current != NULL) {
                cout << current->data << " ";
                current = current->next;
            }
            LinkedListNode<BTreeNode*>* children = toBeTraversed->data->children;
            last->next = children;

            if (children != NULL) {
                while (children->next != NULL) {
                    children = children->next;
                }

                last = children;
            }
        }
        toBeTraversed = toBeTraversed->next;
    }

    if (toBeTraversed != NULL) {
        LinkedListNode<int>* current = toBeTraversed->data->keys;
        while (current != NULL) {
            cout << current->data << " ";
            current = current->next;
        }
    }
}

int main() {
    BTreeNode* root = new BTreeNode;
    root->keys = new LinkedListNode<int>;
    root->keys->data = 2;
    root->isLeaf = 1;
    root->order = 4;
    insertSorted(root, 1);
    insertSorted(root, 3);
    insertSorted(root, 4);
    insertSorted(root, 5);

    root = splitNode(root);

    printTree(root);

    return 0;
}