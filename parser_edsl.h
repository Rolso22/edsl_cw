#ifndef EDSL_CW_PARSER_EDSL_H
#define EDSL_CW_PARSER_EDSL_H

#include <iostream>
#include <utility>
#include <vector>
#include <stack>
#include "Grammar.h"
#include "LalrOne.h"
#include "attr_stack.h"

namespace parser_edsl {
    template<typename TokType, typename Fragment> struct Token;
    struct Rule;
    template<typename TokType> struct Symbol;
    template<typename TokType, typename U> struct NTerm;
    template<typename TokType, template<TokType> typename MyToken, TokType t> struct Terminal;
    template<typename TokType, typename Fragment, typename U> struct AttrToken;

    template <typename TokType, typename Fragment>
    struct Lexer {

        virtual Token<TokType, Fragment>* next_token() {
            std::cout << "next token" << std::endl;
        }

        virtual ~Lexer() {}
    };

    struct Rule {};

    struct Nil {};

    template <typename H, typename T>
    struct Cons {
        typedef H Head;
        typedef T Tail;
    };

    template <typename F, typename Attrs, typename Ret>
    struct ActionRet;

    template <typename F, typename Ret>
    struct ActionRet<F, Nil, Ret> : public ActionBase {
        F action;

        ActionRet(F f): action(f) {}

        void apply(AttrStack &stack) override {
            Ret r = action();
            stack.push(r);
        }
    };

    template <typename F, typename A1, typename Ret>
    struct ActionRet<F, Cons<A1, Nil>, Ret> : public ActionBase {
        F action;

        ActionRet(F f): action(f) {}

