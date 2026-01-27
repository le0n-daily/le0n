#include "le0n/config.h"
#include "le0n/log.h"
#include <cstddef>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

le0n::ConfigVar<int>::ptr g_int_value_config = 
    le0n::Config::Lookup("system.port", (int)8080, "system port");
le0n::ConfigVar<float>::ptr g_float_value_config = 
    le0n::Config::Lookup("system.value", (float)10.2f, "system value");

void print_yaml(const YAML::Node& node, int level){
    // IsScalar: 判断是否为标量（最基础的值，如字符串、数字）
    if(node.IsScalar()){
        LE0N_LOG_INFO(LE0N_LOG_ROOT()) << std::string(level * 4, ' ') // 打印缩进
            << node.Scalar() << " - " << node.Type() << "- " << level; // 打印值 - 类型 - 层级
    } else if(node.IsNull()){
        LE0N_LOG_INFO(LE0N_LOG_ROOT()) << std::string(level * 4, ' ')
            << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()){
        // IsMap: 判断是否为 Map (key-value 键值对)
        // 遍历 Map，it->first 是 key，it->second 是 value (Node)
        for(auto it = node.begin();
            it != node.end(); ++it){
                LE0N_LOG_INFO(LE0N_LOG_ROOT()) << std::string(level * 4, ' ')   
                    << it->first << " - " << it->second.Type() << " - " << level;
                // 递归调用：因为 value 可能还是一个复杂的结构（Map 或 Sequence），所以要进去继续打印
                print_yaml(it -> second, level +1);
            }
    } else if(node.IsSequence()){
        // IsSequence: 判断是否为序列 (数组/列表)，对应 YAML 里的 "-"
        for(size_t i = 0; i < node.size(); ++i){
            LE0N_LOG_INFO(LE0N_LOG_ROOT()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level; // 打印索引
            // 递归调用：打印数组里的每一个元素
            print_yaml(node[i], level +1);
        }
    }
}

void test_yaml(){
    YAML::Node root = YAML::LoadFile("/home/le0n/workspace/le0n/bin/conf/log.yml");
    
    print_yaml(root, 0);

    // 这里可以直接输出 root，因为 yaml-cpp 重载了 operator<<
    //LE0N_LOG_INFO(LE0N_LOG_ROOT()) << root;
}

int main(int argc, char** argv){
    LE0N_LOG_INFO(LE0N_LOG_ROOT()) << g_int_value_config->getValue();
    LE0N_LOG_INFO(LE0N_LOG_ROOT()) << g_float_value_config->toString();

    test_yaml();
    
    return 0;
}