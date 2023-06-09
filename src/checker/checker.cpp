#include "checker.hpp"
#include <iostream>

Token getTokenOfExpression(Expression& exp) {
  switch (exp.type) {
    case ExpressionType::ARRAY_ACCESS: {
      return exp.arrAccess->array;
    }
    case ExpressionType::ARRAY_OR_STRUCT_LITERAL: {
      return getTokenOfExpression(exp.arrayOrStruct->values.curr);
    }
    case ExpressionType::BINARY_OP: {
      return exp.binOp->op;
    }
    case ExpressionType::FUNCTION_CALL: {
      return exp.funcCall->name;
    }
    case ExpressionType::UNARY_OP: {
      return exp.unOp->op;
    }
    case ExpressionType::VALUE: {
      return exp.value;
    }
    case ExpressionType::WRAPPED: {
      return getTokenOfExpression(*exp.wrapped);
    }
    default: {
      std::cerr << "cannot get token of this expression\n";
      exit(1);
    }
  }
}

TokenList Checker::noneValue {Token{0,0,TokenType::NOTHING}};
TokenList Checker::badValue {Token{0,0,TokenType::BAD_VALUE}};
TokenList Checker::boolValue {Token{0,0,TokenType::BOOL}};
TokenList Checker::int32Value {Token{0,0,TokenType::INT32_TYPE}};
TokenList Checker::uint32Value {Token{0,0,TokenType::UINT32_TYPE}};
TokenList Checker::int64Value {Token{0,0,TokenType::INT64_TYPE}};
TokenList Checker::uint64Value {Token{0,0,TokenType::UINT64_TYPE}};
TokenList Checker::charValue {Token{0,0,TokenType::CHAR_TYPE}};
TokenList Checker::stringValue {Token{0,0,TokenType::POINTER}, &Checker::charValue};
TokenList Checker::floatValue {Token{0,0,TokenType::FLOAT_TYPE}};
TokenList Checker::doubleValue {Token{0,0,TokenType::DOUBLE_TYPE}};
TokenList Checker::voidValue {Token{0,0,TokenType::VOID}};
TokenList Checker::ptrValue {Token{0,0,TokenType::POINTER}, &Checker::voidValue};
TokenList Checker::nullptrValue {Token{0,0,TokenType::NULL_PTR}};

CheckerError::CheckerError(CheckerErrorType type, uint32_t tkIndex, Token token): token{token}, dec{nullptr}, tkIndex{tkIndex}, type{type}  {}
CheckerError::CheckerError(CheckerErrorType type, uint32_t tkIndex, Token token, GeneralDec *decPtr): token{token}, dec{decPtr}, tkIndex{tkIndex}, type{type} {}
CheckerError::CheckerError(CheckerErrorType type, uint32_t tkIndex, Expression *expression): dec{}, tkIndex{tkIndex}, type{type} {
  token = getTokenOfExpression(*expression);
}
CheckerError::CheckerError(CheckerErrorType type, uint32_t tkIndex, Expression *expression, GeneralDec *decPtr): dec{decPtr}, tkIndex{tkIndex}, type{type} {
  token = getTokenOfExpression(*expression);
}

std::string CheckerError::getErrorMessage(std::vector<Tokenizer>& tokenizers) {
  auto& tk = tokenizers[tkIndex];
  TokenPositionInfo posInfo = tk.getTokenPositionInfo(token);
  std::string message = tk.filePath + ':' + std::to_string(posInfo.lineNum) + ':' + std::to_string(posInfo.linePos) + '\n';
  switch (type) {
    case CheckerErrorType::CANNOT_BE_CONVERTED_TO_BOOL: message += "Value cannot be converted to boolean\n"; break;
    case CheckerErrorType::CANNOT_DEREFERENCE_NON_POINTER_TYPE: message += "Cannot dereference non-pointer type\n"; break;
    case CheckerErrorType::CANNOT_HAVE_BREAK_HERE: message += "\"break\" can only be inside loops and switch cases\n"; break;
    case CheckerErrorType::CANNOT_HAVE_CONTINUE_HERE: message += "\"continue\" can only be inside loops\n"; break;
    case CheckerErrorType::CANNOT_HAVE_MULTI_TYPE: message += "Multi type not allowed\n"; break;
    case CheckerErrorType::CANNOT_OPERATE_ON_TEMPORARY: message += "Cannot perform this operation on a temporary value\n"; break;
    case CheckerErrorType::CANNOT_ASSIGN_TO_TEMPORARY: message += "Cannot assign a value to a temporary value\n"; break;
    case CheckerErrorType::CANNOT_PTR_A_REF: message += "Cannot have a pointer to a reference type\n"; break;
    case CheckerErrorType::CANNOT_REF_A_REF: message += "Cannot have a reference to a reference type\n"; break;
    case CheckerErrorType::NAME_ALREADY_IN_USE: message += "Name already in use\n"; break;
    case CheckerErrorType::NO_SUCH_FUNCTION: message += "Function does not exist\n"; break;
    case CheckerErrorType::NO_SUCH_MEMBER_FUNCTION: message += "Member function does not exist\n"; break;
    case CheckerErrorType::NO_SUCH_MEMBER_VARIABLE: message += "Member variable does not exist\n"; break;
    case CheckerErrorType::NO_SUCH_TYPE: message += "No such type\n"; break;
    case CheckerErrorType::NO_SUCH_TEMPLATE: message += "No such template\n"; break;
    case CheckerErrorType::NO_SUCH_VARIABLE: message += "No such variable\n"; break;
    case CheckerErrorType::NOT_A_FUNCTION: message += "Not a function\n"; break;
    case CheckerErrorType::NOT_A_STRUCT: message += "Not a struct\n"; break;
    case CheckerErrorType::NOT_A_TEMPLATE: message += "Not a template\n"; break;
    case CheckerErrorType::NOT_A_VARIABLE: message += "Not a variable\n"; break;
    case CheckerErrorType::TYPE_DOES_NOT_MATCH: message += "Type does not match\n"; break;
    case CheckerErrorType::UNEXPECTED_TYPE: message += "Unexpected type\n"; break;
    case CheckerErrorType::VOID_TYPE: message += "Void type not allowed\n"; break;
    case CheckerErrorType::WRONG_NUMBER_OF_ARGS: message += "Incorrect number of arguments\n"; break;
    case CheckerErrorType::CANNOT_ASSIGN: message += "Cannot assign\n"; break;
    case CheckerErrorType::INCORRECT_RETURN_TYPE: message += "Incorrect return type\n"; break;
    case CheckerErrorType::NOT_ALL_CODE_PATHS_RETURN: message += "Not all code paths return a value\n"; break;
    case CheckerErrorType::EMPTY_STRUCT: message += "Empty struct\n"; break;
    case CheckerErrorType::STRUCT_CYCLE: message += "Struct cycle detected. Size of struct is not finite\n"; break;
    default: message += "Error of some kind, sorry bro\n"; break;
  }
  if (dec) {
    message += "Declaration defined as such:\n  ";
    dec->prettyPrintDefinition(tokenizers, message);
    message += "\n\n";
  } else {
    message += '\n';
  }
  return message;
}

