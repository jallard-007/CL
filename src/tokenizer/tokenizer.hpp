#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "../token.hpp"

struct Tokenizer {
  const std::string& content;
  Token peeked;
  const uint32_t size;
  uint32_t position;
  uint32_t lineNum;
  uint32_t lineStart;
  TokenType prevType;

  Tokenizer() = delete;

  explicit Tokenizer(const std::string&);

  std::vector<Token> tokenizeAll();
  Token tokenizeNext();
  Token peekNext();
  void consumePeek();

  void moveToNextNonWhiteSpaceChar();
  void movePastKeywordOrIdentifier();
  void movePastNumber();
  void movePastHexNumber();
  void movePastLiteral(char);
  void moveToNewLine();
  std::string extractToken(Token);
};
