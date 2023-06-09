#include "../nodes.hpp"

const uint8_t indentationSize = 2;

void TokenList::prettyPrint(Tokenizer& tk, std::string& str) {
  if (token.type == TokenType::NOTHING) {
    return;
  }
  std::vector<TokenList *> r;
  for (TokenList *iter = this; iter; iter = iter->next) {
    if (iter->token.type == TokenType::DEC_PTR) {
      break;
    }
    r.emplace_back(iter);
  }
  if (!r.empty()) {
    for (size_t i = r.size() - 1; i > 0 ; --i) {
      str += tk.extractToken(r[i]->token) + " ";
    }
    str += tk.extractToken(r.front()->token);
  }
}

void VariableDec::prettyPrintDefinition(Tokenizer& tk, std::string& str) {
  str += tk.extractToken(name) + ": ";
  type.prettyPrint(tk, str);
}

void VariableDec::prettyPrint(Tokenizer& tk, std::string& str) {
  str += tk.extractToken(name) + ": ";
  type.prettyPrint(tk, str);
  if (initialAssignment) {
    str += " = ";
    initialAssignment->prettyPrint(tk, str);
  }
}

void Statement::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  if (!expression) {
    return;
  }
  switch (type) {
    case StatementType::EXPRESSION:
      expression->prettyPrint(tk, str); break;
    case StatementType::CONTROL_FLOW:
      controlFlow->prettyPrint(tk, str, indentation); break;
    case StatementType::SCOPE:
      scope->prettyPrint(tk, str, indentation); break;
    case StatementType::VARIABLE_DEC:
      varDec->prettyPrint(tk, str); break;
    case StatementType::KEYWORD: str += typeToString.at(keyword.type); break;
    case StatementType::NOTHING: break;
    default: str += "{not yet implemented in pretty printer}"; break;
  }
}

void UnOp::prettyPrint(Tokenizer& tk, std::string& str) {
  if (op.type == TokenType::DECREMENT_POSTFIX || op.type == TokenType::INCREMENT_POSTFIX) {
    operand.prettyPrint(tk, str);
    str += typeToString.at(op.type);
  } else {
    str += typeToString.at(op.type);
    operand.prettyPrint(tk, str);
  }
}

void BinOp::prettyPrint(Tokenizer& tk, std::string& str ) {
  leftSide.prettyPrint(tk, str);
  str += typeToString.at(op.type);
  rightSide.prettyPrint(tk, str);
}

void FunctionCall::prettyPrint(Tokenizer& tk, std::string& str) {
  str += tk.extractToken(name) + '(';
  if (args.curr.type != ExpressionType::NONE) {
    ExpressionList * iter = &args;
    for (; iter->next; iter = iter->next) {
      iter->curr.prettyPrint(tk, str);
      str += ", ";
    }
    iter->curr.prettyPrint(tk, str);
  }
  str += ')';
}

void ArrayAccess::prettyPrint(Tokenizer& tk, std::string& str) {
  str += tk.extractToken(array) + '[';
  offset.prettyPrint(tk, str);
  str += ']';
}

void Scope::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += "{\n";
  if (scopeStatements.curr.type != StatementType::NOTHING) {
    indentation += indentationSize;
    for (StatementList *iter = &scopeStatements; iter; iter = iter->next) {
      if (iter->curr.type != StatementType::NOTHING) {
        str += std::string(indentation, ' ');
        iter->curr.prettyPrint(tk, str, indentation);
        if (iter->curr.type != StatementType::SCOPE && (iter->curr.type != StatementType::CONTROL_FLOW || iter->curr.controlFlow->type == ControlFlowStatementType::RETURN_STATEMENT)) {
          str += ";\n";
        }
      }
    }
    indentation -= indentationSize;
  }
  str += std::string(indentation, ' ') + "}\n";
}

void FunctionDec::prettyPrintDefinition(Tokenizer& tk, std::string& str) {
  str += typeToString.at(TokenType::FUNC);
  str += tk.extractToken(name) + '(';
  if (params.curr.type != StatementType::NOTHING) {
    StatementList * iter = &params;
    for (; iter->next; iter = iter->next) {
      iter->curr.prettyPrint(tk, str, indentationSize);
      str += ", ";
    }
    iter->curr.prettyPrint(tk, str, indentationSize);
  }
  str += "): ";
  returnType.prettyPrint(tk, str);
}

void FunctionDec::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += typeToString.at(TokenType::FUNC);
  str += tk.extractToken(name) + '(';
  if (params.curr.type != StatementType::NOTHING) {
    indentation += indentationSize;
    StatementList * iter = &params;
    for (; iter->next; iter = iter->next) {
      iter->curr.prettyPrint(tk, str, indentation);
      str += ", ";
    }
    iter->curr.prettyPrint(tk, str, indentation);
    indentation -= indentationSize;
  }
  str += "): ";
  returnType.prettyPrint(tk, str);
  str += ' ';
  body.prettyPrint(tk, str, indentation);
}

