//
// Created by Edot on 2023/11/1.
//

#include "Quaternion.h"
#include <iostream>

#include <utility>
#include "TokenValue.h"

QuaternionNS::ImmItem::ImmItem(int value) : QuaternionItem(IMM), value(value){

}
void QuaternionNS::ImmItem::Print(){
    std::cout << value;
}

QuaternionNS::QuaternionItem::QuaternionItem(QuaternionNS::ItemType item_type)
    : item_type(item_type){

}

QuaternionNS::LabItem::LabItem(int lab) : QuaternionItem(LABEL), lab(lab){

}
void QuaternionNS::LabItem::Print(){
    std::cout << "BK" << lab;
}

QuaternionNS::NameItem::NameItem(QuaternionNS::NameType name_type, int address, long long name)
    : QuaternionItem(NAME), name_type(name_type), address(address), name(name){

}
int QuaternionNS::NameItem::PassiveFrameSize(){
    switch(name_type){
        case INT:{
            auto int_item = dynamic_cast<QuaternionNS::IntItem *>(this);
            if(int_item->if_global){
                return 4; // return 0 还是 4 都无所谓了
            } else{
                return address + 4;
            }
        }
        case INT_ARRAY:{
            auto int_item = dynamic_cast<QuaternionNS::IntArrayItem *>(this);
            if(int_item->if_global){
                return 4; // 这也一样，return 0 还是 4 都无所谓
            } else{
                int size = 4;
                for(auto axis : int_item->axes){
                    size *= axis;
                }
                return address + size;
            }
        }
        case INT_POINTER:{
            return address + 4;
        }
        case STR:
        case FUNC:{
            return 4; // 至少要存一个返回地址，FUNC分支会被函数定义语句处的FuncItem触发，所以一定会走到
        }
    }
    return 4; // 不可到达
}

QuaternionNS::IntItem::IntItem(int address, bool if_global, bool if_const, long long name)
    : NameItem(INT, address, name),
      if_global(if_global),
      if_const(if_const){

}
void QuaternionNS::IntItem::Print(){
    if(if_global){
        std::cout << "(global)";
    } else{
        std::cout << "(local)";
    }
    std::cout << "[+" << address << "]";
    std::cout << "int{" << name << "}";
}

QuaternionNS::IntArrayItem::IntArrayItem(int address, bool if_global, int dimension, bool if_const, long long name)
    : NameItem(INT_ARRAY, address, name), if_global(if_global), dimension(dimension), if_const(if_const){

}

void QuaternionNS::IntArrayItem::AddAxis(int axis){
    axes.push_back(axis);
}
void QuaternionNS::IntArrayItem::Print(){
    if(if_global){
        std::cout << "(global)";
    } else{
        std::cout << "(local)";
    }
    std::cout << "[+" << address << "]";
    std::cout << "int";
    for(int i = 0; i < dimension; ++i){
        std::cout << "[";
        std::cout << axes[i];
        std::cout << "]";
    }
    std::cout << "{" << name << "}";
}

QuaternionNS::IntPointerItem::IntPointerItem(int address,
                                             bool if_global,
                                             int dimension,
                                             long long name)
    : NameItem(INT_POINTER, address, name),
      if_global(if_global),
      dimension(dimension){

}

void QuaternionNS::IntPointerItem::AddAxis(int axis){
    axes.push_back(axis);
}
void QuaternionNS::IntPointerItem::Print(){
    if(if_global){
        std::cout << "(global)";
    } else{
        std::cout << "(local)";
    }
    std::cout << "[+" << address << "]";
    std::cout << "int";
    if(dimension == 0){
        std::cout << "(*)";
    } else{
        for(int i = 0; i < dimension; ++i){
            std::cout << "*";
        }
    }
    for(int i = 0; i < dimension - 1; ++i){
        std::cout << "[";
        std::cout << axes[i];
        std::cout << "]";
    }
    std::cout << "{" << name << "}";
}

QuaternionNS::FuncItem::FuncItem(int parameter_num, long long name)
    : NameItem(FUNC, -1, name),
      parameter_num(parameter_num){

}
void QuaternionNS::FuncItem::Print(){
    std::cout << "(global)";
    std::cout << "func(";
    std::cout << parameter_num;
    std::cout << ")";
    std::cout << "{" << name << "}";
}

QuaternionNS::StrItem::StrItem(std::string value, long long name)
    : NameItem(STR, -1, name),
      value(std::move(value)){

}
void QuaternionNS::StrItem::Print(){
    std::cout << value;
}

Quaternion::Quaternion(QuaternionNS::Operator op,
                       QuaternionNS::QuaternionItem *arg_left,
                       QuaternionNS::QuaternionItem *arg_right,
                       QuaternionNS::QuaternionItem *result)
    : op(op), arg_left(arg_left), arg_right(arg_right), result(result){

}

