#include <map>
#include <stdio.h>
#include <string>

#include "parser_edsl.h"

enum TokType {
  END_OF_TEXT = 0,
  PLUS = '+', MUNUS = '-', MUL = '*', DIV = '/', SET = '=',
  SEMICOLON = ';', COMMA = ',', LP = '(', RP = ')',
  PRINT, READ, VARNAME, NUMBER, STRING
};

using namespace pe = parser_edsl;

struct Pos {
  int line;
  int col;
};

struct Fragment {
  Pos begin;
  Pos end;
};

template <TokType t>
struct Token<t>: public pe::Token<TokType, Fragment>
{
  Token(Fragment pos)
    : pe::Token(t, pos)
  {
    /* пусто */
  }
};

template <>
struct Token<VARNAME>:
  public pe::AttrToken<TokType, Fragment, std::string>
{
  Token(Fragment pos, const std::string& name)
    : pe::AttrToken(VARNAME, pos, name)
  {
    /* пусто */
  }
};

template <>
struct Token<NUMBER>: public pe::AttrToken<TokType, Fragment, double>
{
  Token(Fragment pos, double value)
    : pe::AttrToken(NUMBER, pos, value)
  {
    /* пусто */
  }
};

template <>
struct Token<STRING>:
  public pe::AttrToken<TokType, Fragment, std::string>
{
  Token(Fragment pos, const std::string& text)
    : pe::AttrToken(STRING, pos, text)
  {
    /* пусто */
  }
};

template <TokType t>
using Term<t> = pe::Terminal<TokType, Token, t>;


// Program
extern pe::NTerm<TokType, void> Operator;

pe::NTerm<TokType, void> Program =
    pe::Rule() << Operator
  | pe::Rule() << Program << Term<';'>() << Operator
  ;


// Operator
extern pe::NTerm<TokType, void> InputOperator;
extern pe::NTerm<TokType, void> PrintOperator;
extern pe::NTerm<TokType, void> AssignOperator;

pe::NTerm<TokType, void> Operator =
    pe::Rule() << InputOperator
  | pe::Rule() << PrintOperator
  | pe::Rule() << AssignOperator
  ;


// InputOperator
extern pe::NTerm<TokType, double*> Variable;

void input_func(double *variable) {
  scanf("%lf", variable);
}

pe::NTerm<TokType, void> InputOperator =
    pe::Rule() << Term<READ>() << Variable << input_func
  | pe::Rule() << InputOperator << Term<','>() << Variable << input_func
  ;


// PrintOperator
extern pe::NTerm<TokType, double> Expression;

pe::NTerm<TokType, void> PrintOperator =
    pe::Rule() << Term<PRINT>() << Expression <<
    [] (double value) -> void {
      printf("%f", value);
    }
  | pe::Rule() << Term<PRINT>() << Term<STRING>() <<
    [] (std::string str) -> void {
      printf("%s", str.c_str());
    }
  | pe::Rule() << PrintOperator << Term<','>() << Expression <<
    [] (double value) -> void {
      printf(" %f", value);
    }
  | pe::Rule() << PrintOperator << Term<','>() << Term<STRING>() <<
    [] (std::string str) -> void {
      printf(" %s", value);
    }
  ;


pe::NTerm<TokType, void> AssignOperator =
    pe::Rule() << Variable << Term<'='>() << Expression <<
    [](double *variable, double value) -> void {
      *variable = value;
    }
  ;


// Variable
std::map<std::string, double> variables;

pe::NTerm<TokType, double*> Variable =
    pe::Rule() << Term<VARNAME>() <<
    [](std::string name) {
      return &variables[name];
    }
  ;


// Expression
extern pe::NTerm<TokType, double> ExprTerm;  // Слагаемое

pe::NTerm<TokType, double> Expression =
    pe::Rule() << ExprTerm
  | pe::Rule() << Expression << Term<'+'>() << ExprTerm <<
    [](double x, double y) -> double { return x + y; }
  | pe::Rule() << Expression << Term<'-'>() << ExprTerm <<
    [](double x, double y) -> double { return x - y; }
  ;


// ExprTerm
extern pe::NTerm<TokType, double> Factor;

pe::NTerm<TokType, double> ExprTerm =
    pe::Rule() << Factor
  | pe::Rule() << ExprTerm << Term<'*'>() << Factor <<
    [](double x, double y) { return x * y; }
  | pe::Rule() << ExprTerm << Term<'/'>() << Factor <<
    [](double x, double y) { return x / y; }
  ;


// Factor
pe::NTerm<TokType, double> Factor =
    pe::Rule() << Term<NUMBER>()
  | pe::Rule() << Variable << [](double *variable) { return *variable; }
  | pe::Rule() << Term<'('>() << Expression << Term<')'>()
  ;


// Лексический анализатор
class MyLexer : public pe::Lexer<TokType> {
  MyLexer(const char *input_file);

  pe::Token<TokType, Fragment> *next_token() {
    ...
    return new Token<'+'>(coord);
    ...
    return new Token<PRINT>(coord);
    ...
    return new Token<VARNAME>(coord, varname);
    ...
    return new Token<NUMBER>(coord, value);
    ...
    // Конец ввода всегда должен иметь значение 0
    return new Token<0>(coord);
  }
};


// Основная программа
int main(int argc, char *argv[]) {
  if (argc > 1) {
    MyLexer lexer(argv[1]);
    Program.compile_tables();
    Program.parse(lexer);
  }
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
