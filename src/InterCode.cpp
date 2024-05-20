//
// Created by Edot on 2023/10/25.
//

#include "InterCode.h"
#include "TokenValue.h"
#include <iostream>

UglyQuaternion::UglyQuaternion(int id,
                               InterCodeNS::Operator op,
                               const UglyQuaternionItem &arg_left,
                               const UglyQuaternionItem &arg_right,
                               const UglyQuaternionItem &result)
    : id(id), op(op), arg_left(arg_left), arg_right(arg_right), result(result){
}

std::ostream &operator<<(std::ostream &out, UglyQuaternionItem item){
    switch(item.type){
        case InterCodeNS::NAME:{
            if(item.value.name_value->is_global){
                out << "(global)";
            } else{
                out << "(local)";
            }
            switch(item.value.name_value->id_class){
                case VAR_INT:
                case CONST_INT:{
                    out << "INT:";
                    break;
                }
                case FUNC:{
                    out << "FUNC:";
                    break;
                }
                case TEMP_INT:{
                    if(item.value.name_value->IsTemp()){
                        out << "INT:";
                    }
                    break;
                }
                case TEMP_STR:{
                    out << "STR:";
                    break;
                }
            }
            if(item.value.name_value->is_pointer){
                for(int i = 0; i < item.value.name_value->GetIdDimension(); ++i){
                    out << "*";
                }
                if(item.value.name_value->GetIdDimension() == 0){
                    out << "(*)";
                }
            } else if(item.value.name_value->GetIdDimension() > 0){
                for(int i = 0; i < item.value.name_value->GetIdDimension(); ++i){
                    out << "[]";
                }
            }
            if(item.value.name_value->id_class == TEMP_STR){
                out << item.value.name_value->strcon_value << "+"
                    << item.value.name_value->address;
            } else if(item.value.name_value->id_class == FUNC){
                out << item.value.name_value->GetName();
            } else{
                out << item.value.name_value->GetName() << "+"
                    << item.value.name_value->address;
            }
            break;
        }
        case InterCodeNS::IMM:{
            out << item.value.numeric_value;
            break;
        }
        case InterCodeNS::ID:{
            out << "ID" << item.value.numeric_value;
            break;
        }
        case InterCodeNS::EMPTY:{
            out << "-";
            break;
        }
    }
    return out;
}

void UglyQuaternion::Print() const{
    std::string op_str;
    switch(op){
        case InterCodeNS::ADD: op_str = "ADD";
            break;
        case InterCodeNS::SUB: op_str = "SUB";
            break;
        case InterCodeNS::MUL: op_str = "MUL";
            break;
        case InterCodeNS::DIV: op_str = "DIV";
            break;
        case InterCodeNS::MOD: op_str = "MOD";
            break;
        case InterCodeNS::POS: op_str = "POS";
            break;
        case InterCodeNS::NEG: op_str = "NEG";
            break;
        case InterCodeNS::LSS: op_str = "LSS";
            break;
        case InterCodeNS::GRE: op_str = "GRE";
            break;
        case InterCodeNS::LEQ: op_str = "LEQ";
            break;
        case InterCodeNS::GEQ: op_str = "GEQ";
            break;
        case InterCodeNS::EQ: op_str = "EQ";
            break;
        case InterCodeNS::NEQ: op_str = "NEQ";
            break;
        case InterCodeNS::AND: op_str = "AND";
            break;
        case InterCodeNS::OR: op_str = "OR";
            break;
        case InterCodeNS::NOT: op_str = "NOT";
            break;
        case InterCodeNS::ASS: op_str = "ASS";
            break;
        case InterCodeNS::REF: op_str = "REF";
            break;
        case InterCodeNS::LOAD: op_str = "LOAD";
            break;
        case InterCodeNS::STORE: op_str = "STORE";
            break;
        case InterCodeNS::BR: op_str = "BR";
            break;
        case InterCodeNS::BRT: op_str = "BRT";
            break;
        case InterCodeNS::BRF: op_str = "BRF";
            break;
        case InterCodeNS::FUNC: op_str = "FUNC";
            break;
        case InterCodeNS::MAIN: op_str = "MAIN";
            break;
        case InterCodeNS::PARAM: op_str = "PARAM";
            break;
        case InterCodeNS::CALL: op_str = "CALL";
            break;
        case InterCodeNS::RETVAL: op_str = "RETVAL";
            break;
        case InterCodeNS::RET: op_str = "RET";
            break;
        case InterCodeNS::GETINT: op_str = "GETINT";
            break;
        case InterCodeNS::PRINTF: op_str = "PRINTF";
            break;
        case InterCodeNS::PRINT: op_str = "PRINT";
            break;
        case InterCodeNS::BREAK: op_str = "BREAK";
            break;
        case InterCodeNS::CONTINUE: op_str = "CONTINUE";
            break;
        case InterCodeNS::FORIT: op_str = "FORIT";
            break;
        case InterCodeNS::PASS: op_str = "PASS";
            break;
        case InterCodeNS::CONDEF: op_str = "CONDEF";
            break;
    }
    std::cout << id << " " << op_str << " " << arg_left << " " << arg_right << " " << result << std::endl;
}