void EnumDec::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += typeToString.at(TokenType::ENUM) + tk.extractToken(name) + "{\n";
  indentation += indentationSize;
  for (TokenList *iter = &members; iter; iter = iter->next) {
    str += std::string(indentation, ' ') + tk.extractToken(iter->token) + ",\n";
  }
  indentation -= indentationSize;
  str += std::string(indentation, ' ') + "}\n";
}

void GeneralDec::prettyPrintDefinition(std::vector<Tokenizer>& tks, std::string& str) {
  if (type == GeneralDecType::NOTHING) {
    return;
  }
  Tokenizer& tk = tks[tokenizerIndex];
  switch (type) {
    case GeneralDecType::FUNCTION:
      funcDec->prettyPrintDefinition(tk, str); break;
    case GeneralDecType::VARIABLE:
      varDec->prettyPrintDefinition(tk, str); break;
    case GeneralDecType::TEMPLATE:
      tempDec->prettyPrintDefinition(tk, str); break;
    case GeneralDecType::STRUCT:
      structDec->prettyPrintDefinition(tk, str); break;
    case GeneralDecType::TEMPLATE_CREATE:
      tempCreate->prettyPrint(tk, str); break;
    case GeneralDecType::INCLUDE_DEC:
      includeDec->prettyPrint(tk, str); break;
    default: break;
  }
}

void GeneralDec::prettyPrint(std::vector<Tokenizer>& tks, std::string& str) {
  if (type == GeneralDecType::NOTHING) {
    return;
  }
  Tokenizer& tk = tks[tokenizerIndex];
  switch (type) {
    case GeneralDecType::FUNCTION:
      funcDec->prettyPrint(tk, str, 0); break;
    case GeneralDecType::VARIABLE:
      varDec->prettyPrint(tk, str); break;
    case GeneralDecType::TEMPLATE:
      tempDec->prettyPrint(tk, str, 0); break;
    case GeneralDecType::STRUCT:
      structDec->prettyPrint(tk, str, 0); break;
    case GeneralDecType::ENUM:
      enumDec->prettyPrint(tk, str, 0); break;
    default: break;
  }
}

void GeneralDecList::prettyPrint(std::vector<Tokenizer>& tk, std::string& str) {
  GeneralDecList*list = this;
  for (; list->next; list = list->next) {
    list->curr.prettyPrint(tk, str);
    str += '\n';
  }
  list->curr.prettyPrint(tk, str);
}

void StructDec::prettyPrintDefinition(Tokenizer& tk, std::string& str) {
  str += typeToString.at(TokenType::STRUCT) + tk.extractToken(name);
}

void StructDec::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += typeToString.at(TokenType::STRUCT) + tk.extractToken(name) + " {\n";
  indentation += indentationSize;
  for (StructDecList* list = &decs; list; list = list->next) {
    str += std::string(indentation, ' ');
    if (list->type == StructDecType::FUNC) {
      list->funcDec->prettyPrint(tk, str, indentation);
    } else if (list->type == StructDecType::VAR) {
      list->varDec->prettyPrint(tk, str);
      str += ";\n";
    } 
  }
  indentation -= indentationSize;
  str += std::string(indentation, ' ') + "}\n";
}

void TemplateDec::prettyPrintDefinition(Tokenizer& tk, std::string& str) {
  str += typeToString.at(TokenType::TEMPLATE) + '[';
  if (templateTypes.token.type != TokenType::NOTHING) {
    TokenList * iter = &templateTypes;
    for (; iter->next; iter = iter->next) {
      str += tk.extractToken(iter->token);
      str += ", ";
    }
    str += tk.extractToken(iter->token);
  }
  str += "] ";
  if (isStruct) {
    structDec.prettyPrintDefinition(tk, str);
  } else {
    funcDec.prettyPrintDefinition(tk, str);
  }
}

void TemplateDec::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += typeToString.at(TokenType::TEMPLATE) + '[';
  if (templateTypes.token.type != TokenType::NOTHING) {
    TokenList * iter = &templateTypes;
    for (; iter->next; iter = iter->next) {
      str += tk.extractToken(iter->token);
      str += ", ";
    }
    str += tk.extractToken(iter->token);
  }
  str += "] ";
  if (isStruct) {
    structDec.prettyPrint(tk, str, indentation);
  } else {
    funcDec.prettyPrint(tk, str, indentation);
  }
}

void Program::prettyPrint(std::vector<Tokenizer>& tk, std::string& str) {
  decs.prettyPrint(tk, str);
}

