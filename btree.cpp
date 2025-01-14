#include <iostream>

using namespace std;

template<typename t>
struct LinkedListNode{
    t data; 
    int offset; // while writing majority of the code for the btree i forgot to add functionality to store byte offset for finding 
                // the actual row, so this value is there ONLY for BTreeNode to store file offset.
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

void insertKey(BTreeNode* node, int val) {
    LinkedListNode<int>* newVal = new LinkedListNode<int>;
    newVal->data = val;
    newVal->next = NULL;

    if ((node->keys == NULL) || (node->keys->data >= val)) {

        LinkedListNode<int>* temp = node->keys;
        node->keys = newVal;
        newVal->next = temp;
        node->countKeys++;
        return;
    }

    LinkedListNode<int>* current = node->keys;

    while (current->next != NULL) {
        if ((current->data < val) && (current->next->data > val)) {
            LinkedListNode<int>* temp = current->next;
            current->next = newVal;
            newVal->next = temp;
            node->countKeys++;
            return;
        }
        current = current->next;
    }

    current->next = newVal;
    newVal->next = NULL;
    node->countKeys++;
    return;
}

void insertChildren(BTreeNode* node, LinkedListNode<BTreeNode*>* children) {
    // since this function will be called after splitting node we can 
    // assume that the two children passed in arguments will always
    // be placed adjacent to each other

    node->isLeaf = 0;
    if (children == NULL) {
        return;
    }

    LinkedListNode<BTreeNode*>* current = node->children;
    int val = children->data->keys->data;

    if (current == NULL) {
        node->children = children;
        return;
    } 

    if (current->data->keys->data > val) {
        LinkedListNode<BTreeNode*>* temp = node->children;
        node->children = children;

        LinkedListNode<BTreeNode*>* c = children;

        while (c->next != NULL) {
            c = c->next;
        }

        c->next = temp;
        return;
    }

    while (current->next != NULL) {
        if ((current->data->keys->data < val) && (current->next->data->keys->data > val)) {
            LinkedListNode<BTreeNode*>* temp = current->next;
            current->next = children;
            LinkedListNode<BTreeNode*>* c = children;
            int i = 0;
            while (c->next != NULL) {
                c = c->next;
                i++;
            }
            c->next = temp;
            node->countChildren += i;
            return;
        }
        current = current->next;
        
    }

    current->next = children;
    node->countChildren = node->countKeys + 1;
    return;
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
    lc->isLeaf = rc->isLeaf = 1;
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
                rightChildOfCurrent = new LinkedListNode<BTreeNode*>;
                rightChildOfCurrent->data = NULL;
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
            insertKey(root, current->data);
        } else if (i < median) {
            insertKey(lc, current->data);
            insertChildren(lc, newNode->children);
        } else if (i > median) {
            insertKey(rc, current->data);
            insertChildren(rc, newNode->children);
        }
        i++;
        current = current->next;
    }

    LinkedListNode<BTreeNode*>* rootChildren = new LinkedListNode<BTreeNode*>;
    rootChildren->data = lc;
    rootChildren->next = new LinkedListNode<BTreeNode*>;
    rootChildren->next->data = rc;
    rootChildren->next->next = NULL;
    root->children = rootChildren;    

    root->countChildren = 2;
    root->countKeys = 1;
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

    // first we find the node in which we insert val;

    if (node->isLeaf) {
        insertKey(node, val);
        BTreeNode* root = node;

        if (node->countKeys >= node->order) {
            root = splitNode(node);
            root->isLeaf = 0;
            LinkedListNode<BTreeNode*>* c = root->children;

            while (c) {
                c->data->isLeaf = 1;
                c = c->next;
            }
        }

        return root;
    } else {
        BTreeNode* current = node;
        LinkedListNode<BTreeNode*>* path = new LinkedListNode<BTreeNode*>;
        path->data = current;
        path->next = NULL;
        int i = 0;
        while (!current->isLeaf) {
            i = 0;

            while (i < current->countKeys) {
                LinkedListNode<int>* currentVal = findNodeAtIndex(current->keys, i);
                
                if ((i == current->countKeys-1) && (val > currentVal->data)) {
                    current = (findNodeAtIndex(current->children, i+1))->data;
                    i++;
                    break;
                }

                if (val < currentVal->data) {
                    current = (findNodeAtIndex(current->children, i))->data;
                    i++;
                    break;
                }
                i++;
            }
            LinkedListNode<BTreeNode*>* temp = path;
            path = new LinkedListNode<BTreeNode*>;
            path->data = current;
            path->next = temp;
        }
        i--;
        insertKey(path->data, val);
        path->data->isLeaf = 1;
        BTreeNode* c = path->data;
        BTreeNode* prev = path->next->data;
        path = path->next->next;
        int cont = (current->countKeys >= current->order);
        int isFirstIter = 1;

        while (cont) {
            BTreeNode* currentRoot = splitNode(c);
            currentRoot->isLeaf = 0;

            if (isFirstIter) {
                isFirstIter = 0;
                LinkedListNode<BTreeNode*>* child = currentRoot->children;

                while (child) {
                    child->data->isLeaf = 1;
                    child = child->next;
                }
            }

            if (prev == NULL) {
                return currentRoot;
            } else {

                // node that is being split needs to be removed from parents children
                BTreeNode* parent = prev;
                LinkedListNode<BTreeNode*>* child = parent->children;

                if (child->data == c) {
                    parent->children = parent->children->next;
                } else {
                    while (child->next != NULL) {
                        if (child->next->data == c) {
                            child->next = child->next->next;
                            break;
                        }
                        child = child->next;
                    }
                }

                prev->countChildren -= 1;
                insertKey(prev, currentRoot->keys->data);
                insertChildren(prev, currentRoot->children);
                prev->isLeaf = 0;
                c = prev;
                if (path == NULL) {
                    prev = NULL;
                } else {
                    prev = path->data;
                    path = path->next;
                }
            }
               
            cont = (c->countKeys >= c->order);
        }
        return node;
    }    
}

