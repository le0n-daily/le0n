#include "../le0n/log.h"
#include "../le0n/util.h"
#include <iostream>
#include <unistd.h>

// 原有代码已注释
int main(int argc, char** argv) {
    le0n::Logger::ptr logger(new le0n::Logger);
    logger->addAppender(le0n::LogAppender::ptr(new le0n::StdoutLogAppender));

    le0n::FileLogAppender::ptr file_appender(new le0n::FileLogAppender("./log.txt"));
    le0n::LogFormatter::ptr fmt(new le0n::LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(le0n::LogLevel::ERROR);

    logger->addAppender(file_appender);
    
    // 手动测试一下 log
    //le0n::LogEvent::ptr event(new le0n::LogEvent(__FILE__, __LINE__, 0, le0n::GetThreadId(), le0n::GetFiberId(), time(0)));
    //event->getSS() << "hello le0n log";
    //logger->log(le0n::LogLevel::DEBUG, event);
    std::cout << "hello le0n log" << std::endl;
    
    LE0N_LOG_INFO(logger) << "test macro";
    LE0N_LOG_ERROR(logger) << "test macro error";

    LE0N_LOG_FMT_ERROR(logger, "test fmt error %s", "hello");

    auto l = le0n::LoggerMgr::GetInstance()->getLogger("xx");
    LE0N_LOG_INFO(l) << "xxx";
    return 0;
}