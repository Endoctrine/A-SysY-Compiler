//
// Created by Edot on 2023/11/8.
//

#include "TargetCodeGenerator.h"
#include "RegisterPool.h"
#include "Quaternion.h"
#include "Optimizer.h"
#include <map>
#include <iostream>
#include <fstream>

void TargetCodeGenerator::GenerateTargetCode(){
    flow_chart_manager = new FlowChartManager();
    flow_chart_manager->FillFlowChart("testfile.txt", "error.txt");
    if(flow_chart_manager->has_error){
        return;
    }
    auto optimizer = new Optimizer;
    std::cout << ":|" << std::endl;
    optimizer->Optimize(flow_chart_manager);
    // step.0 �ض���
    std::ofstream output("mips.txt");
    std::streambuf *c_out_bkp = std::cout.rdbuf(output.rdbuf());
    // step.1 ȫ�ֱ����ۺ�
    SynthesizeGlobalName();
    std::cout << ".text" << std::endl;
    // step.2 Ϊÿ������������Ŀ����
    GenerateForEachBBlock();
    // step.3 �ض������
    std::cout.rdbuf(c_out_bkp);
    output.close();
    std::cout << ":)" << std::endl;
}

void TargetCodeGenerator::SynthesizeGlobalName(){
    std::map<int, QuaternionNS::QuaternionItem *> global_name_map;
    auto block_iter = flow_chart_manager->flow_chart.begin();
    while(block_iter != flow_chart_manager->flow_chart.end()){
        auto quaternion_iter = (*block_iter)->codes.begin();
        while(quaternion_iter != (*block_iter)->codes.end()){
            if((*quaternion_iter)->op == QuaternionNS::CONDEF){
                // ȫ�ֳ���һ��Ҫ��CONDEF֮������item����Ϊ���item�г�ֵ��Ϣ
                auto name = dynamic_cast<QuaternionNS::NameItem *>((*quaternion_iter)->arg_left);
                if(name->name_type == QuaternionNS::INT){
                    auto int_name = dynamic_cast<QuaternionNS::IntItem *>(name);
                    if(int_name->if_global){
                        global_name_map[int_name->address] = int_name;
                    }
                } else if(name->name_type == QuaternionNS::INT_ARRAY){
                    auto int_array_name = dynamic_cast<QuaternionNS::IntArrayItem *>(name);
                    if(int_array_name->if_global){
                        global_name_map[int_array_name->address] = int_array_name;
                    }
                }
            } else{
                // ���仰˵��ȫ�ֳ����ǡ������ġ�����ȫ�ַǳ����ǡ����صġ�
                TryPutGlobalNameMap(global_name_map, (*quaternion_iter)->arg_left);
                TryPutGlobalNameMap(global_name_map, (*quaternion_iter)->arg_right);
                TryPutGlobalNameMap(global_name_map, (*quaternion_iter)->result);
            }
            quaternion_iter++;
        }
        block_iter++;
    }
    GenerateForGlobalName(global_name_map);
}

void TargetCodeGenerator::TryPutGlobalNameMap(std::map<int, QuaternionNS::QuaternionItem *> &global_name_map,
                                              QuaternionNS::QuaternionItem *arg){
    if(arg && dynamic_cast<QuaternionNS::NameItem *>(arg) != nullptr){
        auto name = dynamic_cast<QuaternionNS::NameItem *>(arg);
        if(global_name_map.find(name->address) != global_name_map.end()){
            // �Ѿ��ҹ��˾Ͳ��������ˣ�����Ǳ��ص�����
            return;
        }
        switch(name->name_type){
            case QuaternionNS::INT:{
                auto int_name = dynamic_cast<QuaternionNS::IntItem *>(name);
                if(!int_name->if_global){
                    return;
                }
                global_name_map[int_name->address] = int_name;
                break;
            }
            case QuaternionNS::INT_ARRAY:{
                auto int_array_name = dynamic_cast<QuaternionNS::IntArrayItem *>(name);
                if(!int_array_name->if_global){
                    return;
                }
                global_name_map[int_array_name->address] = int_array_name;
                break;
            }
            case QuaternionNS::INT_POINTER:{
                auto int_pointer_name = dynamic_cast<QuaternionNS::IntPointerItem *>(name);
                if(!int_pointer_name->if_global){
                    return;
                }
                global_name_map[int_pointer_name->address] = int_pointer_name;
                break;
            }
            default:{
                break;
            }
        }
    }
}

