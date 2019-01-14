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

#define ARGS_DECLAER(name) vector<void *> __##name##_args_in, __##name##_args_out
#define PCSFUNC_DEFINE(name) extern "C" int name(void)

#define GET_ARGS(name,type) CPart::popArg<type>(&__##name##_args_in)
#define ADD_ARGS(name,type,value) CPart::addArg<type>(&__##name##_args_out, value);

#define POP_ARGS(name,type) GET_ARGS( name ,type)
#define PUSH_ARGS(name,type,value) ADD_ARGS( name ,type,value)

#define INT 0
#define DOUBLE 1
#define SUCCESS 0
#define FAIL 1

typedef int(*PCSFUNC)(void);

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
    vector<void *> *libargs_in,*libargs_out;
    vector<Depends> depends;
    int (*func)(void);
    void *handle;
    string src_path,name,libname;
    CPart(PCSFUNC func):func(func),handle(nullptr){}
    CPart(string src_path,string src_name,string name);
    CPart(string name);
    ~CPart();
    void setArgsType(vector<int> fargs_in, vector<int> fargs_out);
    int Run(void);
    void Clear(void);
    
    template<class T> void addArgsIn(T value){
        T *p_value = new T(value);
        args_in.push_back(p_value);
    }
    
    template<class T>
    static void addArg(vector<void *> *args,T value){
        T *p_value = new T(value);
        *p_value = value;
        args->push_back(p_value);
    }
    
    template<class T>
    static T popArg(vector<void *> *args){
        T *p_value = (T *)args->back();
        args->pop_back();
        return *p_value;
    }
};

#endif /* cpart_h */
