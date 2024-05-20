//
// Created by Edot on 2023/9/18.
//

#include "Lexer.h"
#include "Token.h"
#include "FileReader.h"
#include "AlphabetHelper.h"
#include "KeywordHelper.h"

Lexer::Lexer(const char *path){
    file_reader = new FileReader(path);
    line_number = 1;
}

Lexer::~Lexer(){
    delete file_reader;
}

Token Lexer::NextToken(){
    Trim();
    char forward;
    bool r = file_reader->GetChar(forward);
    if(!r){
        return {ENDTK, nullptr, nullptr, line_number};
    } else if(AlphabetHelper::IsNumber(forward)){
        file_reader->PutBack();
        return GetIntConst();
    } else if(AlphabetHelper::IsAlphabet(forward)){
        file_reader->PutBack();
        return GetIdentOrKeyword();
    } else if(forward == '"'){
        file_reader->PutBack();
        return GetFormatString();
    } else{
        file_reader->PutBack();
        return GetOpOrDelimiter();
    }
}

void Lexer::Trim(){
    while(SkipWhitespace() || SkipComment());
}

bool Lexer::SkipWhitespace(){
    char forward;
    bool r = file_reader->GetChar(forward);
    bool valid_skip = false;
    while(AlphabetHelper::IsWhitespace(forward) && r){
        if(forward == '\n'){
            line_number++;
        }
        r = file_reader->GetChar(forward);
        valid_skip = true;
    }
    file_reader->PutBack();
    return valid_skip;
}

bool Lexer::SkipComment(){
    char forward;
    bool r = file_reader->GetChar(forward);
    bool valid_skip = false;
    if(!r){
        return valid_skip;
    }
    if(forward != '/'){
        file_reader->PutBack();
        return valid_skip;
    }
    r = file_reader->GetChar(forward);
    if(!r || (forward != '/' && forward != '*')){
        file_reader->PutBack();
        file_reader->PutBack();
        return valid_skip;
    }
    valid_skip = true;
    if(forward == '/'){
        while(r && forward != '\n'){
            r = file_reader->GetChar(forward);
            if(r && forward == '\n'){
                line_number++;
            }
        }
        return valid_skip;
    }
    if(forward == '*'){
        r = file_reader->GetChar(forward);
        while(true){
            while(r && forward != '*'){
                if(forward == '\n'){
                    line_number++;
                }
                r = file_reader->GetChar(forward);
            }
            r = file_reader->GetChar(forward);
            if(r && forward == '\n'){
                line_number++;
            }
            if(!r || forward == '/'){
                return valid_skip;
            }
        }
    }
    return valid_skip;
}

Token Lexer::GetIdentOrKeyword(){
    std::string lexeme;
    char forward;
    bool r = file_reader->GetChar(forward);
    lexeme += forward;

    r = file_reader->GetChar(forward);
    while((AlphabetHelper::IsNumber(forward) || AlphabetHelper::IsAlphabet(forward) || forward == '_') && r){
        lexeme += forward;
        r = file_reader->GetChar(forward);
    }

    file_reader->PutBack();
    if(KeywordHelper::IsKeyword(lexeme.c_str())){
        return KeywordHelper::MakeKeyword(lexeme.c_str(), line_number);
    } else{
        return {IDENFR, nullptr, lexeme.c_str(), line_number};
    }
}

Token Lexer::GetIntConst(){
    std::string lexeme;
    char forward;
    bool r = file_reader->GetChar(forward);

    while(AlphabetHelper::IsNumber(forward) && r){
        lexeme += forward;
        r = file_reader->GetChar(forward);
    }

    file_reader->PutBack();
    return {INTCON, nullptr, lexeme.c_str(), line_number};
}

Token Lexer::GetFormatString(){
    std::string lexeme = "\"";
    char forward;
    /*given that the first forward must be a quote, eat it.*/
    bool r = file_reader->GetChar(forward);
    r = file_reader->GetChar(forward);
    if(!r){
        HandleError();
    }
    while(forward != '"'){
        lexeme += forward;
        r = file_reader->GetChar(forward);
        if(!r){
            HandleError();
        }
    }
    lexeme += forward;

    /*we don't need to putback given that getchar ends when we get a quote*/
    return {STRCON, nullptr, lexeme.c_str(), line_number};
}

Token Lexer::GetOpOrDelimiter(){
    char forward;
    bool r = file_reader->GetChar(forward);
    switch(forward){
        case '!':r = file_reader->GetChar(forward);
            if(forward == '=' && r){
                return {NEQ, nullptr, "!=", line_number};
            } else{
                file_reader->PutBack();
                return {NOT, nullptr, "!", line_number};
            }
        case '&':r = file_reader->GetChar(forward);
            if(forward != '&' || !r){
                HandleError();
            }
            return {AND, nullptr, "&&", line_number};
        case '|':r = file_reader->GetChar(forward);
            if(forward != '|' || !r){
                HandleError();
            }
            return {OR, nullptr, "||", line_number};
        case '+':return {PLUS, nullptr, "+", line_number};
        case '-':return {MINU, nullptr, "-", line_number};
        case '*':return {MULT, nullptr, "*", line_number};
        case '/':return {DIV, nullptr, "/", line_number};
        case '%':return {MOD, nullptr, "%", line_number};
        case '<':r = file_reader->GetChar(forward);
            if(forward == '=' && r){
                return {LEQ, nullptr, "<=", line_number};
            } else{
                file_reader->PutBack();
                return {LSS, nullptr, "<", line_number};
            }
        case '>':r = file_reader->GetChar(forward);
            if(forward == '=' && r){
                return {GEQ, nullptr, ">=", line_number};
            } else{
                file_reader->PutBack();
                return {GRE, nullptr, ">", line_number};
            }
        case '=':r = file_reader->GetChar(forward);
            if(forward == '=' && r){
                return {EQL, nullptr, "==", line_number};
            } else{
                file_reader->PutBack();
                return {ASSIGN, nullptr, "=", line_number};
            }
        case ';':return {SEMICN, nullptr, ";", line_number};
        case ',':return {COMMA, nullptr, ",", line_number};
        case '(':return {LPARENT, nullptr, "(", line_number};
        case ')':return {RPARENT, nullptr, ")", line_number};
        case '[':return {LBRACK, nullptr, "[", line_number};
        case ']':return {RBRACK, nullptr, "]", line_number};
        case '{':return {LBRACE, nullptr, "{", line_number};
        default:return {RBRACE, nullptr, "}", line_number};
    }
}

void Lexer::HandleError(){
    /*no use now*/
}
