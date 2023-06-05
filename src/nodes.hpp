#pragma once

#include "token.hpp"
#include <vector>

enum class StatementType {
  BINARY_OP,
  UNARY_OP,
  CONSTANT,
  VARIABLE,
};

struct Constant {
  std::string type;
  std::string value;

  Constant() = default;
};

struct Type {
  enum class whichOne {custom, builtIn, none} which = whichOne::none;
  std::string str;
  TokenType tp;
  Type() = default;
  Type(TokenType);
  Type(const std::string&);
};

struct Variable {
  std::string name;
  std::vector<Type> type;

  Variable() = delete;
  Variable(const std::string&);
};

typedef struct BinOp BinOp;
typedef struct UnOp UnOp;

struct Statement {
  union {
    std::unique_ptr<Constant> c;
    std::unique_ptr<UnOp> uOp;
    std::unique_ptr<BinOp> bOp;
    std::unique_ptr<Variable> var;
  };

  Statement() = delete;
  Statement(const Statement&) = delete;
  Statement(std::unique_ptr<Constant>);
  Statement(std::unique_ptr<UnOp>);
  Statement(std::unique_ptr<BinOp>);
  Statement(std::unique_ptr<Variable>);
  ~Statement();

  private:
  enum class unionType {constant, unOp, binOp, var, none} type = unionType::none;
};

struct BinOp {
  TokenType op;
  std::unique_ptr<Statement> leftSide;
  std::unique_ptr<Statement> rightSide;
  BinOp() = default;
  BinOp(const BinOp&) = delete;

};

struct UnOp {
  TokenType op;
  std::unique_ptr<Statement> operand;
  UnOp() = default;
  UnOp(const UnOp&) = delete;
};

struct FunctionDec {
  std::string name;
  std::vector<Variable> params;
  std::vector<Statement> body;
  Type returnType;

  FunctionDec() = default;
  FunctionDec(const std::string&);
};

enum class DecType {
  FUNCTION,
  VARIABLE,
  NONE
};

struct Declaration {
  DecType type;
  union{
    std::unique_ptr<FunctionDec> func;
  };

  Declaration();
  Declaration(Declaration&&);
  Declaration(std::unique_ptr<FunctionDec>);
  ~Declaration();
};

struct Program {
  std::string name;
  std::vector<Declaration> decs;
  Program() = default;
};
