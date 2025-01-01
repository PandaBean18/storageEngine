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
#define TABLE_NAME "table_name"
#define ATTRIBUTE_NAME "attribute_name"
#define TYPE_INT "int"
#define TYPE_CHAR "char"
#define PRIMARY_KEY "primary_key"

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

struct Attribute {
    char* attrName;
    char* attrDatatype;
};

struct Table {
    char* name;
    int rowSize;
};

struct Keyword {
    char* type;
    char* val = NULL; // only for string literals
    char* datatype = NULL; // only for attributes
    int isPrimary = 0; // only for attributes
};

struct CreateTableRequest {
    char* tableName;
    char* columnNames;
    char* dataTypes;
    char* primaryIndexColumn;
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

class SyntaxError : public exception {
    public:
    const char* what() const noexcept override {
        return "There was a syntax error in your SQL query.\n";
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
    } else if ((strcmp(word, "INT") == 0) || (strcmp(word, "int") == 0)) {
        return TYPE_INT;
    } else if ((strcmp(word, "CHAR") == 0) || (strcmp(word, "char") == 0)) {
        return TYPE_CHAR;
    } else if ((strcmp(word, "PRIMARY_KEY") == 0) || (strcmp(word, "primary_key") == 0)) {
        return PRIMARY_KEY;
    } else {
        //cout << word << endl;
        return NULL;
    }
}

char* parseAttribute(char* string, Keyword* keyword) {
    while(*string != ',' && *string != ')') {
        char* currentWord = NULL;
        int currentWordLength = 0;
        while (*string != ' ') {
            if (*string == ',' || *string == ')') {
                char* temp = (char*)realloc(currentWord, currentWordLength+1);
                currentWord = temp;
                currentWord[currentWordLength] = '\0';
                char* currentKeyword = findCurrentKeyword(currentWord);

                if (currentKeyword != PRIMARY_KEY && currentKeyword != TYPE_CHAR && currentKeyword != TYPE_INT) {
                    throw SyntaxError();
                }

                if (currentKeyword == TYPE_CHAR || currentKeyword == TYPE_INT) {
                    if (keyword->datatype != NULL) {
                        throw SyntaxError();
                    }

                    keyword->datatype = currentKeyword;
                } else if (currentKeyword == PRIMARY_KEY) {
                    keyword->isPrimary = 1;
                }
                return string;
            } else {
                char* temp = (char*)realloc(currentWord, currentWordLength+1);
                currentWord = temp;
                currentWord[currentWordLength] = *string;
                currentWordLength++;
            }
            string++;
        }

        if (currentWord == NULL) {
            string++;
            continue;
        }

        char* temp = (char*)realloc(currentWord, currentWordLength+1);
        currentWord = temp;
        currentWord[currentWordLength] = '\0';
        char* currentKeyword = findCurrentKeyword(currentWord);

        if (currentKeyword != PRIMARY_KEY && currentKeyword != TYPE_CHAR && currentKeyword != TYPE_INT) {
            throw SyntaxError();
        }

        if (currentKeyword == TYPE_CHAR || currentKeyword == TYPE_INT) {
            if (keyword->datatype != NULL) {
                throw SyntaxError();
            }

            keyword->datatype = currentKeyword;
        } else if (currentKeyword == PRIMARY_KEY) {
            keyword->isPrimary = 1;
        }
        currentWord = NULL;
        currentWordLength = 0;
        string++;
    }
    return string;
}

CreateTableRequest* convertCreateStringToRequest(char* string) {

}

LinkedListNode<Keyword*>* convertStringToKeywords(char* string) {
    LinkedListNode<Keyword*>* start = NULL;
    LinkedListNode<Keyword*>* end = start;

    char* currentWord = NULL;
    int currentWordSize = 0;
    int isStringLiteral = 0;
    int ignoreUndefinedKeywords = 0;

    while(*string != '\0') {
        if (isStringLiteral) {
            if (*string == '\'') {
                isStringLiteral = 0;
                char* temp = (char*)realloc(currentWord, currentWordSize+1);
                currentWord = temp;
                currentWord[currentWordSize] = '\0';
                Keyword* strLiteral = new Keyword;
                strLiteral->type = STR_LITERAL;
                strLiteral->val = currentWord;
                LinkedListNode<Keyword*>* t = new LinkedListNode<Keyword*>;
                t->data = strLiteral;
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
            Keyword* keyword = new Keyword;
            LinkedListNode<Keyword*>* t = new LinkedListNode<Keyword*>;
            char* temp = (char*)realloc(currentWord, currentWordSize+1);
            currentWord = temp;
            currentWord[currentWordSize] = '\0';
            keyword->type = findCurrentKeyword(currentWord);

            if (keyword->type == NULL) {
                if (ignoreUndefinedKeywords == 0 && end->data->type != TABLE) {
                    throw UndefinedKeywordError();
                } else if (end->data->type == TABLE) {
                    if (start->data->type == CREATE) {
                        keyword->type = TABLE_NAME;
                        keyword->val = currentWord;
                    }
                } else if (ignoreUndefinedKeywords && start->data->type == CREATE) {
                    keyword->type = ATTRIBUTE_NAME;
                    keyword->val = currentWord;
                    string = parseAttribute(string, keyword);
                    if (*string == ')') {
                        string--;
                    } 
                }
            } else {
                keyword->val = NULL;
            }

            t->data = keyword;
            t->next = NULL;
            
            if (end) {
                end->next = t;
                end = end->next;
            } else {
                start = end = t;
            }

            currentWordSize = 0;
            currentWord = NULL;
        } else if (*string == '(') {
            ignoreUndefinedKeywords = 1;
        } else if (*string == ')') {
            if (ignoreUndefinedKeywords == 0) {
                throw SyntaxError();
            } else {
                if (currentWord != NULL) {
                    Keyword* keyword = new Keyword;
                    LinkedListNode<Keyword*>* t = new LinkedListNode<Keyword*>;
                    char* temp = (char*)realloc(currentWord, currentWordSize+1);
                    currentWord = temp;
                    currentWord[currentWordSize] = '\0';
                    keyword->type = findCurrentKeyword(currentWord);

                    if (keyword->type == NULL) {
                        if (ignoreUndefinedKeywords == 0 && end->data->type != TABLE) {
                            throw UndefinedKeywordError();
                        } else if (end->data->type == TABLE) {
                            if (start->data->type == CREATE) {
                                keyword->type = TABLE_NAME;
                                keyword->val = currentWord;
                            }
                        } else if (ignoreUndefinedKeywords && start->data->type == CREATE) {
                            keyword->type = ATTRIBUTE_NAME;
                            keyword->val = currentWord;
                        }
                    } else {
                        keyword->val = NULL;
                    }

                    t->data = keyword;
                    t->next = NULL;
                    
                    if (end) {
                        end->next = t;
                        end = end->next;
                    } else {
                        start = end = t;
                    }

                    currentWordSize = 0;
                    currentWord = NULL;
                }
                ignoreUndefinedKeywords = 0;
            }
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

Table* createTable(char* query) {
    LinkedListNode<Keyword*>* parsedKeywords = convertStringToKeywords(query);

    if (parsedKeywords->data->type != CREATE) {
        throw QueryParseError();
    } 
}

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

    LinkedListNode<Keyword*>* keywords = convertStringToKeywords("CREATE TABLE users (userID INT PRIMARY_KEY, userName CHAR)\0");
    int countKeywords = 0;

    cout << "Input: " << "CREATE TABLE users (userID INT PRIMARY_KEY, userName CHAR)\0" << endl;
    while (keywords) {
        cout << endl << keywords->data->type;
        if (keywords->data->type == STR_LITERAL || keywords->data->type == ATTRIBUTE_NAME || keywords->data->type == TABLE_NAME) {
            cout << "\nValue: " << keywords->data->val << endl;

            if (keywords->data->type == ATTRIBUTE_NAME) {
                cout << "Datatype: " << keywords->data->datatype << endl;
                cout << "Is Primary: " << keywords->data->isPrimary << endl; 
            }

            cout << endl << endl;
        }
        keywords = keywords->next;
        countKeywords++;
    }
    cout << "Total Keywords: " << countKeywords << endl;
    return 0;
}