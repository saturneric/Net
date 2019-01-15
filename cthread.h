//
//  cthread.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cthread_h
#define cthread_h

#include "cpart.h"
#include "cmap.h"

#include <pthread.h>
#include <list>

using std::list;

//计算进程管理结构
class CThread{
public:
//    对应的图结构管理结构
    CMap *p_map;
//    计算模块处理队列
    list<CPart *> line;
//    此计算进程中计算模块的传入参数数据列表
    map<string,vector<void *>> rargs;
//    此计算进程的计算模块的传出参数数据列表
    map<string,vector<void *>> rargs_out;
//    计算模块是否已经执行
    map<string,bool> ifsolved;
//    使用图结构管理结构来构造计算进程管理结构
    CThread(CMap *tp_map);
    ~CThread();
    template<class T>
//    添加相关计算模块的传入参数
    void AddArgs(string name, T value){
        auto k = rargs.find(name);
        T *p_value = new T();
        *p_value = value;
        (*k).second.push_back((void *)p_value);
    }
//    分析图结构来构造处理队列
    void Analyse(void);
//    执行处理队列
    void DoLine(void);
//    建立新线程执行计算模块
    static void * NewThread(void *);
//    为计算模块的调用准备输入参数
    static void PrepareArgsIn(CThread *pct,CPart *);
//    获得计算模块执行后的输出参数
    static void GetArgsOut(CThread *pct,CPart *);
};

struct thread_args{
    CThread *pct;
    CPart *pcp;
    int rtn;
};

#endif /* cthread_h */
