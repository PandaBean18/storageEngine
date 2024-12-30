#include "btree.cpp"
#include "stack"
#include <cstring>

// Defination of keywords;

#define SELECT "select"
#define FROM "from"
#define STR_LITERAL "string_literal"
#define CREATE "create"
#define TABLE "table"
#define WHERE "where"
#define DELIMITER ";"
#define BRACKET_OPEN "("
#define BRACKET_CLOSE ")"

using namespace std;

struct INT {
    int val;
};

struct CHAR {
    int size;
    char* val;
};

struct VARCHAR {
    int maxSize;
    char* val;
};

struct Table {
    char* name;
    int rowSize;
};

struct Keyword {
    char* type;
    char* val = NULL; // only for string literals
};

class QueryParseError : public exception 
{
    public:
    const char* what() const noexcept override
    {
        return "There was an error while trying to parse your SQL query.\n";
    }
};

class UndefinedKeywordError : public exception 
{
    public:
    const char* what() const noexcept override
    {
        return "Undefined Keyword.\n";
    }
};

char* findCurrentKeyword(char* word) {
    if ((strcmp(word, "SELECT") == 0) || (strcmp(word, "select") == 0)) {
        return SELECT;
    } else if ((strcmp(word, "FROM") == 0) || (strcmp(word, "from") == 0)) {
        return FROM;
    } else if ((strcmp(word, "CREATE") == 0) || (strcmp(word, "create") == 0)) {
        return CREATE;
    } else if ((strcmp(word, "TABLE") == 0) || (strcmp(word, "table") == 0)) {
        return TABLE;
    } else if ((strcmp(word, "WHERE") == 0) || (strcmp(word, "where") == 0)) {
        return WHERE;
    } else {
        cout << word << endl;
        throw UndefinedKeywordError();
    }
}

LinkedListNode<char*>* convertStringToKeywords(char* string) {
    LinkedListNode<char*>* start = NULL;
    LinkedListNode<char*>* end = start;

    char* currentWord = NULL;
    int currentWordSize = 0;
    int isStringLiteral = 0;

    while(*string != '\0') {

        if (isStringLiteral) {
            if (*string == '\'') {
                isStringLiteral = 0;
                char* temp = (char*)realloc(currentWord, currentWordSize+1);
                currentWord = temp;
                currentWord[currentWordSize] = '\0';
                LinkedListNode<char*>* t = new LinkedListNode<char*>;
                t->data = currentWord;
                t->next = NULL;

                if (end) {
                    end->next = t;
                    end = end->next;
                } else {
                    start = end = t;
                }

                currentWordSize = 0;
                currentWord = NULL;
            } else {
                char* temp = (char*)realloc(currentWord, currentWordSize+1);
                currentWord = temp;
                currentWord[currentWordSize] = *string;
                currentWordSize++;
            }
        } else if (*string == '\'') {
            isStringLiteral = 1;
        } else if (*string == ' ' && currentWord == NULL) {
            string++;
            continue;
        } else if (*string == ' ') {
            LinkedListNode<char*>* t = new LinkedListNode<char*>;
            char* temp = (char*)realloc(currentWord, currentWordSize+1);
            currentWord = temp;
            currentWord[currentWordSize] = '\0';
            t->data = findCurrentKeyword(currentWord);
            t->next = NULL;
            
            if (end) {
                end->next = t;
                end = end->next;
            } else {
                start = end = t;
            }

            currentWordSize = 0;
            currentWord = NULL;
        } else {
            char* temp = (char*)realloc(currentWord, currentWordSize+1);
            currentWord = temp;
            currentWord[currentWordSize] = *string;
            currentWordSize++;
        }
        string++;
    }
    return start;
}

// Table* createTable(char* query) {

// }

int main() {
    // BTreeNode* root = new BTreeNode;
    // root->keys = new LinkedListNode<int>;
    // root->keys->data = 2;
    // root->keys->next = NULL;
    // root->children = NULL;
    // root->countChildren = 0;
    // root->countKeys = 1;
    // root->isLeaf = 1;
    // root->order = 4;
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

    LinkedListNode<char*>* keywords = convertStringToKeywords("SELECT CREATE FROM  select    create   'hello this is a string literal'\0");
    int countKeywords = 0;
    while (keywords) {
        cout << keywords->data << endl;
        keywords = keywords->next;
        countKeywords++;
    }
    cout << "Total Keywords: " << countKeywords << endl;
    return 0;
}