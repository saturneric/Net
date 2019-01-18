//
//  cpart.h
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cpart_h
#define cpart_h

#include "type.h"
#include "memory.h"

//声明计算模块的传入与传出参数列表
#define ARGS_DECLAER(name) vector<block_info> __##name##_args_in, __##name##_args_out
//声明计算模块的入口
#define PCSFUNC_DEFINE(name) extern "C" int name(void)

#define _PCSFUNC(name) ARGS_DECLAER(name); \
PCSFUNC_DEFINE(name)

//从传入参数列表的第一个值，并删除该值
#define ARGS_IN(name) __##name##_args_in
//向传出参数列表中添加值
#define ARGS_OUT(name) __##name##_args_out

//整型
#define INT 0
//浮点型
#define DOUBLE 1

//调用计算模块成功的返回
#define SUCCESS 0
//调用计算模块失败的返回
#define FAIL -1

//计算模块入口函数类型
typedef int(*PCSFUNC)(void);

class CPart;

//计算模块管理对象间的依赖关系管理结构
class Depends{
public:
//    指向依赖的计算模块管理对象的指针
    CPart *t_cpart;
//    所依赖的输入参数在计算模块输入参数列表中的序号
    vector<int> args;
};

class LibArgsTransfer{
public:
    vector<block_info> *args = nullptr;
    LibArgsTransfer(vector<block_info> &args){
        this->args = &args;
    }
    LibArgsTransfer(){
        
    }
    void addArgPtr(int size, void *p_arg){
        void *pc_arg = malloc(size);
        memcpy(pc_arg, p_arg, size);
        block_info pbifo(size,pc_arg);
        args->push_back(pbifo);
    }
    template<class T>
    T getArg(int idx){
        T *pvle = (T *)(*args)[idx].pvle;
        if((*args)[idx].size == sizeof(T)) return *pvle;
        else throw "arg size conflict";
    }
    template<class T>
    T *getArgPtr(int idx){
        T *pvle = (*args)[idx].pvle;
        return pvle;
    }
    void LibAddArgPtr(int size, void *p_arg){
        block_info pbifo(size,p_arg);
        args->push_back(pbifo);
    }
    template<class T>
    void LibAddArg(T arg){
        T *p_arg = (T *)malloc(sizeof(T));
        *p_arg = arg;
        block_info pbifo(sizeof(T),p_arg);
        args->push_back(pbifo);
    }
    void clear(void){
        for(auto arg : *args)
            free(arg.pvle);
        args->clear();
    }
    ~LibArgsTransfer(){
        //clear();
    }
};

//计算模块类
class CPart{
public:
//    参数形式信息列表
    vector<int> fargs_in, fargs_out;
//    参数操纵列表
    vector<void *> args_in, args_out;
//    lib文件中相关参数操纵列表的地址
    LibArgsTransfer libargs_in, libargs_out;
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
    void AddCPArgsIn(void *arg){
        void *p_value = main_pool.b_get(arg);
        if(p_value == nullptr) throw "information lost";
        args_in.push_back(p_value);
    }
};



#endif /* cpart_h */
