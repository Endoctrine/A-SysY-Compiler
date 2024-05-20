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
    REF, // ARRAY INDEX ADDR
    // ������
    LOAD, // ADDR _ T
    // ��ַ��ֵ
    STORE, // VALUE _ ADDR
    // ��ת
    BR, // ��������ת
    BRT, // Ϊ��ʱ��ת
    BRF, // Ϊ��ʱ��ת
    // ������ʼ
    FUNC, // һ�㺯����ʼ
    MAIN, // ��������ʼ
    // ��������
    PARAM, // ���ݲ���
    CALL, // ���ú���
    // ��������
    RETVAL, // �з���ֵ�ķ���
    RET, // �޷���ֵ�ķ���
    // �������
    GETINT, // ���� getint ����
    PRINTF, // ��ӡ��ʽ������ NAME _ _
    PRINT, // ��ӡ�������� NAME/TEMP/IMM _ _
    // ��չָ����ڸ�����Ԫʽ��Ŀ��������ɣ�
    BREAK, // ѭ���� break ��ռλ
    CONTINUE, // ѭ���� continue ��ռλ
    FORIT, // for ѭ���޸�ѭ���������Ŀ�ʼ
    PASS, // ����䣬ռλ���õģ����� FORIT ʹ�ý���֮��͸ĳ�������ռλ��
    CONDEF, // ���峣����λ��
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
