#ifndef LE0N_CONFIG_H
#define LE0N_CONFIG_H

#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "le0n/log.h"

namespace le0n{

// 配置变量的基类
// 作用：因为配置项的类型各异(int, string, vector...)，我们需要一个基类来实现多态，
// 这样才能把它们都放到一个 map<string, ConfigVarBase::ptr> 里面统一管理。
class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string&name ,const std::string& description = "")
        : m_name(name)
        , m_description(description) {

        }
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
protected:
    std::string m_name;         // 配置参数的名称 (key)
    std::string m_description;  // 配置参数的描述 (help)
};

template <class T> //模板的本质是 “代码生成器” —— 编译器会为每个使用的具体类型“实例化”出一个独立的类。
class ConfigVar : public ConfigVarBase{
public:
    // [BugFix] 需要重新定义 ptr。
    // 如果不定义，会继承父类的 typedef std::shared_ptr<ConfigVarBase> ptr;
    // 导致使用 g_int_value_config->getValue() 时报错：'ConfigVarBase' has no member named 'getValue'
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const std::string& name
            ,const T& default_value
            ,const std::string& description = "")
        : ConfigVarBase(name,description)
        , m_val(default_value){

    }

    std::string toString() override{
        try{
            return boost::lexical_cast<std::string>(m_val);
        } catch (std::exception& e){
            LE0N_LOG_ERROR(LE0N_LOG_ROOT()) << "ConfigVar::toString execption" 
                << e.what() << "convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }
    bool fromString(const std::string& val) override {
        try{
            m_val = boost::lexical_cast<T>(val);
            return true;
        } catch (std::exception& e){
            LE0N_LOG_ERROR(LE0N_LOG_ROOT()) << "ConfigVar::formString execption" 
                << e.what() << "convert: string to" << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() const { return m_val; }
    void setValue(const T& v){ m_val = v; }
private:
    T m_val;
};

class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name
        ,const T& default_value, const std::string& description = ""){
            auto tmp = Lookup<T>(name);
            if(tmp){
                LE0N_LOG_ERROR(LE0N_LOG_ROOT()) << "Lookup name=" <<name << " exists";
                return tmp;
            }

            // [BugFix] find_first_not_of 如果没找到非法字符会返回 string::npos (即 -1 或很大的数)。
            // C++ 中 if(-1) 为真，所以如果直接写 if(find_first_not_of(...)) 会导致合法名字也被判错。
            // 必须显式判断 != std::string::npos。
            if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._1234567890")
                    != std::string::npos){
                LE0N_LOG_ERROR(LE0N_LOG_ROOT()) << "Lookup name invalid" <<name ;
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            s_datas[name] = v;
            return v;
    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name){
        auto it = s_datas.find(name);
        if(it == s_datas.end()){
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }
private:
    static ConfigVarMap s_datas;
};

}

#endif