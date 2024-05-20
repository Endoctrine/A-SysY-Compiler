//
// Created by Edot on 2023/11/8.
//

#ifndef EXP_CPP_TARGET_CODE_GENERATOR_H_
#define EXP_CPP_TARGET_CODE_GENERATOR_H_
#include <map>
#include "Quaternion.h"
#include "FlowChartManager.h"

struct FlowChartManager;
class RegisterPool;

struct TargetCodeGenerator{
    void GenerateTargetCode();
    void SynthesizeGlobalName();
    void GenerateForGlobalName(const std::map<int, QuaternionNS::QuaternionItem *> &global_name_map);
    void GenerateForEachBBlock();
    FlowChartManager *flow_chart_manager = nullptr;
    RegisterPool *register_pool = nullptr;

    void GenerateForCommonQuaternion(Quaternion *quaternion);
    void GenerateForCallQuaternion(std::vector<FlowChartManagerNS::BasicBlock *>::iterator block_iter,
                                   std::vector<Quaternion *>::iterator quaternion_iter);

  private:
    void TryPutGlobalNameMap(std::map<int, QuaternionNS::QuaternionItem *> &global_name_map,
                             QuaternionNS::QuaternionItem *arg);
    int stack_base = 0x7ffffffc;
    int string_count = 0; // 用来计数当前PRINT的是第几个STR，其实就是用来命名标签的
};

#endif //EXP_CPP_TARGET_CODE_GENERATOR_H_
