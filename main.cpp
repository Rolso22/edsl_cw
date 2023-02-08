#include <map>
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>

#include "Grammar.h"
#include "LalrOne.h"

#include "parser_edsl.h"

enum TokType {
    END_OF_TEXT = 0,
    PLUS = '+', MINUS = '-', MUL = '*', DIV = '/', SET = '=',
    SEMICOLON = ';', COMMA = ',', LP = '(', RP = ')',
    PRINT = 'p', READ = 'r', VARNAME = 'v', NUMBER = 'n', STRING = 's'
};

namespace pe = parser_edsl;

struct Pos {
    int line;
    int col;
};

struct Fragment {
    Pos begin;
    Pos end;
};

template <TokType t>
struct Token : public pe::Token<TokType, Fragment>
{
    Token() : pe::Token<TokType, Fragment>(t) {}
    Token(Fragment pos) : pe::Token<TokType, Fragment>(t, pos)
    {
        /* пусто */
    }
};

template <>
struct Token<VARNAME> : public pe::AttrToken<TokType, Fragment, std::string>
{
    Token() : pe::AttrToken<TokType, Fragment, std::string>(VARNAME) {}
    Token(Fragment pos, const std::string& name) : pe::AttrToken<TokType, Fragment, std::string>(VARNAME, pos, name)
    {
        /* пусто */
    }
};

template <>
struct Token<NUMBER> : public pe::AttrToken<TokType, Fragment, double>
{
    Token() : pe::AttrToken<TokType, Fragment, double>(NUMBER) {}
    Token(Fragment pos, double value) : pe::AttrToken<TokType, Fragment, double>(NUMBER, pos, value)
    {
        /* пусто */
    }
};

template <>
struct Token<STRING> : public pe::AttrToken<TokType, Fragment, std::string>
{
    Token() : pe::AttrToken<TokType, Fragment, std::string>(STRING) {}
    Token(Fragment pos, const std::string& text) : pe::AttrToken<TokType, Fragment, std::string>(STRING, pos, text)
    {
        /* пусто */
    }
};

template <TokType t>
using Term = pe::Terminal<TokType, Token, t>;




//// Program
//extern pe::NTerm<TokType, void> Operator;
//
//pe::NTerm<TokType, void> Program =
//        pe::Rule() << Operator
//        | pe::Rule() << Program << Term<SEMICOLON>() << Operator
//;
//
//
//// Operator
//extern pe::NTerm<TokType, void> InputOperator;
//extern pe::NTerm<TokType, void> PrintOperator;
//extern pe::NTerm<TokType, void> AssignOperator;
//
//pe::NTerm<TokType, void> Operator =
//        pe::Rule() << InputOperator
//        | pe::Rule() << PrintOperator
//        | pe::Rule() << AssignOperator
//;
//
//
//// InputOperator
//extern pe::NTerm<TokType, double*> Variable;
//
//pe::NTerm<TokType, void> InputOperator =
//        pe::Rule() << Term<READ>() << Variable << [](double *variable) -> void { scanf("%lf", variable); }
//        | pe::Rule() << InputOperator << Term<COMMA>() << Variable << [](double *variable) -> void { scanf("%lf", variable); }
//;
//
//
//// PrintOperator
//extern pe::NTerm<TokType, double> Expression;
//
//pe::NTerm<TokType, void> PrintOperator =
//        pe::Rule() << Term<PRINT>() << Expression <<
//                   [] (double value) -> void {
//                       printf("%f", value);
//                   }
//        | pe::Rule() << Term<PRINT>() << Term<STRING>() <<
//                     [] (std::string str) -> void {
//                         printf("%s", str.c_str());
//                     }
//        | pe::Rule() << PrintOperator << Term<COMMA>() << Expression <<
//                     [] (double value) -> void {
//                         printf(" %f", value);
//                     }
//        | pe::Rule() << PrintOperator << Term<COMMA>() << Term<STRING>() <<
//                     [] (std::string str) -> void {
//                         printf(" %s", str.c_str());
//                     }
//;
//
//
//pe::NTerm<TokType, void> AssignOperator =
//        pe::Rule() << Variable << Term<SET>() << Expression <<
//                   [](double *variable, double value) -> void {
//                       *variable = value;
//                   }
//;
//
//
//// Variable
//std::map<std::string, double> variables;
//
//pe::NTerm<TokType, double*> Variable =
//        pe::Rule() << Term<VARNAME>() <<
//                   [](std::string name)-> double* {
//                       return &variables[name];
//                   }
//;
//
//
//// Expression
//extern pe::NTerm<TokType, double> ExprTerm;  // Слагаемое
//
//pe::NTerm<TokType, double> Expression =
//        pe::Rule() << ExprTerm
//        | pe::Rule() << Expression << Term<PLUS>() << ExprTerm <<
//                     [](double x, double y) -> double { return x + y; }
//        | pe::Rule() << Expression << Term<MINUS>() << ExprTerm <<
//                     [](double x, double y) -> double { return x - y; }
//;
//
//
//// ExprTerm
//extern pe::NTerm<TokType, double> Factor;
//
//pe::NTerm<TokType, double> ExprTerm =
//        pe::Rule() << Factor
//        | pe::Rule() << ExprTerm << Term<MUL>() << Factor <<
//                     [](double x, double y) -> double { return x * y; }
//        | pe::Rule() << ExprTerm << Term<DIV>() << Factor <<
//                     [](double x, double y) -> double { return x / y; }
//;
//
//
//// Factor
//pe::NTerm<TokType, double> Factor =
//        pe::Rule() << Term<NUMBER>()
//        | pe::Rule() << Variable << [](double *variable) -> double { return *variable; }
//        | pe::Rule() << Term<LP>() << Expression << Term<RP>()
//;

