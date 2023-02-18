
#ifndef EDSL_CW_LALRONE_H
#define EDSL_CW_LALRONE_H

using namespace std;

#include "LrZero.h"
#include "Grammar.h"

class LrZeroTableEntry {
public:
    set<pair<int, pair<int, int>>> propogates_to;
    set<string> lookaheads;
};

class LalrOne {
public:
    list<NonTerminal*> nonterms;
    list<string> terminals;
    list<string> symbols;
    int n_states;
    Grammar* gr;

    vector<map<string, int>> goto_;
    vector<map<string, set<pair<string, int>>>> action;

    LalrOne() {}

    LalrOne(Grammar* grammar) {
        gr = grammar;

        terminals = gr->terminals;
        terminals.emplace_back("0");
        for (auto* nt : gr->nonterms) {
            if (nt != gr->nonterms.front()) {
                nonterms.push_back(nt);
            }
        }
        for (const auto& t : terminals) {
            symbols.push_back(t);
        }
        for (auto* nt : nonterms) {
            symbols.push_back(nt->name);
        }
        auto ccol = get_cannonical_collection();
//        print_ccol(ccol);
        n_states = ccol.size();
        vector<set<pair<int, int>>> ccol_core;
        for (const auto& state : ccol) {
            ccol_core.push_back(drop_itemset_lookaheads(state));
        }
//        for (const auto& state : ccol_core) {
//            cout << "set{";
//            for (const auto& rule : state) {
//                cout << "(" << rule.first << ", " << rule.second << "), ";
//            }
//            cout << "}" << endl;
//        }
//        cout << endl;
        map<set<pair<int, int>>, int> id_from_core;
        for (auto i = 0; i < ccol.size(); i++) {
            id_from_core[ccol_core[i]] = i;
        }
//        for (const auto& items : id_from_core) {
//            cout << "set{";
//            for (const auto& rule : items.first) {
//                cout << "(" << rule.first << ", " << rule.second << "), ";
//            }
//            cout << "}: " << items.second << endl;
//        }
        vector<map<string, int>> goto_precalc;
        for (auto i = 0; i < n_states; i++) {
            goto_precalc.emplace_back();
            goto_.emplace_back();
            action.emplace_back();
            for (auto* nt : nonterms) {
                goto_[i][nt->name] = -1;
            }
            for (auto t : terminals) {
                action[i] = {};
            }
        }
        for (const auto& symbol : symbols) {
            for (auto state_id = 0; state_id < n_states; state_id++) {
                auto next_state = _goto(ccol[state_id], symbol);
                if (next_state.empty()) {
                    continue;
                }
                auto next_state_id = id_from_core[drop_itemset_lookaheads(next_state)];
                goto_precalc[state_id][symbol] = next_state_id;
            }
        }
//        for (auto m : goto_precalc) {
//            cout << "{";
//            for (auto items : m) {
//                cout << items.first << ": " << items.second << ", ";
//            }
//            cout << "}: " << endl;
//        }
        for (auto state_id = 0; state_id < n_states; state_id++) {
            for (const auto& state : ccol[state_id]) {
                auto prod_index = state.first.first;
                auto dot = state.first.second;
                auto next_symbol = state.second;
                auto prod = gr->productions[prod_index];
                auto pname = prod.first;
                auto pbody = prod.second;

                if (dot < pbody.size()) {
                    auto terminal = pbody[dot];
                    if ((find(terminals.begin(), terminals.end(), terminal) == terminals.end())
                            or (goto_precalc[state_id].find(terminal) == goto_precalc[state_id].end())) {
                        continue;
                    }

                    auto next_state_id = goto_precalc[state_id][terminal];
                    action[state_id][terminal].insert(make_pair("s", next_state_id));
                } else {
                    if (prod_index == 0) {
                        action[state_id]["0"].insert(make_pair("acc", -1));
                    } else {
                        action[state_id][next_symbol].insert(make_pair("r", prod_index));
                    }
                }
            }
            for (auto* nt : nonterms) {
                auto name = nt->name;
                if (goto_precalc[state_id].find(name) == goto_precalc[state_id].end()) {
                    continue;
                }
                auto next_state_id = goto_precalc[state_id][name];
                goto_[state_id][name] = next_state_id;
            }
        }
//        print_goto(goto_);
//        print_action(action);
    }

