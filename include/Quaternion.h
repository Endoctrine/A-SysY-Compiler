//
// Created by Edot on 2023/11/1.
//

#ifndef EXP_CPP_QUATERNION_H_
#define EXP_CPP_QUATERNION_H_
#include <string>
#include <vector>
#include "InterCode.h"

namespace QuaternionNS{
enum ItemType{ // ����ҪEMPTY�ˣ�EMPTY����nullptr
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
    StrItem(std::string value, long long name); // name��ʵûɶ��
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
    // ��������������
    ADD, // ��
    SUB, // ��
    MUL, // ��
    DIV, // ��
    MOD, // ģ
    POS, // ȡԭ��ֵ����ʵ����ɶҲ����������ֱ���Ż���
    NEG, // ȡ�෴��
    // ��������
    LSS, // С��
    GRE, // ����
    LEQ, // С�ڵ���
    GEQ, // ���ڵ���
    EQ, // ����
    NEQ, // ������
    AND, // ��
    OR, // ��
    NOT, // ��
    // ������ֵ
    ASS, // ��ֵ
    // ����
    REF, // REF INT_ARRAY_NAME/INT_POINTER_NAME INT_NAME/IMM INT_POINTER_NAME
    // ������
    LOAD, // LOAD INT_ARRAY_NAME/INT_POINTER_NAME _ NAME
    // ��ַ��ֵ
    STORE, // STORE INT_NAME/IMM _ INT_ARRAY_NAME/INT_POINTER_NAME
    // ��ת
    BR, // ��������ת
    BRT, // Ϊ��ʱ��ת
    BRF, // Ϊ��ʱ��ת
    // ������ʼ
    FUNCDEF, // һ�㺯����ʼ��FUNCDEF FUNC_NAME _ _��
    MAIN, // ��������ʼ��MAIN FUNC_NAME _ _��
    // ��������
    PARAM, // PARAM INT_NAME/INT_ARRAY_NAME/INT_POINTER_NAME/IMM _ _
    CALL, // CALL FUNC_NAME IMM INT_NAME
    // ��������
    RETVAL, // RETVAL INT_NAME/IMM _ FUNC_NAME��FUNC_NAMEΪ�������������Ի��ջ֡��С��
    RET, // RET _ _ FUNC_NAME��FUNC_NAMEΪ�������������Ի��ջ֡��С��
    // �������
    GETINT, // ���� getint ����
    PRINTF, // PRINTF STR_NAME _ _
    PRINT, // PRINT INT_NAME/IMM _ _
    // ��չָ����ڸ�����Ԫʽ��Ŀ��������ɣ�
    PASS, // ����䣬ռλ���õ�
    CONDEF, // CONDEF INT_NAME/INT_ARRAY_NAME _ _�����峣����λ�㣩
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
