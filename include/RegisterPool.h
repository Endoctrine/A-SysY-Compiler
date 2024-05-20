//
// Created by Edot on 2023/11/13.
//

#ifndef EXP_CPP_REGISTER_POOL_H_
#define EXP_CPP_REGISTER_POOL_H_
#include <vector>
#include <string>
#include "Quaternion.h"

namespace RegisterPoolNS{
enum RegisterItemType{
    GLOBAL,
    LOCAL,
    IMM
};
struct RegisterItem{
    RegisterItem(RegisterItemType type, int value);
    RegisterItemType type;
    int value; // value�������������Ե�ַ/�������������ᳬ��32λ
    bool write_back = false;
};
}

class RegisterPool{
  public:
    RegisterPool();
    std::string AllocateRegister(QuaternionNS::QuaternionItem *arg, bool is_def = false);
    void SpillOutAll();
  private:
    void PassiveSpillOut(int index); // ��������Ĵ���������Ӧ�ԼĴ����������õ����
    int register_num;
    std::vector<RegisterPoolNS::RegisterItem *> pool;
    std::vector<int> age; // ��¼�Ĵ��������䣬ÿ�������ʱ��������곤�ļĴ���
    std::vector<std::string> name_map; // ���ڽ��Ĵ����ڼĴ������е�λ��ӳ��ɼĴ�����
};

#endif //EXP_CPP_REGISTER_POOL_H_
