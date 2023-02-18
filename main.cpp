#include <map>
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>

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
        read_file();
    };

    std::vector<pe::Token<TokType, Fragment>*> tokens;
    int cur = 0;

    void read_file() {
        if (file.is_open()) {
            string tp;
            int start = 1;
            while (true) {
                getline(file, tp);
                if (tp == "0") {
                    tokens.push_back(new Token<END_OF_TEXT>(Fragment({{start, 0}, {start, 1}})));
                    break;
                }
                auto len = tp.size();
                for (auto i = 0; i < len;) {
                    auto col = i;
                    auto next = tp[i];
                    switch(next) {
                        case ' ':
                            i++;
                            break;
                        case 'v':
                        {
                            col = i;
                            i += 4;
                            string var_name;
                            for (; i < len && tp[i] != ' '; i++) {
                                var_name += tp[i];
                            }
                            tokens.push_back(new Token<VARNAME>(Fragment({{start, col}, {start, i-1}}), var_name));
                            break;
                        }
                        case '"':
                        {
                            col = i;
                            i++;
                            string str;
                            for (; tp[i] != '"'; i++) {
                                str += tp[i];
                            }
                            tokens.push_back(new Token<STRING>(Fragment({{start, col}, {start, ++i}}), str));
                            break;
                        }
                        case 'r':
                        {
                            col = i;
                            i+= 4;
                            tokens.push_back(new Token<READ>(Fragment({{start, col}, {start, i-1}})));
                            break;
                        }
                        case 'p':
                        {
                            col = i;
                            i+= 5;
                            tokens.push_back(new Token<PRINT>(Fragment({{start, col}, {start, i-1}})));
                            break;
                        }
                        case '=':
                            tokens.push_back(new Token<SET>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case '(':
                            tokens.push_back(new Token<LP>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case ')':
                            tokens.push_back(new Token<RP>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case '+':
                            tokens.push_back(new Token<PLUS>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case '-':
                            tokens.push_back(new Token<MINUS>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case '*':
                            tokens.push_back(new Token<MUL>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case '/':
                            tokens.push_back(new Token<DIV>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case ';':
                            tokens.push_back(new Token<SEMICOLON>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case ',':
                            tokens.push_back(new Token<COMMA>(Fragment({{start, col}, {start, i++}})));
                            break;
                        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                        {
                            col = i;
                            string number;
                            number += next;
                            for (i++; i < len && tp[i] != ' '; i++) {
                                number += tp[i];
                            }
                            tokens.push_back(new Token<NUMBER>(Fragment({{start, col}, {start, i++}}), std::stod(number)));
                            break;
                        }
                        default:
                            i++;
                            std::cout << "default:" << next << std::endl;
                            break;
                    }
                }
                start++;
            }

        }
    }

    pe::Token<TokType, Fragment>* next_token() {
        return tokens[cur++];
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