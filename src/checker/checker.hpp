#pragma once

#include "../nodes.hpp"
#include "../nodeMemPool.hpp"
#include <map>

enum class CheckerErrorType {
  NONE,

  // general
  NAME_ALREADY_IN_USE,
  VOID_TYPE,
  TYPE_DOES_NOT_MATCH,
  EXPECTING_N_ARGS,
  UNEXPECTED_TYPE,

  // no such
  NO_SUCH_FUNCTION,
  NO_SUCH_TYPE,
  NO_SUCH_VARIABLE,
  NO_SUCH_TEMPLATE,
  NO_MUCH_MEMBER_VARIABLE,
  NO_MUCH_MEMBER_FUNCTION,

  // semantic errors
  CANNOT_REF_A_REF,
  CANNOT_PTR_A_REF,
  PTR_MUST_POINT_TO_A_TYPE,
  REF_MUST_REF_A_TYPE,
  CANNOT_HAVE_MULTI_TYPE,
  
  // things in the wrong spot
  EXPECTING_VARIABLE,
  EXPECTING_TYPE,
  EXPECTING_FUNCTION,
  EXPECTING_TEMPLATE,
  CANNOT_HAVE_BREAK_HERE,
  CANNOT_HAVE_CONTINUE_HERE,

  NOT_A_VARIABLE,
  NOT_A_FUNCTION,

  WRONG_NUMBER_OF_ARGS,

  MISSING_TYPE,

  // operator type compatibility
  CANNOT_PERFORM_OPERATION_ON_TYPE,

};

struct CheckerError {
  Token *token;
  CheckerErrorType type;
  Declaration *dec;
  CheckerError() = delete;
  CheckerError(CheckerErrorType, Token *);
  CheckerError(CheckerErrorType, Statement *, Declaration*);
  CheckerError(CheckerErrorType, Token *, Declaration*);
};

struct Checker {
  std::map<std::string, std::map<std::string, Declaration *>> structsLookUp;
  std::map<std::string, Declaration *> lookUp;
  std::vector<CheckerError> errors;
  Program& program;
  Tokenizer& tokenizer;
  NodeMemPool &memPool;
  TokenList boolValue {Token{0,0,TokenType::BOOL}};
  TokenList intValue {Token{0,0,TokenType::INT_TYPE}};
  TokenList charValue {Token{0,0,TokenType::CHAR_TYPE}};
  TokenList stringValue {Token{0,0,TokenType::STRING_LITERAL}};
  TokenList doubleValue {Token{0,0,TokenType::DOUBLE_TYPE}};
  TokenList nullptrValue {Token{0,0,TokenType::NULL_PTR}};

  Checker(Program&, Tokenizer&, NodeMemPool &);
  
  void firstTopLevelScan();
  void secondTopLevelScan();

  bool checkFunction(FunctionDec&);
  bool validateFunctionHeader(FunctionDec&);
  
  bool checkScope(Scope&, std::vector<std::string>&, Type&, bool, bool, bool);

  TokenList *checkStatement(Statement&);

  bool checkType(Type&);
};

// first do top level scan and place every dec in the map
// this includes structs and their members (variables, function headers)
// then start at the beginning and do a full search;