//
// Created by Edot on 2023/10/9.
//

#ifndef EXP_CPP_ENV_H_
#define EXP_CPP_ENV_H_
#include <map>
#include <string>

class TokenValue;

class Env{
  public:
    explicit Env(Env *prev);
    Env *GetPrev() const;

    bool TryPut(const std::string &name, TokenValue *entry);
    TokenValue *Get(const std::string &name);
    bool disabled = false; // 如果处于抑制中间变量生成的时期，这个变量会被设置为真，此时的TryPut会报出错误，方便排查
  private:
    Env *prev = nullptr;
    int current_address = 0;
    std::map<std::string, TokenValue *> symbol_table;
};

#endif //EXP_CPP_ENV_H_
