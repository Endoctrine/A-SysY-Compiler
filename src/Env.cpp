//
// Created by Edot on 2023/10/9.
//

#include "Env.h"
#include "TokenValue.h"
#include <sstream>

Env::Env(Env *prev) : prev(prev){
    if(prev == nullptr){
        current_address = 0;
    } else{
        current_address = 4; // 对于非全局变量，从+4开始编址，因为+0用于保存返回地址了
    }
}

Env *Env::GetPrev() const{
    return prev;
}

bool Env::TryPut(const std::string &name, TokenValue *entry){
    if(disabled){
        // 如果处于中间变量抑制状态，报错
        perror("Env::TryPut: temp var generating has been blocked. something is wrong!");
    }
    if(entry->id_class == TEMP_STR){
        Env *global_env = this;
        while(global_env->prev != nullptr){
            global_env = global_env->prev;
        }
        symbol_table["s_" + name] = entry;
        entry->is_global = true;
        entry->address = global_env->current_address;
        global_env->current_address += entry->Size();
        return true;
    } else if(!entry->IsTemp()){
        if(symbol_table.find("w_" + name) != symbol_table.end()){
            return false;
        } else{
            symbol_table["w_" + name] = entry;
            if(prev == nullptr){
                entry->is_global = true;
                entry->address = current_address;
                current_address += entry->Size();
            } else{
                Env *func_env = this;
                while(func_env->prev->prev != nullptr){
                    func_env = func_env->prev;
                }
                entry->is_global = false;
                entry->address = func_env->current_address;
                func_env->current_address += entry->Size();
            }
            return true;
        }
    } else{
        symbol_table["t_" + name] = entry;
        if(prev == nullptr){
            entry->is_global = true;
            entry->address = current_address;
            current_address += entry->Size();
        } else{
            Env *func_env = this;
            while(func_env->prev->prev != nullptr){
                func_env = func_env->prev;
            }
            entry->is_global = false;
            entry->address = func_env->current_address;
            func_env->current_address += entry->Size();
        }
        return true;
    }
}

TokenValue *Env::Get(const std::string &name){
    if(symbol_table.find("w_" + name) != symbol_table.end()){
        return symbol_table["w_" + name];
    } else{
        if(!prev){
            return nullptr;
        } else{
            return prev->Get(name);
        }
    }
}