void Expression::prettyPrint(Tokenizer& tk, std::string& str) {
  if (type == ExpressionType::NONE) {
    return;
  }
  switch (type) {
    case ExpressionType::ARRAY_ACCESS: arrAccess->prettyPrint(tk ,str); break;
    case ExpressionType::ARRAY_OR_STRUCT_LITERAL: arrayOrStruct->prettyPrint(tk, str); break;
    case ExpressionType::BINARY_OP: binOp->prettyPrint(tk, str); break;
    case ExpressionType::FUNCTION_CALL: funcCall->prettyPrint(tk, str); break;
    case ExpressionType::UNARY_OP: unOp->prettyPrint(tk, str); break;
    case ExpressionType::VALUE: str += tk.extractToken(value); break;
    case ExpressionType::WRAPPED: str += '('; wrapped->prettyPrint(tk, str); str += ')'; break;
    case ExpressionType::NONE: break;
    default: str += "{not yet implemented in pretty printer}"; break;
  }
}

void ArrayOrStructLiteral::prettyPrint(Tokenizer& tk, std::string& str) {
  str += '[';
  ExpressionList*list = &values;
  for (; list->next; list = list->next) {
    list->curr.prettyPrint(tk ,str);
    str += ", ";
  }
  list->curr.prettyPrint(tk, str);
  str += ']';
}

void ControlFlowStatement::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  if (type == ControlFlowStatementType::NONE) {
    return;
  }
  switch (type) {
    case ControlFlowStatementType::CONDITIONAL_STATEMENT: conditional->prettyPrint(tk, str, indentation); break;
    case ControlFlowStatementType::FOR_LOOP: forLoop->prettyPrint(tk, str, indentation); break;
    case ControlFlowStatementType::RETURN_STATEMENT: returnStatement->prettyPrint(tk, str); break;
    case ControlFlowStatementType::SWITCH_STATEMENT: switchStatement->prettyPrint(tk, str, indentation); break;
    case ControlFlowStatementType::WHILE_LOOP: whileLoop->prettyPrint(tk, str, indentation); break;
    case ControlFlowStatementType::NONE: break;
    default: str += "{not yet implemented in pretty printer}"; break;
  }
}

void ForLoop::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += typeToString.at(TokenType::FOR) + '(';
  initialize.prettyPrint(tk, str, indentation);
  if (condition.type == ExpressionType::NONE) {
    str += ';';
  } else {
    str += "; ";
  }
  condition.prettyPrint(tk, str);
  if (iteration.type == ExpressionType::NONE) {
    str += ';';
  } else {
    str += "; ";
  }
  iteration.prettyPrint(tk, str);
  str += ") ";
  body.prettyPrint(tk, str, indentation);
}

void ReturnStatement::prettyPrint(Tokenizer& tk, std::string& str) {
  str += typeToString.at(TokenType::RETURN);
  if (returnValue.type != ExpressionType::NONE) {
    str += ' ';
    returnValue.prettyPrint(tk, str);
  }
}

void SwitchStatement::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += typeToString.at(TokenType::SWITCH);
  switched.prettyPrint(tk, str);
  str += ' ';
  body.prettyPrint(tk, str, indentation);
}

void SwitchScopeStatementList::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += "{\n";
  indentation += indentationSize;
  for (SwitchScopeStatementList *list = this; list; list = list->next) {
    str += std::string(indentation, ' ');
    if (list->caseExpression) {
      str += typeToString.at(TokenType::CASE);
      list->caseExpression->prettyPrint(tk, str);
    } else {
      str += typeToString.at(TokenType::DEFAULT);
    }
    if (list->caseBody) {
      str += ' ';
      list->caseBody->prettyPrint(tk, str, indentation);
    } else {
      str += '\n';
    }
  }
  indentation -= indentationSize;
  str += std::string(indentation, ' ') + "}\n";
}

void WhileLoop::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += typeToString.at(TokenType::WHILE);
  statement.prettyPrint(tk ,str, indentation);
}

void IfStatement::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  condition.prettyPrint(tk, str);
  str += ' ';
  body.prettyPrint(tk, str, indentation);
}

void ConditionalStatement::prettyPrint(Tokenizer& tk, std::string& str, uint32_t indentation) {
  str += typeToString.at(TokenType::IF);
  ifStatement.prettyPrint(tk, str, indentation);
  for (ElifStatementList*list = elifStatement; list; list = list->next) {
    str += std::string(indentation, ' ') + typeToString.at(TokenType::ELIF);
    list->elif.prettyPrint(tk, str, indentation);
  }
  if (elseStatement) {
    str += std::string(indentation, ' ') + typeToString.at(TokenType::ELSE);
    elseStatement->prettyPrint(tk, str, indentation);
  }
}

void IncludeDec::prettyPrint(Tokenizer& tk, std::string& str) {
  str += typeToString.at(TokenType::INCLUDE) + ' ' + tk.extractToken(file);
}

void TemplateCreation::prettyPrint(Tokenizer& tk, std::string& str) {
  str += typeToString.at(TokenType::CREATE) + " [";
  if (templateTypes.token.type != TokenType::NOTHING) {
    str += tk.extractToken(templateTypes.token);
    TokenList * list = templateTypes.next;
    while (list) {
      str += ", " + tk.extractToken(list->token);
      list = list->next;
    }
  }
  str += "] " + typeToString.at(TokenType::AS) + tk.extractToken(templateName) + ";\n";
}
