//
// Created by Edot on 2023/9/18.
//

#include "AlphabetHelper.h"

bool AlphabetHelper::IsAlphabet(const char &c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool AlphabetHelper::IsNumber(const char &c){
    return c >= '0' && c <= '9';
}

bool AlphabetHelper::IsWhitespace(const char &c){
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