ResultingType::ResultingType(TokenList* type, bool isLValue): type{type}, isLValue{isLValue} {}

Checker::Checker(Program& prog, std::vector<Tokenizer>& tks, NodeMemPool& mem):
structsLookUp{}, lookUp{}, errors{}, program{prog}, tokenizers{tks}, memPool{mem} {}

bool Checker::check() {
  firstTopLevelScan();
  if (!errors.empty()) {
    return false;
  }
  secondTopLevelScan();
  if (!errors.empty()) {
    return false;
  }
  std::vector<StructDec *> chain;
  for (GeneralDecList *list = &program.decs; list; list = list->next) {
    if (list->curr.type != GeneralDecType::STRUCT) {
      continue;
    }
    if (!list->curr.structDec->checked) {
      checkForStructCycles(list->curr, chain);
    }
  }
  if (!errors.empty()) {
    return false;
  }
  fullScan();
  if (!errors.empty()) {
    return false;
  }
  return true;
}

/**
 * Scans all global declarations and registers them in the table, checking that the name is available
 * Also registers struct members in the struct table
*/
void Checker::firstTopLevelScan() {
  for (GeneralDecList *list = &program.decs; list; list = list->next) {
    Tokenizer& tk = tokenizers[list->curr.tokenizerIndex];
    switch (list->curr.type) {
      case GeneralDecType::FUNCTION: {
        GeneralDec* &decPtr = lookUp[tk.extractToken(list->curr.funcDec->name)];
        if (decPtr) {
          errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, list->curr.funcDec->name, decPtr);
        } else {
          decPtr = &list->curr;
        }
        break;
      }
      case GeneralDecType::VARIABLE: {
        GeneralDec* &decPtr = lookUp[tk.extractToken(list->curr.varDec->name)];
        if (decPtr) {
          errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, list->curr.varDec->name, decPtr);
        } else {
          decPtr = &list->curr;
        }
        break;
      }
      case GeneralDecType::STRUCT: {
        const std::string structName = tk.extractToken(list->curr.structDec->name);
        GeneralDec* &decPtr = lookUp[structName];
        if (decPtr) {
          errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, list->curr.structDec->name, decPtr);
        } else {
          decPtr = &list->curr;
          auto& structDecLookUp = structsLookUp[structName];
          if (list->curr.structDec->decs.type == StructDecType::NONE) {
            errors.emplace_back(CheckerErrorType::EMPTY_STRUCT, tk.tokenizerIndex, list->curr.structDec->name);
            break;
          }
          for (StructDecList* inner = &list->curr.structDec->decs; inner; inner = inner->next) {
            StructDecList** innerStructDecPtr;
            Token token;
            if (inner->type == StructDecType::VAR) {
              token = inner->varDec->name;
              innerStructDecPtr = &structDecLookUp[tk.extractToken(inner->varDec->name)];
            } else {
              token = inner->funcDec->name;
              innerStructDecPtr = &structDecLookUp[tk.extractToken(inner->funcDec->name)];
            }
            if (*innerStructDecPtr) {
              GeneralDec *errorDec = memPool.makeGeneralDec();
              if ((*innerStructDecPtr)->type == StructDecType::FUNC) {
                errorDec->type = GeneralDecType::FUNCTION;
                errorDec->funcDec = (*innerStructDecPtr)->funcDec;
              } else {
                errorDec->type = GeneralDecType::VARIABLE;
                errorDec->varDec = (*innerStructDecPtr)->varDec;
              }
              errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, token, errorDec);
            } else {
              *innerStructDecPtr = inner;
            }
          }
        }

        break;
      }
      case GeneralDecType::TEMPLATE: {
        Token token{0,0,TokenType::NOTHING};
        if (list->curr.tempDec->isStruct) {
          token = list->curr.tempDec->structDec.name;
        } else {
          // dec.temp->dec.decType == DecType::FUNCTION
          token = list->curr.tempDec->funcDec.name;
        }
        GeneralDec* &decPtr = lookUp[tk.extractToken(token)];
        if (decPtr) {
          errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, token, decPtr);
        } else {
          decPtr = &list->curr;
        }
        break;
      }
      case GeneralDecType::TEMPLATE_CREATE: {
        GeneralDec* &decPtr = lookUp[tk.extractToken(list->curr.tempCreate->typeName)];
        if (decPtr) {
          errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, list->curr.tempCreate->typeName, decPtr);
        } else {
          decPtr = &list->curr;
        }
        break;
      }
      default: {
        break;
      }
    }
  }
}

