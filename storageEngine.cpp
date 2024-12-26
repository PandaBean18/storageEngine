#include "btree.cpp"

using namespace std;

int main() {
    BTreeNode* root = new BTreeNode;
    root->keys = new LinkedListNode<int>;
    root->keys->data = 2;
    root->keys->next = NULL;
    root->children = NULL;
    root->countChildren = 0;
    root->countKeys = 1;
    root->isLeaf = 1;
    root->order = 4;
    int a;
    while (1) {
        cout<< endl << "> ";
        cin >> a;

        if (a == -1) {
            break;
        }

        root = insert(root, a);
        printTree(root);
        cout << endl;
        cout << "Number of children of root: " << root->countChildren << endl;
    }
}