void TargetCodeGenerator::GenerateForGlobalName(const std::map<int, QuaternionNS::QuaternionItem *> &global_name_map){
    std::cout << ".data" << std::endl;
    std::cout << "global_base:.word" << std::endl;
    int current_address = 0;
    auto global_name_iter = global_name_map.begin();
    while(global_name_iter != global_name_map.end()){
        auto name = dynamic_cast<QuaternionNS::NameItem *>(global_name_iter->second);
        // current_address�͵�ǰ���ŵ�address֮��Ŀ�ȱ��.space�
        if(name->address - current_address){
            std::cout << ".space " << name->address - current_address << std::endl;
            std::cout << ".word" << std::endl;
        }
        switch(name->name_type){
            case QuaternionNS::INT:{
                auto int_name = dynamic_cast<QuaternionNS::IntItem *>(name);
                if(int_name->if_const){
                    std::cout << int_name->const_init_val[0] << std::endl;
                } else{
                    std::cout << 0 << std::endl;
                }
                current_address = int_name->address + 4;
                break;
            }
            case QuaternionNS::INT_ARRAY:{
                auto int_array_name = dynamic_cast<QuaternionNS::IntArrayItem *>(name);
                int word_size = 1;
                for(int axis : int_array_name->axes){
                    word_size *= axis;
                }
                if(int_array_name->if_const){
                    for(int init_val : int_array_name->const_init_val){
                        std::cout << init_val << std::endl;
                    }
                    if(static_cast<int>(int_array_name->const_init_val.size()) < word_size){
                        std::cout << ".space "
                                  << (word_size - static_cast<int>(int_array_name->const_init_val.size())) * 4
                                  << std::endl;
                        std::cout << ".word" << std::endl;
                    }
                } else{
                    std::cout << ".space " << word_size * 4 << std::endl;
                    std::cout << ".word" << std::endl;
                }
                current_address = int_array_name->address + word_size * 4;
                break;
            }
            case QuaternionNS::INT_POINTER:{
                auto int_pointer_name = dynamic_cast<QuaternionNS::IntPointerItem *>(name);
                std::cout << 0 << std::endl;
                current_address = int_pointer_name->address + 4;
                break;
            }
            default:{
                break;
            }
        }
        global_name_iter++;
    }
}

void TargetCodeGenerator::GenerateForEachBBlock(){
    // ���ȳ�ʼ���Ĵ�����
    register_pool = new RegisterPool();
    auto block_iter = flow_chart_manager->flow_chart.begin();
    // �ҵ���������ջ֡��С����ʵ���Բ��ң����ǲ�̫���ۣ�
    int main_frame_size;
    while(block_iter != flow_chart_manager->flow_chart.end()){
        if((*block_iter)->codes[0]->op == QuaternionNS::MAIN){
            auto main_func = dynamic_cast<QuaternionNS::FuncItem *>((*block_iter)->codes[0]->arg_left);
            main_frame_size = main_func->frame_size;
            break;
        }
        block_iter++;
    }

    block_iter = flow_chart_manager->flow_chart.begin();
    // ��һ����Ϊ��һ����������֮ǰ�Ĳ�������Ŀ����
    while(block_iter != flow_chart_manager->flow_chart.end()){
        if((*block_iter)->codes[0]->op == QuaternionNS::FUNCDEF || (*block_iter)->codes[0]->op == QuaternionNS::MAIN){
            break;
        }
        // ���ɿ�ͷ��ǩ
        std::cout << "BK" << (*block_iter)->id << ":" << std::endl;
        auto quaternion_iter = (*block_iter)->codes.begin();
        while(quaternion_iter != (*block_iter)->codes.end()){
            GenerateForCommonQuaternion(*quaternion_iter);
            quaternion_iter++;
        }
        // ��ռĴ�����
        register_pool->SpillOutAll();
        block_iter++;
    }
    // ��ʼ��ջָ�룬��ת��������
    std::cout << "li $sp " << stack_base << std::endl;
    std::cout << "subiu $sp $sp " << main_frame_size << std::endl;
    std::cout << "jal MAIN" << std::endl;
    std::cout << "nop" << std::endl;
    // ��ת����β
    std::cout << "b END" << std::endl;
    std::cout << "nop" << std::endl;
    // �ڶ�����Ϊʣ�µĻ���������Ŀ����
    while(block_iter != flow_chart_manager->flow_chart.end()){
        // ���ɿ�ͷ��ǩ
        std::cout << "BK" << (*block_iter)->id << ":" << std::endl;
        auto quaternion_iter = (*block_iter)->codes.begin();
        while(quaternion_iter != (*block_iter)->codes.end()){
            if((*quaternion_iter)->op == QuaternionNS::CALL){
                GenerateForCallQuaternion(block_iter, quaternion_iter);
            } else{
                GenerateForCommonQuaternion(*quaternion_iter);
            }
            quaternion_iter++;
        }
        // ��ռĴ�����
        register_pool->SpillOutAll();
        block_iter++;
    }
    // ���ɽ�β�ı�ǩ
    std::cout << "END:" << std::endl;
}