/**
 * Validates function types, global variable types, struct member variable types, struct member function types.
 * Everything that was registered in the first pass
*/
void Checker::secondTopLevelScan() {
  for (GeneralDecList* list = &program.decs; list; list = list->next) {
    Tokenizer& tk = tokenizers[list->curr.tokenizerIndex];
    switch (list->curr.type) {
      case GeneralDecType::FUNCTION: {
        validateFunctionHeader(tk, *list->curr.funcDec);
        break;
      }
      case GeneralDecType::VARIABLE: {
        checkType(tk, list->curr.varDec->type);
        break;
      }
      case GeneralDecType::STRUCT: {
        validateStructTopLevel(tk, list->curr.tempDec->structDec);
        break;
      }
      case GeneralDecType::TEMPLATE: {
        // parser validates that there is at least one type
        std::vector<std::string> templateTypes;
        TokenList *templateIdentifiers = &list->curr.tempDec->templateTypes;
        // add templated types to global lookup
        bool errorFound = false;
        do {
          templateTypes.push_back(tk.extractToken(templateIdentifiers->token));
          GeneralDec *&tempTypeDec = lookUp[templateTypes.back()];
          if (tempTypeDec) {
            errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, templateIdentifiers->token, tempTypeDec);
            errorFound = true;
            break;
          }
          tempTypeDec = memPool.makeGeneralDec();
          tempTypeDec->type = GeneralDecType::STRUCT;
          templateIdentifiers = templateIdentifiers->next;
        } while (templateIdentifiers);
        // validate top level types
        if (errorFound) {
          break;
        }
        if (list->curr.tempDec->isStruct) {
          validateStructTopLevel(tk, list->curr.tempDec->structDec);
        } else {
          validateFunctionHeader(tk, list->curr.tempDec->funcDec);
        }
        // remove templated types
        while (!templateTypes.empty()) {
          auto tempType = lookUp.find(templateTypes.back());
          memPool.release(tempType->second);
          lookUp.erase(tempType);
          templateTypes.pop_back();
        }
        break;
      }
      case GeneralDecType::TEMPLATE_CREATE: {
        // check that the template exists
        GeneralDec* dec = lookUp[tk.extractToken(list->curr.tempCreate->templateName)];
        if (!dec) {
          errors.emplace_back(CheckerErrorType::NO_SUCH_TEMPLATE, tk.tokenizerIndex, list->curr.tempCreate->templateName);
          break;
        } else if (dec->type != GeneralDecType::TEMPLATE) {
          errors.emplace_back(CheckerErrorType::NOT_A_TEMPLATE, tk.tokenizerIndex, list->curr.tempCreate->templateName, dec);
          break;
        }
        // check that the number of types match and that the types exist
        TokenList *tempList = &dec->tempDec->templateTypes, *createList = &list->curr.tempCreate->templateTypes;
        for (;tempList && createList; tempList = tempList->next, createList = createList->next) {
          if (createList->token.type == TokenType::IDENTIFIER) {
            GeneralDec *templateType = lookUp[tk.extractToken(createList->token)];
            if (!templateType) {
              errors.emplace_back(CheckerErrorType::NO_SUCH_TYPE, tk.tokenizerIndex, createList->token);
              tempList = nullptr;
              createList = nullptr;
              break;
            }
          }
        }
        if (tempList || createList) {
          if (createList) {
            errors.emplace_back(CheckerErrorType::WRONG_NUMBER_OF_ARGS, tk.tokenizerIndex, createList->token, dec);
          } else {
            errors.emplace_back(CheckerErrorType::WRONG_NUMBER_OF_ARGS, tk.tokenizerIndex, list->curr.tempCreate->templateTypes.token, dec);
          }
          break;
        }
        break;
        // have to deep copy the template declaration, replace all occurrences of templated types with the actual
      }
      default: break;
    }

  }
}

void Checker::fullScan() {
  for (GeneralDecList* list = &program.decs; list; list = list->next) {
    Tokenizer& tk = tokenizers[list->curr.tokenizerIndex];
    switch (list->curr.type) {
      case GeneralDecType::FUNCTION: {
        checkFunction(tk, *list->curr.funcDec);
        break;
      }
      
      default: break;
    }
  }
}
bool Checker::validateFunctionHeader(Tokenizer& tk, FunctionDec &funcDec) {
  bool valid = true;
  // check return type
  if (!checkType(tk, funcDec.returnType)) {
    if (errors.back().type == CheckerErrorType::VOID_TYPE) {
      errors.pop_back();
    } else {
      valid = false;
    }
  }
  // check parameters
  if (funcDec.params.curr.type != StatementType::NOTHING) {
    StatementList* params = &funcDec.params;
    do {
      if (!checkType(tk, params->curr.varDec->type)) {
        valid = false;
      }
      params = params->next;
    } while (params);
  }
  return valid;
}

void Checker::checkForStructCycles(GeneralDec &generalDec, std::vector<StructDec *>& structChain) {
  structChain.emplace_back(generalDec.structDec);
  // get the tokenizer for this declaration
  Tokenizer &tk = tokenizers[generalDec.tokenizerIndex];
  for (StructDecList *list = &generalDec.structDec->decs; list; list = list->next) {
    if (list->type != StructDecType::VAR) {
      continue;
    }
    // check for cycle
    TokenList* tokenList = &list->varDec->type;
    if (tokenList->token.type == TokenType::REFERENCE) {
      tokenList = tokenList->next;
    }
    if (tokenList->token.type != TokenType::IDENTIFIER) {
      continue;
    }
    GeneralDec *dec = lookUp[tk.extractToken(tokenList->token)];
    if (dec->structDec->checked) {
      continue; // dec already checked
    }
    for (StructDec *chainLink : structChain) {
      if (chainLink == dec->structDec) {
        // cycle found
        errors.emplace_back(CheckerErrorType::STRUCT_CYCLE, generalDec.tokenizerIndex, tokenList->token, dec);
        chainLink->hasCycle = true;
        break;
      }
    }
    if (!dec->structDec->hasCycle) {
      checkForStructCycles(*dec, structChain);
    }
  }
  generalDec.structDec->checked = true;
  structChain.pop_back();
}

