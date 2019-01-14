//
//  cpart.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "cpart.h"

CPart::CPart(string src_path,string src_name,string name){
    this->src_path = src_path;
    this->name = name;
    unsigned long qp = src_name.find(".",0);
    string t_libname = "lib"+src_name.substr(0,qp)+".so";
    if(!~access(("Libs/"+t_libname).data(), F_OK)){
        system(("g++ -fPIC -shared -std=c++11 -o ./Libs/"+t_libname+" "+src_path+"/"+src_name).data());
    }
    this->libname = t_libname;
    this->handle = dlopen(("Libs/"+t_libname).data(), RTLD_NOW | RTLD_GLOBAL);
    this->func = (PCSFUNC) dlsym(this->handle, this->name.data());
    this->libargs_in = (vector<void *> *) dlsym(this->handle, ("__"+name+"_args_in").data());
    this->libargs_out = (vector<void *> *) dlsym(this->handle, ("__"+name+"_args_out").data());
}

CPart::CPart(string name){
    this->name = name;
    this->libname = "lib"+name+".so";
    this->handle = dlopen(this->libname.data(), RTLD_LAZY | RTLD_GLOBAL);
    this->func = (PCSFUNC) dlsym(this->handle, this->name.data());
}

CPart::~CPart(){
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

void CPart::setArgsType(vector<int> fargs_in, vector<int> fargs_out){
    this->fargs_in = fargs_in;
    this->fargs_out = fargs_out;
}

int CPart::Run(void){
    unsigned long count = fargs_in.size()-1;
    for(auto k = args_in.rbegin(); k != args_in.rend();k++,count--){
        if(fargs_in[count] == INT){
            CPart::addArg(libargs_in, *((int *)(*k)));
        }
        else if(fargs_in[count] == DOUBLE){
            CPart::addArg(libargs_in, *((double *)(*k)));
        }
    }
    if(func() == SUCCESS){
        int count = 0;
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

void CPart::Clear(void){
    for(auto k = 0; k < args_in.size(); k++){
        if(fargs_in[k] == INT) delete (int *)(args_in[k]);
        else delete (double *)(args_in[k]);
    }
    for(auto k = 0; k < args_out.size(); k++){
        if(fargs_in[k] == INT) delete (int *)(args_out[k]);
        else delete (double *)(args_out[k]);
    }
}
