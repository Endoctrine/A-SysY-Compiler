//
// Created by Edot on 2023/10/9.
//

#include "TokenValue.h"

#include <utility>
TokenValue::TokenValue(std::string name, IdClass id_class, int id_dimension, bool is_pointer)
    : name(std::move(name)), id_class(id_class), id_dimension(id_dimension), is_pointer(is_pointer){
}

void TokenValue::AddValue(int value){
    values.push_back(value);
}

void TokenValue::AddAxis(int axis){
    shape.push_back(axis);
}

void TokenValue::AddParameter(TokenValue *parameter){
    parameters.push_back(parameter);
}

bool TokenValue::IsSameType(TokenValue *token_value) const{
    if(id_class != token_value->id_class){
        return false;
    }
    if(id_dimension != token_value->id_dimension){
        return false;
    }
    if(id_class == FUNC){
        if(parameters.size() != token_value->parameters.size()){
            return false;
        }
        for(int i = 0; i < parameters.size(); ++i){
            if(!parameters[i]->IsSameType(token_value->parameters[i])){
                return false;
            }
        }
    }
    return true;
}

int TokenValue::GetValue() const{
    if(id_class == CONST_INT){
        return values[0];
    } else{
        return static_cast<int>(0xcccccccc);
    }
}

int TokenValue::GetValue(int index) const{
    if(id_class == CONST_INT){
        if(index >= values.size()){
            perror("index out of bound!");
            return 0;
        }
        return values[index];
    } else{
        return static_cast<int>(0xcccccccc);
    }
}

int TokenValue::GetValue(int index_1, int index_2) const{
    if(id_class == CONST_INT){
        if(index_1 * shape[1] + index_2 >= values.size()){
            perror("index out of bound!");
            return 0;
        }
        return values[index_1 * shape[1] + index_2];
    } else{
        return static_cast<int>(0xcccccccc);
    }
}

int TokenValue::GetValue(const std::vector<int> &indexes){
    if(id_class == CONST_INT){
        int offset = 0;
        int i = 1;
        for(int index : indexes){
            if(i < id_dimension){
                offset += index * shape[i];
            } else{
                offset += index;
            }
            i++;
        }
        return values[offset];
    } else{
        return static_cast<int>(0xcccccccc);
    }
}

int TokenValue::ValuesSize() const{
    return static_cast<int>(values.size()) * TokenValue::TypeSize();
}

int TokenValue::Size() const{
    if(is_pointer){
        return 4;
    }
    int result = TypeSize();
    for(int i = 0; i < id_dimension; ++i){
        result *= shape[i];
    }
    return result;
}

int TokenValue::GetIdDimension() const{
    return id_dimension;
}

int TokenValue::ParametersSize() const{
    return static_cast<int>(parameters.size());
}

bool TokenValue::IsParamTypeMatched(std::vector<int> &params_dimensions) const{
    for(int i = 0; i < parameters.size() && i < params_dimensions.size(); ++i){
        if(parameters[i]->id_dimension != params_dimensions[i]){
            return false;
        }
    }
    return true;
}

bool TokenValue::NotVar() const{
    return id_class != VAR_INT;
}

const std::string &TokenValue::GetName() const{
    return name;
}

int TokenValue::TypeSize() const{
    switch(id_class){
        case TEMP_STR:
            return static_cast<int>(strcon_value.length()) + 1 + 4 - (static_cast<int>(name.length()) + 1) % 4;
        default:return 4;
    }
}
bool TokenValue::IsTemp() const{
    return id_class == TEMP_INT || id_class == TEMP_STR;
}



