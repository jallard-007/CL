#include <catch2/catch_test_macros.hpp>
#include "../parser/parser.hpp"

NodeMemPool memPool2;

TEST_CASE("Something", "[prettyPrinter]") {
  const std::string str = 
R"(func getType(type: Type ref): Token {
  tp: Token = tokenizer.peekNext();
  prev: TokenList ptr = nullptr;
  curr: TokenList ptr = @type.tokens;
  while (tp.type != TokenType.END_OF_FILE) {
    if (isTypeDelimiter(tp.type)) {
      if (curr->next) {
        memPool.release(curr->next);
        curr->next = nullptr;
      }
      if (curr && curr->curr.type == TokenType.NOTHING) {
        prev->next = nullptr;
        memPool.release(curr);
      }
      break;
    }
    tokenizer.consumePeek();
    curr->curr = tp;
    curr->next = memPool.getTokenList();
    prev = curr;
    curr = curr->next;
    tp = tokenizer.peekNext();
  }
  return tp;
}
)";
  std::vector<Tokenizer> tks;
  tks.emplace_back("./src/prettyPrint/test_prettyPrint.cpp", str);
  Parser parser{tks.back(), memPool2};
  REQUIRE(parser.parse());
  REQUIRE(parser.expected.empty());
  REQUIRE(parser.unexpected.empty());
  std::string output;
  parser.program.prettyPrint(tks, output);
  CHECK(str == output);
}

TEST_CASE("Other", "[prettyPrinter]") {
  const std::string str = 
R"(func getType(type: Type ref): Token {
  if (1) {
  }
  elif (1) {
  }
  else {
  }
  switch x {
    case 2
    case 3
    case 4 {
    }
    default {
    }
  }
}
)";
  std::vector<Tokenizer> tks;
  tks.emplace_back("./src/prettyPrint/test_prettyPrint.cpp",  str);
  Parser parser{tks.back(), memPool2};
  parser.parse();
  REQUIRE(parser.expected.empty());
  REQUIRE(parser.unexpected.empty());
  std::string output;
  parser.program.prettyPrint(tks, output);
  CHECK(str == output);
}