#include "util.h"

namespace le0n {

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
    return 0;// 先就这样假装有了
}
}