        void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            Ret r = action(a1);
            stack.push(r);
        }
    };

    template <typename F, typename A1, typename A2, typename Ret>
    struct ActionRet<F, Cons<A1, Cons<A2, Nil>>, Ret> : public ActionBase {
        F action;

        ActionRet(F f): action(f) {}

        void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            A2 a2 = stack.pop<A2>();
            Ret r = action(a2, a1);
            stack.push(r);
        }
    };

    template <typename F, typename A1, typename A2, typename A3, typename Ret>
    struct ActionRet<F, Cons<A1, Cons<A2, Cons<A3, Nil>>>, Ret> : public ActionBase {
        F action;

        ActionRet(F f): action(f) {}

        virtual void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            A2 a2 = stack.pop<A2>();
            A3 a3 = stack.pop<A3>();
            Ret r = action(a3, a2, a1);
            stack.push(r);
        }
    };

    template <typename F, typename Attrs>
    struct ActionNoRet;

    template <typename F>
    struct ActionNoRet<F, Nil> : public ActionBase {
        F action;

        ActionNoRet(F f): action(f) {}

        void apply(AttrStack &stack) override {
            action();
        }
    };

    template <typename F, typename A1>
    struct ActionNoRet<F, Cons<A1, Nil>> : public ActionBase {
        F action;

        ActionNoRet(F f): action(f) {}

        void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            action(a1);
        }
    };

    template <typename F, typename A1, typename A2>
    struct ActionNoRet<F, Cons<A1, Cons<A2, Nil>>> : public ActionBase {
        F action;

        ActionNoRet(F f): action(f) {}

        void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            A2 a2 = stack.pop<A2>();
            action(a2, a1);
        }
    };

    template <typename F, typename A1, typename A2, typename A3>
    struct ActionNoRet<F, Cons<A1, Cons<A2, Cons<A3, Nil>>>> : public ActionBase {
        F action;

        ActionNoRet(F f): action(f) {}

        void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            A2 a2 = stack.pop<A2>();
            A3 a3 = stack.pop<A3>();
            action(a3, a2, a1);
        }
    };

    template <typename TokType, typename Attrs>
    struct Chain {
        std::vector<Symbol<TokType>> symbols;

        Chain() {}
        Chain(std::vector<Symbol<TokType>> symbols)
            : symbols(symbols)
        {}
    };

    template <typename TokType>
    struct RuleAction {
        std::vector<Symbol<TokType>> right;
        ActionBase *action;

        RuleAction(std::vector<Symbol<TokType>> right, ActionBase *action)
            : right(right), action(action)
        {}
    };

    template <typename TokType, typename Attrs, typename Action>
    struct ChainAction {
        Chain<TokType, Attrs> chain;
        Action action;

        ChainAction(Chain<TokType, Attrs> chain, Action action): chain(chain), action(action) {}
    };

    template <typename TokType, typename Attr>
    auto chain_action_for_chain(Chain<TokType, Cons<Attr, Nil>> chain) {
        return ChainAction(chain, [](Attr x) { return x; });
    }

    template <typename TokType>
    auto chain_action_for_chain(Chain<TokType, Nil> chain) {
        return ChainAction(chain, []() {});
    }

    template <typename T> struct Type2Type {};

    template <typename T, typename Attrs>
    struct PushTokenAttr {
        static T* make_object();

        template <typename TokType, typename Fragment, typename Attr>
        static Cons<Attr, Attrs>
        push_token_attr(AttrToken<TokType, Fragment, Attr> *);

        static Attrs push_token_attr(...);

        typedef decltype(push_token_attr(make_object())) Result;
    };

    template <typename TokType, typename U>
    constexpr auto operator<< (Rule, NTerm<TokType, U> &nterm) {
        return Chain<TokType, Nil>() << nterm;
    }

    template <typename TokType, template<TokType> typename Token, TokType t>
    constexpr auto operator<< (Rule, Terminal<TokType, Token, t> term) {
        return Chain<TokType, Nil>() << term;
    }

    template <typename TokType, typename Attrs, typename Attr>
    Chain<TokType, Cons<Attr, Attrs>>
    operator<< (Chain<TokType, Attrs> chain, NTerm<TokType, Attr> &nterm) {
        Chain<TokType, Cons<Attr, Attrs>> nc(chain.symbols);
        nc.symbols.push_back(Symbol(nterm));
        return nc;
    }

    template <typename TokType, typename Attrs>
    Chain<TokType, Attrs>
    operator<< (Chain<TokType, Attrs> chain, NTerm<TokType, void> &nterm) {
        chain.symbols.push_back(Symbol(nterm));
        return chain;
    }

    template <typename TokType, typename Attrs, template<TokType> typename Token, TokType t>
    Chain<TokType, typename PushTokenAttr<Token<t>, Attrs>::Result>
    operator<< (Chain<TokType, Attrs> chain, Terminal<TokType, Token, t> term) {
        chain.symbols.push_back(Symbol(term));
        Chain<TokType, typename PushTokenAttr<Token<t>, Attrs>::Result> nc(chain.symbols);
        return nc;
    }

    template <typename TokType, typename Attrs, typename Action>
    ChainAction<TokType, Attrs, Action>
    operator<< (Chain<TokType, Attrs> chain, Action action) {
        return ChainAction<TokType, Attrs, Action>(chain, action);
    }


    template <typename TokType>
    class NTermBase {
    protected:
        std::vector<RuleAction<TokType>> rule_actions;
        std::string name;

        NTermBase(std::string name) : name(std::move(name)) {}
    public:
        std::string get_name() {
            return name;
        }

        void create_rules(std::vector<std::pair<std::string, vector<std::pair<std::string, ActionBase*>>>>& str_rules, std::vector<std::string>& visited) {
            visited.push_back(this->name);
            std::cout << "create(" << this->name << ")\n";
            vector<std::pair<std::string, ActionBase*>> action_prods;
            for (auto rule : this->rule_actions) {
                std::string str_rule;
                for (auto s : rule.right) {
                    if (s.is_terminal) {
                        str_rule += (char) s.sym.term;
                        str_rule += " ";
                    } else {
                        str_rule += s.sym.nterm->name;
                        str_rule += " ";
                        if (std::find(visited.begin(), visited.end(), s.sym.nterm->name) == visited.end()) {
                            s.sym.nterm->create_rules(str_rules, visited);
                        }
                    }
                }
                str_rule = str_rule.substr(0, str_rule.length() - 1);
                action_prods.push_back(std::make_pair(str_rule, rule.action));
            }
            str_rules.push_back(std::make_pair(this->name, action_prods));
        }
    };

    template <typename TokType>
    struct Symbol {
        bool is_terminal;
        union {
            TokType term;
            NTermBase<TokType> *nterm;
        } sym;

        template <template<TokType> typename Token, TokType U>
        Symbol(Terminal<TokType, Token, U> &): is_terminal(true)
        {
            sym.term = U;
        }

        Symbol(NTermBase<TokType> &nterm): is_terminal(false)
        {
            sym.nterm = &nterm;
        }
    };

    template<typename TokType, typename Fragment>
    struct Token {
        Fragment pos;
        TokType type;

        Token(TokType t, Fragment position) {
            pos = position;
            type = t;
        }

        virtual void get_attr(AttrStack& stack) {
//            std::cout << "token push nothing" << std::endl;
        }
    };

    template<typename TokType, typename Fragment, typename U>
    struct AttrToken  : public Token<TokType, Fragment> {
        Fragment pos;
        TokType type;
        U value;

        AttrToken(TokType t, Fragment position, U value) : Token<TokType, Fragment>(t, position) {
            this->value = value;
            type = t;
            pos = position;
        }

        void get_attr(AttrStack& stack) override {
//            std::cout << "attr_token push value" << std::endl;
            stack.template push(value);
        }
    };

    template<typename TokType, template<TokType> typename Token, TokType t>
    struct Terminal {
        TokType T = t;
        Terminal() {
//            std::cout << "token type: " << token.type << std::endl;
//            std::cout << "term t: " << static_cast<char>(T) << std::endl;
        }
    };

    template<typename TokType, typename U>
    struct NTerm: public NTermBase<TokType> {
        std::vector<std::pair<std::string, vector<std::pair<std::string, ActionBase*>>>> rules;
        LalrOne lalr_one;
        std::vector<std::tuple<std::string, int, ActionBase*>> all_action_rules;

        NTerm() : NTermBase<TokType>() {}
        NTerm(std::string name) : NTermBase<TokType>(name)
        {}

        template <typename Attrs, typename Action>
        NTerm<TokType, U>& operator| (ChainAction<TokType, Attrs, Action> ca) {
            this->rule_actions.push_back(RuleAction<TokType>(ca.chain.symbols, make_action<Attrs>(ca.action, Type2Type<U>())));
//            for (auto rule : this->rule_actions) {
//                std::cout << "rule/op|(" << this->name << ", " << this << ") ";
//                for (auto s : rule.right) {
//                    if (s.is_terminal) {
//                        std::cout << "term: " << s.sym.term << ", ";
//                    } else {
//                        std::cout << "nterm: " << s.sym.nterm << "/" << s.sym.nterm->get_name() << ", ";
//                    }
//                }
//                std::cout << std::endl;
//            }
            return *this;
        }

        template <typename Attrs>
        NTerm<TokType, U>& operator| (Chain<TokType, Attrs> chain) {
            return *this | chain_action_for_chain(chain);
        }

        NTerm<TokType, U>& operator| (Rule) {
            return *this | Chain<TokType, Nil>();
        }

        template <typename Attrs, typename Action, typename Ret>
        ActionBase *make_action(Action action, Type2Type<Ret>) {
            return new ActionRet<Action, Attrs, Ret>(action);
        }

        template <typename Attrs, typename Action>
        ActionBase *make_action(Action action, Type2Type<void>) {
            return new ActionNoRet<Action, Attrs>(action);
        }

        void compile_table() {
            std::cout << "compile tables" << std::endl;
            std::vector<std::string> visited;
            this->create_rules(rules, visited);
            for (auto rule : this->rules) {
                std::cout << "rule(" << rule.first << "): ";
                for (auto prod : rule.second) {
                    std::cout << prod.first << ", ";
                }
                std::cout << std::endl;
            }
            std::list<NonTerminal*> nt_list;
            for (auto rule : this->rules) {
                nt_list.push_back(new NonTerminal(rule.first, rule.second));
            }
            auto *gr = new Grammar(nt_list, this->name);
            this->all_action_rules = gr->action_productions;
            lalr_one = LalrOne(gr);
        }

        template<typename Fragment>
        void parse(Lexer<TokType, Fragment>& lexer) {
            auto* stack = new AttrStack();
            std::stack<int> state_stack;
            state_stack.push(0);
            auto next_token = lexer.next_token();
            while (true) {
                auto next_action = lalr_one.get_next_action(state_stack.top(), next_token->type);
                if (next_action.first == "s") {
                    next_token->get_attr(*stack);
                    state_stack.push(next_action.second);
                    next_token = lexer.next_token();
                } else if (next_action.first == "r") {
                    const auto [nterm, prod_size, action] = all_action_rules[next_action.second];
                    action->apply(*stack);
                    for (auto i = 0; i < prod_size; i++) {
                        state_stack.pop();
                    }
                    auto next_state = lalr_one.get_next_goto(state_stack.top(), nterm);
                    if (next_state == -1) {
                        std::cout << "\nerror reduce: (" << next_token->pos.begin.line << ", " << next_token->pos.begin.col << ") - " <<
                                  "(" << next_token->pos.end.line << ", " << next_token->pos.end.col << ")" << std::endl;
                        break;
                    }
                    state_stack.push(next_state);
                } else if (next_action.first == "acc") {
                    std::cout << "\nparsing is done: accept" << std::endl;
                    break;
                } else {
                    std::cout << "\nerror shift: (" << next_token->pos.begin.line << ", " << next_token->pos.begin.col << ") - " <<
                              "(" << next_token->pos.end.line << ", " << next_token->pos.end.col << ")" << std::endl;
                    break;
                }
            }
        }

        void print_rules() {
            std::cout << "print_rules(" << this->get_name() << ")\n";
            for (auto rule : this->rule_actions) {
                std::cout << "rule: ";
                for (auto s : rule.right) {
                    if (s.is_terminal) {
                        std::cout << "term: " << s.sym.term << "(" << (char) s.sym.term << "), ";
                    } else {
                        std::cout << "nterm: " << s.sym.nterm->get_name() << ", ";
                    }
                }
                std::cout << std::endl;
            }
        }
    };
}

#endif //EDSL_CW_PARSER_EDSL_H
