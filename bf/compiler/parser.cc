#include "bf/compiler/parser.h"

#include <iterator>

namespace dev::spiralgerbil::bf {
namespace {

NodeList ParseNodeList(std::istream* token_stream, bool head) {
  NodeList nodes;
  int repeat_count = 0;
  char token;
  while (token = token_stream->get(), !token_stream->eof()) {
    switch (token) {
      case '>':
        repeat_count++;
        if (token_stream->peek() != '>') {
          nodes.emplace_back<ast::Move>(repeat_count);
          repeat_count = 0;
        }
        break;
      case '<':
        repeat_count++;
        if (token_stream->peek() != '<') {
          nodes.emplace_back<ast::Move>(-repeat_count);
          repeat_count = 0;
        }
        break;
      case '+':
        repeat_count++;
        if (token_stream->peek() != '+') {
          nodes.emplace_back<ast::Add>(repeat_count);
          repeat_count = 0;
        }
        break;
      case '-':
        repeat_count++;
        if (token_stream->peek() != '-') {
          nodes.emplace_back<ast::Add>(-repeat_count);
          repeat_count = 0;
        }
        break;
      case '.':
        nodes.emplace_back<ast::Output>();
        break;
      case ',':
        nodes.emplace_back<ast::Input>();
        break;
      case '[':
        nodes.emplace_back<ast::Loop>(ParseNodeList(token_stream, false));
        break;
      case ']':
        return nodes;
      default:
        break;
    }
  }
  LOG_IF(FATAL, !head) << "Unmatched [";
  return nodes;
}

}  // namespace


std::unique_ptr<ast::Tree> Parse(std::istream* token_stream) {
  NodeList program = ParseNodeList(token_stream, true);
  if (!token_stream->eof()) {
    LOG(ERROR) << "Unmatched ]";
  }
  return std::make_unique<ast::Tree>(std::move(program));
}

}  // namespace dev::spiralgerbil::bf