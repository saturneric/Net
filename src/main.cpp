//
//  net.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "type.h"
#include "memory.h"
#include "clock.h"
#include "net.h"
#include "cproj.h"
#include "cpart.h"
#include "cmap.h"
#include "cthread.h"

int update(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);
int construct(string instruct,vector<string> &config, vector<string> &lconfig, vector<string> &target);

struct instructions{
    int (*unpack)(string instruct,vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*construct)(string instruct,vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*update)(string instruct,vector<string> &, vector<string> &, vector<string> &) = NULL;
};

int main(int argc, const char *argv[]){
//    命令
    string instruct;
//    设置
    vector<string> config;
//    长设置
    vector<string> long_config;
//    目标
    vector<string> target;
//    设置函数
    struct instructions istns;
    istns.construct = construct;
    istns.update = update;
    
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
        else printf("Function not found.\n");
    }
    else if (instruct == "update"){
        if(istns.update != nullptr) istns.update(instruct,config,long_config,target);
        else printf("Function not found.\n");
    }
    else{
        printf("Instruction \"%s\" doesn't make sense.\n",instruct.data());
    }
    
    return 0;
}

bool config_search(vector<string> &configs,string tfg){
    for(auto config : configs){
        if(config == tfg) return true;
    }
    return false;
}

int update(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    try {
        Proj nproj(targets[0], "netc.proj");
        nproj.UpdateProcess();
    } catch (const char *err_info) {
        printf("\033[31mError: %s\n\033[0m",err_info);
    }
    printf("\033[32mSucceed.\n\033[0m");
    return 0;
}

int construct(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    try{
//        读取工程文件
        Proj nproj(targets[0],"netc.proj");
//        检查数据库文件是否存在
        string tdb_path = targets[0] + "/dbs/" + nproj.GetName() +".db";
#ifdef DEBUG
        printf("Search Database %s\n",tdb_path.data());
#endif
        if(!access(tdb_path.data(), R_OK)){
//            设置为强制执行
            if(config_search(configs, "-f")){
                if(remove(tdb_path.data()) == -1){
                    printf("\033[31m");
                    printf("Error: Process meet unknown error.\n");
                    printf("\033[0m");
                    return -1;
                }
            }
            else{
                printf("\033[33m");
                printf("Warning:Database has already existed. Use -f to continue process.\n");
                printf("\033[0m");
                return 0;
            }
        }
//        总体信息检查
        nproj.GeneralCheckInfo();
//        收集信息
        nproj.SearchInfo();
//        构建入口函数索引
        nproj.BuildFuncIndex();
//        检查cpt文件信息
        nproj.CheckCptInfo();
//        编译涉及源文件
        nproj.CompileUsedSrcFiles();
//        检查入口函数信息
        nproj.CheckFuncInfo();
//        建立数据库
        nproj.DBProcess();
    }
    catch(char const *error_info){
        printf("\033[31mError:");
        printf("%s\033[0m\n",error_info);
        return -1;
    }
    printf("\033[32mSucceed.\n\033[0m");
    return 0;
}

void wiki_cpart(void){
    CPart ncp("./PCS","./Libs","a.cpp","A");
    void *a = main_pool.bv_malloc<double>(2.0);
    void *b = main_pool.bv_malloc<double>(3.5);
    void *c = main_pool.bv_malloc<int>(5);
    ncp.AddCPArgsIn(a);
    ncp.AddCPArgsIn(b);
    ncp.AddCPArgsIn(c);
    ncp.Run();
    void *oa = ncp.args_out[0];
    printf("%d",*((int *)oa));
    main_pool.b_free(a);
    main_pool.b_free(b);
    main_pool.b_free(c);
}


