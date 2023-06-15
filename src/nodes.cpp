#include "nodes.hpp"
#include <iostream>

bool hasData(StatementType type) {
  return type >= StatementType::BINARY_OP && type <= StatementType::WRAPPED_VALUE;
}

Unexpected::Unexpected(Token token): token{token} {}

bool Unexpected::operator==(const Unexpected &ref) const {
  return ref.token == token;
}

Expected::Expected(ExpectedType type, uint32_t line, uint32_t column):
  line{line}, column{column}, tokenType{TokenType::NOTHING}, expectedType{type} {}

Expected::Expected(ExpectedType type, uint32_t line, uint32_t column, TokenType tokenType):
  line{line}, column{column}, tokenType{tokenType}, expectedType{type} {}

bool Expected::operator==(const Expected &ref) const {
  return ref.tokenType == tokenType && ref.expectedType == expectedType && ref.column == column && ref.line == line;
}

bool Type::operator==(const Type& tk) const {
  return tk.tokens == tokens;
}

VariableDec::VariableDec(Token token): type{}, name{token}, initialAssignment{nullptr} {}

bool VariableDec::operator==(const VariableDec& varDec) const {
  if (!(name == varDec.name) && !(type == varDec.type)) {
    return false;
  }
  if (!initialAssignment && !varDec.initialAssignment) {
    return true;
  } else if (!initialAssignment || !varDec.initialAssignment) {
    return false;
  }
  return *initialAssignment == *varDec.initialAssignment;
}

Statement::Statement(): unOp{nullptr}, type{StatementType::NONE} {}

Statement::Statement(StatementType type): unOp{nullptr}, type{type} {}

void Statement::operator=(Statement&& st) noexcept {
  type = st.type;
  switch (st.type) {
    case StatementType::UNARY_OP:
      unOp = st.unOp; st.unOp = nullptr; break;
    case StatementType::BINARY_OP:
      binOp = st.binOp; st.binOp = nullptr; break;
    case StatementType::VARIABLE_DEC:
      dec = st.dec; st.dec = nullptr; break;
    case StatementType::FUNCTION_CALL:
      funcCall = st.funcCall; st.funcCall = nullptr; break;
    case StatementType::ARRAY_ACCESS:
      arrAccess = st.arrAccess; st.arrAccess = nullptr; break;
    case StatementType::WRAPPED_VALUE:
      wrapped = st.wrapped; st.wrapped = nullptr; break;
    case StatementType::SCOPE:
      scope = st.scope; st.scope = nullptr; break;
    case StatementType::ARRAY_OR_STRUCT_LITERAL:
      arrOrStructLiteral = st.arrOrStructLiteral; st.arrOrStructLiteral = nullptr; break;
    case StatementType::FOR_LOOP_HEADER:
      list = st.list; st.list = nullptr; break;
    case StatementType::KEY_W_BODY:
      keyWBody = st.keyWBody; st.keyWBody = nullptr; break;
    case StatementType::KEYWORD:
      key = st.key; break;
    case StatementType::VALUE:
      var = st.var; st.var = nullptr; break;
    default: break;
  }
  st.type = StatementType::NONE;
}

bool Statement::operator==(const Statement& st) const {
  if (type != st.type) {
    return false;
  }
  switch (type) {
    case StatementType::UNARY_OP:
      if (!unOp && !st.unOp) {
        return true;
      }
      if (!unOp || !st.unOp) {
        return false;
      }
      return *unOp == *st.unOp;
    case StatementType::BINARY_OP:
      if (!binOp && !st.binOp) {
        return true;
      }
      if (!binOp || !st.binOp) {
        return false;
      }
      return *binOp == *st.binOp;
    case StatementType::VARIABLE_DEC:
      if (!dec && !st.dec) {
        return true;
      }
      if (!dec || !st.dec) {
        return false;
      }
      return *dec == *st.dec;
    case StatementType::FUNCTION_CALL:
      if (!funcCall && !st.funcCall) {
        return true;
      }
      if (!funcCall || !st.funcCall) {
        return false;
      }
      return *funcCall == *st.funcCall;
    case StatementType::ARRAY_ACCESS:
      if (!arrAccess && !st.arrAccess) {
        return true;
      }
      if (!arrAccess || !st.arrAccess) {
        return false;
      }
      return *arrAccess == *st.arrAccess;
    case StatementType::WRAPPED_VALUE:
      if (!wrapped && !st.wrapped) {
        return true;
      }
      if (!wrapped || !st.wrapped) {
        return false;
      }
      return *wrapped == *st.wrapped;
    case StatementType::ARRAY_OR_STRUCT_LITERAL:
      if (!arrOrStructLiteral && !st.arrOrStructLiteral) {
        return true;
      }
      if (!arrOrStructLiteral || !st.arrOrStructLiteral) {
        return false;
      }
      return *arrOrStructLiteral == *st.arrOrStructLiteral;
    case StatementType::FOR_LOOP_HEADER:
      if (!list && !st.list) {
        return true;
      }
      if (!list || !st.list) {
        return false;
      }
      return *list == *st.list;
    case StatementType::SCOPE:
      if (!scope && !st.scope) {
        return true;
      }
      if (!scope || !st.scope) {
        return false;
      }
      return *scope == *st.scope;
    case StatementType::KEYWORD:
      return key == st.key;
    case StatementType::VALUE:
      if (!var && !st.var) {
        return true;
      }
      if (!var || !st.var) {
        return false;
      }
      return *var == *st.var;
    default:
      return true;
  }
}

