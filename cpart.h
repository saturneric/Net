//
//  cpart.h
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cpart_h
#define cpart_h

#include <dlfcn.h>
#include <string>
#include <vector>

using std::vector;
using std::string;

#define PCSFUNC_DEFINE(name) extern "C" int name(vector<void *> args_in,vector<void *> *args_out)
#define GET_ARGS(type) CPart::popArg<type>(args_in)
#define ADD_ARGS(type,value) CPart::addArg<type>(args_in, value);
#define INT 0
#define DOUBLE 1
#define SUCCESS 0
#define FAIL 1

typedef int(*PCSFUNC)(vector<void *>,vector<void *> *);

class CPart;

class Depends{
public:
    CPart *t_cpart;
    vector<int> args;
};

class CPart{
public:
    vector<int> fargs_in, fargs_out;
    vector<void *> args_in, args_out;
    vector<Depends> depends;
    int (*func)(vector<void *>,vector<void *> *);
    void *handle;
    string src_path,name,libname;
    CPart(PCSFUNC func):func(func),handle(nullptr){
        
    }
    
    CPart(string src_path,string name){
        this->src_path = src_path;
        this->name = name;
        system(("g++ -fPIC -shared -std=c++11 -o ./Libs/lib"+name+".so "+src_path).data());
        this->libname = "lib"+name+".so";
        this->handle = dlopen(this->libname.data(), RTLD_LAZY | RTLD_GLOBAL);
        this->func = (PCSFUNC) dlsym(this->handle, this->name.data());
    }
    
    CPart(string name){
        this->name = name;
        this->libname = "lib"+name+".so";
        this->handle = dlopen(this->libname.data(), RTLD_LAZY | RTLD_GLOBAL);
        this->func = (PCSFUNC) dlsym(this->handle, this->name.data());
    }
    
    ~CPart(){
        for(auto k = 0; k < args_in.size(); k++){
            if(fargs_in[k] == 0) delete (int *)(args_in[k]);
            else delete (double *)(args_in[k]);
        }
        for(auto k = 0; k < args_out.size(); k++){
            if(fargs_in[k] == 0) delete (int *)(args_out[k]);
            else delete (double *)(args_out[k]);
        }
        if(handle != nullptr)
            dlclose(handle);
    }
    
    void setArgsType(vector<int> fargs_in, vector<int> fargs_out){
        this->fargs_in = fargs_in;
        this->fargs_out = fargs_out;
    }
    
    void addArgsInt(int value){
        int *p_value = new int(value);
        *p_value = value;
        args_in.push_back(p_value);
    }
    
    int Run(void){
        return func(args_in,&args_out);
    }
    
    template<class T>
    static void addArg(vector<void *> &args,T value){
        T *p_value = new T(value);
        *p_value = value;
        args.push_back(p_value);
    }
    template<class T>
    static T popArg(vector<void *> &args){
        T *p_value = (T *)args.back();
        args.pop_back();
        return *p_value;
    }
};



#endif /* cpart_h */