pe::NTerm<TokType, void> F =
        pe::Rule() << Term<PLUS>() << []() -> void {std::cout << "hello" << "\n";}
        | pe::Rule() <<
;



// Лексический анализатор
struct MyLexer {
    Fragment fr = Fragment({{0, 0}, {0, 0}});
    std::fstream file;

    MyLexer() {
        file.open("/home/ramagf/CLionProjects/edsl_cw/input.txt",ios::in);
        if (file.is_open()) {
            std::cout << "open" << std::endl;
        }
    }

    pe::Token<TokType, Fragment>* get_type(const std::string& t) const {
        std::string delimiter = " ";
        std::string token = t.substr(0, t.find(delimiter));
        if (token == "PLUS") {
            return new Token<PLUS>(fr);
        }
        if (token == "MINUS") {
            return new Token<MINUS>(fr);
        }
        if (token == "MUL") {
            return new Token<MUL>(fr);
        }
        if (token == "DIV") {
            return new Token<DIV>(fr);
        }
        if (token == "SET") {
            return new Token<SET>(fr);
        }
        if (token == "SEMICOLON") {
            return new Token<SEMICOLON>(fr);
        }
        if (token == "COMMA") {
            return new Token<COMMA>(fr);
        }
        if (token == "LP") {
            return new Token<LP>(fr);
        }
        if (token == "RP") {
            return new Token<RP>(fr);
        }
        if (token == "PRINT") {
            return new Token<PRINT>(fr);
        }
        if (token == "READ") {
            return new Token<READ>(fr);
        }
        if (token == "NUMBER") {
            std::string value = t.substr(t.find(delimiter));
            return new Token<NUMBER>(fr, std::stod(value));
        }
        if (token == "VARNAME") {
            std::string value = t.substr(t.find(delimiter));
            return new Token<VARNAME>(fr, value);
        }
        if (token == "STRING") {
            std::string value = t.substr(t.find(delimiter));
            return new Token<STRING>(fr, value);
        }
        return new Token<END_OF_TEXT>(fr);
    }

    pe::Token<TokType, Fragment>* next_token() {
        if (file.is_open()) {
            string tp;
            getline(file, tp);
            if (tp.empty()) {
                return new Token<END_OF_TEXT>(fr);
            }
            pe::Token<TokType, Fragment>* t = get_type(tp);
//            std::cout << "type_: " << t->type << std::endl;
            return t;
        }
        return new Token<END_OF_TEXT>(fr);
    }
};

// Основная программа
int main(int argc, char *argv[]) {
//    if (argc > 1) {
//        MyLexer lexer(argv[1]);
//        Program.compile_tables();
//        Program.parse(lexer);
//    }
    auto fr = Fragment({{0, 0}, {0, 0}});

    auto tt = Term<NUMBER>();
    auto t = Token<PLUS>(fr);
    auto vt = Token<VARNAME>(fr, "x");
    auto nt = Token<NUMBER>(fr, 5.0);
    auto st = Token<STRING>(fr, "text");
    std::cout << static_cast<char>(t.type) << std::endl;
    std::cout << static_cast<char>(vt.type) << std::endl;
    std::cout << static_cast<char>(nt.type) << std::endl;
    std::cout << static_cast<char>(st.type) << std::endl;
//    auto *gr = new Grammar({new NonTerminal("P", {"P ';' O", "O"}),
//                               new NonTerminal("O", {"AO"}),
//                               new NonTerminal("AO", {"V '=' ET"}),
//                               new NonTerminal("V", {"'vn'"}),
//                               new NonTerminal("ET", {"F", "ET '*' F"}),
//                               new NonTerminal("F", {"'n'", "V"})});
//    auto lalr_one = LalrOne(gr);
    std::cout << std::endl;
    auto mylexer = MyLexer();
//    mylexer.next_token();
//    mylexer.next_token();
//    mylexer.next_token();
    auto tp = mylexer.next_token();
    std::cout << "type: " << tp->type << std::endl;
    if (tp->type == NUMBER) {
        std::cout << static_cast<pe::AttrToken<TokType, Fragment, double>*>(tp)->value << std::endl;
    }
    std::cout << "type: " << mylexer.next_token()->type << std::endl;
    std::cout << "type: " << mylexer.next_token()->type << std::endl;

    return 0;
}

// Синтаксис:
//   NTerm<Тип токена, тип атрибута> Нетерминал =
//     Альтернатива | Альтернатива | ...;
//   Тип токена — перечисление, int, char, определяющий тип токена.
//   Тип атрибута — тип атрибута нетерминала (число или указатель
//   на дерево разбора и т.д.). Тип может быть пустым (void).
//   Нетерминал — имя нетерминала, глобальная переменная.
//
//   Альтернатива:
//     Rule() << элемент << элемент... [<< функция обработки]
//
//   Элемент:
//     Нетерминал
//     Терминал — экземпляр класса parser_edsl::Terminal
//
//   Функция обработки должна возвращать значение типа атрибута
//   нетерминала и принимать значения атрибутов, представленных в данной
//   альтернативе (как нетерминалами с непустым типом атрибута, так
//   и терминалами с атрибутами).
//   Функция обработки может отсутствовать, если
//   1. Нетерминал имеет пустой (void) атрибут и элементы в альтернативе
//   атрибутов не несут.
//   2. Нетерминал имеет непустой атрибут и только один элемент несёт
//   атрибут, причём типы атрибутов должны совпадать. Атрибут нетерминала
//   получает значение атрибута элемента.
//
//   Примечание: при вычислении выражения Rule() << элемент тип токена
//   должен определяться из элемента: это может быть нетерминал,
//   терминал с атрибутом или выражение имеющее тип «тип токена».