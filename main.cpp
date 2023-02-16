#include <map>
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>

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
    Token(Fragment pos) : pe::Token<TokType, Fragment>(t, pos) {}
};

template <>
struct Token<VARNAME> : public pe::AttrToken<TokType, Fragment, std::string>
{
    Token(Fragment pos, const std::string& name) : pe::AttrToken<TokType, Fragment, std::string>(VARNAME, pos, name) {}
};

template <>
struct Token<NUMBER> : public pe::AttrToken<TokType, Fragment, double>
{
    Token(Fragment pos, double value) : pe::AttrToken<TokType, Fragment, double>(NUMBER, pos, value) {}
};

template <>
struct Token<STRING> : public pe::AttrToken<TokType, Fragment, std::string>
{
    Token(Fragment pos, const std::string& text) : pe::AttrToken<TokType, Fragment, std::string>(STRING, pos, text) {}
};

template <TokType t>
using Term = pe::Terminal<TokType, Token, t>;


// Program
extern pe::NTerm<TokType, void> Operator;

pe::NTerm<TokType, void> Program =
        pe::NTerm<TokType, void>("Program")
        | pe::Rule() << Operator
        | pe::Rule() << Program << Term<SEMICOLON>() << Operator
;


// Operator
extern pe::NTerm<TokType, void> InputOperator;
extern pe::NTerm<TokType, void> PrintOperator;
extern pe::NTerm<TokType, void> AssignOperator;

pe::NTerm<TokType, void> Operator =
        pe::NTerm<TokType, void>("Operator")
        | pe::Rule() << InputOperator
        | pe::Rule() << PrintOperator
        | pe::Rule() << AssignOperator
;


// InputOperator
extern pe::NTerm<TokType, double*> Variable;

pe::NTerm<TokType, void> InputOperator =
        pe::NTerm<TokType, void>("InputOperator")
        | pe::Rule() << Term<READ>() << Variable << [](double *variable) -> void { scanf("%lf", variable); }
        | pe::Rule() << InputOperator << Term<COMMA>() << Variable << [](double *variable) -> void { scanf("%lf", variable); }
;


// PrintOperator
extern pe::NTerm<TokType, double> Expression;

pe::NTerm<TokType, void> PrintOperator =
        pe::NTerm<TokType, void>("PrintOperator")
        | pe::Rule() << Term<PRINT>() << Expression <<
                   [] (double value) -> void {
                       printf("%f", value);
                   }
        | pe::Rule() << Term<PRINT>() << Term<STRING>() <<
                     [] (std::string str) -> void {
                         printf("%s", str.c_str());
                     }
        | pe::Rule() << PrintOperator << Term<COMMA>() << Expression <<
                     [] (double value) -> void {
                         printf(" %f", value);
                     }
        | pe::Rule() << PrintOperator << Term<COMMA>() << Term<STRING>() <<
                     [] (std::string str) -> void {
                         printf(" %s", str.c_str());
                     }
;


pe::NTerm<TokType, void> AssignOperator =
        pe::NTerm<TokType, void>("AssignOperator")
        | pe::Rule() << Variable << Term<SET>() << Expression <<
                   [](double *variable, double value) -> void {
                       *variable = value;
                   }
;


// Variable
std::map<std::string, double> variables;

pe::NTerm<TokType, double*> Variable =
        pe::NTerm<TokType, double*>("Variable")
        | pe::Rule() << Term<VARNAME>() <<
                   [](const std::string& name)-> double* {
                       return &variables[name];
                   }
;


// Expression
extern pe::NTerm<TokType, double> ExprTerm;  // Слагаемое

pe::NTerm<TokType, double> Expression =
        pe::NTerm<TokType, double>("Expression")
        | pe::Rule() << ExprTerm
        | pe::Rule() << Expression << Term<PLUS>() << ExprTerm <<
                     [](double x, double y) -> double { return x + y; }
        | pe::Rule() << Expression << Term<MINUS>() << ExprTerm <<
                     [](double x, double y) -> double { return x - y; }
;


// ExprTerm
extern pe::NTerm<TokType, double> Factor;

