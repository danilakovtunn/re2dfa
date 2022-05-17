#include "api.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <set>

using namespace std;

vector<char>::reverse_iterator iter;
vector<char> poliz;
vector<set<int>> followpos;
vector<char> positions;
int num = 0;

class Node
{
  public:
    char value;
    bool nullable;
    set<int> firstpos;
    set<int> lastpos;
    Node *left;
    Node *right;
};

Node *get_tree() {
    if (iter == poliz.rend()) { //дошли до конца
        return NULL;
    }
    Node *elem = new Node;
    if (*iter != '|' && *iter != '&' && *iter != '*') { //встретили символ алфавита
        elem->value = *iter;
        if (elem->value == '@') {
            elem->nullable = true;
        } else {
            elem->nullable = false;
            elem->firstpos.insert(num);
            elem->lastpos.insert(num);
        }
        positions[num-1] = *iter;
        num--;
        elem->left = NULL;
        elem->right = NULL;
        ++iter;
        return elem;
    } else if (*iter == '*') {
        elem->value = *iter;
        ++iter;
        elem->left = NULL;
        elem->right = get_tree();
        elem->nullable = true;
        elem->firstpos = elem->right->firstpos;
        elem->lastpos = elem->right->lastpos;
        return elem;
    } else if (*iter == '&') {
        elem->value = *iter;
        ++iter;
        elem->right = get_tree();
        elem->left = get_tree();
        elem->nullable = elem->right->nullable && elem->left->nullable;
        elem->firstpos = elem->left->firstpos;
        if (elem->left->nullable) {
            elem->firstpos.insert(elem->right->firstpos.begin(), elem->right->firstpos.end());
        }
        elem->lastpos = elem->right->lastpos;
        if (elem->right->nullable) {
            elem->lastpos.insert(elem->left->lastpos.begin(), elem->left->lastpos.end());
        }
        return elem;
    } else if (*iter == '|') {
        elem->value = *iter;
        ++iter;
        elem->right = get_tree();
        elem->left = get_tree();
        elem->nullable = elem->right->nullable || elem->left->nullable;
        elem->firstpos = elem->left->firstpos;
        elem->firstpos.insert(elem->right->firstpos.begin(), elem->right->firstpos.end());
        elem->lastpos = elem->right->lastpos;
        elem->lastpos.insert(elem->left->lastpos.begin(), elem->left->lastpos.end());
        return elem;
    }
}

void print_tree(Node *tree) {
    if (tree == NULL)
        return;
    cout << tree->value << " nullable: " << tree->nullable << " firstpos: ";
    for (int const& val : tree->firstpos) 
        cout << val << " ";
    cout << "lastpos:  ";
    for (int const& val : tree->lastpos) 
        cout << val << " ";
    cout << endl;

    print_tree(tree->right);
    print_tree(tree->left);
}

void get_followpos(Node *tree) {
    if (tree == NULL)
        return;
    get_followpos(tree->left);
    get_followpos(tree->right);
    
    if (tree->value != '&' && tree->value != '*')
        return;
    if (tree->value == '&') {
        for (int const& val : tree->left->lastpos) {
            followpos[val - 1].insert(tree->right->firstpos.begin(), tree->right->firstpos.end());
        }
    }
    if (tree->value == '*') {
        for (int const& val : tree->right->lastpos) {
            followpos[val - 1].insert(tree->right->firstpos.begin(), tree->right->firstpos.end());
        }
    }
}

void print_followpos() {
    int k = 1;
    for (set elem : followpos) {
        cout << k++ << ": ";
        for (int const& val : elem) 
            cout << val << " ";
        cout << endl;
    }
}

string set_to_string(set<int> s) {
    string str;
    for (int elem: s) {
        str += to_string(elem) + " ";
    }
    return str;
}