    pair<string, int> get_next_action(int state, int term) {
        string s;
        s += term == 0 ? '0' : (char) term;
        if (action[state].find(s) == action[state].end()) {
            return make_pair("error", -1);
        }
        return *action[state][s].begin();
    }

    int get_next_goto(int state, const string& nterm) {
        if (goto_[state].find(nterm) == goto_[state].end()) {
            return -1;
        }
        return goto_[state][nterm];
    }

    void print_action(const vector<map<string, set<pair<string, int>>>>& act) {
        for (const auto& state : act) {
            cout << "{ ";
            for (const auto& rule : state) {
                cout << rule.first << " : ";
                for (const auto& r : rule.second) {
                    cout << "(" << r.first << r.second << ")";
                }
                cout << ", ";
            }
            cout << "}" << endl;
        }
        cout << endl;
    }

    void print_goto(const vector<map<string, int>>& gt) {
        for (const auto& state : gt) {
            cout << "{ ";
            for (const auto& items : state) {
                cout << items.first << ": " << items.second << ", ";
            }
            cout << "}" << endl;
        }
        cout << endl;
    }

    vector<set<pair<pair<int, int>, string>>> get_cannonical_collection() {
        auto lr_zero = LrZero(gr);
        vector<set<pair<int, int>>> kstates;
        for (const auto& state : lr_zero.states) {
            kstates.push_back(lr_zero.kernels(state));
        }
//        cout << "kstates: ";
//        for (const auto& k : kstates) {
//            cout << "set";
//            for (auto st : k) {
//                cout << "(" << st.first << ", " << st.second << ")" << ", ";
//            }
//        }
//        cout << endl;
        n_states = kstates.size();
        vector<map<pair<int, int>, LrZeroTableEntry*>> table;
        for (const auto& state : kstates) {
            map<pair<int, int>, LrZeroTableEntry*> m;
            for (auto pr : state) {
                m[pr] = new LrZeroTableEntry();
            }
            table.push_back(m);
        }
        table[0][make_pair(0, 0)]->lookaheads.insert("0");

        for (auto i_state_id = 0; i_state_id < n_states; i_state_id++) {
            vector<string> state_symbols;
            for (const auto& states : lr_zero.dfa_goto) {
                if (states.first.first == i_state_id) {
                    state_symbols.push_back(states.first.second);
                }
            }
            for (auto i_item : kstates[i_state_id]) {
                auto closure_set = closure({make_pair(i_item, "$#")});

                for (const auto& sym : state_symbols) {
                    auto j_state_id = lr_zero.dfa_goto[make_pair(i_state_id, sym)];

                    for (const auto& pr : closure_set) {
                        auto prod_index = pr.first.first;
                        auto dot = pr.first.second;
                        auto next_symbol = pr.second;
                        auto prod = gr->productions[prod_index];
                        auto pname = prod.first;
                        auto pbody = prod.second;

                        if (dot == pbody.size() or pbody[dot] != sym) {
                            continue;
                        }

                        auto j_item = make_pair(prod_index, dot + 1);
                        if (next_symbol == "$#") {
                            table[i_state_id][i_item]->propogates_to.insert(make_pair(j_state_id, j_item));
                        } else {
                            table[j_state_id][j_item]->lookaheads.insert(next_symbol);
                        }
                    }
                }
            }
        }
        bool repeat = true;
        while (repeat) {
            repeat = false;
            for (auto i_state_id = 0; i_state_id < table.size(); i_state_id++) {
                for (auto items : table[i_state_id]) {
                    auto i_item = items.first;
                    auto i_cell = items.second;
                    for (auto pr : i_cell->propogates_to) {
                        auto j_state_id = pr.first;
                        auto j_item = pr.second;
                        auto j_cell = table[j_state_id][j_item];
                        auto j_cell_lookaheads_len = j_cell->lookaheads.size();
                        j_cell->lookaheads.insert(i_cell->lookaheads.begin(), i_cell->lookaheads.end());
                        if (j_cell_lookaheads_len < j_cell->lookaheads.size()) {
                            repeat = true;
                        }
                    }
                }
            }
        }
//        print_table(table);
        vector<set<pair<pair<int, int>, string>>> result;
        for (auto i_state_id = 0; i_state_id < n_states; i_state_id++) {
            result.emplace_back();
            for (auto items : table[i_state_id]) {
                auto i_item = items.first;
                auto i_cell = items.second;
                for (const auto& sym : i_cell->lookaheads) {
                    auto item_set = make_pair(i_item, sym);
                    result[i_state_id].insert(item_set);
                }
            }
            result[i_state_id] = closure(result[i_state_id]);
        }
        return result;
    }

