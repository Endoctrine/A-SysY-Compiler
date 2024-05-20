//
// Created by Edot on 2023/9/18.
//

#include "KeywordHelper.h"
#include "Token.h"

std::unordered_set<std::string> KeywordHelper::keyword_set = {
    "main",
    "const",
    "int",
    "break",
    "continue",
    "if",
    "else",
    "for",
    "getint",
    "printf",
    "return",
    "void"
};

bool KeywordHelper::IsKeyword(const char *word){
    return keyword_set.find(word) != keyword_set.end();
}

Token KeywordHelper::MakeKeyword(const char *keyword, int line_number){
    std::string keyword_str(keyword);
    if(keyword_str == "main"){
        return {MAINTK, nullptr, keyword, line_number};
    } else if (keyword_str == "const"){
        return {CONSTTK, nullptr, keyword, line_number};
    } else if (keyword_str == "int"){
        return {INTTK, nullptr, keyword, line_number};
    } else if (keyword_str == "break"){
        return {BREAKTK, nullptr, keyword, line_number};
    } else if (keyword_str == "continue"){
        return {CONTINUETK, nullptr, keyword, line_number};
    } else if (keyword_str == "if"){
        return {IFTK, nullptr, keyword, line_number};
    } else if(keyword_str == "else"){
        return {ELSETK, nullptr, keyword, line_number};
    } else if(keyword_str == "for"){
        return {FORTK, nullptr, keyword, line_number};
    } else if(keyword_str == "getint"){
        return {GETINTTK, nullptr, keyword, line_number};
    } else if(keyword_str == "printf"){
        return {PRINTFTK, nullptr, keyword, line_number};
    } else if(keyword_str == "return"){
        return {RETURNTK, nullptr, keyword, line_number};
    } else if(keyword_str == "void"){
        return {VOIDTK, nullptr, keyword, line_number};
    } else{
        return {ENDTK, nullptr, nullptr, line_number};
    }
}
