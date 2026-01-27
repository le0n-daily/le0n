#include "le0n/config.h"
#include "le0n/log.h"
#include <algorithm>
#include <list>
#include <sstream>
#include <string>
#include <utility>

namespace le0n{

Config::ConfigVarMap Config::s_datas;

// 查找配置项基类，找不到返回 nullptr
ConfigVarBase::ptr Config::LookupBase(const std::string& name){
    auto it = s_datas.find(name);
    return it == s_datas.end() ? nullptr : it -> second;
}

//"A.B", 10
//A:
//  B: 10
//  C: str

// 辅助函数：递归遍历 YAML 节点
// 作用：将层级化的 YAML 结构（如 A.B.C）打平成 key-value 形式（key="A.B.C", value=node）
// prefix: 当前路径前缀（如 "system"）
// node: 当前遍历到的 YAML 节点
// output: 输出列表，存所有解析出的叶子节点
static void ListAllMember(const std::string& prefix,
                        const YAML::Node& node,
                        std::list<std::pair<std::string, const YAML::Node>>& output){
            // 检查 key 是否包含非法字符
            if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._1234567890")
                    != std::string::npos){
                        LE0N_LOG_INFO(LE0N_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
                        return;
            }
            output.push_back(std::make_pair(prefix, node));
            if(node.IsMap()){
                for(auto it = node.begin();
                        it != node.end(); ++it){
                                // 递归：如果是 Map，继续往下找，key 拼接上 "."
                                ListAllMember(prefix.empty() ? it->first.Scalar() 
                                        : prefix + "." + it->first.Scalar(), it->second, output);
                        }
            }
}

// 从 YAML 加载配置
// 1. 遍历 YAML 所有节点，打平成 key-value。
// 2. 遍历打平后的列表，查找是否有对应的 ConfigVar。
// 3. 如果有，调用 fromString 更新配置值。
void Config::LoadFromYaml(const YAML::Node &root){
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);

    for(auto& i : all_nodes){
        std::string key = i.first;
        if(key.empty()){
            continue;
        }

        // 统一转小写，保证大小写不敏感
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var = LookupBase(key);
        
        // 如果这个 key 在代码里注册过 (Lookup 过了)
        if(var){
            if(i.second.IsScalar()){
                var->fromString(i.second.Scalar());
            } else {
                // 如果是复杂类型 (Sequence/Map)，转成 string 再交给 fromString (后续支持)
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}

}