QuaternionNS::QuaternionItem *Quaternion::MakeConvertedItem(UglyQuaternionItem ugly_item){
    switch(ugly_item.type){
        case InterCodeNS::NAME:{
            switch(ugly_item.value.name_value->id_class){
                case CONST_INT:{
                    if(ugly_item.value.name_value->GetIdDimension() == 0){
                        auto item = new QuaternionNS::IntItem(ugly_item.value.name_value->address,
                                                              ugly_item.value.name_value->is_global,
                                                              true,
                                                              (long long) ugly_item.value.name_value);
                        for(int value : ugly_item.value.name_value->values){
                            item->const_init_val.push_back(value);
                        }
                        item->if_param = ugly_item.value.name_value->is_param;
                        return item;
                    } else{
                        auto item = new QuaternionNS::IntArrayItem(ugly_item.value.name_value->address,
                                                                   ugly_item.value.name_value->is_global,
                                                                   ugly_item.value.name_value->GetIdDimension(),
                                                                   true,
                                                                   (long long) ugly_item.value.name_value);
                        for(int i = 0; i < ugly_item.value.name_value->GetIdDimension(); ++i){
                            item->AddAxis(ugly_item.value.name_value->shape[i]);
                        }
                        for(int value : ugly_item.value.name_value->values){
                            item->const_init_val.push_back(value);
                        }
                        return item;
                    }
                }
                case VAR_INT:
                case TEMP_INT:{
                    if(ugly_item.value.name_value->is_pointer){
                        auto item = new QuaternionNS::IntPointerItem(ugly_item.value.name_value->address,
                                                                     ugly_item.value.name_value->is_global,
                                                                     ugly_item.value.name_value->GetIdDimension(),
                                                                     (long long) ugly_item.value.name_value);
                        for(int i = 1; i < ugly_item.value.name_value->GetIdDimension(); ++i){
                            // ^在Parse的时候，不论是指针还是数组的UglyQuaternionItem，其axes.size()都等于dimension
                            // ^如果是指针，则Parse的时候会先塞一个0进去占位子
                            // ^所以此处导出的时候从第一个开始导
                            item->AddAxis(ugly_item.value.name_value->shape[i]);
                        }
                        return item;
                    } else{
                        if(ugly_item.value.name_value->GetIdDimension() == 0){
                            auto item = new QuaternionNS::IntItem(ugly_item.value.name_value->address,
                                                                  ugly_item.value.name_value->is_global,
                                                                  false,
                                                                  (long long) ugly_item.value.name_value);
                            item->if_param = ugly_item.value.name_value->is_param;
                            return item;
                        } else{
                            auto item = new QuaternionNS::IntArrayItem(ugly_item.value.name_value->address,
                                                                       ugly_item.value.name_value->is_global,
                                                                       ugly_item.value.name_value->GetIdDimension(),
                                                                       false,
                                                                       (long long) ugly_item.value.name_value);
                            for(int i = 0; i < ugly_item.value.name_value->GetIdDimension(); ++i){
                                item->AddAxis(ugly_item.value.name_value->shape[i]);
                            }
                            return item;
                        }
                    }
                }
                case FUNC:{
                    return new QuaternionNS::FuncItem(ugly_item.value.name_value->ParametersSize(),
                                                      (long long) ugly_item.value.name_value);
                }
                case TEMP_STR:{
                    return new QuaternionNS::StrItem(ugly_item.value.name_value->strcon_value,
                                                     (long long) ugly_item.value.name_value);
                }
            }
            break;
        }
        case InterCodeNS::IMM:{
            return new QuaternionNS::ImmItem(static_cast<int>(ugly_item.value.numeric_value));
        }
        case InterCodeNS::ID:{
            return new QuaternionNS::LabItem(static_cast<int>(ugly_item.value.numeric_value));
        }
        case InterCodeNS::EMPTY:{
            return nullptr;
        }
    }
    perror("unexpected InterCodeNS::ItemType");
    exit(-1);
}

