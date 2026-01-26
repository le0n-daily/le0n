#include "le0n/config.h"
#include "le0n/log.h"

le0n::ConfigVar<int>::ptr g_int_value_config = 
    le0n::Config::Lookup("system.port", (int)8080, "system port");
le0n::ConfigVar<float>::ptr g_float_value_config = 
    le0n::Config::Lookup("system.value", (float)10.2f, "system value");


int main(int argc, char** argv){
    LE0N_LOG_INFO(LE0N_LOG_ROOT()) << g_int_value_config->getValue();
    LE0N_LOG_INFO(LE0N_LOG_ROOT()) << g_float_value_config->toString();
    return 0;
}