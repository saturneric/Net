//
//  net.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "instruct.h"

int main(int argc, const char *argv[]){
//    命令
    string instruct;
//    设置
    vector<string> config;
//    长设置
    vector<string> long_config;
//    目标
    vector<string> target;
//    注册函数
    struct instructions istns;
    istns.construct = construct;
    istns.update = update;
    istns.server = server;
    istns.init = init;
    istns.set = set;
    istns.client = client;
    
//    解析命令
    int if_instruct = 1;
    for(int i = 1; i < argc; i++){
        string sargv = argv[i];
        if(sargv[0] == '-'){
            config.push_back(sargv);
        }
        else if(sargv[0] == '-' && sargv[1] == '-'){
            long_config.push_back(sargv);
        }
        else{
            if(if_instruct){
                instruct = sargv;
                if_instruct = 0;
            }
            else{
                target.push_back(sargv);
            }
        }
    }
//    处理命令
    if(instruct == "construct"){
        if(istns.construct != nullptr) istns.construct(instruct,config,long_config,target);
        else error::printError("Function not found.");
    }
    else if (instruct == "update"){
        if(istns.update != nullptr) istns.update(instruct,config,long_config,target);
        else error::printError("Function not found.");
    }
    else if (instruct == "server"){
        if(istns.update != nullptr) istns.server(instruct,config,long_config,target);
        else error::printError("Function not found.");
    }
    else if (instruct == "init"){
        if(istns.update != nullptr) istns.init(instruct,config,long_config,target);
        else error::printError("Function not found.");
    }
    else if (instruct == "set"){
        if(istns.update != nullptr) istns.set(instruct,config,long_config,target);
        else error::printError("Function not found.");
    }
    else if (instruct == "client"){
        if(istns.update != nullptr) istns.client(instruct,config,long_config,target);
        else error::printError("Function not found.");
    }
    else{
        printf("\033[33mInstruction \"%s\" doesn't make sense.\n\033[0m",instruct.data());
    }
    
    return 0;
}