Statement::Statement(Statement&& st) noexcept {
  operator=(std::move(st));
}

Statement::Statement(UnOp *ptr) {
  unOp = ptr;
  type = StatementType::UNARY_OP;
}

Statement::Statement(BinOp *ptr) {
  binOp = ptr;
  type = StatementType::BINARY_OP;
}

/**
 * ptr has to point to a declaration which points to a variable declaration, otherwise bad things might happen
*/
Statement::Statement(Declaration *ptr) {
  dec = ptr;
  type = StatementType::VARIABLE_DEC;
}

Statement::Statement(FunctionCall *ptr) {
  funcCall = ptr;
  type = StatementType::FUNCTION_CALL;
}

Statement::Statement(ArrayAccess *ptr) {
  arrAccess = ptr;
  type = StatementType::ARRAY_ACCESS;
}

Statement::Statement(Statement *ptr) {
  wrapped = ptr;
  type = StatementType::WRAPPED_VALUE;
}

Statement::Statement(Scope *ptr) {
  scope = ptr;
  type = StatementType::SCOPE;
}

Statement::Statement(ArrOrStructLiteral *ptr) {
  arrOrStructLiteral = ptr;
  type = StatementType::ARRAY_OR_STRUCT_LITERAL;
}

Statement::Statement(KeywordWithBody *ptr) {
  keyWBody = ptr;
  type = StatementType::KEY_W_BODY;
}

Statement::Statement(ForLoopHeader *ptr) {
  list = ptr;
  type = StatementType::FOR_LOOP_HEADER;
}

Statement::Statement(Token *ptr) {
  var = ptr;
  type = StatementType::VALUE;
}

Statement *Statement::getChild() {
  switch (type) {
    case StatementType::UNARY_OP:
      return &unOp->operand;
    case StatementType::BINARY_OP:
      return &binOp->rightSide;
    default:
      return nullptr;
  }
}

ExpectedType Statement::addStatementToNode(Statement&& st) {
  switch (type) {
    case StatementType::UNARY_OP:
      if (unOp->operand) {
        return ExpectedType::TOKEN;
      }
      // bin op, un op, value, funcCall, array access, wrapped
      if (!hasData(st.type)) {
        return ExpectedType::BAD;
      }
      unOp->operand = std::move(st);
      return ExpectedType::NOTHING;

    case StatementType::BINARY_OP:
      if (binOp->rightSide) {
        return ExpectedType::TOKEN;
      }
      // bin op, un op, value, funcCall, array access, wrapped
      if (!hasData(st.type)) {
        return ExpectedType::BAD;
      }
      binOp->rightSide = std::move(st);
      return ExpectedType::NOTHING;

    case StatementType::KEY_W_BODY:
      if (keyWBody->body) {
        return ExpectedType::BAD;
      }
      if (!keyWBody->header && keyWBody->keyword.type != TokenType::ELSE) {
        if (keyWBody->keyword.type == TokenType::FOR) {
          if (st.type == StatementType::FOR_LOOP_HEADER) {
            keyWBody->header = std::move(st);
            return ExpectedType::NOTHING;
          } else {
            return ExpectedType::FOR_LOOP_HEADER;
          }
        }
        else if (hasData(st.type)) {
          keyWBody->header = std::move(st);
          return ExpectedType::NOTHING;
        } else if (st.type == StatementType::ARRAY_OR_STRUCT_LITERAL && keyWBody->keyword.type == TokenType::RETURN) {
          keyWBody->header = std::move(st);
          return ExpectedType::NOTHING;
        } else {
          return ExpectedType::EXPRESSION;
        }
      }
      if (keyWBody->keyword.type == TokenType::RETURN) {
        return ExpectedType::TOKEN;
      }
      if (st.type == StatementType::SCOPE) {
        keyWBody->body.scopeStatements.curr = std::move( st.scope->scopeStatements.curr);
        keyWBody->body.scopeStatements.next = st.scope->scopeStatements.next;
        return ExpectedType::NOTHING;
      }
      return ExpectedType::SCOPE;

    default:
      return ExpectedType::TOKEN;
  }
}