void Checker::validateStructTopLevel(Tokenizer& tk, StructDec& structDec) {
  for (StructDecList *inner = &structDec.decs; inner; inner = inner->next) {
    if (inner->type == StructDecType::VAR) {
      checkType(tk, inner->varDec->type);
    }
    else if (inner->type == StructDecType::FUNC) {
      validateFunctionHeader(tk, *inner->funcDec);
    }
  }
}

/**
 * Validates the internals of a function
 * \param funcDec the function declaration to check
 * \returns true if the function is valid
 */
void Checker::checkFunction(Tokenizer& tk, FunctionDec& funcDec) {
  // validate parameter names
  std::vector<std::string> locals;
  if (funcDec.params.curr.type != StatementType::NOTHING) {
    StatementList *list = &funcDec.params;
    while (list) {
      locals.emplace_back(tk.extractToken(list->curr.varDec->name));
      GeneralDec* &paramDec = lookUp[locals.back()];
      if (paramDec) {
        errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, list->curr.varDec->name, paramDec);
        return;
      }
      // type already checked on second top level scan, just add it
      paramDec = memPool.makeGeneralDec();
      paramDec->varDec = list->curr.varDec;
      paramDec->type = GeneralDecType::VARIABLE;
      list = list->next;
    }
  }
  bool requireReturn = funcDec.returnType.token.type != TokenType::VOID;
  if (!checkScope(tk, funcDec.body, funcDec.returnType, false, false) && requireReturn) {
    errors.emplace_back(CheckerErrorType::NOT_ALL_CODE_PATHS_RETURN, tk.tokenizerIndex, funcDec.name);
  }
  while (!locals.empty()) {
    // remove locals from table
    lookUp.erase(locals.back());
    locals.pop_back();
  }
}

/**
 * \param scope The scope to check
 * \param locals name of all local variables allocated 
 * \param returnType the return type of the scope
 * \param isReturnRequired set to true if a return is required within this scope
 * \returns true if all code paths return a value
*/
bool Checker::checkScope(Tokenizer& tk, Scope& scope, TokenList& returnType, bool isLoop, bool isSwitch) {
  std::vector<std::string> locals;
  StatementList* list = &scope.scopeStatements;
  bool wasReturned = false;
  do {
    switch (list->curr.type) {
      case StatementType::CONTROL_FLOW: {
        switch (list->curr.controlFlow->type) {
          // ForLoop forLoop;
          case ControlFlowStatementType::FOR_LOOP: {
            auto& forLoop = *list->curr.controlFlow->forLoop;
            if (forLoop.initialize.type == StatementType::VARIABLE_DEC) {
              checkLocalVarDec(tk, *forLoop.initialize.varDec, locals);
            } else if (forLoop.initialize.type == StatementType::EXPRESSION) {
              checkExpression(tk, *forLoop.initialize.expression);
            } else if (forLoop.initialize.type != StatementType::NOTHING) {
              exit(1);
            }
            ResultingType res = checkExpression(tk, forLoop.condition);
            if (res.type->token.type != TokenType::BAD_VALUE && res.type->token.type != TokenType::NOTHING && !canBeConvertedToBool(*res.type)) {
              errors.emplace_back(CheckerErrorType::CANNOT_BE_CONVERTED_TO_BOOL, tk.tokenizerIndex, &forLoop.condition);
            }
            checkExpression(tk, forLoop.iteration);
            checkScope(tk, forLoop.body, returnType, isLoop, isSwitch);
            if (forLoop.initialize.type == StatementType::VARIABLE_DEC) {
              lookUp.erase(locals.back());
              locals.pop_back();
            }
            break;
          }
          case ControlFlowStatementType::CONDITIONAL_STATEMENT: {
            auto & cond = *list->curr.controlFlow->conditional;
            {
              ResultingType res = checkExpression(tk, cond.ifStatement.condition);
              if (res.type->token.type != TokenType::BAD_VALUE && 
              !canBeConvertedToBool(*res.type)) {
                errors.emplace_back(CheckerErrorType::CANNOT_BE_CONVERTED_TO_BOOL, tk.tokenizerIndex, &cond.ifStatement.condition);
              }
            }
            checkScope(tk, cond.ifStatement.body, returnType, isLoop, isSwitch);
            for(ElifStatementList* elifList = cond.elifStatement; elifList; elifList = elifList->next) {
              ResultingType res = checkExpression(tk, elifList->elif.condition);
              if (res.type->token.type != TokenType::BAD_VALUE && 
              !canBeConvertedToBool(*res.type)) {
                errors.emplace_back(CheckerErrorType::CANNOT_BE_CONVERTED_TO_BOOL, tk.tokenizerIndex, &cond.ifStatement.condition);
              }
              checkScope(tk, elifList->elif.body, returnType, isLoop, isSwitch);
            }
            if (cond.elseStatement) {
              checkScope(tk, *cond.elseStatement, returnType, isLoop, isSwitch);
            }
            break;
          }
          case ControlFlowStatementType::RETURN_STATEMENT: {
            wasReturned = true;
            ResultingType res = checkExpression(tk, list->curr.controlFlow->returnStatement->returnValue);
            if (res.type->token.type == TokenType::NOTHING && returnType.token.type == TokenType::VOID) {
              break; // ok
            }
            if (!checkAssignment(returnType, *res.type)) {
              errors.emplace_back(CheckerErrorType::INCORRECT_RETURN_TYPE, tk.tokenizerIndex, &list->curr.controlFlow->returnStatement->returnValue);
            }
            break;
          }
          case ControlFlowStatementType::SWITCH_STATEMENT: {
            break;
          }
          case ControlFlowStatementType::WHILE_LOOP: {
            checkExpression(tk, list->curr.controlFlow->whileLoop->statement.condition);
            checkScope(tk, list->curr.controlFlow->whileLoop->statement.body, returnType, isLoop, isSwitch);
            break;
          }
          case ControlFlowStatementType::NONE: {
            break;
          }
        }
        break;
      }
      
      case StatementType::EXPRESSION: {
        checkExpression(tk, *list->curr.expression);
        break;
      }
      
      case StatementType::KEYWORD: {
        if (list->curr.keyword.type == TokenType::CONTINUE) {
          if (!isLoop) {
            errors.emplace_back(CheckerErrorType::CANNOT_HAVE_CONTINUE_HERE, tk.tokenizerIndex, list->curr.keyword);
          }
          break;
        }
        else if (list->curr.keyword.type == TokenType::BREAK) {
          if (!isLoop && !isSwitch) {
            errors.emplace_back(CheckerErrorType::CANNOT_HAVE_BREAK_HERE, tk.tokenizerIndex, list->curr.keyword);
          }
          break;
        } else {
          // idk. are there any others?
          // nope
          exit(1);
        }
        break;
      }
      
      case StatementType::SCOPE: {
        checkScope(tk, *list->curr.scope, returnType, isLoop, isSwitch);
        break;
      }

      case StatementType::VARIABLE_DEC: {
        checkLocalVarDec(tk, *list->curr.varDec, locals);
        break;
      }

      case StatementType::NOTHING: {
        break;
      }
    }
    list = list->next;
  } while (list);
  while (!locals.empty()) {
    // remove locals from table
    lookUp.erase(locals.back());
    locals.pop_back();
  }
  return wasReturned;
}

