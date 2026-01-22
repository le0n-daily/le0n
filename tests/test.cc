#include "../le0n/log.h"
#include <thread>
#include <iostream>
#include "../le0n/util.h"

/**
 * 这个文件用于帮助你深入理解日志模块的 "语法机制" 和 "架构设计"。
 * 我们通过手动模拟宏展开、自定义组件来拆解它的工作原理。
 */

// ==========================================
// 阶段二辅助：自定义一个 Appender
// 目的：验证 Logger 是如何分发日志给 Appender 的
// ==========================================
class MyCustomAppender : public le0n::LogAppender {
public:
    virtual void log(std::shared_ptr<le0n::Logger> logger, le0n::LogLevel::Level level, le0n::LogEvent::ptr event) override {
        // 证明：Logger 确实调用了我的 log 方法
        std::cout << "[MyCustomAppender] 收到日志！内容是: " << event->getContent() << std::endl;
    }
};

void test_phase_1_syntax() {
    std::cout << "========== 阶段一：语法与机制 (RAII 与 宏展开) ==========" << std::endl;
    
    le0n::Logger::ptr logger(new le0n::Logger);// 智能指针创建 Logger 实例（对象）
    le0n::LogAppender::ptr appender(new le0n::StdoutLogAppender);
    appender->setLevel(le0n::LogLevel::DEBUG); // [Bug修复] 原代码未初始化 m_level，导致可能是垃圾值
    logger->addAppender(appender);

    // ---------------------------------------------------------
    // 实验 1：宏的“真面目”
    // 你平时写的是：LE0N_LOG_INFO(logger) << "test macro";
    // 下面是手动展开后的代码（去掉了 if 判断优化，只保留核心）：
    // ---------------------------------------------------------
    
    std::cout << "--- 开始模拟宏展开 ---" << std::endl;
    {
        // 1. 创建 Event (数据)
        le0n::LogEvent::ptr event(new le0n::LogEvent(logger, le0n::LogLevel::INFO, 
                                    __FILE__, __LINE__, 0, le0n::GetThreadId(), 
                                    le0n::GetFiberId(), time(0)));
        
        // 2. 创建 Wrap (触发器)
        le0n::LogEventWrap wrap(event);

        // 3. 写入内容 (流式操作)
        // wrap.getSS() 返回 stringstream，所以可以使用 <<
        wrap.getSS() << "这是手动展开宏生成的日志";

        // 4. 遇到右大括号 '}'，wrap 对象销毁
        // --> 触发 ~LogEventWrap() 析构函数
        // --> 析构函数内部调用 logger->log(event->getLevel(), event)
        // --> 日志被输出
    } 
    std::cout << "--- 模拟宏展开结束 ---" << std::endl;
    
    // 思考：如果 LogEventWrap 没有析构函数，或者没有把 event 存起来，日志还能打印吗？
}

void test_phase_2_architecture() {
    std::cout << "\n========== 阶段二：架构与设计 (模块解耦) ==========" << std::endl;

    // 1. 创建管理员 (Logger)
    le0n::Logger::ptr logger(new le0n::Logger);
    // 清空默认的 Appender，方便观察
    // (注意：目前的 Logger 实现没有 clearAppender 接口，我们手动添加新的)

    // 2. 创建并挂载我们的自定义 Appender
    le0n::LogAppender::ptr my_appender(new MyCustomAppender());
    // 给它设置一个简单的格式器，只输出消息本身
    le0n::LogFormatter::ptr simple_fmt(new le0n::LogFormatter("%m%n")); 
    my_appender->setFormatter(simple_fmt);
    
    logger->addAppender(my_appender);

    // 3. 再挂载一个标准的文件 Appender
    le0n::FileLogAppender::ptr file_appender(new le0n::FileLogAppender("./test_arch.log"));
    // 给文件设置一个复杂的格式
    le0n::LogFormatter::ptr complex_fmt(new le0n::LogFormatter("[File] %d %m%n"));
    file_appender->setFormatter(complex_fmt);
    
    logger->addAppender(file_appender);

    // 4. 发送一条日志
    std::cout << "--- 发送一条日志，观察分发过程 ---" << std::endl;
    LE0N_LOG_INFO(logger) << "这条日志会被分发给两个 Appender";

    // 验证：
    // 控制台应该会由 MyCustomAppender 打印 "[MyCustomAppender] ..."
    // 文件 test_arch.log 应该会有 "[File] 202x... 这条日志..."
    
    std::cout << "--- 分发结束，请检查 test_arch.log ---" << std::endl;
}

int main(int argc, char** argv) {
    test_phase_1_syntax();
    test_phase_2_architecture();
    return 0;
}
