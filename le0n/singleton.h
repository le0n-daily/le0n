#ifndef __LE0N_SINGLETON_H__
#define __LE0N_SINGLETON_H__

#include <memory>
namespace le0n{

template<class T, class X = void,int N = 0>
class Singleton{
public:
    static T* GetInstance(){
        static T v;
        return &v;
    }
};

template<class T, class X = void,int N = 0>
class SingletonPtr{
    static std::shared_ptr<T> GetInstance(){
        static std::shared_ptr<T> v(new T());
        return v;
    }
};

}

#endif
