//
//  cpart.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "memory.h"
#include "cpart.h"

/**
 设置计算模块的接口形式
 
 @param src_path 源文件所在的目录地址
 @param src_name 源文件的名字
 @param name 计算模块的名字
 @param ffresh 每次建立该结构都重新编译一次源文件
 */
CPart::CPart(string src_path,string src_name,string name,bool ffresh):func(nullptr),handle(nullptr){
    this->src_path = src_path;
    this->name = name;
    
    this->src_name = src_name;
//    去掉源文件的后缀
    unsigned long qp = src_name.find(".",0);
    if(qp == string::npos){
        qp = src_name.size()-1;
    }
//    生成lib文件的文件名
    string t_libname = "lib"+src_name.substr(0,qp)+".so";
    this->libname = t_libname;
//    如果lib文件存在且不要求每次建立该结构都重新编译一次源文件的话就不执行编译
    if(!~access(("Libs/"+t_libname).data(), F_OK) || ffresh){
        BuildSo();
        GetSo();
    }

}


/**
 执行源文件编译操作，生成动态链接库

 @return 执行成功将返回0
 */
int CPart::BuildSo(void){
    int rtn = system(("g++ -fPIC -shared -std=c++11 -o ./Libs/"+libname+" "+src_path+"/"+src_name).data());
    if(rtn != -1 && rtn != 127)
        return 0;
    else throw "fail to build lib file";
}


/**
 获得lib文件的操作柄，并获得计算模块的入口地址及传入传出参数列表的地址。

 @return 执行成功则返回0
 */
int CPart::GetSo(void){
    //    读取lib文件
    this->handle = dlopen(("Libs/"+libname).data(), RTLD_NOW | RTLD_LOCAL);
    if(this->handle == nullptr) throw "can not open lib file";
    //    获得该模块的入口
    this->func = (PCSFUNC) dlsym(this->handle, this->name.data());
    if(this->func == nullptr) throw "can not get func "+this->name;
    //    获得向该模块传入参数的vector的地址
    this->libargs_in.args = (vector<block_info> *) dlsym(this->handle, ("__"+name+"_args_in").data());
    if(this->libargs_in.args == nullptr) throw "can not get the address of __"+name+"_args_in";
    //    获得该函数传出参数所在的vector的地址
    this->libargs_out.args = (vector<block_info> *) dlsym(this->handle, ("__"+name+"_args_out").data());
    if(this->libargs_out.args == nullptr) throw "can not get the address of __"+name+"_args_out";
    return 0;
}

CPart::~CPart(){
//    释放储存接口输入参数所占用的内存
    for(auto arg : args_in) main_pool.b_free(arg);
//    释放储存接口输出参数所占用的内存
    for(auto arg : args_out) main_pool.b_free(arg);
//    停止对lib文件的操作
    if(handle != nullptr)
        dlclose(handle);
}


/**
 设置计算模块的接口形式

 @param fargs_in 调用接口形式
 @param fargs_out 输出接口形式
 */
void CPart::setArgsType(vector<int> fargs_in, vector<int> fargs_out){
    this->fargs_in = fargs_in;
    this->fargs_out = fargs_out;
}

/**
 向计算模块传入参数，并运行计算模块，而后获得计算结果

 @return 如果执行成功则返回SUCCESS
 */
int CPart::Run(void){
    if(func == nullptr) throw "func is nullptr";
//    对计算模块传入参数
    for(auto arg : args_in)
        libargs_in.addArgPtr(main_pool.size(arg), arg);
//    执行计算模块
    if(func() == SUCCESS){
//储存计算结果
        for(auto libarg : *libargs_out.args){
//            获得内存块的访问量
            void *arg = main_pool.bp_malloc(libarg.size, libarg.pvle);
            args_out.push_back(arg);
        }
        libargs_out.clear();
        return SUCCESS;
    }
    else return -1;
}


/**
 清空上一次的计算参数及结果数据，并释放相关内存空间
 */
void CPart::Clear(void){
//    释放传入参数所占的空间
    for(auto arg : args_in) main_pool.b_free(arg);
    args_in.clear();
//    释放传出参数所占用的内存空间
    for(auto arg : args_out) main_pool.b_free(arg);
    args_out.clear();
}
