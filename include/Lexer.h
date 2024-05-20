//
// Created by Edot on 2023/9/18.
//

#ifndef EXP2_CPP_LEXER_H_
#define EXP2_CPP_LEXER_H_
#include <vector>

class Token;
class FileReader;

class Lexer{
  public:
    explicit Lexer(const char *path);
    ~Lexer();
    Token NextToken();

  private:
    void Trim();
    bool SkipWhitespace();
    bool SkipComment();
    Token GetIdentOrKeyword();
    Token GetIntConst();
    Token GetFormatString();
    Token GetOpOrDelimiter();
    void HandleError();

    std::vector<Token> buffer;
    FileReader *file_reader;
    int line_number;
};

#endif //EXP2_CPP_LEXER_H_
