//
// Created by Edot on 2023/9/18.
//

#ifndef EXP2_CPP_TOKEN_H_
#define EXP2_CPP_TOKEN_H_
#include <string>

enum TokenType{
    IDENFR,
    INTCON,
    STRCON,
    MAINTK,
    CONSTTK,
    INTTK,
    BREAKTK,
    CONTINUETK,
    IFTK,
    ELSETK,
    NOT,
    AND,
    OR,
    FORTK,
    GETINTTK,
    PRINTFTK,
    RETURNTK,
    PLUS,
    MINU,
    VOIDTK,
    MULT,
    DIV,
    MOD,
    LSS,
    LEQ,
    GRE,
    GEQ,
    EQL,
    NEQ,
    ASSIGN,
    SEMICN,
    COMMA,
    LPARENT,
    RPARENT,
    LBRACK,
    RBRACK,
    LBRACE,
    RBRACE,
    ENDTK
};
typedef enum TokenType TokenType;

class TokenValue;

class Token{
  public:
    Token(TokenType token_type, const TokenValue *token_value, const char *lexeme, int line_number);
    ~Token() = default;
    TokenType GetTokenType() const;
    const TokenValue *GetTokenValue() const;
    const std::string &GetLexeme() const;
    int GetLineNumber() const;
    std::string ToString() const;

    bool IsFormatLegal() const;
    int FormatterCount() const;

  private:
    TokenType token_type;
    const TokenValue *token_value;
    std::string lexeme;
    int line_number;
};

#endif //EXP2_CPP_TOKEN_H_
