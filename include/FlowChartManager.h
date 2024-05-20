//
// Created by Edot on 2023/11/7.
//

#ifndef EXP_CPP_FLOWCHART_MANAGER_H_
#define EXP_CPP_FLOWCHART_MANAGER_H_
#include <vector>

struct Quaternion;

namespace FlowChartManagerNS{
    struct BasicBlock{
        explicit BasicBlock(int id);
        int id;
        std::vector<BasicBlock *> precursor; // ǰ��
        std::vector<BasicBlock *> successor; // ���
        std::vector<Quaternion *> codes; // ��Ԫʽ
    };
}

struct FlowChartManager{
    void FillFlowChart(const char *src_path, const char *err_path);
    void Println();
    std::vector<FlowChartManagerNS::BasicBlock *> flow_chart;
    bool has_error = false;
};

#endif //EXP_CPP_FLOWCHART_MANAGER_H_
