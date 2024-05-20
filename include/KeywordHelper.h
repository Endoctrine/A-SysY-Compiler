//
// Created by Edot on 2023/9/18.
//

#ifndef EXP2_CPP_KEYWORD_HELPER_H_
#define EXP2_CPP_KEYWORD_HELPER_H_
#include <unordered_set>
#include <string>

class Token;

class KeywordHelper{
  public:
    static bool IsKeyword(const char *word);
    static Token MakeKeyword(const char *keyword, int line_number);
  private:
    static std::unordered_set<std::string> keyword_set;
};

#endif //EXP2_CPP_KEYWORD_HELPER_H_
