//
// Created by Edot on 2023/12/19.
//

#ifndef EXP_CPP_OPTIMIZER_H_
#define EXP_CPP_OPTIMIZER_H_
#include <vector>
#include "Quaternion.h"

struct FlowChartManager;

struct Optimizer{
    void Optimize(FlowChartManager *flow_chart_manager);
    int global_register_count = 8; // 全局寄存器数量
    static bool MarkUsed(std::vector<long long> &used_var, QuaternionNS::QuaternionItem *item);
    static bool Used(const std::vector<long long> &used_var, QuaternionNS::QuaternionItem *item);
};

#endif //EXP_CPP_OPTIMIZER_H_