pe::NTerm<TokType, double> ExprTerm =
        pe::NTerm<TokType, double>("ExprTerm")
        | pe::Rule() << Factor
        | pe::Rule() << ExprTerm << Term<MUL>() << Factor <<
                     [](double x, double y) -> double { return x * y; }
        | pe::Rule() << ExprTerm << Term<DIV>() << Factor <<
                     [](double x, double y) -> double { return x / y; }
;


// Factor
pe::NTerm<TokType, double> Factor =
        pe::NTerm<TokType, double>("Factor")
        | pe::Rule() << Term<NUMBER>()
        | pe::Rule() << Variable << [](double *variable) -> double { return *variable; }
        | pe::Rule() << Term<LP>() << Expression << Term<RP>()
;


// Лексический анализатор
struct MyLexer : public pe::Lexer<TokType, Fragment> {
    std::string file_path;
    int begin = 1;
    std::fstream file;

    MyLexer(const char *input_file) {
        file_path = input_file;
        file.open(file_path,ios::in);
//        if (file.is_open()) {
//            std::cout << "open" << std::endl;
//        }
    };

    pe::Token<TokType, Fragment>* get_type(const std::string& t) {
        std::string delimiter = " ";
        std::string token = t.substr(0, t.find(delimiter));
        if (token == "PLUS") {
            return new Token<PLUS>(Fragment({{begin, 0}, {begin++, 4}}));
        }
        if (token == "MINUS") {
            return new Token<MINUS>(Fragment({{begin, 0}, {begin++, 5}}));
        }
        if (token == "MUL") {
            return new Token<MUL>(Fragment({{begin, 0}, {begin++, 3}}));
        }
        if (token == "DIV") {
            return new Token<DIV>(Fragment({{begin, 0}, {begin++, 3}}));
        }
        if (token == "SET") {
            return new Token<SET>(Fragment({{begin, 0}, {begin++, 3}}));
        }
        if (token == "SEMICOLON") {
            return new Token<SEMICOLON>(Fragment({{begin, 0}, {begin++, 7}}));
        }
        if (token == "COMMA") {
            return new Token<COMMA>(Fragment({{begin, 0}, {begin++, 5}}));
        }
        if (token == "LP") {
            return new Token<LP>(Fragment({{begin, 0}, {begin++, 2}}));
        }
        if (token == "RP") {
            return new Token<RP>(Fragment({{begin, 0}, {begin++, 2}}));
        }
        if (token == "PRINT") {
            return new Token<PRINT>(Fragment({{begin, 0}, {begin++, 5}}));
        }
        if (token == "READ") {
            return new Token<READ>(Fragment({{begin, 0}, {begin++, 4}}));
        }
        if (token == "NUMBER") {
            std::string value = t.substr(t.find(delimiter));
            value = value.substr(1, value.size());
            return new Token<NUMBER>(Fragment({{begin, 0}, {begin++, 6}}), std::stod(value));
        }
        if (token == "VARNAME") {
            std::string value = t.substr(t.find(delimiter));
            value = value.substr(1, value.size());
            return new Token<VARNAME>(Fragment({{begin, 0}, {begin++, 6}}), value);
        }
        if (token == "STRING") {
            std::string value = t.substr(t.find(delimiter));
            value = value.substr(1, value.size());
            return new Token<STRING>(Fragment({{begin, 0}, {begin++, 6}}), value);
        }
        return new Token<END_OF_TEXT>(Fragment({{begin, 0}, {begin++, 3}}));
    }

    pe::Token<TokType, Fragment>* next_token() {
        if (file.is_open()) {
            string tp;
            getline(file, tp);
            if (tp.empty()) {
                return new Token<END_OF_TEXT>(Fragment({{begin, 0}, {begin++, 3}}));
            }
            return get_type(tp);
        } else {
            std::cout << "file is closed" << std::endl;
        }
        return new Token<END_OF_TEXT>(Fragment({{begin, 0}, {begin++, 3}}));
    }
};

// Основная программа
int main(int argc, char *argv[]) {
    if (argc > 1) {
        MyLexer lexer(argv[1]);
        Program.compile_table();
        Program.parse(lexer);
    }
    return 0;
}