bool Checker::checkLocalVarDec(Tokenizer& tk, VariableDec& varDec, std::vector<std::string>& locals) {
  // add local to table
  locals.emplace_back(tk.extractToken(varDec.name));
  GeneralDec*& dec = lookUp[locals.back()];
  if (dec) {
    errors.emplace_back(CheckerErrorType::NAME_ALREADY_IN_USE, tk.tokenizerIndex, varDec.name, dec);
    return false;
  }
  if (!checkType(tk, varDec.type)) {
    return false;
  }
  dec = memPool.makeGeneralDec();
  dec->type = GeneralDecType::VARIABLE;
  dec->varDec = &varDec;
  if (dec->varDec->initialAssignment) {
    ResultingType expressionType = checkExpression(tk, *varDec.initialAssignment);
    if (expressionType.type->token.type == TokenType::BAD_VALUE) {
      return false;
    }
    ResultingType varType {&varDec.type, true};
    if (!checkAssignment(*varType.type, *expressionType.type)) {
      errors.emplace_back(CheckerErrorType::CANNOT_ASSIGN, tk.tokenizerIndex, varDec.initialAssignment);
      return false;
    }
  }
  return true;
}

/**
 * Returns the resulting type from an expression
 * the ResultingType always contains a valid pointer
 * \param structMap pointer to a struct's lookup map. only used for the right side of binary member access operators
*/
ResultingType Checker::checkExpression(Tokenizer& tk, Expression& expression, std::map<std::string, StructDecList *>* structMap) {
  switch(expression.type) {
    case ExpressionType::BINARY_OP: {
      ResultingType leftSide = checkExpression(tk, expression.binOp->leftSide);

      if (expression.binOp->op.type == TokenType::LOGICAL_AND || expression.binOp->op.type == TokenType::LOGICAL_OR) {
        if (leftSide.type->token.type != TokenType::BAD_VALUE) {
          if (!canBeConvertedToBool(*leftSide.type)) {
            errors.emplace_back(CheckerErrorType::CANNOT_BE_CONVERTED_TO_BOOL, tk.tokenizerIndex, &expression.binOp->leftSide);
          }
        }
        ResultingType rightSide = checkExpression(tk, expression.binOp->rightSide);
        if (!canBeConvertedToBool(*rightSide.type)) {
          errors.emplace_back(CheckerErrorType::CANNOT_BE_CONVERTED_TO_BOOL, tk.tokenizerIndex, &expression.binOp->rightSide);
        }
        return {&boolValue, false};
      }
      
      if (isLogicalOp(expression.binOp->op.type)) {
        if (leftSide.type->token.type == TokenType::IDENTIFIER || leftSide.type->token.type == TokenType::VOID) {
          errors.emplace_back(CheckerErrorType::CANNOT_COMPARE_TYPE, tk.tokenizerIndex, &expression.binOp->leftSide);
        }
        ResultingType rightSide = checkExpression(tk, expression.binOp->leftSide);
        if (rightSide.type->token.type == TokenType::IDENTIFIER || rightSide.type->token.type == TokenType::VOID) {
          errors.emplace_back(CheckerErrorType::CANNOT_COMPARE_TYPE, tk.tokenizerIndex, &expression.binOp->rightSide);
        }
        return {&boolValue, false};
      }

      // member access or number with decimal
      if (expression.binOp->op.type == TokenType::DOT) {
        TokenType tkType = leftSide.type->token.type;
        if (tkType == TokenType::DECIMAL_NUMBER || tkType == TokenType::HEX_NUMBER || tkType == TokenType::BINARY_NUMBER) {
          if (expression.binOp->rightSide.type != ExpressionType::VALUE) {
            errors.emplace_back(CheckerErrorType::EXPECTING_NUMBER, tk.tokenizerIndex, &expression.binOp->rightSide);
          }
          else {
            tkType = expression.binOp->rightSide.value.type;
            if (tkType != TokenType::DECIMAL_NUMBER && tkType != TokenType::HEX_NUMBER && tkType != TokenType::BINARY_NUMBER) {
              errors.emplace_back(CheckerErrorType::EXPECTING_NUMBER, tk.tokenizerIndex, &expression.binOp->rightSide);
            }
          }
          return {&doubleValue, false};
        } else {
          if (leftSide.type->token.type == TokenType::BAD_VALUE) {
            return {&badValue, false};
          }
          return checkMemberAccess(tk, leftSide, expression);
        }
      }
      
      // pointer member access
      if (expression.binOp->op.type == TokenType::PTR_MEMBER_ACCESS) {
        if (leftSide.type->token.type == TokenType::BAD_VALUE) {
          return {&badValue, false};
        }
        if (leftSide.type->token.type != TokenType::POINTER) {
          errors.emplace_back(CheckerErrorType::CANNOT_DEREFERENCE_NON_POINTER_TYPE, tk.tokenizerIndex, expression.binOp->op);
          return {&badValue, false};
        }
        leftSide.type = leftSide.type->next;
        return checkMemberAccess(tk, leftSide, expression);
      }

      ResultingType rightSide = checkExpression(tk, expression.binOp->rightSide);
      if (isAssignment(expression.binOp->op.type)) {
        if (leftSide.type->token.type == TokenType::BAD_VALUE || rightSide.type->token.type == TokenType::BAD_VALUE) {
          return {&badValue, false};
        }
        if (!leftSide.isLValue) {
          errors.emplace_back(CheckerErrorType::CANNOT_ASSIGN_TO_TEMPORARY, tk.tokenizerIndex, &expression.binOp->leftSide);
        }
        else if (!checkAssignment(*leftSide.type, *rightSide.type)) {
          errors.emplace_back(CheckerErrorType::CANNOT_ASSIGN, tk.tokenizerIndex, &expression);
        }
        return {leftSide.type, true};
      }

      if (leftSide.type->token.type == TokenType::BAD_VALUE && rightSide.type->token.type == TokenType::BAD_VALUE) {
        return {&badValue, false};
      } else if (leftSide.type->token.type == TokenType::BAD_VALUE) {
        return {rightSide.type, false};
      } else if (rightSide.type->token.type == TokenType::BAD_VALUE) {
        return {leftSide.type, false};
      }

      if (leftSide.type->token.type == TokenType::IDENTIFIER || rightSide.type->token.type == TokenType::IDENTIFIER) {
        errors.emplace_back(CheckerErrorType::OPERATION_NOT_DEFINED, tk.tokenizerIndex, &expression);
        return {&badValue, false};
      }
      if (leftSide.type->token.type == TokenType::VOID || rightSide.type->token.type == TokenType::VOID) {
        errors.emplace_back(CheckerErrorType::OPERATION_ON_VOID, tk.tokenizerIndex, &expression);
        return {&badValue, false};
      }
      TokenList& largest = largestType(*leftSide.type, *rightSide.type);
      if (largest.token.type < TokenType::INT32_TYPE) {
        return {&int32Value, false};
      }
      return {&largest, false};
    }
    
    case ExpressionType::UNARY_OP: {
      if (expression.unOp->op.type == TokenType::DEREFERENCE) {
        ResultingType res = checkExpression(tk, expression.unOp->operand);
        if (res.type->token.type != TokenType::POINTER) {
          errors.emplace_back(CheckerErrorType::CANNOT_DEREFERENCE_NON_POINTER_TYPE, tk.tokenizerIndex, expression.unOp->op);
          return {&badValue, false};
        }
        return {res.type->next, true};
      }
      if (expression.unOp->op.type == TokenType::NOT) {
        ResultingType res = checkExpression(tk, expression.unOp->operand);
        if (!canBeConvertedToBool(*res.type)) {
          errors.emplace_back(CheckerErrorType::CANNOT_BE_CONVERTED_TO_BOOL, tk.tokenizerIndex, expression.unOp->op);
        }
        return {&boolValue, false};
      }
      if (expression.unOp->op.type == TokenType::ADDRESS_OF || expression.unOp->op.type == TokenType::INCREMENT_POSTFIX || expression.unOp->op.type == TokenType::INCREMENT_PREFIX || expression.unOp->op.type == TokenType::DECREMENT_PREFIX || expression.unOp->op.type == TokenType::DECREMENT_POSTFIX) {
        ResultingType res = checkExpression(tk, expression.unOp->operand);
        if (!res.isLValue) {
          errors.emplace_back(CheckerErrorType::CANNOT_OPERATE_ON_TEMPORARY, tk.tokenizerIndex, expression.unOp->op);
        }
        if (expression.unOp->op.type == TokenType::ADDRESS_OF) {
          TokenList *ptrToType = memPool.makeTokenList();
          ptrToType->token.type = TokenType::POINTER;
          ptrToType->next = res.type;
          return {ptrToType, false};
        }
        return {res.type, false};
      }
      if (expression.unOp->op.type == TokenType::NEGATIVE) {
        // nothing for now
        return {checkExpression(tk, expression.unOp->operand).type, false};
      }
      return {&badValue, false};
    }
    
    case ExpressionType::VALUE: {
      if (expression.value.type == TokenType::IDENTIFIER) {
        GeneralDec *decPtr;
        if (structMap) {
          StructDecList *structDec = (*structMap)[tk.extractToken(expression.value)];
          if (!structDec) {
            errors.emplace_back(CheckerErrorType::NO_SUCH_MEMBER_VARIABLE, tk.tokenizerIndex, expression.value);
            return {&badValue, false};
          }
          if (structDec->type != StructDecType::VAR) {
            errors.emplace_back(CheckerErrorType::NOT_A_VARIABLE, tk.tokenizerIndex, expression.value);
            return {&badValue, false};
          }
          decPtr = memPool.makeGeneralDec();
          decPtr->type = GeneralDecType::VARIABLE;
          decPtr->varDec = structDec->varDec;
        } else {
          decPtr = lookUp[tk.extractToken(expression.value)];
          if (!decPtr) {
            errors.emplace_back(CheckerErrorType::NO_SUCH_VARIABLE, tk.tokenizerIndex, expression.value);
            return {&badValue, false};
          }
          if (decPtr->type != GeneralDecType::VARIABLE) {
            errors.emplace_back(CheckerErrorType::NOT_A_VARIABLE, tk.tokenizerIndex, expression.value, decPtr);
            return {&badValue, false};
          }
        }
        if (decPtr->varDec->type.token.type == TokenType::REFERENCE) {
          return {decPtr->varDec->type.next, true};
        }
        return {&decPtr->varDec->type, true};
      }
      if (expression.value.type == TokenType::DECIMAL_NUMBER) {
        // need to get the actual number and see if it fits in a 32bit int, if not, unsigned, if not, 64bit
        // for now, just dump all numbers as ints
        return {&int32Value, false};
      }
      if (expression.value.type == TokenType::NULL_PTR) {
        return {&nullptrValue, false};
      }
      if (expression.value.type == TokenType::FALSE || expression.value.type == TokenType::TRUE) {
        return {&boolValue, false};
      }
      if (expression.value.type == TokenType::STRING_LITERAL) {
        return {&stringValue, false};
      }
      return {&charValue, false};
    }
    
    case ExpressionType::FUNCTION_CALL: {
      GeneralDec *decPtr;
      // member function
      if (structMap) {
        StructDecList *structDec = (*structMap)[tk.extractToken(expression.funcCall->name)];
        if (!structDec) {
          errors.emplace_back(CheckerErrorType::NO_SUCH_MEMBER_FUNCTION, tk.tokenizerIndex, expression.funcCall->name);
          return {&badValue, false};
        }
        if (structDec->type != StructDecType::FUNC) {
          errors.emplace_back(CheckerErrorType::NOT_A_FUNCTION, tk.tokenizerIndex, expression.funcCall->name);
          return {&badValue, false};
        }
        decPtr = memPool.makeGeneralDec();
        decPtr->type = GeneralDecType::FUNCTION;
        decPtr->funcDec = structDec->funcDec;
      }
      // normal function call
      else {
        decPtr = lookUp[tk.extractToken(expression.funcCall->name)];
        if (!decPtr) {
          // dec does not exist
          errors.emplace_back(CheckerErrorType::NO_SUCH_FUNCTION, tk.tokenizerIndex, expression.funcCall->name);
          return {&badValue, false};
        }
        if (decPtr->type != GeneralDecType::FUNCTION) {
          // not a function
          errors.emplace_back(CheckerErrorType::NOT_A_FUNCTION, tk.tokenizerIndex, expression.funcCall->name, decPtr);
          return {&badValue, false};
        }
      }
      // valid function, now check parameters
      // parameters are already validated on second top level scan. so assume the statements are all varDecs and valid
      StatementList* paramList = &decPtr->funcDec->params;
      ExpressionList* argList = &expression.funcCall->args;
      do {
        ResultingType resultingType = checkExpression(tk, argList->curr);
        if (resultingType.type->token.type != TokenType::BAD_VALUE) {
          if (!paramList->curr.varDec) {
            if (resultingType.type->token.type != TokenType::VOID) {
              errors.emplace_back(CheckerErrorType::WRONG_NUMBER_OF_ARGS, tk.tokenizerIndex, expression.funcCall->name, decPtr);
            }
          }
          else if (argList->curr.type == ExpressionType::NONE) {
            argList = nullptr;
            break;
          }
          else if (!checkAssignment(paramList->curr.varDec->type, *resultingType.type)) {
            // types dont match
            errors.emplace_back(CheckerErrorType::TYPE_DOES_NOT_MATCH, tk.tokenizerIndex, &argList->curr, decPtr);
          }
        }
        // type matches
        paramList = paramList->next;
        argList = argList->next;
      } while (argList && paramList);
      if (argList || paramList) {
        errors.emplace_back(CheckerErrorType::WRONG_NUMBER_OF_ARGS, tk.tokenizerIndex, expression.funcCall->name, decPtr);
      }
      if (decPtr->funcDec->returnType.token.type == TokenType::REFERENCE) {
        return {decPtr->funcDec->returnType.next, true};
      }
      return {&decPtr->funcDec->returnType, false};
    }
    
    case ExpressionType::ARRAY_ACCESS: {
      return {&badValue, false};
    }
    
    case ExpressionType::WRAPPED: {
      return checkExpression(tk, *expression.wrapped);
    }
    
    case ExpressionType::ARRAY_OR_STRUCT_LITERAL: {
      return {&badValue, false};
    }

    case ExpressionType::NONE: {
      return {&noneValue, false};
    }
    
    default: {
      exit(1);
    }
  }
}