int InterCode::AddCode(InterCodeNS::Operator op,
                       const UglyQuaternionItem &arg_left,
                       const UglyQuaternionItem &arg_right,
                       const UglyQuaternionItem &result){
    ugly_codes[current_id] = new UglyQuaternion(current_id,
                                                op,
                                                arg_left,
                                                arg_right,
                                                result);
    current_id++;
    return current_id - 1;

}

UglyQuaternion *InterCode::GetQuaternion(int id){
    return ugly_codes[id];
}

UglyQuaternionItem InterCode::MakeLabel(int id){
    UglyQuaternionItem item;
    item.type = InterCodeNS::ID;
    item.value.numeric_value = id;
    return item;
}

UglyQuaternionItem InterCode::MakeImm(int imm){
    UglyQuaternionItem item;
    item.type = InterCodeNS::IMM;
    item.value.numeric_value = imm;
    return item;
}

UglyQuaternionItem InterCode::MakeName(TokenValue *entry){
    UglyQuaternionItem item;
    item.type = InterCodeNS::NAME;
    item.value.name_value = entry;
    return item;
}

int InterCode::GetCurrentId() const{
    return current_id;
}

void InterCode::ForItBegin(){
    AddCode(InterCodeNS::FORIT, {}, {}, {});
}

void InterCode::ForEnd(int it_start){
    for(int i = current_id - 1; i >= 0; i--){
        if(ugly_codes[i]->op == InterCodeNS::BREAK){
            ugly_codes[i]->op = InterCodeNS::BR;
            ugly_codes[i]->result = MakeLabel(current_id);
        }
        if(ugly_codes[i]->op == InterCodeNS::CONTINUE){
            ugly_codes[i]->op = InterCodeNS::BR;
            ugly_codes[i]->result = MakeLabel(it_start);
        }
        if(ugly_codes[i]->op == InterCodeNS::FORIT){
            ugly_codes[i]->op = InterCodeNS::PASS;
            break;
        }
    }
}

void InterCode::AddForBreak(){
    AddCode(InterCodeNS::BREAK, {}, {}, {});
}

void InterCode::AddForContinue(){
    AddCode(InterCodeNS::CONTINUE, {}, {}, {});
}

void InterCode::AddMainFunc(){
    AddCode(InterCodeNS::MAIN, {}, {}, {});
}

void InterCode::Print(){
    for(int i = 1; i < current_id; ++i){
        ugly_codes[i]->Print();
    }
}

std::string InterCode::GetTempName(){
    current_temp++;
    return "_t" + std::to_string(current_temp);
}

