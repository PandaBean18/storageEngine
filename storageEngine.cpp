#include "btree.cpp"
#include <cstring>
#include <string>
#include <fstream>
#include <filesystem>

// Defination of keywords;

#define SELECT "select"
#define INSERT "insert"
#define INTO "into"
#define FROM "from"
#define VALUES "values"
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
#define INSERT_VALUE "insert_value"

using namespace std;
using namespace std::filesystem;

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
    LinkedListNode<char*>* columnNames;
    LinkedListNode<char*>* dataTypes;
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
    } else if ((strcmp(word, "VALUES") == 0) || (strcmp(word, "values") == 0)) {
        return VALUES;
    } else if ((strcmp(word, "INSERT") == 0) || (strcmp(word, "insert") == 0)) {
        return INSERT;
    } else if ((strcmp(word, "INTO") == 0) || (strcmp(word, "into") == 0)){
        return INTO;
    } else {
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

CreateTableRequest* convertCreateStringToRequest(LinkedListNode<Keyword*>* keywords) {
    CreateTableRequest* createTableReq = new CreateTableRequest;
    LinkedListNode<char*>* cols = createTableReq->columnNames = NULL;
    LinkedListNode<char*>* colDataTypes = createTableReq->dataTypes = NULL;
    int countCols = 0, countDataTypes = 0;

    if (keywords->data->type != CREATE) {
        throw QueryParseError();
    }

    while (keywords) {
        if (keywords->data->type == TABLE_NAME) {
            createTableReq->tableName = keywords->data->val;
        } else if (keywords->data->type == ATTRIBUTE_NAME) {
            LinkedListNode<char*>* t = new LinkedListNode<char*>;

            t->data = keywords->data->val;
            t->next = NULL;

            if (t->data == NULL) {
                throw SyntaxError();
            }

            if (createTableReq->columnNames == NULL) {
                createTableReq->columnNames = t;
                cols = t;
                countCols++;
            } else {
                cols->next = t;
                cols = t;
                countCols++;
            }

            LinkedListNode<char*>* t1 = new LinkedListNode<char*>; // reusing the  variable as they are both linkedLists of character data
            t1->data = keywords->data->datatype;
            t1->next = NULL;

            if (t1->data == NULL) {
                throw SyntaxError();
            }

            if (createTableReq->dataTypes == NULL) {
                createTableReq->dataTypes = t1;
                colDataTypes = t1;
                countDataTypes++;
            } else {
                colDataTypes->next = t1;
                colDataTypes = t1;
                countDataTypes++;
            }

            if (keywords->data->isPrimary) {
                createTableReq->primaryIndexColumn = keywords->data->val;
            }
        }
        keywords = keywords->next;
    }

    if (countCols != countDataTypes) {
        throw SyntaxError();
    }

    return createTableReq;
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
                if (ignoreUndefinedKeywords == 0 && (end->data->type != TABLE && end->data->type != INTO)) {
                    throw UndefinedKeywordError();
                } else if (end->data->datatype == INTO) {
                    if (start->data->type == INSERT) {
                        keyword->type = TABLE_NAME;
                        keyword->val = currentWord;
                    }
                } else if (end->data->type == TABLE || end->data->type == INTO) {
                    if (start->data->type == CREATE || start->data->type == INSERT) {
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
            if (start->data->type == CREATE) {
                ignoreUndefinedKeywords = 1;
            } else if (end->data->type == VALUES) {
                Keyword* k = new Keyword;
                k->type = INSERT_VALUE;
                string++;
                char* val = NULL;
                int len = 0;
                while (*string != ')') {
                    val = (char*)realloc(val, len+1);
                    val[len] = *string;
                    len++;
                    string++;
                }
                val = (char*)realloc(val, len+1);
                val[len] = '\0';

                k->val = val;

                end->next = new LinkedListNode<Keyword*>;
                end = end->next;
                end->data = k;
                end->next = NULL;

            }
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

void createTable(CreateTableRequest* req) {
    // first line: Column names with their data types
    // second: primary key index column
    char* path = (char*)malloc(sizeof(char)*5);
    path[0] = '.';
    path[1] = '/';
    path[2] = 'd';
    path[3] = 'b';
    path[4] = '/';
    int len = 5;
    
    for (char* c = req->tableName; *c != '\0'; c++) {
        path = (char*)realloc(path, len+1);
        path[len] = *c;
        len++;
    }
    
    path = (char*)realloc(path, len+1);
    path[len] = '\0';

    create_directory("db");

    ofstream writeFile = ofstream(path, ios_base::out);

    LinkedListNode<char*>* columnNames = req->columnNames;
    LinkedListNode<char*>* columnDataTypes = req->dataTypes;

    while (columnNames) {
        char* name = columnNames->data;

        while (*name != '\0') {
            writeFile.write(name, 1);
            name++;
        }

        writeFile.write(":", 1);

        char* dataType = columnDataTypes->data;

        while (*dataType != '\0') {
            writeFile.write(dataType, 1);
            dataType++;
        }
        writeFile.write(" ", 1);
        columnDataTypes = columnDataTypes->next;
        columnNames = columnNames->next;
    }

    writeFile.write("\n", 1);

    char* primary = req->primaryIndexColumn;

    while (*primary != '\0') {
        writeFile.write(primary, 1);
        primary++;
    }

    writeFile.write("\n", 1);

    writeFile.close();
}

int main() {
    char* inp = NULL;
    int count = 0;
    cout << "qbd> ";
    string a;
    getline(cin, a);
    
    for (string::iterator it = a.begin(); it != a.end(); it++) {
        inp = (char*)realloc(inp, count+1);
        inp[count] = *it;

        count++;
    }

    inp = (char*)realloc(inp, count+1);
    inp[count] = '\0';

    LinkedListNode<Keyword*>* keywords = convertStringToKeywords(inp);
    // CreateTableRequest* c = convertCreateStringToRequest(keywords);
    // LinkedListNode<char*>* cols = c->columnNames;
    // LinkedListNode<char*>* dataTypes = c->dataTypes;

    //createTable(c);

    while (keywords) {
        cout << keywords->data->type << endl;
        keywords = keywords->next;
    }

    return 0;
}