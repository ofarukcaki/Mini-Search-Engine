#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct locationInfo {
    std::string file;
    int lineNumber;
    int index;

    locationInfo(std::string file,
                 int lineNumber,
                 int index) {
        this->file = file;
        this->lineNumber = lineNumber;
        this->index = index;
    }
};

struct node {
    char c; // character, '\0' for null termiantor
    bool isLeaf;
    std::vector<locationInfo *>
        locations;
    // std::vector<node *> childs; // pointer vector for childrens
    node *childs[40];

    // constructor
    // node(char _c, bool _isLeaf) {
    //     c = _c;
    //     isLeaf = _islea
    // }
};

// get index of normalzied char
int getIndex(char c) {
    int _c = int(c);
    if (_c >= 48 && _c <= 57) {
        return c - 48;
    } else if (_c >= 97 && _c <= 122) {
        return _c - 87;
    }
    return -1;
}

void insert(node *root, string str, string file, int lineNumber, int index) {
    // struct node *current = root;
    node *current = root;
    // cout << "_" << str << "_\n";
    for (int i = 0; i < str.length(); i++) {
        if (current->childs[getIndex(str[i])]) {
            // node already avaialble
        } else {
            // insert new child and set values
            node *child = new node();
            child->isLeaf = false;
            child->c = str[i];
            current->childs[getIndex(str[i])] = child;
        }
        current = current->childs[getIndex(str[i])];
    }

    // insert leaf!(null terminator node)
    if (current->childs[39]) {
        // there is already a leaf node, insert pair
        locationInfo *loc = new locationInfo(file, lineNumber, index);
        current->childs[39]->locations.push_back(loc);
    } else {
        // create leaf node for the first time
        node *leaf = new node();
        leaf->isLeaf = true;
        leaf->c = '\0';

        locationInfo *loc = new locationInfo(file, lineNumber, index);
        leaf->locations.push_back(loc);
        current->childs[39] = leaf;
    }
}

// convert string to lowercase
std::string toLowerCase(string _str) {
    std::string str = _str;
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

void insertString(node *root, string _str, string file, int lineNumber, int index) {
    std::string str = toLowerCase(_str);
    int len = str.length();

    insert(root, str, file, lineNumber, index);
}

// starting from the index given, find the word's length on the string
int lengthOfWord(string &str, int start) {
    int len = 0;
    for (int i = start; i < str.length(); i++) {
        if (getIndex(str[i]) == -1)
            break;
        len++;
    }
    return len;
}

void parseLine(node *root, string line, string file, int lineNumber) {
    string lc = toLowerCase(line);
    // cout << lc << endl;

    // loop through the line and find the index of all
    bool space = true;
    for (int i = 0; i < lc.length(); i++) {
        if (getIndex(lc[i]) == -1) {
            space = true;
        } else {
            if (space == true) {
                insertString(root, lc.substr(i, lengthOfWord(lc, i)), file, lineNumber, i);
            } else {
                continue;
            }
            space = false;
        }
    }
}

// find all leafs of given node
void DFSprint(node *root) {
    node *current = root;
    if (current->isLeaf) {
        // print all eow
        for (auto x : current->locations) {
            cout << x->file << "  Line:" << x->lineNumber << "  Index:" << x->index + 1 << endl;
        }

    } else {
        for (int i = 0; i < 40; i++) {
            if (current->childs[i]) {
                DFSprint(current->childs[i]);
            }
        }
    }
}

bool isContainAll(vector<locationInfo *> locations, vector<string> files) {
    std::set<string> _set;

    for (const auto x : locations) {
        for (const auto file : files) {
            if (x->file.compare(file) == 0) {
                _set.insert(file);
            }
        }
    }
    return _set.size() == files.size() ? true : false;
}

void printLocs(node *root) {
    for (auto x : root->locations) {
        cout << ">>> " << x->file << endl;
    }
}

void commonDFS(node *root, vector<string> files, string path) {
    node *current = root;
    if (current->isLeaf) {
        // print all eow
        if (isContainAll(current->locations, files)) {
            cout << path << endl;
        }
    } else {
        for (int i = 0; i < 40; i++) {
            if (current->childs[i]) {
                commonDFS(current->childs[i], files, path + current->c);
            }
        }
    }
}

void search(node *root, string Query) {

    string query = toLowerCase(Query);
    node *current = root;

    bool found = true;
    for (int i = 0; i < query.length(); i++) {
        if (current->childs[getIndex(query[i])]) {
            // go to child
            current = current->childs[getIndex(query[i])];
        } else {
            // not found
            cout << "'" << Query << "' not found!" << endl;
            found = false;
            break;
        }
    }

    if (found) {
        // DFS traversal
        cout << "Words starting with '" << Query << "' is found in following files:\n";
        DFSprint(current);
    }
}

void readFile(node *root, string fileName) {
    std::ifstream infile(fileName);

    std::string line;
    int lineNum = 1;
    while (std::getline(infile, line)) {
        // cout << "_" << line << "_" << endl;
        parseLine(root, line, fileName, lineNum);
        lineNum++;
    }
}

void printStringFromNode(node *root) {
    for (auto x : root->childs) {
        if (x) {
            cout << x->c << endl;
        }
    }
}

void printCommons(node *root, vector<string> files) {
    cout << "searching in files: \n";
    for (int i = 0; i < files.size(); i++) {
        cout << files[i] << endl;
    }

    cout << "\nCommon word(s):\n";
    commonDFS(root, files, " ");
}

int main(int argc, char **argv) {
    std::vector<string> files;
    node *root = new node();

    // std::string path = "./sampleTextFiles";
    std::string path = argv[1];
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        // std::cout << entry.path() << std::endl;
        readFile(root, entry.path().string());
        files.push_back(entry.path().string());
    }

    while (true) {
        int result;
        // promth
        cout << "\nPlease select your query type:" << endl;
        cout << "1) Search a word in all files\n";
        cout << "2) Find common word(s) in files\n> ";
        cin >> result;

        if (result == 1) {
            string res;
            cout << "Enter your query: ";
            cin >> res;
            search(root, res);

        } else if (result == 2) {
            char r;
            cout << "\nDo you want to search in all files? [Y/n]: ";
            cin >> r;
            if (r == 'Y' || r == 'y') {
                printCommons(root, files);
            } else {
                cout << endl
                     << "Available files: \n";
                for (int i = 0; i < files.size(); i++) {
                    cout << i + 1 << ") " << files[i] << endl;
                }
                cout << "How many files you want to include: ";
                int inpNum;
                cin >> inpNum;
                cout << "Please enter the numbers of the files you want to include (one per line):\n";
                vector<int> numbers;
                vector<string> filesToSearch;
                for (int i = 0; i < inpNum; i++) {
                    int temp;
                    cout << "> ";
                    cin >> temp;
                    filesToSearch.push_back(files[temp - 1]);
                }
                printCommons(root, filesToSearch);
            }
        }
    }

    // assert(getIndex('0') == 0);
    // assert(getIndex('a') == 10);

    return 0;
}