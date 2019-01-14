//
//  cpart.h
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cpart_h
#define cpart_h

#include <unistd.h>
#include <dlfcn.h>
#include <string>
#include <vector>

using std::vector;
using std::string;

//声明计算模块的传入与传出参数列表
#define ARGS_DECLAER(name) vector<void *> __##name##_args_in, __##name##_args_out
//声明计算模块的入口
#define PCSFUNC_DEFINE(name) extern "C" int name(void)

#define GET_ARGS(name,type) CPart::popArg<type>(&__##name##_args_in)
#define ADD_ARGS(name,type,value) CPart::addArg<type>(&__##name##_args_out, value);


//从传入参数列表的第一个值，并删除该值
#define POP_ARGS(name,type) GET_ARGS( name ,type)
//向传出参数列表中添加值
#define PUSH_ARGS(name,type,value) ADD_ARGS( name ,type,value)

#define INT 0
#define DOUBLE 1

//调用计算模块成功的返回
#define SUCCESS 0
//调用计算模块失败的返回
#define FAIL -1

typedef int(*PCSFUNC)(void);

class CPart;

//计算模块管理对象间的依赖关系
class Depends{
public:
//    指向依赖的计算模块管理对象的指针
    CPart *t_cpart;
//    所依赖的输入参数在计算模块输入参数列表中的序号
    vector<int> args;
};

//计算模块管理对象
class CPart{
public:
//    参数形式信息列表
    vector<int> fargs_in, fargs_out;
//    参数操纵列表
    vector<void *> args_in, args_out;
//    lib文件中相关参数操纵列表的地址
    vector<void *> *libargs_in,*libargs_out;
//    所依赖的计算对象列表
    vector<Depends> depends;
//    lib文件中的计算模块的入口地址
    int (*func)(void);
//    lib文件操作柄
    void *handle;
//    源文件所在目录
    string src_path;
//    计算模块名
    string name;
//    lib文件名
    string libname;
//    源文件名
    string src_name;
    
//    当计算模块随着该工具同时编译时可以直接使用该构造函数
    CPart(PCSFUNC func):func(func),handle(nullptr){}
//    一般构造函数，计算模块在文件中以源文件的形式独立存在时使用该构造函数
    CPart(string src_path,string src_name,string name,bool ffresh = true);
//    析构函数
    ~CPart();
//    设置输入输出参数形式信息
    void setArgsType(vector<int> fargs_in, vector<int> fargs_out);
//    编译源文件
    int BuildSo(void);
//    获得lib文件操作柄
    int GetSo(void);
//    运行计算模块
    int Run(void);
//    清空计算历史记录
    void Clear(void);
    
//    在对象的传入参数列表中添加参数值
    template<class T> void addArgsIn(T value){
        T *p_value = new T(value);
        if(p_value == nullptr) throw "fail to malloc";
        args_in.push_back(p_value);
    }
//    一般由lib文件中的计算模块调用的向vector中添加参数并分配内存空间而后初始化
    template<class T>
    static void addArg(vector<void *> *args,T value){
        T *p_value = new T(value);
        if(p_value == nullptr) throw "fail to malloc";
        args->push_back(p_value);
    }
//    一般由lib文件中的计算模块调用的从vector中获得参数并释放其占用的内存空间而后返回相关值
    template<class T>
    static T popArg(vector<void *> *args){
        if(args == nullptr) throw "the pointer to vector is null";
        T *p_value = (T *)args->back();
        T value = *p_value;
        args->pop_back();
        return value;
    }
};

#endif /* cpart_h */