void TargetCodeGenerator::GenerateForCommonQuaternion(Quaternion *quaternion){
    std::cout << std::endl;
    switch(quaternion->op){
        case QuaternionNS::ADD:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "addu " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::SUB:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "subu " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::POS:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "addiu " << r_result << " " << r_left << " " << 0 << std::endl;
            break;
        }
        case QuaternionNS::NEG:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "not " << r_result << " " << r_left << std::endl;
            std::cout << "addiu " << r_result << " " << r_result << " " << 1 << std::endl;
            break;
        }
            /*------*/
        case QuaternionNS::MUL:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "mul " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::DIV:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "div " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::MOD:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "div " << r_left << " " << r_right << std::endl;
            std::cout << "mfhi " << r_result << std::endl;
            break;
        }
            /*------*/
        case QuaternionNS::LSS:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "slt " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::GRE:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "sgt " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::LEQ:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "sle " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::GEQ:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "sge " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::EQ:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "seq " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::NEQ:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "sne " << r_result << " " << r_left << " " << r_right << std::endl;
            break;
        }
        case QuaternionNS::AND:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "sltu " << r_result << " $zero " << r_left << std::endl;
            std::cout << "sltu $fp $zero " << r_right << std::endl;
            std::cout << "and " << r_result << " " << r_result << " $fp" << std::endl;
            break;
        }
        case QuaternionNS::OR:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "or " << r_result << " " << r_left << " " << r_right << std::endl;
            std::cout << "sltu " << r_result << " $zero " << r_result << std::endl;
            break;
        }
        case QuaternionNS::NOT:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "li " << r_result << " 1" << std::endl;
            std::cout << "sltu " << r_result << " " << r_left << " " << r_result << std::endl;
            break;
        }
            /*------*/
        case QuaternionNS::ASS:{
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "addiu " << r_result << " " << r_left << " 0" << std::endl;
            break;
        }
        case QuaternionNS::REF:{
            std::string r_right = register_pool->AllocateRegister(quaternion->arg_right);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            auto arg_left = dynamic_cast<QuaternionNS::NameItem *>(quaternion->arg_left);
            if(arg_left->name_type == QuaternionNS::INT_ARRAY){
                auto int_array_arg = dynamic_cast<QuaternionNS::IntArrayItem *>(arg_left);
                int unit_size = 4;
                for(int i = 1; i < int_array_arg->dimension; ++i){
                    unit_size *= int_array_arg->axes[i];
                }
                std::cout << "li " << r_result << " " << unit_size << std::endl;
                std::cout << "mul " << r_result << " " << r_right << " " << r_result << std::endl;
                std::cout << "addu " << r_result << " " << r_result << " " << int_array_arg->address << std::endl;
                if(int_array_arg->if_global){
                    std::cout << "la $fp global_base" << std::endl;
                    std::cout << "addu " << r_result << " $fp " << r_result << std::endl;
                } else{
                    std::cout << "addu " << r_result << " $sp " << r_result << std::endl;
                }
            } else if(arg_left->name_type == QuaternionNS::INT_POINTER){
                std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
                auto int_pointer_arg = dynamic_cast<QuaternionNS::IntPointerItem *>(arg_left);
                int unit_size = 4;
                for(int i = 0; i < int_pointer_arg->dimension - 1; ++i){
                    unit_size *= int_pointer_arg->axes[i];
                }
                std::cout << "li " << r_result << " " << unit_size << std::endl;
                std::cout << "mul " << r_result << " " << r_right << " " << r_result << std::endl;
                std::cout << "addu " << r_result << " " << r_left << " " << r_result << std::endl;
            } else{
                perror("Other name type should not be there.");
            }
            break;
        }
        case QuaternionNS::LOAD:{
            auto arg_left = dynamic_cast<QuaternionNS::NameItem *>(quaternion->arg_left);
            if(arg_left->name_type != QuaternionNS::INT_POINTER){
                perror("only INT_POINTER can LOAD.");
            }
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "lw " << r_result << " 0(" << r_left << ")" << std::endl;
            break;
        }
        case QuaternionNS::STORE:{
            auto arg_result = dynamic_cast<QuaternionNS::NameItem *>(quaternion->result);
            if(arg_result->name_type != QuaternionNS::INT_POINTER){
                perror("only INT_POINTER can STORE.");
            }
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            // ע�⣬��������һ����ַ�д涫�������������result��һ���result��ͬ��
            // ^�����result��һ��ָ�룬���ǲ���Ҫ��ָ������д��ַ������REF�ɵ����飩��
            // ^����Ҫ��ָ��洢�ĵ�ַ��д��������������is_def == false����REF�е�result��is_def == true��
            std::string r_result = register_pool->AllocateRegister(quaternion->result);
            std::cout << "sw " << r_left << " 0(" << r_result << ")" << std::endl;
            break;
        }
            /*------*/
        case QuaternionNS::BR:{
            // ������ռĴ����أ�����������������޸Ĺ��ı���
            register_pool->SpillOutAll();
            auto label = dynamic_cast<QuaternionNS::LabItem *>(quaternion->result);
            std::cout << "b BK" << label->lab << std::endl;
            std::cout << "nop" << std::endl;
            break;
        }
        case QuaternionNS::BRT:{
            // ������ռĴ����أ�����������������޸Ĺ��ı���
            register_pool->SpillOutAll();
            auto label = dynamic_cast<QuaternionNS::LabItem *>(quaternion->result);
            auto r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::cout << "bnez " << r_left << " BK" << label->lab << std::endl;
            std::cout << "nop" << std::endl;
            break;
        }
        case QuaternionNS::BRF:{
            // ������ռĴ����أ�����������������޸Ĺ��ı���
            register_pool->SpillOutAll();
            auto label = dynamic_cast<QuaternionNS::LabItem *>(quaternion->result);
            auto r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::cout << "beqz " << r_left << " BK" << label->lab << std::endl;
            std::cout << "nop" << std::endl;
            break;
        }
            /*------*/
        case QuaternionNS::FUNCDEF:{
            auto func_item = dynamic_cast<QuaternionNS::FuncItem *>(quaternion->arg_left);
            std::cout << "FUNC" << func_item->name << ":" << std::endl;
            break;
        }
        case QuaternionNS::MAIN:{
            std::cout << "MAIN:" << std::endl;
            break;
        }
            /*------*/
        case QuaternionNS::PARAM:{
            // ʲôҲ���������ȴ���CALL��ʱ��Ż��õ�
            break;
        }
        case QuaternionNS::CALL:{
            perror("Oh... CALL quaternion should not be there.");
            break;
        }
            /*------*/
        case QuaternionNS::RETVAL:{
            auto func_item = dynamic_cast<QuaternionNS::FuncItem *>(quaternion->result);
            std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
            std::cout << "addiu $v0 " << r_left << " 0" << std::endl;
            register_pool->SpillOutAll();
            std::cout << "addiu $sp $sp " << func_item->frame_size << std::endl;
            std::cout << "jr $ra" << std::endl;
            std::cout << "nop" << std::endl;
            break;
        }
        case QuaternionNS::RET:{
            auto func_item = dynamic_cast<QuaternionNS::FuncItem *>(quaternion->result);
            register_pool->SpillOutAll();
            std::cout << "addiu $sp $sp " << func_item->frame_size << std::endl;
            std::cout << "jr $ra" << std::endl;
            std::cout << "nop" << std::endl;
            break;
        }
            /*------*/
        case QuaternionNS::GETINT:{
            std::string r_result = register_pool->AllocateRegister(quaternion->result, true);
            std::cout << "li $v0 5" << std::endl;
            std::cout << "syscall" << std::endl;
            std::cout << "addiu " << r_result << " $v0 0" << std::endl;
            break;
        }
        case QuaternionNS::PRINTF:{
            perror("Oh... PRINTF quaternion should not be there.");
            break;
        }
        case QuaternionNS::PRINT:{
            if(quaternion->arg_left->item_type == QuaternionNS::IMM){
                std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
                std::cout << "addiu $a0 " << r_left << " 0" << std::endl;
                std::cout << "li $v0 1" << std::endl;
                std::cout << "syscall" << std::endl;
            } else if(quaternion->arg_left->item_type == QuaternionNS::NAME){
                auto name_item = dynamic_cast<QuaternionNS::NameItem *>(quaternion->arg_left);
                switch(name_item->name_type){
                    case QuaternionNS::INT:{
                        std::string r_left = register_pool->AllocateRegister(quaternion->arg_left);
                        std::cout << "addiu $a0 " << r_left << " 0" << std::endl;
                        std::cout << "li $v0 1" << std::endl;
                        std::cout << "syscall" << std::endl;
                        break;
                    }
                    case QuaternionNS::STR:{
                        auto str_item = dynamic_cast<QuaternionNS::StrItem *>(name_item);
                        std::cout << ".data" << std::endl;
                        std::cout << "str" << string_count << ":.asciiz \"" << str_item->value << "\"" << std::endl;
                        std::cout << ".text" << std::endl;
                        std::cout << "la $a0 str" << string_count << std::endl;
                        std::cout << "li $v0 4" << std::endl;
                        std::cout << "syscall" << std::endl;
                        string_count++;
                        break;
                    }
                    case QuaternionNS::INT_ARRAY:
                    case QuaternionNS::INT_POINTER:
                    case QuaternionNS::FUNC:{
                        perror("Oh... can not PRINT this type of item.");
                        break;
                    }
                }
            } else{
                perror("Oh... PRINT is followed by wrong thing.");
            }
            break;
        }
            /*------*/
        case QuaternionNS::PASS:{
            std::cout << "nop" << std::endl;
            break;
        }
        case QuaternionNS::CONDEF:{
            auto name_item = dynamic_cast<QuaternionNS::NameItem *>(quaternion->arg_left);
            if(name_item->name_type == QuaternionNS::INT){
                auto int_name = dynamic_cast<QuaternionNS::IntItem *>(name_item);
                if(int_name->if_global){
                    break;
                }
                // ע�⣬��Ȼ������r_left�����������ǳ���������䣬��Ҫд��
                std::string r_left = register_pool->AllocateRegister(quaternion->arg_left, true);
                std::cout << "li " << r_left << " " << int_name->const_init_val[0] << std::endl;
            } else if(name_item->name_type == QuaternionNS::INT_ARRAY){
                auto int_array_name = dynamic_cast<QuaternionNS::IntArrayItem *>(name_item);
                if(int_array_name->if_global){
                    break;
                }
                int array_base_address = int_array_name->address;
                for(int i = 0; i < int_array_name->const_init_val.size(); ++i){
                    std::string r_init_val =
                        register_pool->AllocateRegister(new QuaternionNS::ImmItem(int_array_name->const_init_val[i]));
                    std::cout << "sw " << r_init_val << " " << array_base_address + i * 4 << "($sp)" << std::endl;
                }
            } else{
                perror("only int_name and int_array_name can CONDEF");
            }
            break;
        }
    }
}

