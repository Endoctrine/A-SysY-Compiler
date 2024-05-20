//
// Created by Edot on 2023/10/25.
//

#ifndef EXP_CPP_INTER_CODE_H_
#define EXP_CPP_INTER_CODE_H_
#include <map>
#include <string>

class TokenValue;
class Token;

namespace InterCodeNS{
enum Operator{
    // 基本的算数运算
    ADD, // 加
    SUB, // 减
    MUL, // 乘
    DIV, // 除
    MOD, // 模
    POS, // 取原有值，其实就是啥也不做，可以直接优化掉
    NEG, // 取相反数
    // 布尔运算
    LSS, // 小于
    GRE, // 大于
    LEQ, // 小于等于
    GEQ, // 大于等于
    EQ, // 等于
    NEQ, // 不等于
    AND, // 且
    OR, // 或
    NOT, // 非
    // 变量赋值
    ASS, // 赋值
    // 引用
    REF, // ARRAY INDEX ADDR
    // 解引用
    LOAD, // ADDR _ T
    // 地址赋值
    STORE, // VALUE _ ADDR
    // 跳转
    BR, // 无条件跳转
    BRT, // 为真时跳转
    BRF, // 为假时跳转
    // 函数开始
    FUNC, // 一般函数开始
    MAIN, // 主函数开始
    // 函数调用
    PARAM, // 传递参数
    CALL, // 调用函数
    // 函数返回
    RETVAL, // 有返回值的返回
    RET, // 无返回值的返回
    // 输入输出
    GETINT, // 用于 getint 函数
    PRINTF, // 打印格式化符号 NAME _ _
    PRINT, // 打印单个数字 NAME/TEMP/IMM _ _
    // 扩展指令（用于辅助四元式或目标代码生成）
    BREAK, // 循环中 break 的占位
    CONTINUE, // 循环中 continue 的占位
    FORIT, // for 循环修改循环变量语句的开始
    PASS, // 空语句，占位子用的（例如 FORIT 使用结束之后就改成这玩意占位）
    CONDEF, // 定义常量的位点
};
enum ItemType{
    NAME,
    IMM,
    ID,
    EMPTY,
};

union ItemValue{
    long long numeric_value;
    TokenValue *name_value;
};
}

struct UglyQuaternionItem{
    enum InterCodeNS::ItemType type = InterCodeNS::EMPTY;
    union InterCodeNS::ItemValue value = {};
};

struct UglyQuaternion{
    UglyQuaternion(int id,
                   InterCodeNS::Operator op,
                   const UglyQuaternionItem &arg_left,
                   const UglyQuaternionItem &arg_right,
                   const UglyQuaternionItem &result);
    void Print() const;
    int id;
    enum InterCodeNS::Operator op;
    UglyQuaternionItem arg_left;
    UglyQuaternionItem arg_right;
    UglyQuaternionItem result;
};

class InterCode{
  public:
    int AddCode(InterCodeNS::Operator op,
                 const UglyQuaternionItem &arg_left,
                 const UglyQuaternionItem &arg_right,
                 const UglyQuaternionItem &result);
    static UglyQuaternionItem MakeLabel(int id);
    static UglyQuaternionItem MakeImm(int imm);
    static UglyQuaternionItem MakeName(TokenValue *entry);
    UglyQuaternion *GetQuaternion(int id);
    int GetCurrentId() const;
    std::string GetTempName();
    void ForItBegin();
    void ForEnd(int it_start);
    void AddForBreak();
    void AddForContinue();
    void AddMainFunc();
    void Print();
    std::map<int, UglyQuaternion*> ugly_codes;
  private:
    int current_id = 1;
    int current_temp = 0;
};

#endif //EXP_CPP_INTER_CODE_H_
