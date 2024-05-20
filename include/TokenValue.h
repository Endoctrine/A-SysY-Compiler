//
// Created by Edot on 2023/10/9.
//

#ifndef EXP_CPP_TOKEN_VALUE_H_
#define EXP_CPP_TOKEN_VALUE_H_
#include <string>
#include <vector>

enum IdClass{
    CONST_INT,
    VAR_INT,
    FUNC,
    TEMP_INT,
    TEMP_STR
};
typedef enum IdClass IdClass;

class TokenValue{
  public:
    TokenValue(std::string name, IdClass id_class, int id_dimension, bool is_pointer);
    void AddValue(int value);
    void AddAxis(int axis);
    void AddParameter(TokenValue *parameter);
    int GetIdDimension() const;
    const std::string &GetName() const;

    int GetValue() const;
    int GetValue(int index) const;
    int GetValue(int index_1, int index_2) const;
    int GetValue(const std::vector<int> &indexes);

    int ValuesSize() const;
    int Size() const;
    int ParametersSize() const;
    bool IsParamTypeMatched(std::vector<int> &params_dimensions) const;
    bool IsSameType(TokenValue *token_value) const;
    bool NotVar() const;
    bool IsTemp() const;
    int TypeSize() const;

  public:
    int address = -1;
    bool is_global = false;
    bool is_pointer;
    bool is_param = false; // ���������������ǲ��Ǻ���������
    // ^��ʲô���أ���������֮���ȫ�ּĴ��������������ʶ����������������Ҳ���������������ȫ�ּĴ���
    std::string strcon_value;
    IdClass id_class;
    std::vector<int> shape;
    std::vector<int> values;

  private:
    std::string name;
    int id_dimension;
    std::vector<TokenValue *> parameters;
};

#endif //EXP_CPP_TOKEN_VALUE_H_