ExpectedType Statement::isValid() const {
  switch (type) {
    case StatementType::UNARY_OP:
      if (!unOp->operand) {
        return ExpectedType::EXPRESSION;
      }
      break;
    case StatementType::BINARY_OP:
      if (!binOp->rightSide) {
        return ExpectedType::EXPRESSION;
      }
      break;
    case StatementType::KEY_W_BODY: {
      // if, elif, while, for, switch
    }
    default:
      break;
  }
  return ExpectedType::NOTHING;
}

KeywordWithBody::KeywordWithBody(Token token): body{}, header{}, keyword{token} {}

KeywordWithBody::KeywordWithBody(KeywordWithBody&& rval): body{std::move(rval.body)}, header{std::move(rval.header)}, keyword{rval.keyword} {
  rval.keyword.type = TokenType::NOTHING;
}

ArrayAccess::ArrayAccess(Token token): array{token} {}

bool ArrayAccess::operator==(const ArrayAccess& arrAcc) const {
  return array == arrAcc.array && offset == arrAcc.offset;
}

bool Scope::operator==(const Scope& sp) const {
  return sp.scopeStatements == scopeStatements;
}

BinOp::BinOp(Token op): leftSide{}, rightSide{}, op{op} {}

BinOp::BinOp(BinOp&& binOp) noexcept: leftSide{std::move(binOp.leftSide)}, rightSide{std::move(binOp.rightSide)}, op{binOp.op} {
  binOp.op.type = TokenType::NOTHING;
}

bool BinOp::operator==(const BinOp& bo) const {
  if (!(op == bo.op)) {
    return false;
  }
  // short circuit right side
  bool r = false;
  if (rightSide || bo.rightSide) {
    if (!rightSide || !bo.rightSide) {
      return false;
    }
    r = true;
  }
  if (leftSide && bo.leftSide) {
    if (!(leftSide == bo.leftSide)) {
      return false;
    }
  } else if (leftSide || bo.leftSide) {
    return false;
  }
  if (r) {
    if (!(rightSide == bo.rightSide)) {
      return false;
    }
  }
  return true;
}

UnOp::UnOp(Token op): op{op} {}

UnOp::UnOp(UnOp&& unOp) noexcept : operand{std::move(unOp.operand)} , op{unOp.op} {}

bool UnOp::operator==(const UnOp& uo) const {
  if (operand && uo.operand) {
    if (!(operand == uo.operand)) {
      return false;
    }
  } else if (operand || uo.operand) {
    return false;
  }
  return true;
}

Enum::Enum(): name{0,0,TokenType::NOTHING} {}

FunctionDec::FunctionDec(Token token): name{token} {}

FunctionDec::FunctionDec(FunctionDec&& fd):
  params{std::move(fd.params)}, body{std::move(fd.body)},
  returnType{std::move(fd.returnType)}, name{fd.name} {}

FunctionCall::FunctionCall(Token token): name{token} {}

bool FunctionCall::operator==(const FunctionCall& fc) const {
  return name == fc.name && args == fc.args;
}

Struct::Struct(Token tok): name{tok} {}

Declaration::Declaration(): func{nullptr}, decType{DecType::NONE} {}

Declaration::Declaration(Declaration&& dec) noexcept : decType{dec.decType} {
  switch (dec.decType) {
    case DecType::FUNCTION:
      func = dec.func; dec.func = nullptr; break;
    case DecType::VARIABLE_DEC:
      varDec = dec.varDec; dec.varDec = nullptr; break;
    case DecType::STRUCT:
      struc = dec.struc; dec.struc = nullptr; break;
    case DecType::TEMPLATE:
      temp = dec.temp; dec.temp = nullptr; break;
    case DecType::ENUM:
      enm = dec.enm; dec.enm = nullptr; break;
    default: break;
  }
  dec.decType = DecType::NONE;
}

Declaration::Declaration(FunctionDec *ptr): func{ptr}, decType{DecType::FUNCTION} {}

Declaration::Declaration(VariableDec *ptr): varDec{ptr}, decType{DecType::VARIABLE_DEC} {}

Declaration::Declaration(Template *ptr): temp{ptr}, decType{DecType::TEMPLATE} {}

Declaration::Declaration(Struct *ptr): struc{ptr}, decType{DecType::STRUCT} {}

Declaration::Declaration(Enum *ptr): enm{ptr}, decType{DecType::ENUM} {}

Program::Program(Program&& prog) noexcept : name{std::move(prog.name)}, decs{std::move(prog.decs)} {}