/**
 * Validates a type
 * \param type the type to check
 * \returns true if the type is a valid type, false otherwise (adds the error to errors)
 * \note in the case of the type being just 'void', will return false even though it is valid for function return types.
 *  check if the emplaced error is 'void' and remove it if called for a function return type
*/
bool Checker::checkType(Tokenizer& tk, TokenList& type) {
  /**
   * Used to track the type info. 0 means we can have a ref, 0-2 means pointer, and 3 means an actual type was found
   * Can go forward, but cant go back. 
   * start -> 0
   * ref -> 1
   * ptr -> 2
   * type -> 3
   * Examples:
   * "ref ptr int"
   *    processing "ref" moves to 1, "ptr" moves to 2, and "int" to 3
   * 
   * "ptr"
   *    processing "ptr" moves to 2, no type after, error
   * 
   * "ptr ref int"
   *    processing "ptr" moves to 2, "ref" invalid since type is not 0, error
   * 
   * "ptr int int"
   *    processing "ptr" moves to 2, first "int" to 3, second "int" already at 3, error
   * 
  */
  uint8_t typeType = 0;

  CheckerErrorType errorType = CheckerErrorType::NONE;
  TokenList *list = &type;
  do {
    if (isBuiltInType(list->token.type)) {
      const TokenType tokenType = list->token.type;
      if (tokenType == TokenType::POINTER) {
        if (typeType == 3) {
          errorType = CheckerErrorType::UNEXPECTED_TYPE;
          break;
        }
        typeType = 2;
      }
      else {
        if (typeType == 3) {
          errorType = CheckerErrorType::CANNOT_HAVE_MULTI_TYPE;
          break;
        }
        typeType = 3;
      }
    }
    else if (list->token.type == TokenType::REFERENCE) {
      if (typeType == 1) {
        errorType = CheckerErrorType::CANNOT_REF_A_REF;
        break;
      } else if (typeType == 2) {
        errorType = CheckerErrorType::CANNOT_PTR_A_REF;
        break;
      } else if (typeType == 3) {
        errorType = CheckerErrorType::UNEXPECTED_TYPE;
        break;
      }
      typeType = 1;
    }
    else {
      if (typeType == 3) {
        errorType = CheckerErrorType::CANNOT_HAVE_MULTI_TYPE;
        break;
      }
      GeneralDec* &typeDec = lookUp[tk.extractToken(list->token)];
      if (!typeDec) {
        errorType = CheckerErrorType::NO_SUCH_TYPE;
        break;
      }
      if (typeDec->type != GeneralDecType::STRUCT) {
        errors.emplace_back(CheckerErrorType::EXPECTING_TYPE, tk.tokenizerIndex, list->token, typeDec);
        return false;
      }
      if (list->next) {
        errorType = CheckerErrorType::CANNOT_HAVE_MULTI_TYPE;
        break;
      }
      list->next = memPool.makeTokenList();
      list->next->token.type = TokenType::DEC_PTR;
      list->next->next = (TokenList *)&typeDec;
      return true;
    }
    list = list->next;
  } while (list);
  if (errorType == CheckerErrorType::NONE) {
    return true;
  }
  errors.emplace_back(errorType, tk.tokenizerIndex, list->token);
  return false;
}

