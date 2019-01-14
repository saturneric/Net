//
//  cpart.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "cpart.h"

/**
 设置计算模块的接口形式
 
 @param src_path 源文件所在的目录地址
 @param src_name 源文件的名字
 @param name 计算模块的名字
 @param ffresh 每次建立该结构都重新编译一次源文件
 */
CPart::CPart(string src_path,string src_name,string name,bool ffresh):func(nullptr),handle(nullptr),libargs_in(nullptr),libargs_out(nullptr){
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
    this->handle = dlopen(("Libs/"+libname).data(), RTLD_NOW | RTLD_GLOBAL);
    if(this->handle == nullptr) throw "can not open lib file";
    //    获得该模块的入口
    this->func = (PCSFUNC) dlsym(this->handle, this->name.data());
    if(this->func == nullptr) throw "can not get func "+this->name;
    //    获得向该模块传入参数的vector的地址
    this->libargs_in = (vector<void *> *) dlsym(this->handle, ("__"+name+"_args_in").data());
    if(this->libargs_in == nullptr) throw "can not get the address of __"+name+"_args_in";
    //    获得该函数传出参数所在的vector的地址
    this->libargs_out = (vector<void *> *) dlsym(this->handle, ("__"+name+"_args_out").data());
    if(this->libargs_out == nullptr) throw "can not get the address of __"+name+"_args_out";
    return 0;
}

CPart::~CPart(){
//    释放储存接口输入参数所占用的内存
    for(auto k = 0; k < args_in.size(); k++){
        if(fargs_in[k] == 0) delete (int *)(args_in[k]);
        else delete (double *)(args_in[k]);
    }
//    释放储存接口输出参数所占用的内存
    for(auto k = 0; k < args_out.size(); k++){
        if(fargs_in[k] == 0) delete (int *)(args_out[k]);
        else delete (double *)(args_out[k]);
    }
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
    unsigned long count = fargs_in.size()-1;
    for(auto k = args_in.rbegin(); k != args_in.rend();k++,count--){
        if(fargs_in[count] == INT){
            CPart::addArg(libargs_in, *((int *)(*k)));
        }
        else if(fargs_in[count] == DOUBLE){
            CPart::addArg(libargs_in, *((double *)(*k)));
        }
    }
//    执行计算模块
    if(func() == SUCCESS){
        int count = 0;
//储存计算结果
        for(auto k = libargs_out->begin(); k != libargs_out->end();k++,count++){
            if(fargs_out[count] == INT){
                CPart::addArg(&args_out, *((int *)(*k)));
            }
            else if(fargs_out[count] == DOUBLE){
                CPart::addArg(&args_out, *((double *)(*k)));
            }
        }
        return SUCCESS;
    }
    else return -1;
}


/**
 清空上一次的计算参数及结果数据，并释放相关内存空间
 */
void CPart::Clear(void){
//    释放传入参数所占的空间
    for(auto k = args_in.size() - 1; ~k; k--){
        if(fargs_in[k] == INT) delete (int *)(args_in[k]);
        else delete (double *)(args_in[k]);
        args_in.pop_back();
    }
//    释放传出参数所占用的内存空间
    for(auto k = args_out.size() - 1; ~k; k--){
        if(fargs_in[k] == INT) delete (int *)(args_out[k]);
        else delete (double *)(args_out[k]);
        args_out.pop_back();
    }
}
