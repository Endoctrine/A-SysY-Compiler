//
// Created by Edot on 2023/9/18.
//

#include <sstream>
#include "Token.h"
#include <iostream>

Token::Token(TokenType token_type, const TokenValue *token_value, const char *lexeme, int line_number)
    : token_type(token_type), token_value(token_value), line_number(line_number){
    if(lexeme != nullptr){
        this->lexeme = lexeme;
    }
}

TokenType Token::GetTokenType() const{
    return token_type;
}

const TokenValue *Token::GetTokenValue() const{
    return token_value;
}

const std::string &Token::GetLexeme() const{
    return lexeme;
}

int Token::GetLineNumber() const{
    return line_number;
}

std::string Token::ToString() const{
    std::stringstream ss;
//    ss << line_number << ": ";
    switch(token_type){
        case IDENFR:ss << "IDENFR" << " " << lexeme;
            break;
        case INTCON:ss << "INTCON" << " " << lexeme;
            break;
        case STRCON:ss << "STRCON" << " " << lexeme;
            break;
        case MAINTK:ss << "MAINTK" << " " << lexeme;
            break;
        case CONSTTK:ss << "CONSTTK" << " " << lexeme;
            break;
        case INTTK:ss << "INTTK" << " " << lexeme;
            break;
        case BREAKTK:ss << "BREAKTK" << " " << lexeme;
            break;
        case CONTINUETK:ss << "CONTINUETK" << " " << lexeme;
            break;
        case IFTK:ss << "IFTK" << " " << lexeme;
            break;
        case ELSETK:ss << "ELSETK" << " " << lexeme;
            break;
        case NOT:ss << "NOT" << " " << lexeme;
            break;
        case AND:ss << "AND" << " " << lexeme;
            break;
        case OR:ss << "OR" << " " << lexeme;
            break;
        case FORTK:ss << "FORTK" << " " << lexeme;
            break;
        case GETINTTK:ss << "GETINTTK" << " " << lexeme;
            break;
        case PRINTFTK:ss << "PRINTFTK" << " " << lexeme;
            break;
        case RETURNTK:ss << "RETURNTK" << " " << lexeme;
            break;
        case PLUS:ss << "PLUS" << " " << lexeme;
            break;
        case MINU:ss << "MINU" << " " << lexeme;
            break;
        case VOIDTK:ss << "VOIDTK" << " " << lexeme;
            break;
        case MULT:ss << "MULT" << " " << lexeme;
            break;
        case DIV:ss << "DIV" << " " << lexeme;
            break;
        case MOD:ss << "MOD" << " " << lexeme;
            break;
        case LSS:ss << "LSS" << " " << lexeme;
            break;
        case LEQ:ss << "LEQ" << " " << lexeme;
            break;
        case GRE:ss << "GRE" << " " << lexeme;
            break;
        case GEQ:ss << "GEQ" << " " << lexeme;
            break;
        case EQL:ss << "EQL" << " " << lexeme;
            break;
        case NEQ:ss << "NEQ" << " " << lexeme;
            break;
        case ASSIGN:ss << "ASSIGN" << " " << lexeme;
            break;
        case SEMICN:ss << "SEMICN" << " " << lexeme;
            break;
        case COMMA:ss << "COMMA" << " " << lexeme;
            break;
        case LPARENT:ss << "LPARENT" << " " << lexeme;
            break;
        case RPARENT:ss << "RPARENT" << " " << lexeme;
            break;
        case LBRACK:ss << "LBRACK" << " " << lexeme;
            break;
        case RBRACK:ss << "RBRACK" << " " << lexeme;
            break;
        case LBRACE:ss << "LBRACE" << " " << lexeme;
            break;
        case RBRACE:ss << "RBRACE" << " " << lexeme;
            break;
        case ENDTK:ss << "ENDTK";
            break;
    }
    return ss.str();
}

bool Token::IsFormatLegal() const{
    if(token_type != STRCON){
        perror("token type mismatch!");
        return false;
    }
    if(lexeme[lexeme.size() - 1] != '"'){
        return false;
    }
    for(int i = 1; i < lexeme.size() - 1; ++i){
        if(lexeme[i] == '\\'){
            if(lexeme[i + 1] != 'n'){
                return false;
            }
        } else if(lexeme[i] == '%'){
            if(lexeme[i + 1] != 'd'){
                return false;
            }
        } else{
            if(!(lexeme[i] == 32 || lexeme[i] == 33 || (lexeme[i] >= 40 && lexeme[i] <= 126))){
                return false;
            }
        }
    }
    return true;
}

int Token::FormatterCount() const{
    if(token_type != STRCON){
        perror("token type mismatch!");
        return 0;
    }
    int ans = 0;
    for(int i = 0; i < lexeme.size() - 1; ++i){
        if(lexeme[i] == '%' && lexeme[i + 1] == 'd'){
            ans++;
        }
    }
    return ans;
}
