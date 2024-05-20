//
// Created by Edot on 2023/11/1.
//

#ifndef EXP_CPP_QUATERNION_H_
#define EXP_CPP_QUATERNION_H_
#include <string>
#include <vector>
#include "InterCode.h"

namespace QuaternionNS{
enum ItemType{ // 不需要EMPTY了，EMPTY就是nullptr
    IMM,
    LABEL,
    NAME
};
enum NameType{
    INT,
    INT_ARRAY,
    INT_POINTER,
    STR,
    FUNC
};
struct QuaternionItem{
    explicit QuaternionItem(ItemType item_type);
    virtual ~QuaternionItem() = default;
    virtual void Print() = 0;
    QuaternionNS::ItemType item_type;
};
struct ImmItem : public QuaternionItem{
    explicit ImmItem(int value);
    void Print() override;
    int value;
};
struct LabItem : public QuaternionItem{
    explicit LabItem(int lab);
    void Print() override;
    int lab;
};
struct NameItem : public QuaternionItem{
    NameItem(NameType name_type, int address, long long name);
    int PassiveFrameSize();
    QuaternionNS::NameType name_type;
    int address;
    long long name;
};
struct IntItem : public NameItem{
    IntItem(int address, bool if_global, bool if_const, long long name);
    void Print() override;
    bool if_global;
    bool if_const;
    bool if_param = false;
    bool if_register = false;
    int register_id;
    std::vector<int> const_init_val;
};
struct IntArrayItem : public NameItem{
    IntArrayItem(int address, bool if_global, int dimension, bool if_const, long long name);
    void AddAxis(int axis);
    void Print() override;
    bool if_global;
    int dimension; // dimension === axes.size;
    std::vector<int> axes;
    bool if_const;
    std::vector<int> const_init_val;
};
struct IntPointerItem : public NameItem{
    IntPointerItem(int address, bool if_global, int dimension, long long name);
    void AddAxis(int axis);
    void Print() override;
    bool if_global;
    int dimension; // dimension === axes.size + 1;
    std::vector<int> axes;
};
struct StrItem : public NameItem{
    StrItem(std::string value, long long name); // name其实没啥用
    void Print() override;
    std::string value;
};
struct FuncItem : public NameItem{
    explicit FuncItem(int parameter_num, long long name);
    void Print() override;
    int parameter_num;
    int frame_size = 0;
};
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
    REF, // REF INT_ARRAY_NAME/INT_POINTER_NAME INT_NAME/IMM INT_POINTER_NAME
    // 解引用
    LOAD, // LOAD INT_ARRAY_NAME/INT_POINTER_NAME _ NAME
    // 地址赋值
    STORE, // STORE INT_NAME/IMM _ INT_ARRAY_NAME/INT_POINTER_NAME
    // 跳转
    BR, // 无条件跳转
    BRT, // 为真时跳转
    BRF, // 为假时跳转
    // 函数开始
    FUNCDEF, // 一般函数开始（FUNCDEF FUNC_NAME _ _）
    MAIN, // 主函数开始（MAIN FUNC_NAME _ _）
    // 函数调用
    PARAM, // PARAM INT_NAME/INT_ARRAY_NAME/INT_POINTER_NAME/IMM _ _
    CALL, // CALL FUNC_NAME IMM INT_NAME
    // 函数返回
    RETVAL, // RETVAL INT_NAME/IMM _ FUNC_NAME（FUNC_NAME为所属函数，可以获得栈帧大小）
    RET, // RET _ _ FUNC_NAME（FUNC_NAME为所属函数，可以获得栈帧大小）
    // 输入输出
    GETINT, // 用于 getint 函数
    PRINTF, // PRINTF STR_NAME _ _
    PRINT, // PRINT INT_NAME/IMM _ _
    // 扩展指令（用于辅助四元式或目标代码生成）
    PASS, // 空语句，占位子用的
    CONDEF, // CONDEF INT_NAME/INT_ARRAY_NAME _ _（定义常量的位点）
};
}

struct Quaternion{
    Quaternion(QuaternionNS::Operator op,
               QuaternionNS::QuaternionItem *arg_left,
               QuaternionNS::QuaternionItem *arg_right,
               QuaternionNS::QuaternionItem *result);
    static QuaternionNS::QuaternionItem *MakeConvertedItem(UglyQuaternionItem ugly_item);
    static QuaternionNS::Operator MapOperator(InterCodeNS::Operator op);
    static void PrintOperator(QuaternionNS::Operator op);
    void Println() const;
    enum QuaternionNS::Operator op;
    QuaternionNS::QuaternionItem *arg_left;
    QuaternionNS::QuaternionItem *arg_right;
    QuaternionNS::QuaternionItem *result;
};

#endif //EXP_CPP_QUATERNION_H_
