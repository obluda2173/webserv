#ifndef TOKENSTREAM_H
#define TOKENSTREAM_H

#include <cctype>
#include <set>
#include <string>
#include <vector>

enum TokenType {
    IDENTIFIER,
    STRING,
    PUNCT,
    END_OF_FILE,
};

typedef struct Token {
    TokenType type;
    std::string value;
    int line, column;
    Token(TokenType type_, std::string value_, int line_, int column_)
        : type(type_), value(value_), line(line_), column(column_) {}
    Token() {}
} Token;

class TokenStream {
  private:
    std::string _text;
    std::vector< Token > _tokens;
    size_t _pos, _index;
    int _line, _col;
    std::string _tokenDesc(TokenType ttype, const std::string& tvalue) const;
    void _tokenize();
    void _advance();
    void _expectChar(char expected);

  public:
    TokenStream(const std::string& input);
    const Token& peek(int ahead = 0) const;
    Token next();
    void unget();
    bool hasMore() const;
    bool accept(TokenType ttype, const std::string& tvalue = "");
    void expect(TokenType ttype, const std::string& tvalue = "");
    std::vector< std::string > collectArguments(const std::set< std::string >& terminators,
                                                std::vector< std::string > invalidArgs);
};

std::string toString(int value);

#endif // TOKENSTREAM_H