QuaternionNS::Operator Quaternion::MapOperator(InterCodeNS::Operator op){
    switch(op){
        case InterCodeNS::ADD:return QuaternionNS::ADD;
        case InterCodeNS::SUB:return QuaternionNS::SUB;
        case InterCodeNS::MUL:return QuaternionNS::MUL;
        case InterCodeNS::DIV:return QuaternionNS::DIV;
        case InterCodeNS::MOD:return QuaternionNS::MOD;
        case InterCodeNS::POS:return QuaternionNS::POS;
        case InterCodeNS::NEG:return QuaternionNS::NEG;
        case InterCodeNS::LSS:return QuaternionNS::LSS;
        case InterCodeNS::GRE:return QuaternionNS::GRE;
        case InterCodeNS::LEQ:return QuaternionNS::LEQ;
        case InterCodeNS::GEQ:return QuaternionNS::GEQ;
        case InterCodeNS::EQ:return QuaternionNS::EQ;
        case InterCodeNS::NEQ:return QuaternionNS::NEQ;
        case InterCodeNS::AND:return QuaternionNS::AND;
        case InterCodeNS::OR:return QuaternionNS::OR;
        case InterCodeNS::NOT:return QuaternionNS::NOT;
        case InterCodeNS::ASS:return QuaternionNS::ASS;
        case InterCodeNS::REF:return QuaternionNS::REF;
        case InterCodeNS::LOAD:return QuaternionNS::LOAD;
        case InterCodeNS::STORE:return QuaternionNS::STORE;
        case InterCodeNS::BR:return QuaternionNS::BR;
        case InterCodeNS::BRT:return QuaternionNS::BRT;
        case InterCodeNS::BRF:return QuaternionNS::BRF;
        case InterCodeNS::FUNC:return QuaternionNS::FUNCDEF;
        case InterCodeNS::MAIN:return QuaternionNS::MAIN;
        case InterCodeNS::PARAM:return QuaternionNS::PARAM;
        case InterCodeNS::CALL:return QuaternionNS::CALL;
        case InterCodeNS::RETVAL:return QuaternionNS::RETVAL;
        case InterCodeNS::RET:return QuaternionNS::RET;
        case InterCodeNS::GETINT:return QuaternionNS::GETINT;
        case InterCodeNS::PRINTF:return QuaternionNS::PRINTF;
        case InterCodeNS::PRINT:return QuaternionNS::PRINT;
        case InterCodeNS::PASS: return QuaternionNS::PASS;
        case InterCodeNS::CONDEF:return QuaternionNS::CONDEF;
        default:{
            perror("unexpected InterCodeNS::Operator");
            exit(-1);
        }
    }
}

void Quaternion::Println() const{
    PrintOperator(op);
    std::cout << " ";
    if(arg_left){
        arg_left->Print();
    } else{
        std::cout << "-";
    }
    std::cout << " ";
    if(arg_right){
        arg_right->Print();
    } else{
        std::cout << "-";
    }
    std::cout << " ";
    if(result){
        result->Print();
    } else{
        std::cout << "-";
    }
    std::cout << std::endl;
}

void Quaternion::PrintOperator(QuaternionNS::Operator op){
    switch(op){
        case QuaternionNS::ADD:std::cout << "ADD";
            break;
        case QuaternionNS::SUB:std::cout << "SUB";
            break;
        case QuaternionNS::MUL:std::cout << "MUL";
            break;
        case QuaternionNS::DIV:std::cout << "DIV";
            break;
        case QuaternionNS::MOD:std::cout << "MOD";
            break;
        case QuaternionNS::POS:std::cout << "POS";
            break;
        case QuaternionNS::NEG:std::cout << "NEG";
            break;
        case QuaternionNS::LSS:std::cout << "LSS";
            break;
        case QuaternionNS::GRE:std::cout << "GRE";
            break;
        case QuaternionNS::LEQ:std::cout << "LEQ";
            break;
        case QuaternionNS::GEQ:std::cout << "GEQ";
            break;
        case QuaternionNS::EQ:std::cout << "EQ";
            break;
        case QuaternionNS::NEQ:std::cout << "NEQ";
            break;
        case QuaternionNS::AND:std::cout << "AND";
            break;
        case QuaternionNS::OR:std::cout << "OR";
            break;
        case QuaternionNS::NOT:std::cout << "NOT";
            break;
        case QuaternionNS::ASS:std::cout << "ASS";
            break;
        case QuaternionNS::REF:std::cout << "REF";
            break;
        case QuaternionNS::LOAD:std::cout << "LOAD";
            break;
        case QuaternionNS::STORE:std::cout << "STORE";
            break;
        case QuaternionNS::BR:std::cout << "BR";
            break;
        case QuaternionNS::BRT:std::cout << "BRT";
            break;
        case QuaternionNS::BRF:std::cout << "BRF";
            break;
        case QuaternionNS::FUNCDEF:std::cout << "FUNCDEF";
            break;
        case QuaternionNS::MAIN:std::cout << "MAIN";
            break;
        case QuaternionNS::PARAM:std::cout << "PARAM";
            break;
        case QuaternionNS::CALL:std::cout << "CALL";
            break;
        case QuaternionNS::RETVAL:std::cout << "RETVAL";
            break;
        case QuaternionNS::RET:std::cout << "RET";
            break;
        case QuaternionNS::GETINT:std::cout << "GETINT";
            break;
        case QuaternionNS::PRINTF:std::cout << "PRINTF";
            break;
        case QuaternionNS::PRINT:std::cout << "PRINT";
            break;
        case QuaternionNS::PASS:std::cout << "PASS";
            break;
        case QuaternionNS::CONDEF:std::cout << "CONDEF";
            break;
    }
}