void TargetCodeGenerator::GenerateForCallQuaternion(std::vector<FlowChartManagerNS::BasicBlock *>::iterator block_iter,
                                                    std::vector<Quaternion *>::iterator quaternion_iter){
    std::cout << std::endl;
    // ��ñ����ú���
    auto call_quaternion = *quaternion_iter;
    auto func_item = dynamic_cast<QuaternionNS::FuncItem *>(call_quaternion->arg_left);

    // �����ҵ���Ӧ�Ĳ�����ѹջ
    int param_count = 0;
    int skip_count = 0;
    while(param_count < func_item->parameter_num){
        while(quaternion_iter != (*block_iter)->codes.begin()){
            quaternion_iter--; // �ȵ�����ʹ�ã���Ϊһ�����õ��Ĳ���CALL������end()��ͬʱ������Ҳ����©����һ��
            if((*quaternion_iter)->op == QuaternionNS::CALL){
                // ����һ������PARAM
                skip_count += dynamic_cast<QuaternionNS::FuncItem *>((*quaternion_iter)->arg_left)->parameter_num;
            } else if((*quaternion_iter)->op == QuaternionNS::PARAM){
                if(skip_count == 0){
                    // ���ҵĲ�����ѹ����
                    if((*quaternion_iter)->arg_left->item_type == QuaternionNS::NAME){
                        // ���������飬����һ��
                        auto name_item = dynamic_cast<QuaternionNS::NameItem *>((*quaternion_iter)->arg_left);
                        switch(name_item->name_type){
                            case QuaternionNS::INT:
                            case QuaternionNS::INT_POINTER:{
                                std::string r_param = register_pool->AllocateRegister((*quaternion_iter)->arg_left);
                                std::cout << "sw " << r_param << " "
                                          << -func_item->frame_size + (func_item->parameter_num - param_count) * 4
                                          << "($sp)" << std::endl;
                                break;
                            }
                            case QuaternionNS::INT_ARRAY:{
                                auto array_item = dynamic_cast<QuaternionNS::IntArrayItem *>(name_item);
                                int array_base_address = array_item->address;
                                std::string r_address =
                                    register_pool->AllocateRegister(new QuaternionNS::ImmItem(array_base_address));
                                if(array_item->if_global){
                                    std::cout << "la $fp global_base" << std::endl;
                                    std::cout << "addu $fp " << r_address << " $fp" << std::endl;
                                } else{
                                    std::cout << "addu $fp " << r_address << " $sp" << std::endl;
                                }
                                std::cout << "sw $fp "
                                          << -func_item->frame_size + (func_item->parameter_num - param_count) * 4
                                          << "($sp)" << std::endl;
                                break;
                            }
                            case QuaternionNS::STR:
                            case QuaternionNS::FUNC:{
                                perror("STR or FUNC can't PARAM");
                                break;
                            }
                        }
                    } else{
                        // ��������
                        std::string r_imm = register_pool->AllocateRegister((*quaternion_iter)->arg_left);
                        std::cout << "sw " << r_imm << " "
                                  << -func_item->frame_size + (func_item->parameter_num - param_count) * 4
                                  << "($sp)" << std::endl;
                    }
                    param_count++;
                } else{
                    // �����ҵĲ���������
                    skip_count--;
                }
                if(param_count >= func_item->parameter_num){
                    break; // ����Ѿ����ˣ����������ڵ�ǰ��ı���������֮�����ѭ��Ҳ������
                }
            }
        }
        if(param_count < func_item->parameter_num){ // ��ֹ�Ѿ���begin�˻��������������仰˵�������ǰ���������������ǿ϶���û��begin
            block_iter--;
        }
        quaternion_iter = (*block_iter)->codes.end();
    }

    // ��ռĴ�����
    register_pool->SpillOutAll();

    // ���淵�ص�ַ
    std::cout << "sw $ra 0($sp)" << std::endl;

    // ѹջ
    std::cout << "subiu $sp $sp " << func_item->frame_size << std::endl;

    // ��ת
    std::cout << "jal FUNC" << func_item->name << std::endl;
    std::cout << "nop" << std::endl;

    // �ָ����ص�ַ
    std::cout << "lw $ra 0($sp)" << std::endl;

    // ���շ���ֵ
    std::string r_result = register_pool->AllocateRegister(call_quaternion->result, true);
    std::cout << "addiu " << r_result << " $v0 0" << std::endl;
}