ResultingType Checker::checkMemberAccess(Tokenizer& tk, ResultingType& leftSide, Expression& expression) {
  if (expression.binOp->rightSide.type == ExpressionType::VALUE) {
    if (expression.binOp->rightSide.value.type != TokenType::IDENTIFIER) {
      errors.emplace_back(CheckerErrorType::EXPECTED_IDENTIFIER, tk.tokenizerIndex, expression.binOp->rightSide.value);
      return {&badValue, false};
    }
  }
  else if (expression.binOp->rightSide.type != ExpressionType::FUNCTION_CALL && expression.binOp->rightSide.type != ExpressionType::ARRAY_ACCESS) {
    errors.emplace_back(CheckerErrorType::EXPECTED_IDENTIFIER, tk.tokenizerIndex, expression.binOp->rightSide.value);
    return {&badValue, false};
  }
  auto dec = lookUp[tk.extractToken(leftSide.type->token)];
  if (!dec || dec->type != GeneralDecType::STRUCT)  {
    errors.emplace_back(CheckerErrorType::NOT_A_STRUCT, tk.tokenizerIndex, &expression.binOp->leftSide);
    return {&badValue, false};
  }
  auto& structMap = structsLookUp.at(tk.extractToken(leftSide.type->token));
  return checkExpression(tk, expression.binOp->rightSide, &structMap);
}