    void print_ccol(const vector<set<pair<pair<int, int>, string>>>& ccol) {
        for (const auto& state : ccol) {
            cout << "set{";
            for (const auto& rule : state) {
                cout << "((" << rule.first.first << ", " << rule.first.second << "), " << rule.second << "), ";
            }
            cout << "}" << endl;
        }
        cout << endl;
    }

    void print_table(const vector<map<pair<int, int>, LrZeroTableEntry*>>& table) {
        for (const auto& m : table) {
            for (auto state : m) {
                cout << "(" << state.first.first << ", " << state.first.second << "): ";
                cout << "{ propogates: ";
                for (auto lr : state.second->propogates_to) {
                    cout << "(" << lr.first << ", (" << lr.second.first << ", " << lr.second.second << ")), ";
                }
                cout << "}, lookaheads: ";
                for (const auto& lr : state.second->lookaheads) {
                    cout << lr << ", ";
                }
                cout << "} ";
            }
            cout << endl;
        }
        cout << endl;
    }

    set<pair<pair<int, int>, string>> closure(const set<pair<pair<int, int>, string>>& item_set) {
        auto result = item_set;
        vector<pair<pair<int, int>, string>> current(item_set.begin(), item_set.end());
        vector<pair<pair<int, int>, string>> new_elements;
        while(!current.empty()) {
            new_elements = {};
            for (const auto& pr : current) {
                auto prod_index = pr.first.first;
                auto dot = pr.first.second;
                auto lookahead = pr.second;
                auto prod = gr->productions[prod_index];
                auto pname = prod.first;
                auto pbody = prod.second;

                if (dot == pbody.size() or !gr->is_nonterm(pbody[dot])) {
                    continue;
                }

                auto nt_name = pbody[dot];
                auto nt = gr->nt_by_name[nt_name];
                auto nt_offset = gr->nonterm_offset[nt];
                auto following_terminals = gr->first_set(dot + 1 == pbody.size() ? lookahead : pbody[dot + 1]);

                for (auto idx = 0; idx < nt->productions.size(); idx++) {
                    for (const auto& term : following_terminals) {
                        auto new_item_set = make_pair(make_pair(nt_offset + idx, 0), term);
                        if (result.find(new_item_set) == result.end()) {
                            result.insert(new_item_set);
                            new_elements.push_back(new_item_set);
                        }
                    }
                }
            }
            current = new_elements;
        }
        return result;
    }

    set<pair<pair<int, int>, string>> _goto(const set<pair<pair<int, int>, string>>& item_set, const string& inp) {
        set<pair<pair<int, int>, string>> result;
        for (const auto& pr : item_set) {
            auto prod_id = pr.first.first;
            auto dot = pr.first.second;
            auto lookahead = pr.second;
            auto prod = gr->productions[prod_id];
            auto pname = prod.first;
            auto pbody = prod.second;

            if (dot == pbody.size() or pbody[dot] != inp) {
                continue;
            }
            auto new_item = make_pair(make_pair(prod_id, dot + 1), lookahead);
            result.insert(new_item);
        }
        result = closure(result);
        return result;
    }

//    set<pair<pair<int, int>, string>> kernels(const set<pair<pair<int, int>, string>>& item_set) {
//        set<pair<pair<int, int>, string>> result;
//        for (auto pr : item_set) {
//            if (pr.first.first == 0 or pr.first.second > 0) {
//                result.insert(pr);
//            }
//        }
//        return result;
//    }

    set<pair<int, int>> drop_itemset_lookaheads(const set<pair<pair<int, int>, string>>& item_set) {
        set<pair<int, int>> result;
        for (const auto& pr : item_set) {
            result.insert(make_pair(pr.first.first, pr.first.second));
        }
        return result;
    }

};

#endif //EDSL_CW_LALRONE_H
