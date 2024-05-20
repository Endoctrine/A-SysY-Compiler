//
// Created by Edot on 2023/11/13.
//

#include "RegisterPool.h"
#include <iostream>

RegisterPoolNS::RegisterItem::RegisterItem(RegisterPoolNS::RegisterItemType type, int value)
    : type(type), value(value){
}

RegisterPool::RegisterPool() : register_num(8){
    // $fp�Ĵ�����������������ʱ����������
    name_map.emplace_back("$t0");
    name_map.emplace_back("$t1");
    name_map.emplace_back("$t2");
    name_map.emplace_back("$t3");
    name_map.emplace_back("$t4");
    name_map.emplace_back("$t5");
    name_map.emplace_back("$t6");
    name_map.emplace_back("$t7");
    for(int i = 0; i < register_num; ++i){
        pool.emplace_back(nullptr);
        age.emplace_back(0);
    }
}

std::string RegisterPool::AllocateRegister(QuaternionNS::QuaternionItem *arg, bool is_def){
    RegisterPoolNS::RegisterItem *item;
    switch(arg->item_type){
        case QuaternionNS::IMM:{
            auto imm_arg = dynamic_cast<QuaternionNS::ImmItem *>(arg);
            item = new RegisterPoolNS::RegisterItem(RegisterPoolNS::IMM, imm_arg->value);
            break;
        }
        case QuaternionNS::LABEL:{
            // ���ƾ��棬��ʵûɶ��
            item = new RegisterPoolNS::RegisterItem(RegisterPoolNS::IMM, 0);
            perror("LABEL item should not be there.");
            break;
        }
        case QuaternionNS::NAME:{
            auto name_arg = dynamic_cast<QuaternionNS::NameItem *>(arg);
            switch(name_arg->name_type){
                case QuaternionNS::INT:{
                    auto int_arg = dynamic_cast<QuaternionNS::IntItem *>(name_arg);
                    if(int_arg->if_register){ // ����ǼĴ���������ֱ�ӷ���
                        return "$s" + std::to_string(int_arg->register_id);
                    } else if(int_arg->if_global){
                        item = new RegisterPoolNS::RegisterItem(RegisterPoolNS::GLOBAL, int_arg->address);
                    } else{
                        item = new RegisterPoolNS::RegisterItem(RegisterPoolNS::LOCAL, int_arg->address);
                    }
                    break;
                }
                case QuaternionNS::INT_POINTER:{
                    auto int_pointer_arg = dynamic_cast<QuaternionNS::IntPointerItem *>(name_arg);
                    if(int_pointer_arg->if_global){
                        item = new RegisterPoolNS::RegisterItem(RegisterPoolNS::GLOBAL, int_pointer_arg->address);
                    } else{
                        item = new RegisterPoolNS::RegisterItem(RegisterPoolNS::LOCAL, int_pointer_arg->address);
                    }
                    break;
                }
                case QuaternionNS::INT_ARRAY:
                case QuaternionNS::STR:
                case QuaternionNS::FUNC:{
                    // ���ƾ��棬��ʵûɶ��
                    item = new RegisterPoolNS::RegisterItem(RegisterPoolNS::IMM, 0);
                    perror("INT_ARRAY, STR, FUNC item should not be there.");
                    break;
                }
            }
            break;
        }
    }

    // �ϻ�
    for(int i = 0; i < register_num; ++i){
        age[i] = age[i] + 1;
    }

    // ����Ѿ��ڼĴ������У����������
    for(int i = 0; i < register_num; ++i){
        if(pool[i] && pool[i]->type == item->type && pool[i]->value == item->value){
            age[i] = 0;
            pool[i]->write_back = pool[i]->write_back || is_def;
            return name_map[i];
        }
    }

    // �ҵ����ϵļĴ���
    int oldest_index = 0;
    for(int i = 0; i < register_num; ++i){
        if(age[i] > age[oldest_index]){
            oldest_index = i;
        }
    }
    age[oldest_index] = 0;

    PassiveSpillOut(oldest_index);
    pool[oldest_index] = item;
    pool[oldest_index]->write_back = pool[oldest_index]->write_back || is_def;

    // ����״η����ǡ�ʹ�á�������Ҫ�ô�
    if(!is_def){
        switch(item->type){
            case RegisterPoolNS::GLOBAL:{
                std::cout << "li " << name_map[oldest_index] << " " << item->value << std::endl;
                std::cout << "lw " << name_map[oldest_index] << " global_base(" << name_map[oldest_index] << ")"
                          << std::endl;
                break;
            }
            case RegisterPoolNS::LOCAL:{
                std::cout << "lw " << name_map[oldest_index] << " " << item->value << "($sp)" << std::endl;
                break;
            }
            case RegisterPoolNS::IMM:{
                std::cout << "li " << name_map[oldest_index] << " " << item->value << std::endl;
                break;
            }
        }
    }
    return name_map[oldest_index];
}

void RegisterPool::SpillOutAll(){
    for(int i = 0; i < register_num; ++i){
        PassiveSpillOut(i);
    }
}

void RegisterPool::PassiveSpillOut(int index){
    RegisterPoolNS::RegisterItem *item = pool[index];
    if(item == nullptr){
        return;
    }
    if(!item->write_back){
        pool[index] = nullptr;
        return;
    }
    switch(item->type){
        case RegisterPoolNS::GLOBAL:{
            std::cout << "li $fp " << pool[index]->value << std::endl;
            std::cout << "sw " << name_map[index] << " global_base($fp)" << std::endl;
            break;
        }
        case RegisterPoolNS::LOCAL:{
            std::cout << "sw " << name_map[index] << " " << pool[index]->value << "($sp)" << std::endl;
            break;
        }
        case RegisterPoolNS::IMM:{
            // ʲôҲ������
            break;
        }
    }
    pool[index] = nullptr;
}