bool checkAssignment(const TokenList& leftSide, const TokenList& rightSide) {
  if (leftSide.token.type == TokenType::VOID || rightSide.token.type == TokenType::VOID
    || leftSide.token.type == TokenType::BAD_VALUE || rightSide.token.type == TokenType::BAD_VALUE) {
    return false;
  }
  if (leftSide.token.type == TokenType::POINTER) {
    if (rightSide.token.type != TokenType::POINTER) {
      return rightSide.token.type == TokenType::NULL_PTR;
    }
    const TokenList* rType = &rightSide;
    const TokenList* lType = &leftSide;
    do {
      if (rType->token.type != lType->token.type) {
        if (rType->token.type == TokenType::VOID || lType->token.type == TokenType::VOID) {
          return true;
        }
        return false;
      }
      if (rType->token.type == TokenType::DEC_PTR) {
        return rType->next == lType->next;
      }
      rType = rType->next;
      lType = lType->next;
    } while (rType && lType);
  }
  else if (leftSide.token.type == TokenType::IDENTIFIER || rightSide.token.type == TokenType::IDENTIFIER) {
    if (leftSide.token.type != TokenType::IDENTIFIER || rightSide.token.type != TokenType::IDENTIFIER) {
      return false;
    }
    if (leftSide.next->token.type == TokenType::DEC_PTR || rightSide.next->token.type == TokenType::DEC_PTR) {
      return leftSide.next->next == rightSide.next->next;
    }
    return false;
  }
  return true;
}

// only builtin types can be converted to bool, except for void.
bool canBeConvertedToBool(TokenList& type) {
  return isBuiltInType(type.token.type) && type.token.type != TokenType::VOID;
}

TokenList& Checker::largestType(TokenList& typeA, TokenList& typeB) {
  if (typeA.token.type == TokenType::POINTER || typeB.token.type == TokenType::POINTER) {
    return ptrValue;
  }
  if (typeA.token.type > typeB.token.type) {
    return typeA;
  }
  return typeB;
}
