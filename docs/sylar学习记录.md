---
title: sylar cpp webserver学习记录
data: 2025-08-01 16:16
tags:
  - cpp编程开发
categories:
  - cpp编程开发
description: cpp webserver学习笔记
---

# 服务器框架

总的就以自己的名字来命名文件夹的吧

~~~sh
le0n:le0n/ $ pwd 
/home/le0n/workspace/le0n
le0n:le0n/ $ ls
CMakeLists.txt  Makefile  bin  build  cmake  le0n  lib  tests
#bin -- 二进制
#build -- 中间文件路径
#cmake -- cmake函数文件夹
#CMakeList.txt -- cmake的定义文件
#lib -- 库的输出路径
#le0n -- 源代码路径
#tests -- 测试代码
~~~

* 日志系统

~~~text
 1> 
 	Log4J
 	  logger（定义日志类型）
			|
			|---------Formatter（日志格式）
			|
	  Appender（日志输出的地方）
~~~

* 配置系统

Config --> Yaml

yaml-cpp: github 搜

~~~sh
mkdir build && cd build && cmake .. && make install
#也可以直接apt install来安装
~~~

关于yaml的用法

~~~cpp
YAML::Node node = YAML::loadFile(filename);
node.IsMap()
for(auto it = node.begin();it != node.end(); ++it){
    it->first, it->second
}

node.IsSequence()
for(size_t i=0;i < node.size();++i){

}

node.IsScalar();
~~~

完整代码示例：
~~~cpp
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
~~~



* 协程库封装
* socket函数库
* http协议开发

* 分布式协议

--->http解析解析较慢，为了提高性能

* 推荐系统