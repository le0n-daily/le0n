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

/**
 * @brief LogEvent 构造函数
 * 初始化所有日志事件属性
 */
LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            , const char* file, int32_t line, uint32_t elapse
            , uint32_t thread_id, uint32_t fiber_id, uint64_t time)
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_logger(logger)
    ,m_level(level) {

    }

LogEvent::~LogEvent() {
    
}

/**
 * @brief Logger 构造函数
 * @param name 日志器名称
 * 默认级别为 DEBUG，默认格式为 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
 */
Logger::Logger(const std::string& name) 
    :m_name(name)
    ,m_level(LogLevel::DEBUG){
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
};

// 添加日志输出地（Appender）
void Logger::addAppender(LogAppender::ptr appender){
    if(!appender->getFormatter()){
        // 如果 Appender 没有自己的格式化器，则继承 Logger 的
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

// 删除日志输出地
void Logger::delAppender(LogAppender::ptr appender){
    for(auto it = m_appenders.begin();
            it != m_appenders.end(); ++it){
        if(*it == appender){
            m_appenders.erase(it);
            break;
        }
    }
}

/**
 * @brief 核心日志方法
 * 当日志级别满足要求时，分发给所有 Appender
 */
void Logger::log(LogLevel::Level level,LogEvent::ptr event){
    if(level >= m_level){
        auto self = shared_from_this();
        for(auto& i : m_appenders){
            i->log(self,level,event);
        }
    }
}

// --- 下面是各种级别的快捷入口 ---

void Logger::debug(LogEvent::ptr event){
    log(LogLevel::DEBUG,event);
}

void Logger::info(LogEvent::ptr event){
    log(LogLevel::INFO,event);
}
void Logger::warn(LogEvent::ptr event){
    log(LogLevel::WARN,event);
}

void Logger::error(LogEvent::ptr event){
    log(LogLevel::ERROR,event);
}

void Logger::fatal(LogEvent::ptr event){
    log(LogLevel::FATAL,event);
}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename){
    reopen(); // 新增：构造时打开文件
}

bool FileLogAppender::reopen(){
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream;//0->false,其他->true
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) {
    if(level >= m_level){
        m_filestream << m_formatter->format(logger,level,event);
    }
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) {
        if(level >= m_level){
            std::string str = m_formatter->format(logger,level,event);
            std::cout << str;
        }
    }

LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern) {
        init();// 初始化解析模式字符串
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event){
    std::stringstream ss;
    for(auto& i : m_items){
        i->format(ss,logger,level,event);
    }
    return ss.str();
}

/**
 * @brief 初始化日志格式解析器
 * 
 * 解析模式字符串 m_pattern，将其拆解为一个个 FormatItem。
 * 模式字符串示例: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
 * 这个解析器的核心能力只有两点：
 *      普通占位符：%x （如 %m, %p）。
 *      带参数占位符：%x{...} （目前源码里主要用于 %d 时间格式）。
 * 解析逻辑是一个简单的状态机：
 * - 遍历字符串
 * - 遇到 % 开始解析特殊项
 * - 遇到 { 开始解析特殊项的参数（如时间格式）
 * - 遇到 } 结束参数解析
 * - 普通字符直接作为 StringFormatItem
 */
