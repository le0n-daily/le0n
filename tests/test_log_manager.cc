#include "../le0n/log.h"
#include "../le0n/util.h"
#include <iostream>
#include <cassert>

/**
 * 这个文件用于演示和测试 LoggerManager（日志管理器）以及 Singleton（单例模式）的作用。
 * 
 * 核心概念：
 * 1. 单例模式 (Singleton)：确保整个程序中只有一个 LoggerManager 实例。
 *    这就好比一个公司只有一个 "档案室管理员"。无论你在公司的哪个部门（哪个文件、哪个线程），
 *    只要你喊一声 "管理员"，出来的永远是同一个人。
 * 
 * 2. LoggerManager：负责管理所有的 Logger。
 *    它像是一个 "电话簿"，你可以通过名字（如 "system", "network", "db"）来获取对应的 Logger。
 *    如果没有这个名字的 Logger，它通常会返回一个默认的（root），或者创建一个新的（取决于具体实现）。
 */

void test_singleton_mechanism() {
    std::cout << "========== 实验 1: 验证单例模式的唯一性 ==========" << std::endl;
    
    // 获取第一次
    le0n::LoggerManager* mgr1 = le0n::LoggerMgr::GetInstance();
    
    // 获取第二次
    le0n::LoggerManager* mgr2 = le0n::LoggerMgr::GetInstance();
    
    std::cout << "mgr1 地址: " << mgr1 << std::endl;
    std::cout << "mgr2 地址: " << mgr2 << std::endl;
    
    if (mgr1 == mgr2) {
        std::cout << "✅ 验证成功：两个指针指向同一个对象！这就是单例。" << std::endl;
    } else {
        std::cout << "❌ 验证失败：单例模式未生效！" << std::endl;
    }
}

void test_logger_management() {
    std::cout << "\n========== 实验 2: LoggerManager 的管理能力 ==========" << std::endl;
    
    // 1. 获取 root logger (默认的)
    // 根据 log.cc 的实现，getLogger 如果找不到名字，会返回 root
    le0n::Logger::ptr root = le0n::LoggerMgr::GetInstance()->getLogger("root");
    std::cout << "Root Logger Name: " << root->getName() << std::endl;
    
    // 2. 尝试获取一个不存在的 Logger，比如 "system"
    // 注意：目前的简单实现中，getLogger 找不到时返回 m_root。
    // 这意味着你无法通过 getLogger 自动创建新 Logger，除非手动去 m_loggers 里插。
    // *但是在完整的日志系统中，getLogger("xxx") 通常意味着“给我一个叫xxx的logger，没有就造一个”*
    // 我们来看看当前的实现行为：
    le0n::Logger::ptr sys_logger = le0n::LoggerMgr::GetInstance()->getLogger("system");
    
    std::cout << "获取 'system' logger，实际得到的 Name: " << sys_logger->getName() << std::endl;
    
    if (sys_logger == root) {
        std::cout << "💡 发现：当前的 getLogger 实现比较简单，找不到时回退到了 root。" << std::endl;
        std::cout << "   这意味着我们现在只能用一个全局的 logger，或者需要手动扩展 LoggerManager。" << std::endl;
    } else {
        std::cout << "💡 发现：getLogger 自动创建了新的 logger！" << std::endl;
    }

    // 3. 验证全局性
    // 无论在哪里调用，拿到的都是同一个管理器，这保证了配置的一致性。
    LE0N_LOG_INFO(root) << "通过管理器获取的 Root Logger 打印日志";
}

int main() {
    test_singleton_mechanism();
    test_logger_management();
    return 0;
}
