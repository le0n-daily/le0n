#include "log.h"
#include <map>
#include <iostream>
#include <functional>
#include <time.h>
#include <string.h>
#include <stdarg.h>

namespace le0n{

/**
 * @brief 将日志级别转成字符串
 * 使用宏定义 XX 来减少重复代码，方便维护。
 */
const char* LogLevel::ToString(LogLevel::Level level){
    switch(level){
#define XX(name) \
        case LogLevel::name: \
        return #name;
        break;
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

/**
 * @brief LogEventWrap 构造函数
 * @param e LogEvent 的智能指针
 */
LogEventWrap::LogEventWrap(LogEvent::ptr e)
    :m_event(e) {

}

/**
 * @brief LogEventWrap 析构函数
 * 核心功能：在对象销毁时，自动将日志写入 Logger。
 * 这里的 log 调用是日志系统真正工作的触发点。
 */
LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

/**
 * @brief 格式化写入日志内容
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
void LogEvent::format(const char* fmt, ...){
    va_list al;
    va_start(al,fmt);
    format(fmt, al);
    va_end(al);
}

/**
 * @brief 格式化写入日志内容
 * @param fmt 格式化字符串
 * @param al va_list 结构
 */
void LogEvent::format(const char* fmt, va_list al){
    char* buf = nullptr;
    // vasprintf 会自动分配内存存储格式化后的字符串
    int len = vasprintf(&buf, fmt, al);
    if(len != -1){
        m_ss << std::string(buf, len);
        free(buf); // vasprintf 分配的内存需要手动释放，uaf可能
    }
}

// =========================================================
// 以下是各种 LogFormatter::FormatItem 的具体实现
// 每个类负责解析并输出日志格式中的某一部分
// =========================================================

class MessageFormatItem : public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent(); // %m: 消息体
    }
};

class LevelFormatItem : public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level); // %p: 日志级别
    }
};
  
class ElapseFormatItem : public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse(); // %r: 启动后的毫秒数
    }
};

class NameFormatItem : public LogFormatter::FormatItem{
public:
    NameFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName(); // %c: 日志器名称
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {    
        os << event->getThreadId(); // %t: 线程ID
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId(); // %F: 协程ID
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem{
public:
    // %d{Format}
    DateTimeFormatItem(const std::string& fmt = "%Y:%m:%d %H:%M:%S")
        :m_format(fmt) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm); // 线程安全的时间转换
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem{
public:
    FilenameFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile(); // %f: 文件名
    }
};

class LineFormatItem : public LogFormatter::FormatItem{
public:
    LineFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine(); // %l: 行号
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem{
public:
    NewLineFormatItem(const std::string& str = ""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl; // %n: 换行符
    }
};

class StringFormatItem : public LogFormatter::FormatItem{
public:
    StringFormatItem(const std::string& str)
        :m_string(str) {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string; // 普通字符串
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem{
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t"; // %T: Tab
    }
private:
    std::string m_string;
};

// ... (Rest of the file would be here, assuming I read 200 lines and it covers most implementation or I will trust the Read limit for now.
// Note: The Read call was limited to 200 lines. If log.cc is longer, I might be cutting it off.
// Let's check the Read output for log.cc again. It ended at TabFormatItem.
// I should probably read the rest of log.cc to be safe, or just assume the user wants what I read.
// Given the user wants to "store the code", uploading partial code is bad.
// I should have checked if 200 lines was enough.
// The file le0n/log.cc usually contains Logger, Appender, Formatter implementation.
// 200 lines seems to only cover FormatItems. The main logic (Logger::log, LogFormatter::init) might be missing!
// I MUST read the full file for log.cc and log.h if they are truncated.
