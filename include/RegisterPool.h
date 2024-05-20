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
    int value; // value有两种情况：相对地址/立即数，都不会超过32位
    bool write_back = false;
};
}

class RegisterPool{
  public:
    RegisterPool();
    std::string AllocateRegister(QuaternionNS::QuaternionItem *arg, bool is_def = false);
    void SpillOutAll();
  private:
    void PassiveSpillOut(int index); // 溢出单个寄存器，可以应对寄存器本身闲置的情况
    int register_num;
    std::vector<RegisterPoolNS::RegisterItem *> pool;
    std::vector<int> age; // 记录寄存器的年龄，每次溢出的时候，溢出最年长的寄存器
    std::vector<std::string> name_map; // 用于将寄存器在寄存器池中的位置映射成寄存器名
};

#endif //EXP_CPP_REGISTER_POOL_H_