BTreeNode* search(BTreeNode* root, int val) {
    LinkedListNode<int>* keys = root->keys;
    LinkedListNode<BTreeNode*>* children = root->children;

    while (1) {
        if (root->isLeaf == 0) {
            if (keys->data == val) {
                return root;
            } else if (keys->data > val) {
                root = children->data;
                keys = root->keys;
                children = root->children;
            } else if (val > keys->data && keys->next == NULL) {
                root = children->next->data;
                keys = root->keys;
                children = root->children;
            } else {
                keys = keys->next;
                children = children->next;
            }
        } else {
            while (keys) {
                if (keys->data == val) {
                    return root;
                }
                keys = keys->next;
            }
            return NULL;
        }
    }

}

void printTree(BTreeNode* root) {
    int count = 1;
    int mult = 0;
    LinkedListNode<BTreeNode*>* toBeTraversed = new LinkedListNode<BTreeNode*>;
    toBeTraversed->data = root;
    toBeTraversed->next = NULL;
    LinkedListNode<BTreeNode*>* last = toBeTraversed;

    while (toBeTraversed != NULL) {
        LinkedListNode<int>* values = toBeTraversed->data->keys;
        LinkedListNode<BTreeNode*>* children = toBeTraversed->data->children;

        while (values) {
            cout << values->data;

            cout << " ";
            values = values->next;
            
        }

        while (children) {
            last->next = new LinkedListNode<BTreeNode*>;
            last->next->data = children->data;
            last = last->next;
            last->next = NULL;

            children = children->next;
            mult++;
        }

        count--;

        if (count == 0) {
            count = mult;
            mult = 0;
            cout << endl;
        }
        toBeTraversed = toBeTraversed->next;
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
    root->order = 3;
    // int a;
    // while (1) {
    //     cout<< endl << "> ";
    //     cin >> a;

    //     if (a == -1) {
    //         break;
    //     }

    //     root = insert(root, a);
    //     printTree(root);
    //     cout << endl;
    //     cout << "Number of children of root: " << root->countChildren << endl;
    // }

    // return 0;


    root = insert(root, 0);
    root = insert(root, 1);
    root = insert(root, 3);
    root = insert(root, 4);
    root = insert(root, 5);
    root = insert(root, 6);
    root = insert(root, 7);
    root = insert(root, 8);
    root = insert(root, 9);
    root = insert(root, 10);
    printTree(root);
    cout << endl;

    delete root;
    return 0;
}