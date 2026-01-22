#ifndef __LE0N_LOG_H__
#define __LE0N_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include "singleton.h"

/**
 * @brief 使用流式方式将日志级别level的日志写入到logger
 * 
 * 核心逻辑：
 * 1. 检查日志级别是否允许输出。
 * 2. 创建一个 LogEvent 智能指针，封装了当前文件、行号、时间等信息。
 * 3. 使用 LogEventWrap 包装这个 Event。
 * 4. LogEventWrap::getSS() 返回一个 stringstream，用户可以使用 << 写入消息。
 * 5. 宏结束处，LogEventWrap 临时对象析构，在析构函数中调用 logger->log() 提交日志。
 */
#define LE0N_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        le0n::LogEventWrap(le0n::LogEvent::ptr(new le0n::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, le0n::GetThreadId(), \
                le0n::GetFiberId(), time(0)))).getSS()

// 各种级别的流式日志宏
#define LE0N_LOG_DEBUG(logger) LE0N_LOG_LEVEL(logger, le0n::LogLevel::DEBUG)
#define LE0N_LOG_INFO(logger) LE0N_LOG_LEVEL(logger, le0n::LogLevel::INFO)
#define LE0N_LOG_WARN(logger) LE0N_LOG_LEVEL(logger, le0n::LogLevel::WARN)
#define LE0N_LOG_ERROR(logger) LE0N_LOG_LEVEL(logger, le0n::LogLevel::ERROR)
#define LE0N_LOG_FATAL(logger) LE0N_LOG_LEVEL(logger, le0n::LogLevel::FATAL)

/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 * 
 * 核心逻辑与流式类似，区别在于直接调用 format 方法进行 printf 风格的格式化。
 */
#define LE0N_LOG_FMT_LEVEL(logger, level, fmt, ...) \
        if(logger->getLevel() <= level) \
            le0n::LogEventWrap(le0n::LogEvent::ptr(new le0n::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, le0n::GetThreadId(), \
                le0n::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

// 各种级别的格式化日志宏
#define LE0N_LOG_FMT_DEBUG(logger, fmt, ...) LE0N_LOG_FMT_LEVEL(logger, le0n::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define LE0N_LOG_FMT_INFO(logger, fmt, ...) LE0N_LOG_FMT_LEVEL(logger, le0n::LogLevel::INFO, fmt, __VA_ARGS__)
#define LE0N_LOG_FMT_WARN(logger, fmt, ...) LE0N_LOG_FMT_LEVEL(logger, le0n::LogLevel::WARN, fmt, __VA_ARGS__)
#define LE0N_LOG_FMT_ERROR(logger, fmt, ...) LE0N_LOG_FMT_LEVEL(logger, le0n::LogLevel::ERROR, fmt, __VA_ARGS__)
#define LE0N_LOG_FMT_FATAL(logger, fmt, ...) LE0N_LOG_FMT_LEVEL(logger, le0n::LogLevel::FATAL, fmt, __VA_ARGS__)

namespace le0n{

class Logger;

// 日志级别：用于区分日志的重要性，便于过滤
// 比如：只看 ERROR 级别的日志，忽略 DEBUG
class LogLevel{
public:
    enum Level{
        UNKNOWN = 0,
        DEBUG = 1,  // 调试信息
        INFO = 2,   // 一般信息
        WARN = 3,   // 警告信息
        ERROR = 4,  // 错误信息
        FATAL = 5   // 致命错误
    };
    /**
     * @brief 将日志级别转换为字符串用于输出
     */
    static const char* ToString(LogLevel::Level level);
    /**
    * C++ const 用法精要（极简版）
    * 🎯 核心原则：能加就加，就近原则
    * 
    * 🔑 五大场景：
    * 
    * 1. 变量常量：值不可变
    *    const int MAX = 100;
    * 
    * 2. 指针三剑客（口诀：左定右动）：
    *    const int* p;    // 左定：数据常量，*p不可改
    *    int* const p;    // 右动：指针常量，p不可改
    *    const int* const p; // 两者都常量
    * 
    * 3. 函数参数（防修改+高效）：
    *    void func(const std::string& msg); // 参数只读，避免拷贝
    * 
    * 4. 成员函数（不修改对象状态）：
    *    int getValue() const; // 承诺不修改成员变量
    * 
    * 5. 双重保护（最安全）：
    *    void log(const std::string& msg) const;
    *    // 参数msg只读 + 对象状态只读
    * 
    * 🧠 记忆口诀：
    * "参数const防改入，函数const防改己"
    * "左定值，右定址"
    * 
    * ✅ 检查表：
    * - 不修改的变量 → 加const
    * - 不修改的参数 → 用const&
    * - 不修改成员的函数 → 末尾加const  
    * - 返回内部数据 → 返回const类型
    * 
    * 💡 黄金法则：const是安全带，能系就系！
    */
};

// 日志事件：封装了日志发生瞬间的所有信息（时间、位置、线程、内容等）
// 作用：数据传输对象 (DTO)。它封装了日志发生那一瞬间的所有上下文信息。将这些散落的信息打包，方便传递给 Format 和 Appender
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    /**
     * @brief 构造函数
     * @param[in] logger 日志器
     * @param[in] level 日志级别
     * @param[in] file 文件名
     * @param[in] line 文件行号
     * @param[in] elapse 程序启动依赖的耗时(毫秒)
     * @param[in] thread_id 线程id
     * @param[in] fiber_id 协程id
     * @param[in] time 日志事件(秒)
     * @param[in] thread_name 线程名称
     */
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            , const char* file, int32_t m_line, uint32_t elapse
            , uint32_t thread_id, uint32_t fiber_id, uint64_t time);
    ~LogEvent();

    const char* getFile() const {return m_file;}
    int32_t getLine() const {return m_line;}
    uint32_t getElapse() const {return m_elapse;}
    uint32_t getThreadId() const {return m_threadId;}
    uint32_t getFiberId() const {return m_fiberId;}
    uint64_t getTime() const {return m_time;}
    
    // 获取日志内容（用户通过 << 写入的部分）
    std::string getContent() const {return m_ss.str();}
    std::shared_ptr<Logger> getLogger() const {return m_logger;}
    LogLevel::Level getLevel() const {return m_level;}

    // 获取 stringstream，主要用于流式日志写入
    std::stringstream& getSS() {return m_ss;}
    /**
     * @brief 使用格式化字符串格式化日志内容
     * @param[in] fmt 格式化字符串
     * @param[in] ... 可变参数
     */
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);
private:
    const char* m_file = nullptr;   //文件名
    int32_t m_line = 0;             //行号
    uint32_t m_elapse = 0;          //程序启动到现在的毫秒数
    uint32_t m_threadId = 0;        //线程id
    uint32_t m_fiberId = 0;         //协程id
    uint64_t m_time = 0;            //时间戳
    std::stringstream m_ss;         //日志内容（消息体）

    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

/**
 * @brief 日志事件包装器
 * @details 这是一个 RAII 风格的类。它的存在是为了实现“自动提交”。
 * 当这个对象被创建时，它持有 LogEvent。
 * 当这个对象被销毁时（析构函数），它会自动调用 logger->log() 将 Event 提交出去。
 */
class LogEventWrap{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();    //LogEventWrap 利用析构函数触发真正写日志的操作
    LogEvent::ptr getEvent() const { return m_event;}
    std::stringstream& getSS();
private:
    LogEvent::ptr m_event;
};

/**
 * @brief 日志格式器：负责将 LogEvent 对象转换成字符串
 */
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    /**
     * @brief 构造函数
     * @param[in] pattern 格式模板
     * @details 
     *  %m 消息
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
