//
// Created by Edot on 2023/12/19.
//

#include "Optimizer.h"
#include "FlowChartManager.h"
#include <map>
#include <random>
#include <vector>
#include <algorithm>
#include <ctime>

void Optimizer::Optimize(FlowChartManager *flow_chart_manager){
    // optimize 1.冗余代码删除
    std::vector<long long> used_var;
    for(auto block : flow_chart_manager->flow_chart){
        for(auto code : block->codes){
            if(code->op == QuaternionNS::REF ||
                code->op == QuaternionNS::STORE ||
                code->op == QuaternionNS::BRT ||
                code->op == QuaternionNS::BRF ||
                code->op == QuaternionNS::PARAM ||
                code->op == QuaternionNS::RETVAL ||
                code->op == QuaternionNS::GETINT ||
                code->op == QuaternionNS::PRINT
                ){
                MarkUsed(used_var, code->arg_left);
                MarkUsed(used_var, code->arg_right);
                MarkUsed(used_var, code->result);
            }
        }
    }

    bool changed = true;
    while(changed){
        changed = false;
        for(auto block : flow_chart_manager->flow_chart){
            for(auto code : block->codes){
                if(code->op == QuaternionNS::ADD || // 基本算数运算
                    code->op == QuaternionNS::SUB ||
                    code->op == QuaternionNS::MUL ||
                    code->op == QuaternionNS::DIV ||
                    code->op == QuaternionNS::MOD ||
                    code->op == QuaternionNS::POS ||
                    code->op == QuaternionNS::NEG ||
                    // 布尔运算
                    code->op == QuaternionNS::LSS ||
                    code->op == QuaternionNS::GRE ||
                    code->op == QuaternionNS::LEQ ||
                    code->op == QuaternionNS::GEQ ||
                    code->op == QuaternionNS::EQ ||
                    code->op == QuaternionNS::NEQ ||
                    code->op == QuaternionNS::AND ||
                    code->op == QuaternionNS::OR ||
                    code->op == QuaternionNS::NOT ||
                    // 赋值
                    code->op == QuaternionNS::ASS
                    ){
                    if(Used(used_var, code->result)){
                        changed = changed || MarkUsed(used_var, code->arg_left);
                        changed = changed || MarkUsed(used_var, code->arg_right);
                    }
                }
            }
        }
    }

    for(auto block : flow_chart_manager->flow_chart){
        for(auto code_ptr = block->codes.begin(); code_ptr != block->codes.end();){
            auto code = *code_ptr;
            if(code->op == QuaternionNS::ADD || // 基本算数运算
                code->op == QuaternionNS::SUB ||
                code->op == QuaternionNS::MUL ||
                code->op == QuaternionNS::DIV ||
                code->op == QuaternionNS::MOD ||
                code->op == QuaternionNS::POS ||
                code->op == QuaternionNS::NEG ||
                // 布尔运算
                code->op == QuaternionNS::LSS ||
                code->op == QuaternionNS::GRE ||
                code->op == QuaternionNS::LEQ ||
                code->op == QuaternionNS::GEQ ||
                code->op == QuaternionNS::EQ ||
                code->op == QuaternionNS::NEQ ||
                code->op == QuaternionNS::AND ||
                code->op == QuaternionNS::OR ||
                code->op == QuaternionNS::NOT ||
                // 赋值
                code->op == QuaternionNS::ASS ||
                // 解引用
                code->op == QuaternionNS::LOAD
                ){
                if(!Used(used_var, code->result)){
                    code_ptr = block->codes.erase(code_ptr);
                    continue;
                }
            }
            code_ptr++;
        }
    }

    // optimize 2.全局寄存器分配
    // step 1.统计所有等待分配的变量
    std::map<QuaternionNS::QuaternionItem *, int> var_reference_count;
    for(auto block : flow_chart_manager->flow_chart){
        for(auto code : block->codes){
            if(code->arg_left){

            }
            if(code->arg_right){

            }
            if(code->result){

            }
        }
    }
    std::vector<long long> register_var_name;
    for(auto block : flow_chart_manager->flow_chart){
        for(auto code : block->codes){
            if(dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)
                && !dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)->if_global
                && !dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)->if_param){
                if(std::find(register_var_name.begin(),
                             register_var_name.end(),
                             dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)->name) == register_var_name.end()){
                    register_var_name.push_back(dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)->name);
                }
            }
            if(dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)
                && !dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)->if_global
                && !dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)->if_param){
                if(std::find(register_var_name.begin(),
                             register_var_name.end(),
                             dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)->name) == register_var_name.end()){
                    register_var_name.push_back(dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)->name);
                }
            }
            if(dynamic_cast<QuaternionNS::IntItem *>(code->result)
                && !dynamic_cast<QuaternionNS::IntItem *>(code->result)->if_global
                && !dynamic_cast<QuaternionNS::IntItem *>(code->result)->if_param){
                if(std::find(register_var_name.begin(),
                             register_var_name.end(),
                             dynamic_cast<QuaternionNS::IntItem *>(code->result)->name) == register_var_name.end()){
                    register_var_name.push_back(dynamic_cast<QuaternionNS::IntItem *>(code->result)->name);
                }
            }
        }
    }
    std::shuffle(register_var_name.begin(), register_var_name.end(), std::mt19937(std::random_device()()));
//    srand(static_cast<unsigned int>(time(nullptr)));
//    std::random_shuffle(register_var_name.begin(), register_var_name.end());
    for(int i = 0; i < global_register_count && i < register_var_name.size(); ++i){
        for(auto block : flow_chart_manager->flow_chart){
            for(auto code : block->codes){
                if(dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)
                    && dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)->name == register_var_name[i]){
                    dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)->if_register = true;
                    dynamic_cast<QuaternionNS::IntItem *>(code->arg_left)->register_id = i;
                }
                if(dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)
                    && dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)->name == register_var_name[i]){
                    dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)->if_register = true;
                    dynamic_cast<QuaternionNS::IntItem *>(code->arg_right)->register_id = i;
                }
                if(dynamic_cast<QuaternionNS::IntItem *>(code->result)
                    && dynamic_cast<QuaternionNS::IntItem *>(code->result)->name == register_var_name[i]){
                    dynamic_cast<QuaternionNS::IntItem *>(code->result)->if_register = true;
                    dynamic_cast<QuaternionNS::IntItem *>(code->result)->register_id = i;
                }
            }
        }
    }
}

bool Optimizer::MarkUsed(std::vector<long long> &used_var, QuaternionNS::QuaternionItem *item){
    if(dynamic_cast<QuaternionNS::IntItem *>(item)){
        if(std::find(used_var.begin(),
                     used_var.end(),
                     dynamic_cast<QuaternionNS::IntItem *>(item)->name) == used_var.end()){
            used_var.push_back(dynamic_cast<QuaternionNS::IntItem *>(item)->name);
            return true;
        }
    }
    return false;
}

bool Optimizer::Used(const std::vector<long long int> &used_var, QuaternionNS::QuaternionItem *item){
    if(dynamic_cast<QuaternionNS::IntItem *>(item)){
        return std::find(used_var.begin(),
                         used_var.end(),
                         dynamic_cast<QuaternionNS::IntItem *>(item)->name) != used_var.end();
    }
    return false;
}

