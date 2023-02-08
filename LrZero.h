#include <set>
#include <map>
#include "list"

#ifndef EDSL_CW_LR_ZERO_H
#define EDSL_CW_LR_ZERO_H

using namespace std;

#include "Grammar.h"


class LrZero {
public:
    vector<set<pair<int, int>>> states;
    map<set<pair<int, int>>, int> id_from_state;
    map<pair<int, string>, int> dfa_goto;
    Grammar* gr;

    LrZero(Grammar* grammar) {
        gr = grammar;
        states = {closure({{make_pair(0, 0)}})};

        auto next_id = 0;
        id_from_state[states.back()] = next_id;
        next_id += 1;
        set<set<pair<int, int>>> seen(states.begin(), states.end());
        auto set_queue = states;
        vector<set<pair<int, int>>> new_elements;
        while (!set_queue.empty()) {
            new_elements = {};
            for (const auto& item_set : set_queue) {
                auto item_set_id = id_from_state[item_set];

                for (const auto& symbol : gr->symbols) {
                    auto next_item_set = _goto(item_set, symbol);
                    if (next_item_set.empty()) {
                        continue;
                    }

                    if (seen.find(next_item_set) == seen.end()) {
                        new_elements.push_back(next_item_set);
                        seen.insert(next_item_set);

                        states.push_back(next_item_set);
                        id_from_state[states.back()] = next_id;
                        next_id += 1;
                    }
                    dfa_goto[make_pair(item_set_id, symbol)] = id_from_state[next_item_set];
                }
            }
            set_queue = new_elements;
        }
//        cout << "states: ";
//        for (const auto& elem : states) {
//            cout << "set";
//            for (auto pr : elem) {
//                cout << "(" << pr.first << ", " << pr.second << ")" << ", ";
//            }
//        }
//        cout << endl;
//        cout << "id_from_state: ";
//        for (const auto& state : id_from_state) {
//            cout << "set";
//            for (auto pr : state.first) {
//                cout << "(" << pr.first << ", " << pr.second << ")" << ", ";
//            }
//            cout << ": ";
//            cout << state.second << " ; ";
//        }
//        cout << endl;
//        cout << "goto: ";
//        for (const auto& rule : dfa_goto) {
//            cout << "(" << rule.first.first << ", " << rule.first.second << ")" << ", ";
//            cout << ": ";
//            cout << rule.second << " ; ";
//        }
//        vector<set<pair<int, int>>> kstates;
//        for (const auto& state : states) {
//            kstates.push_back(kernels(state));
//        }
//        cout << endl;
//        cout << "kstates: ";
//        for (const auto& k : kstates) {
//            cout << "set";
//            for (auto st : k) {
//                cout << "(" << st.first << ", " << st.second << ")" << ", ";
//            }
//        }
    }

    set<pair<int, int>> closure(const set<pair<int, int>>& item_set) {
        auto result = item_set;
        vector<pair<int, int>> set_queue(item_set.begin(), item_set.end());
        vector<pair<int, int>> new_elements;
        while (!set_queue.empty()) {
            new_elements = {};
            for (auto pr : set_queue) {
                auto item_prod_id = pr.first;
                auto dot = pr.second;
                auto prod = gr->productions[item_prod_id];
                auto pname = prod.first;
                auto pbody = prod.second;

                if (dot == pbody.size() or !gr->is_nonterm(pbody[dot])) {
                    continue;
                }

                auto nt_name = pbody[dot];
                auto nt = gr->nt_by_name[nt_name];
                auto nt_offset = gr->nonterm_offset[nt];

                for (auto idx = 0; idx < nt->productions.size(); idx++) {
                    auto new_item_set = make_pair(nt_offset + idx, 0);
                    if (result.find(new_item_set) == result.end()) {
                        new_elements.push_back(new_item_set);
                        result.insert(new_item_set);
                    }
                }
            }
            set_queue = new_elements;
        }
        return result;
    }

    set<pair<int, int>> _goto(const set<pair<int, int>>& item_set, const string& inp) {
        set<pair<int, int>> result;

        for (auto pr : item_set) {
            auto prod_index = pr.first;
            auto dot = pr.second;
            auto prod = gr->productions[prod_index];
            auto pname = prod.first;
            auto pbody = prod.second;

            if (dot < pbody.size() and pbody[dot] == inp) {
                result.insert(make_pair(prod_index, dot + 1));
            }
        }
        result = closure(result);
        return result;
    }

    set<pair<int, int>> kernels(const set<pair<int, int>>& item_set) {
        set<pair<int, int>> result;
        for (auto pr : item_set) {
            if (pr.first == 0 or pr.second > 0) {
                result.insert(pr);
            }
        }
        return result;
    }
};


#endif //EDSL_CW_LR_ZERO_H
