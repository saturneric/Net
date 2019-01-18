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
#include "compute.h"

//整型
#define INT 0
//浮点型
#define DOUBLE 1

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

//计算模块类
class CPart{
public:
//    参数格式信息列表
    vector<int> fargs_in, fargs_out;
//    输入参数与输出参数缓冲区
    vector<void *> args_in, args_out;
//    计算过程入口与出口管理类
    LibArgsTransfer libargs_in, libargs_out;
//   依赖计算对象列表
    vector<Depends> depends;
//    计算过程的入口函数的地址
    int (*func)(void) = nullptr;
//    动态链接库操作柄
    void *handle = nullptr;
//    源文件所在目录
    string src_path;
//    计算模块名
    string name;
//    动态链接库路径
    string lib_path;
//    动态链接库名
    string lib_name;
//    源文件名
    string src_name;
    
//    唯一的构造函数
    CPart(string src_path,string lib_path,string src_name,string name,bool ffresh = true);
//    析构函数
    ~CPart();
//    设置输入输出参数格式信息列表
    void setArgsType(vector<int> fargs_in, vector<int> fargs_out);
//    编译源文件
    int BuildSo(void);
//    获得动态链接库操作柄
    int GetSoHandle(void);
//    运行计算过程
    int Run(void);
//    清空传入传出参数缓冲区
    void Clear(void);
//    在传入参数缓冲区中添加参数
    void AddCPArgsIn(void *arg);
};


#endif /* cpart_h */