void LogFormatter::init(){
    // vector 存储解析出的 pattern item
    // tuple<str, fmt, type>
    // type: 0-普通字符串, 1-格式项
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr; // 暂存普通字符串
    
    for(size_t i = 0;i < m_pattern.size(); ++i){
        // 如果不是 %，则是普通字符，比如说 '[' 添加到 nstr 中，随下一个格式化字符，在 fmt_status=0 处一起添加到 vec 中
        if(m_pattern[i] != '%'){
            nstr.append(1, m_pattern[i]);
            continue;
        }

        // 处理 %% 转义为 %
        if ((i + 1) < m_pattern.size()) {
            if(m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;   //i n 双指针--时间复杂度O(n),
        int fmt_status = 0; // 0: 解析普通格式字符, 1: 解析格式参数 {}
        size_t fmt_begin = 0;

        std::string str; // 格式标识符，如 d, t, m
        std::string fmt; // 格式参数，如 %Y-%m-%d
        
        // 状态机解析格式字符串
        while(n < m_pattern.size()) {
            // 状态0：如果不为字母且不为 { }，说明该格式项结束
            if(fmt_status == 0 && !isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}') {
                break;
            }
            if(fmt_status == 0) {
                if(m_pattern[n] == '{') {
                    // 遇到 {，之前的字符为格式标识符
                    str = m_pattern.substr(i + 1, n - i - 1);
                    //std::cout << "*" << str << std::endl;
                    fmt_status = 1;//解析格式，进入参数解析状态
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }
            if(fmt_status == 1) {
                if(m_pattern[n] == '}') {
                    // 遇到 }，参数解析结束
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    //std::cout << "#" << fmt << std::endl;
                    fmt_status = 2;// 解析完成
                    ++n;
                    break;
                }
            }
            ++n;//如果没有命中任何 if 条件，且没有最后的 ++n，n 就永远不会自增
        }

        if(fmt_status == 0) {
            // 没有参数的格式项，如 %m
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            str = m_pattern.substr(i + 1, n - i - 1);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if(fmt_status == 1) {
            //std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl; 
            // 解析出错，{ 未闭合  
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        } else if(fmt_status == 2) {
            // 带有参数的格式项，如 %d{%Y...}
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        }
    }

    if(!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    
    // 映射表：字符 -> FormatItem 工厂函数
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& fmt)>> s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt){ return FormatItem::ptr(new C(fmt)); }}

        XX(m, MessageFormatItem),   //%m -- 消息体
        XX(p, LevelFormatItem),     //%p -- level
        XX(r, ElapseFormatItem),   //%r -- 启动后的事件
        XX(c, NameFormatItem),      //%c -- 日志名称
        XX(t, ThreadIdFormatItem),  //%t -- 线程id
        XX(n, NewLineFormatItem),   //%n -- 回车换行
        XX(d, DateTimeFormatItem),  //%d -- 时间
        XX(f, FilenameFormatItem),  //%f -- 文件名
        XX(l, LineFormatItem),      //%l -- 行号
        XX(T, TabFormatItem),       //%T -- tab 缩进
        XX(F, FiberIdFormatItem),   //%F -- 协程id
#undef XX
    };

    /**
    * 遍历解析结果，按图索骥生成对应的格式化组件，并装配进流水线 m_items 中。
    * 这个循环的作用是：将解析得到的 pattern 零件 (vec) 转化为可实际执行的格式化组件 (m_items)。
    * * 1. 实例化 (Instantiation)：
    * 根据标识符（如 'd', 'p'），从静态映射表 s_format_items 中找到对应的工厂函数（Lambda），
    * 创建具体的 FormatItem 子类对象（如 DateTimeFormatItem）。
    * * 2. 归一化 (Normalization)：
    * 将“普通字符串”和“格式化占位符”统一包装成 FormatItem 基类指针，
    * 存入 m_items 数组，形成一个有序的“功能插件流水线”。
    * * 3. 预处理与参数绑定 (Pre-computation & Binding)：
    * 在创建对象时顺便将参数（如时间格式 %Y-%m-%d）注入对象内部。
    * 这样在实际打印日志时，无需再查表或解析字符串，直接遍历 m_items 调用 format 即可，
    * 实现了“启动时解析一次，运行时极速执行”。
    * * 4. 容错处理 (Error Handling)：
    * 识别并处理未定义的非法格式字符，将其转化为错误提示字符串，保证系统不崩溃。
    */
    for(auto& i : vec){
        if(std::get<2>(i) == 0){
            // 普通字符串
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }else{
            // 格式项
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end()){
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            }else{
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
        //下面是测试时候用到的代码
        //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
    //std::cout << "m_items size: " << m_items.size() << std::endl;
}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    // 默认添加一个标准输出 Appender
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    // 如果找不到，返回默认的 root logger
    return it == m_loggers.end() ? m_root : it->second;
}

}
