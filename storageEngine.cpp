#include <iostream>

using namespace std;

template<typename t>
struct LinkedListNode{
    t data; 
    LinkedListNode* next;   
};

class EndOfNodeError : public exception 
{
    public:
    const char* what() const noexcept override
    {
        return "Reached the end of the LinkedList before reaching the desired node.\n";
    }
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

template<typename t>
LinkedListNode<t>* findNodeAtIndex(LinkedListNode<t>* root, int index) {
    int i = 0;
    LinkedListNode<t>* current = root;
    while (i < index) {
        i++;
        if (current == NULL) {
            throw EndOfNodeError();
        }
        current = current->next;
    }
    return current;
}

// The function takes a node and a value as parameters and inserts the
// value into the node in a sorted manner. It does not split incase
// the root is filled.
void insertSorted(BTreeNode* node, BTreeNode* val) {
    //todo: update function to take input of val as BTreeNode of one key
    // this way if the value to be inserted is new val being inserted at
    // root, we can add it directly but also if it is a value coming from
    // splitNode
    LinkedListNode<int>* newVal = new LinkedListNode<int>;
    newVal->data = val->keys->data;
    newVal->next = NULL;


    if (!(node->keys)) {
        node->keys = val->keys;
        node->countKeys = 1;
        node->children = val->children;
        node->countChildren = val->countChildren;
        return;
    }

    LinkedListNode<int>* current = node->keys;
    int inserted = 0;
    int i = 0;

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
        i++;
    }

    if (!inserted) {
        current->next = newVal;
    }

    node->countKeys++;

    if (val->countChildren != 0) {
        i = i-1;
        LinkedListNode<BTreeNode*>* l = findNodeAtIndex(node->children, i-1);
        l->next = val->children;
    }
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
        // since we are splitting the node here and node is at max capacity
        // lc and rc can not overflow.
        BTreeNode* newNode = new BTreeNode;
        newNode->keys = new LinkedListNode<int>;
        newNode->keys->data = current->data;
        newNode->keys->next = NULL;
        newNode->countKeys = 1;

        if (node->isLeaf == 1) {
            newNode->countChildren = 0;
        } else {
            newNode->countChildren = 2;
            newNode->children = new LinkedListNode<BTreeNode*>;
            LinkedListNode<BTreeNode*>* leftChildOfCurrent = findNodeAtIndex(node->children, i);
            LinkedListNode<BTreeNode*>* rightChildOfCurrent = leftChildOfCurrent->next;
            
            if (rightChildOfCurrent == NULL) {
                throw EndOfNodeError();
            }

            newNode->children->data = leftChildOfCurrent->data;
            newNode->children->next = new LinkedListNode<BTreeNode*>;
            newNode->children->next->data = rightChildOfCurrent->data;
            newNode->children->next->next = NULL;

        }

        if (i == median) {
            newNode->countChildren = 2;
            newNode->children = new LinkedListNode<BTreeNode*>;
            newNode->children->data = lc;
            newNode->children->next = new LinkedListNode<BTreeNode*>;
            newNode->children->next->data = rc;
            newNode->children->next->next = NULL;
            insertSorted(root, newNode);
        } else if (i < median) {
            insertSorted(lc, newNode);
        } else if (i > median) {
            insertSorted(rc, newNode);
        }
        i++;
        current = current->next;
    }

    // if (!node->isLeaf) {
    //     i = 0;
    //     LinkedListNode<BTreeNode*>* childrenLC;
    //     LinkedListNode<BTreeNode*>* childrenRC;
    //     LinkedListNode<BTreeNode*>* current = node->children;
    //     childrenLC = node->children;

    //     while (i < median) {
    //         current = current->next;
    //         i++;
    //     }
        
    //     childrenRC = current->next;
    //     current->next = NULL;
    //     lc->children = childrenLC;
    //     lc->countChildren = i;
    //     lc->countKeys = i-1;
    //     rc->children = childrenRC;
    //     rc->countChildren = (node->countChildren)-i;
    //     rc->countKeys = (node->countChildren)-i+1;
    // }

    // LinkedListNode<BTreeNode*>* rootChildren = new LinkedListNode<BTreeNode*>;
    // rootChildren->data = lc;
    // rootChildren->next = new LinkedListNode<BTreeNode*>;
    // rootChildren->next->data = rc;
    // rootChildren->next->next = NULL;
    // rc->isLeaf = lc->isLeaf = 1; 
    // root->children = rootChildren;    
    // root->countChildren = 2;
    // root->countKeys = 1;
    return root;
}

/// @brief combined function for inserting a value into the btree
/// @param node 
/// @param val 
/// @return BTreeNode*
BTreeNode* insert(BTreeNode* node, int val) {
    BTreeNode* newNode = new BTreeNode;
    newNode->keys = new LinkedListNode<int>;
    newNode->keys->data = val;
    newNode->keys->next = NULL;
    newNode->countKeys = 1;

    insertSorted(node, newNode);
    BTreeNode* root = node;

    if (node->countKeys >= node->order) {
        root = splitNode(node);
    }

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

    if (toBeTraversed != NULL && toBeTraversed->data != nullptr) {
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
    root->keys->next = NULL;
    root->children = NULL;
    root->countChildren = 0;
    root->countKeys = 1;
    root->isLeaf = 1;
    root->order = 5;
    root = insert(root, 1);
    root = insert(root, 3);
    root = insert(root, 4);
    root = insert(root, 5);
    root = insert(root, 6);
    
    // insertSorted(root, 1);
    // insertSorted(root, 3);
    // insertSorted(root, 4);
    // insertSorted(root, 5);

    //root = splitNode(root);

    printTree(root);

    return 0;
}