bool ArrOrStructLiteral::operator==(const ArrOrStructLiteral & l) const {
  return list == l.list;
}

bool FunctionDec::operator==(const FunctionDec& ref) const {
  return ref.name == name && ref.returnType == returnType && ref.params == params && ref.body == body;
}

bool Enum::operator==(const Enum& ref) const {
  return ref.name == name && ref.members == members;
}

bool Declaration::operator==(const Declaration& ref) const {
  if (ref.decType != decType) {
    return false;
  }

  switch (decType) {
    case DecType::TEMPLATE:
      if (ref.temp && temp) {
        if (!(*ref.temp == *temp)) {
          return false;
        }
      } else if (ref.temp || temp) {
        return false;
      }
      return true;
    case DecType::ENUM:
      if (ref.enm && enm) {
        if (!(*ref.enm == *enm)) {
          return false;
        }
      } else if (ref.enm || enm) {
        return false;
      }
      return true;
    case DecType::VARIABLE_DEC:
      if (ref.varDec && varDec) {
        if (!(*ref.varDec == *varDec)) {
          return false;
        }
      } else if (ref.varDec || varDec) {
        return false;
      }
      return true;
    case DecType::STRUCT:
      if (ref.struc && struc) {
        if (!(*ref.struc == *struc)) {
          return false;
        }
      } else if (ref.struc || struc) {
        return false;
      }
      return true;
    case DecType::FUNCTION:
      if (ref.func && func) {
        if (!(*ref.func == *func)) {
          return false;
        }
      } else if (ref.func || func) {
        return false;
      }
      return true;
    default:
      return true;
  }
}

bool Struct::operator==(const Struct& ref) const {
  return (ref.name == name) && (ref.decs == decs);
}

bool Template::operator==(const Template& ref) const {
  return ref.templateIdentifiers == templateIdentifiers && ref.dec == dec;
}

bool Program::operator==(const Program& ref) const {
  return ref.name == name && ref.decs == decs;
}

bool KeywordWithBody::operator==(const KeywordWithBody& ref) const {
  if (!(ref.keyword == keyword)) {
    return false;
  }
  if (ref.body && body) {
    if (!(ref.body == body)) {
      return false;
    }
  } else if (ref.body || body) {
    return false;
  }
  if (ref.header && header) {
    if (!(ref.header == header)) {
      return false;
    }
  } else if (ref.header || header) {
    return false;
  }
  return true;
}

Statement::operator bool() const {
  return type != StatementType::NONE;
}

bool ForLoopHeader::operator==(const ForLoopHeader& ref) const {
  return list == ref.list;
}

TokenList::TokenList(): curr{0,0,TokenType::NOTHING}, next{nullptr} {}
TokenList::TokenList(Token tk): curr{tk}, next{nullptr} {}

bool TokenList::operator==(const TokenList& ref) const {
  const TokenList* refCurr = &ref;
  const TokenList* thisCurr = this;
  while (refCurr->next && thisCurr->next) {
    if (!(refCurr->curr == thisCurr->curr)) {
      return false;
    }
    refCurr = refCurr->next;
    thisCurr = thisCurr->next;
  }
  if (refCurr->next || thisCurr->next) {
    return false;
  }
  return true;
}

bool StatementList::operator==(const StatementList& ref) const {
  const StatementList* refCurr = &ref;
  const StatementList* thisCurr = this;
  while (refCurr->next && thisCurr->next) {
    if (!(refCurr->curr == thisCurr->curr)) {
      return false;
    }
    refCurr = refCurr->next;
    thisCurr = thisCurr->next;
  }
  if (refCurr->next || thisCurr->next) {
    return false;
  }
  return true;
}

StatementList::StatementList(): curr{}, next{} {}

std::string Expected::getErrorMessage(const std::string& file) {
  std::string message = file + ':' + std::to_string(line) + ':' + std::to_string(column) + '\n';
  if (expectedType == ExpectedType::TOKEN) {
    if (tokenType == TokenType::IDENTIFIER) {
      return message + "\nExpected Identifier\n";
    }
    return message + "\nExpected Token: " + typeToString.at(tokenType) + '\n';
  } else if (expectedType == ExpectedType::EXPRESSION) {
    return message + "\nExpected Expression\n";
  } else if (expectedType == ExpectedType::SCOPE) {
    return message + "\nExpected Scope\n";
  }
  return message;
}

std::string Unexpected::getErrorMessage(Tokenizer& tk, const std::string& file) {
  return file + ":" + std::to_string(token.lineNum) + ":" + std::to_string(token.linePos) +
  "\nUnexpected token: " + tk.extractToken(token) + '\n';
}

StatementList::operator bool() const {
  return curr.type != StatementType::NONE;
}
Scope::operator bool() const {
  return scopeStatements;
}
