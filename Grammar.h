#include <string>
#include <utility>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include "list"
#include "iostream"

using namespace std;

#ifndef EDSL_CW_GRAMMAR_H
#define EDSL_CW_GRAMMAR_H


class NonTerminal {
public:
    string name;
    vector<vector<string>> productions;
    map<string, NonTerminal*> nt_by_name;

    NonTerminal(string name, const list<string>& productions) {
        this->name = move(name);
        for (auto s : productions) {
            string delimiter = " ";
            size_t pos = 0;
            string token;
            vector<string> lst;
            while ((pos = s.find(delimiter)) != string::npos) {
                token = s.substr(0, pos);
                lst.push_back(token);
                s.erase(0, pos + delimiter.length());
            }
            lst.push_back(s);
            this->productions.push_back(lst);
        }
    }
};

class Grammar {
public:
    list<NonTerminal*> nonterms;
    list<string> terminals;
    map<NonTerminal*, int> nonterm_offset;
    vector<pair<string, vector<string>>> productions;
    vector<string> symbols;
    map<string, NonTerminal*> nt_by_name;
    map<string, set<string>> first_sets;

    Grammar(list<NonTerminal*> nterms) {
        auto* start_nonterminal = new NonTerminal("$accept", {nterms.front()->name});
        nterms.sort([](const NonTerminal* a, const NonTerminal* b){ return a->name < b->name; });
        nterms.push_front(start_nonterminal);
        nonterms = nterms;

        for (auto* nt : nonterms) {
            nt_by_name[nt->name] = nt;
        }

        for (auto* nt : nonterms) {
            for (const auto& prod : nt->productions) {
                for (const auto& symbol : prod) {
                    if (nt_by_name.find(symbol) == nt_by_name.end()) {
                        terminals.push_back(symbol);
                    }
                }
            }
            nt->nt_by_name = nt_by_name;
        }
        terminals.sort();

        for (auto* nt : nonterms) {
            nonterm_offset[nt] = productions.size();
            for (const auto& prod : nt->productions) {
                productions.push_back(make_pair(nt->name, prod));
            }
        }
        for (auto* nt : nonterms) {
            symbols.push_back(nt->name);
        }
        for (auto t : terminals) {
            symbols.push_back(t);
        }

        build_first_sets(this);

//        // print symbols
//        cout << "symbols: ";
//        for (auto s : symbols) {
//            cout << s << " , ";
//        }
//        cout << endl;
//        // print nonterms
//        cout << "nonterminals: ";
//        for (auto* nt : nonterms) {
//            cout << nt->name << " -> ";
//            for (auto prod : nt->productions) {
//                for (auto s: prod) {
//                    cout << s;
//                }
//                cout << " | ";
//            }
//            cout << endl;
//        }
//        // print terminals
//        cout << "terminals: ";
//        for (auto t : terminals) {
//            cout << t << " , ";
//        }
//        cout << endl;
//        //print nonterm_offset
//        cout << "nonterm_offset: ";
//        for (auto elem : nonterm_offset) {
//            cout << elem.first->name << " = " << elem.second << " ; ";
//        }        cout << endl;
//        //print productions
//        cout << "productions: ";
//        for (auto elem : productions) {
//            cout << elem.first << " -> ";
//            for (auto prod : elem.second) {
//                if (nt_by_name.find(prod) != nt_by_name.end()) {
//                    cout << nt_by_name[prod] << "(" << nt_by_name[prod]->name << ")";
//                } else {
//                    cout << prod;
//                }
//            }
//            cout << endl;
//        }
//        cout << endl;
//
//        for (auto s : symbols) {
//            cout << s << " : ";
//            for (auto ss : first_set(s)) {
//                cout << ss << " , ";
//            }
//            cout << endl;
//        }
    }

    set<string> first_set(const string& x) {
        if (first_sets.find(x) != first_sets.end()) {
            return first_sets[x];
        } else {
            return {x};
        }
    }

    void build_first_sets(Grammar* gr) {
        for (const auto& s : symbols) {
            if (gr->nt_by_name.find(s) != gr->nt_by_name.end()) {
                gr->first_sets[s] = {};
            } else {
                gr->first_sets[s] = {s};
            }
        }
        bool repeat = true;
        while (repeat) {
            repeat = false;
            for (auto* nt : gr->nonterms) {
                auto* curfs = &gr->first_sets[nt->name];
                auto len_curfs = curfs->size();

                for (const auto& prod : gr->nt_by_name[nt->name]->productions) {
                    for (const auto& sym : prod) {
                        curfs->insert(gr->first_sets[sym].begin(), gr->first_sets[sym].end());
                        break;
                    }
                    if (curfs->size() > len_curfs) {
                        repeat = true;
                    }
                }
            }
        }
//        for (auto s : *first_sets) {
//            cout << s.first << " : ";
//            for (auto f : s.second) {
//                cout << f << " , ";
//            }
//            cout << endl;
//        }
    }

    bool is_nonterm(const string& x) {
        return (nt_by_name.find(x) != nt_by_name.end());
    }

};


#endif //EDSL_CW_GRAMMAR_H