DFA re2dfa(const std::string &s) {
	DFA res = DFA(Alphabet(s));
    
    //   ПОСТРОЕНИЕ ПОЛИЗ
    stack<char> st;
    char sym = '\0', elem;
    for (const char& c : s) {
        if (c == '*') {
            poliz.push_back('*');
            sym = '*';
        } else if (c == '|') {
            if (sym == '\0' || sym == '(' || sym == '|') {
                poliz.push_back('@');
                num++;
            }
            if (!st.empty()) {
                while (!st.empty()) {
                    elem = st.top();
                    st.pop();
                    if (elem == '&' || elem == '|') {
                        poliz.push_back(elem);
                    } else {
                        st.push(elem);
                        break;
                    }
                }
            }
            st.push('|');
            sym = '|';
        } else if (c == '(') {
            if (sym == 'a' || sym == '*' || sym == ')') {
                while (!st.empty()) {
                    elem = st.top();
                    st.pop();
                    if (elem == '&') {
                        poliz.push_back(elem);
                    } else {
                        st.push(elem);
                        break;
                    }
                }
                st.push('&');
            } 
            st.push('(');
            sym = '(';
        } else if (c == ')') {
            if (sym == '|' || sym == '(') {
                poliz.push_back('@');
                num++;
            }
            while ((elem = st.top()) != '(') {
                st.pop();
                poliz.push_back(elem);
            }
            st.pop();
            sym = ')';
        } else {
            if (sym == 'a' || sym == '*' || sym == ')') {
                while (!st.empty()) {
                    elem = st.top();
                    st.pop();
                    if (elem == '&') {
                        poliz.push_back(elem);
                    } else {
                        st.push(elem);
                        break;
                    }
                }
                st.push('&');
            }
            poliz.push_back(c);
            num++;
            sym = 'a';
        }
    }
    if (sym == '|' || sym == '\0') {
        num++;
        poliz.push_back('@');
    }
    while(!st.empty()) {
        elem = st.top();
        st.pop();
        poliz.push_back(elem);
    }
    poliz.push_back('#');
    poliz.push_back('&');
    num++;
    
    cout << s << endl;
    for (char& c : poliz)
        cout << c;
    cout << endl;
    cout << num << endl;
    
    // ПОСТРОЕНИЕ ДЕРЕВА ПО ПОЛИЗУ
    iter = poliz.rbegin();
    int n = num;
    positions = vector<char>(num);
    Node *tree = get_tree();
    for (const char &i : positions)
        cout << i;
    cout << endl;
    print_tree(tree);
    
    //  ПОСТРОЕНИЕ ФОЛЛОУПОС ПО ДЕРЕВУ
    num = n;
    followpos = vector<set<int>>(num);
    get_followpos(tree);
    print_followpos();

    // ПОСТРОЕНИЕ ДКА ПО ФОЛЛОУПОС
	string str = set_to_string(tree->firstpos);
    res.create_state("0", false);
	res.set_initial("0");
    
    set<set<int>> usage;
    set<set<int>> help;
    set<string> Q;
    map <string, int> mymap;
    usage.insert(tree->firstpos);
    
    if (tree->firstpos.find(num) != tree->firstpos.end()) {
        res.make_final("0");
    }
    Alphabet alph = Alphabet(s);
    int cur_num = 1;
    mymap.insert(map<string, int>::value_type(str, 0));
    while (!usage.empty()) {
        for (set<int> elem : usage) {
            cout << "cur state:" << set_to_string(elem) << endl;
            for (const char *ac = alph.begin(); ac != alph.end(); ac++) {
                set<int> S;
                int k = 0;
                for (char pc : positions) {
                    if ((*ac == pc) && (elem.find(k + 1) != elem.end())) {
                        S.insert(followpos[k].begin(), followpos[k].end());
                    }
                    k++;
                }
                if (!S.empty()) {
                    if (mymap.find(set_to_string(S)) == mymap.end()) {
                        cout << "add state: " << set_to_string(S) << "\n";
                        res.create_state(to_string(cur_num));
                        mymap.insert(map<string, int>::value_type(set_to_string(S), cur_num));

                        cout << num << endl;
                        if (S.find(num) != S.end()) {
                            res.make_final(to_string(mymap.at(set_to_string(S))));
                        }
                        help.insert(S);
                        cur_num++;
                    }
                    cout << set_to_string(elem) << "(--" << *ac << "->) " << set_to_string(S) << endl;
                    res.set_trans(to_string(mymap.at(set_to_string(elem))), *ac, to_string(mymap.at(set_to_string(S))));
                }
            }
        }
        usage.swap(help);
        help.clear();
    }
    return res;
}
