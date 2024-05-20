//
// Created by Edot on 2023/11/7.
//

#include "FlowChartManager.h"
#include "InterCode.h"
#include "Parser.h"
#include "Quaternion.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <set>

void FlowChartManager::FillFlowChart(const char *src_path, const char *err_path){
    std::vector<Quaternion *> codes;
    auto inter_code = new InterCode;
    Parser parser(src_path, inter_code);
    std::ofstream output(err_path);
    std::streambuf *c_out_bkp = std::cout.rdbuf(output.rdbuf());
    parser.Parse();

    std::cout.rdbuf(c_out_bkp);
    output.close();
    if(parser.has_error){
        has_error = true;
        return;
    }
    for(int i = 1; i < inter_code->GetCurrentId(); ++i){
        auto arg_left = Quaternion::MakeConvertedItem(inter_code->ugly_codes[i]->arg_left);
        auto arg_right = Quaternion::MakeConvertedItem(inter_code->ugly_codes[i]->arg_right);
        auto result = Quaternion::MakeConvertedItem(inter_code->ugly_codes[i]->result);
        auto op = Quaternion::MapOperator(inter_code->ugly_codes[i]->op);
        if(op == QuaternionNS::MAIN){
            arg_left = new QuaternionNS::FuncItem(0, 0); // ���������ڷ��ű��еĵ�ַ��������0
        }
        codes.push_back(new Quaternion(op, arg_left, arg_right, result));
    }
    // ���ˣ��ɵ�InterCode����Ҳû����
    // ��������ʲô��
    // 1.���ȼ���ÿ������������ڴ�ռ�
    std::cout << "O_o" << std::endl;

    QuaternionNS::FuncItem *current_func_item = nullptr;
    for(auto quaternion : codes){
        if(quaternion->op == QuaternionNS::FUNCDEF || quaternion->op == QuaternionNS::MAIN){
            current_func_item = dynamic_cast<QuaternionNS::FuncItem *>(quaternion->arg_left);
        }
        // ����arg_left�����ջ�ռ�
        auto arg_item = dynamic_cast<QuaternionNS::NameItem *>(quaternion->arg_left);
        if(arg_item && current_func_item){
            current_func_item->frame_size =
                arg_item->PassiveFrameSize() > current_func_item->frame_size ? arg_item->PassiveFrameSize()
                                                                             : current_func_item->frame_size;
        }
        // ����arg_right�����ջ�ռ�
        arg_item = dynamic_cast<QuaternionNS::NameItem *>(quaternion->arg_right);
        if(arg_item && current_func_item){
            current_func_item->frame_size =
                arg_item->PassiveFrameSize() > current_func_item->frame_size ? arg_item->PassiveFrameSize()
                                                                             : current_func_item->frame_size;
        }
        // ����result�����ջ�ռ�
        arg_item = dynamic_cast<QuaternionNS::NameItem *>(quaternion->result);
        if(arg_item && current_func_item){
            current_func_item->frame_size =
                arg_item->PassiveFrameSize() > current_func_item->frame_size ? arg_item->PassiveFrameSize()
                                                                             : current_func_item->frame_size;
        }
    }

    // 1.1����ÿ��CALL��FUNC��frame_size
    for(auto quaternion : codes){
        if(quaternion->op == QuaternionNS::CALL){
            auto func_use_item = dynamic_cast<QuaternionNS::FuncItem *>(quaternion->arg_left);
            for(auto code : codes){
                if(code->op == QuaternionNS::FUNCDEF || code->op == QuaternionNS::MAIN){
                    auto func_def_item = dynamic_cast<QuaternionNS::FuncItem *>(code->arg_left);
                    if(func_def_item->name == func_use_item->name){
                        func_use_item->frame_size = func_def_item->frame_size;
                    }
                }
            }
        }
    }

    // 1.2����ÿ��RET/RETVAL������FUNC��frame_size
    for(int i = 0; i < codes.size(); i++){
        auto quaternion = codes[i];
        if(quaternion->op == QuaternionNS::RET || quaternion->op == QuaternionNS::RETVAL){
            for(int j = i - 1; j >= 0; j--){
                if(codes[j]->op == QuaternionNS::FUNCDEF || codes[j]->op == QuaternionNS::MAIN){
                    quaternion->result = codes[j]->arg_left;
                    break;
                }
            }
        }
    }


    // 2.�ֿ�
    // ��ʲô�֣�
    // 1.BR/BRT/BRF
    // 2.FUNCDEF/MAIN
    // 3.RET/RETVAL

    // 2.1��⿪ʼ���
    std::vector<int> start_statement; // ��ʼ�����codes�е�λ��
    start_statement.push_back(0); // ��0���ǿ�ʼ���
    // ע�⣬�����codes�����ڵ�λ�ñ�����Сһ
    for(int i = 0; i < codes.size(); i++){
        auto quaternion = codes[i];
        if(quaternion->op == QuaternionNS::BR ||
            quaternion->op == QuaternionNS::BRT ||
            quaternion->op == QuaternionNS::BRF){
            start_statement.push_back(i + 1);
            auto lab_item = dynamic_cast<QuaternionNS::LabItem *>(quaternion->result);
            start_statement.push_back(lab_item->lab - 1);
        } else if(quaternion->op == QuaternionNS::FUNCDEF ||
            quaternion->op == QuaternionNS::MAIN){
            start_statement.push_back(i);
        } else if(quaternion->op == QuaternionNS::RET ||
            quaternion->op == QuaternionNS::RETVAL){
            start_statement.push_back(i + 1); // ע�⣬����main�������һ��һ����return�����һ����ʼ���һ���պó���codes�ķ�Χ
        }
    }
    // ȥ��
    std::set<int> distinct_tool_set(start_statement.begin(), start_statement.end());
    start_statement.assign(distinct_tool_set.begin(), distinct_tool_set.end());

    // 2.2����������
    std::sort(start_statement.begin(), start_statement.end());
    for(int i = 0; i < start_statement.size() - 1/*����������Ҫ��1�������ǻ�����������*/; ++i){
        auto basic_block = new FlowChartManagerNS::BasicBlock(i);
        flow_chart.push_back(basic_block);
        for(int j = start_statement[i]; j < start_statement[i + 1]; ++j){
            basic_block->codes.push_back(codes[j]);
        }
        Quaternion *last_statement = codes[start_statement[i + 1] - 1];
        if(last_statement->op == QuaternionNS::BR ||
            last_statement->op == QuaternionNS::BRF ||
            last_statement->op == QuaternionNS::BRT){
            // ������ת��λ�ã���Ϊ��תĿ���������flow_chart�е�λ��
            auto label_item = dynamic_cast<QuaternionNS::LabItem *>(last_statement->result);
            for(int j = 0; j < start_statement.size() - 1; ++j){
                if(start_statement[j] == label_item->lab - 1){
                    label_item->lab = j;
                }
            }
        }
    }

    // 2.3ȷ��ÿ���������ǰ������
    // ��ʵֻ��Ҫȷ����̾Ϳ�����
    // ���1.����ת�����Ļ����飺������
    // ���2.��RET/RETVAL�����Ļ����飺�޺��
    // ���3.�������������������Ļ����飺�������һ��������һ����ͷ����FUNCDEF/MAIN����̾�����һ��
    for(int i = 0; i < flow_chart.size(); i++){
        auto block = flow_chart[i];
        Quaternion *last_statement = block->codes.back();
        if(last_statement->op == QuaternionNS::BR){
            auto lab_item = dynamic_cast<QuaternionNS::LabItem *>(last_statement->result);
            block->successor.push_back(flow_chart[lab_item->lab]);
            flow_chart[lab_item->lab]->precursor.push_back(block);
        } else if(last_statement->op == QuaternionNS::BRF || last_statement->op == QuaternionNS::BRT){
            auto lab_item = dynamic_cast<QuaternionNS::LabItem *>(last_statement->result);
            block->successor.push_back(flow_chart[lab_item->lab]);
            flow_chart[lab_item->lab]->precursor.push_back(block);
            if(i + 1 < flow_chart.size()){
                block->successor.push_back(flow_chart[i + 1]);
                flow_chart[i + 1]->precursor.push_back(block);
            }
        } else if(last_statement->op != QuaternionNS::RET && last_statement->op != QuaternionNS::RETVAL){
            if(i + 1 < flow_chart.size() && flow_chart[i + 1]->codes[0]->op != QuaternionNS::FUNCDEF
                && flow_chart[i + 1]->codes[0]->op != QuaternionNS::MAIN){
                block->successor.push_back(flow_chart[i + 1]);
                flow_chart[i + 1]->precursor.push_back(block);
            }
        }
    }

    // 3.��PRINTFչ��
    // չ��ΪPRINT INT_NAME/IMM/STR_NAME _ _
    // ԭ�е�PRINTF���PASS
    for(int i = 0; i < flow_chart.size(); ++i){
        for(int j = 0; j < flow_chart[i]->codes.size(); j++){
            auto code = flow_chart[i]->codes[j];
            if(code->op == QuaternionNS::PRINTF){
                code->op = QuaternionNS::PASS;
                std::string format_string(dynamic_cast<QuaternionNS::StrItem *>(code->arg_left)->value);
                // ����%d�ָ��ַ���
                std::vector<std::string> split_result;
                std::string::iterator begin = format_string.begin() + 1;
                std::string::iterator iter = begin;
                while(iter != format_string.end() - 1){
                    if(iter + 1 != format_string.end() && *iter == '%' && *(iter + 1) == 'd'){
                        split_result.emplace_back(begin, iter);
                        begin = iter + 2;
                        iter = begin;
                    } else{
                        iter++;
                    }
                }
                split_result.emplace_back(begin, iter);
                // ����Ѱ��PRINT�����в���
                // �Ȱѵ�һ�����ȥ
                auto to_be_printed = new QuaternionNS::StrItem(
                    split_result[0], -1
                );
                auto print_str_quaternion = new Quaternion(
                    QuaternionNS::PRINT,
                    to_be_printed,
                    nullptr,
                    nullptr
                );
                flow_chart[i]->codes.insert(flow_chart[i]->codes.begin() + (j + 1), print_str_quaternion);
                // ���ұ�Ĳ���
                int count = 0;
                int current_i = i;
                int current_j = j + 2;
                while(count < split_result.size() - 1){
                    if(current_j >= flow_chart[current_i]->codes.size()){
                        current_j = 0;
                        current_i++;
                    } else{
                        if(flow_chart[current_i]->codes[current_j]->op == QuaternionNS::PRINT){
                            count++;
                            to_be_printed = new QuaternionNS::StrItem(
                                split_result[count], -1
                            );
                            print_str_quaternion = new Quaternion(
                                QuaternionNS::PRINT,
                                to_be_printed,
                                nullptr,
                                nullptr
                            );
                            flow_chart[current_i]->codes.insert(
                                flow_chart[current_i]->codes.begin() + (current_j + 1),
                                print_str_quaternion
                            );
                            current_j++;
                        }
                        current_j++;
                    }
                }

                // ���ִ���
                code->arg_left = nullptr;
                code->arg_right = nullptr;
                code->result = nullptr;
            }
        }
    }

    // ���ˣ���ͼ������ϣ����Խ��д����Ż���Ŀ���������
    Println();
}

void FlowChartManager::Println(){
    for(auto block : flow_chart){
        std::cout << "============BK" << block->id << "=============" << std::endl;
        std::cout << "PRE:";
        for(auto pre : block->precursor){
            std::cout << "BK" << pre->id << " ";
        }
        std::cout << std::endl;
        std::cout << "-------" << std::endl;
        for(auto code : block->codes){
            code->Println();
        }
        std::cout << "-------" << std::endl;
        std::cout << "SUC:";
        for(auto suc : block->successor){
            std::cout << "BK" << suc->id << " ";
        }
        std::cout << std::endl;
        std::cout << "===========================" << std::endl;
    }
}

FlowChartManagerNS::BasicBlock::BasicBlock(int id) : id(id){}
