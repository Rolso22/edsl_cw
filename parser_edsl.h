#ifndef EDSL_CW_PARSER_EDSL_H
#define EDSL_CW_PARSER_EDSL_H

#include <iostream>
#include <utility>
#include <vector>
#include <functional>
#include "Grammar.h"
#include "LalrOne.h"

namespace parser_edsl {
    template<typename TokType, typename Fragment> struct Token;
    struct Rule;
    template<typename TokType> struct Symbol;
    template<typename TokType, typename U> struct NTerm;
    template<typename TokType, template<TokType> typename MyToken, TokType t> struct Terminal;
    template<typename TokType, typename Fragment, typename U> struct AttrToken;

    struct Rule {};

    struct Nil {};

    template <typename H, typename T>
    struct Cons {
        typedef H Head;
        typedef T Tail;
    };

    struct AttrStackItemBase {
        AttrStackItemBase *next;

        AttrStackItemBase(AttrStackItemBase *next): next(next) {}

        virtual ~AttrStackItemBase() {}
    };

    template <typename T>
    struct AttrStackItem : public AttrStackItemBase {
        T value;

        AttrStackItem(T value, AttrStackItemBase *next)
            : AttrStackItemBase(next)
            , value(value)
        {}
    };

    class AttrStack {
        AttrStackItemBase *stack;
    public:
        AttrStack(): stack(nullptr) {}

        template <typename T>
        void push(T value) {
            stack = new AttrStackItem<T>(value, stack);
        }

        template <typename T>
        T pop() {
            auto top = dynamic_cast<AttrStackItem<T>*>(stack);
            stack = stack->next;
            T value = top->value;
            delete top;
            return value;
        }
    };

    struct ActionBase {
        virtual ~ActionBase() {}

        virtual void apply(AttrStack &stack) = 0;
    };

    template <typename F, typename Attrs, typename Ret>
    struct ActionRet;

    template <typename F, typename Ret>
    struct ActionRet<F, Nil, Ret> : public ActionBase {
        F action;

        ActionRet(F f): action(f) {}

        virtual void apply(AttrStack &stack) override {
            Ret r = action();
            stack.push(r);
        }
    };

    template <typename F, typename A1, typename Ret>
    struct ActionRet<F, Cons<A1, Nil>, Ret> : public ActionBase {
        F action;

        ActionRet(F f): action(f) {}

        virtual void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            Ret r = action(a1);
            stack.push(r);
        }
    };

    template <typename F, typename A1, typename A2, typename Ret>
    struct ActionRet<F, Cons<A1, Cons<A2, Nil>>, Ret> : public ActionBase {
        F action;

        ActionRet(F f): action(f) {}

        virtual void apply(AttrStack &stack) override {
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

        virtual void apply(AttrStack &stack) override {
            action();
        }
    };

    template <typename F, typename A1>
    struct ActionNoRet<F, Cons<A1, Nil>> : public ActionBase {
        F action;

        ActionNoRet(F f): action(f) {}

        virtual void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            action(a1);
        }
    };

    template <typename F, typename A1, typename A2>
    struct ActionNoRet<F, Cons<A1, Cons<A2, Nil>>> : public ActionBase {
        F action;

        ActionNoRet(F f): action(f) {}

        virtual void apply(AttrStack &stack) override {
            A1 a1 = stack.pop<A1>();
            A2 a2 = stack.pop<A2>();
            action(a2, a1);
        }
    };

    template <typename F, typename A1, typename A2, typename A3>
    struct ActionNoRet<F, Cons<A1, Cons<A2, Cons<A3, Nil>>>> : public ActionBase {
        F action;

        ActionNoRet(F f): action(f) {}

        virtual void apply(AttrStack &stack) override {
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
        return ChainAction(chain, []() { std::cout << "return Nil" << std::endl; });
    }

    template <typename T> struct Type2Type {};

//    template <typename TokType, typename Attrs, typename Action, typename Next>
//    struct DelayedRuleStack {
//        Chain<TokType, Attrs> chain;
//        Action action;
//        Next next;
//
//        DelayedRuleStack(Chain<TokType, Attrs> chain, Action action, Next next)
//            : chain(chain), action(action), next(next)
//        {}
//
//        DelayedRuleStack(ChainAction<TokType, Attrs, Action> ca, Next next)
//            : chain(ca.chain), action(ca.action), next(next)
//        {}
//
//        template <typename Ret>
//        void add_rule_action(std::vector<RuleAction<TokType>> &rule_action, Type2Type<Ret> t2t) {
//            rule_action.push_back(RuleAction<TokType>(chain.symbols, new ActionRet<Action, Attrs, Ret>(action)));
//            next.add_rule_action(rule_action, t2t);
//        }
//
//        void add_rule_action(std::vector<RuleAction<TokType>> &rule_action, Type2Type<void> t2t) {
//            rule_action.push_back(RuleAction<TokType>(chain.symbols, new ActionNoRet<Action, Attrs>(action)));
//            next.add_rule_action(rule_action, t2t);
//        }
//    };
//
//    struct EmptyStack {
//        template <typename TokType, typename T2T>
//        void add_rule_action(std::vector<RuleAction<TokType>> &, T2T) {}
//    };

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

        NTermBase() {}
        NTermBase(std::string name) : name(std::move(name)) {}
    public:
        std::string get_name() {
            return name;
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

        Token() {}
        Token(TokType t) {
            type = t;
        }
        Token(TokType t, Fragment position) {
            pos = position;
            type = t;
//            std::cout << "sizeof(TokType): " << sizeof(TokType) << std::endl;
//            std::cout << "token: " << static_cast<char>(type) << " -> " << type << std::endl;
//        std::cout << std::is_enum<TokType>() << std::endl;
        }
    };

    template<typename TokType, typename Fragment, typename U>
    struct AttrToken  : public Token<TokType, Fragment> {
        Fragment pos;
        TokType type;
        U value = 0;

        AttrToken(TokType t) {
            type = t;
        }
        AttrToken(TokType t, Fragment position, U value) : Token<TokType, Fragment>(t, position) {
            this->value = value;
            type = t;
            pos = position;
//            std::cout << "attr: " << static_cast<char>(type) << " -> " << type << std::endl;
//            std::cout << "value: " << value << std::endl;
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
        std::vector<pair<std::string, vector<std::string>>> rules;

        NTerm() : NTermBase<TokType>() {}
        NTerm(std::string name) : NTermBase<TokType>(name)
        {}

        template <typename Attrs, typename Action>
        NTerm<TokType, U>& operator| (ChainAction<TokType, Attrs, Action> ca) {
            this->rule_actions.push_back(RuleAction<TokType>(ca.chain.symbols, make_action<Attrs>(ca.action, Type2Type<U>())));
            for (auto rule : this->rule_actions) {
                std::cout << "rule/op|(" << this->name << ", " << this << ") ";
                for (auto s : rule.right) {
                    if (s.is_terminal) {
                        std::cout << "term: " << s.sym.term << ", ";
                    } else {
                        std::cout << "nterm: " << s.sym.nterm << "/" << s.sym.nterm->get_name() << ", ";
                    }
                }
                std::cout << std::endl;
            }
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

        void create_rules(std::vector<pair<std::string, vector<std::string>>>& str_rules, std::vector<std::string> visited) {

        }

        void compile_table() {
            std::cout << "compile tables";

        }

        auto get_rules() {
            return this->rule_actions;
        };

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
