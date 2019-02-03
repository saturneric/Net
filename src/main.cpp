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
int server(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);
int init(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);
int set(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets);

struct instructions{
    int (*unpack)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*construct)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*update)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*server)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*set)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
    int (*init)(string, vector<string> &, vector<string> &, vector<string> &) = NULL;
};

namespace error {
    void printError(string error_info){
        printf("\033[31mError: %s\n\033[0m",error_info.data());
    }
    void printWarning(string warning_info){
        printf("\033[33mWarning: %s\n\033[0m",warning_info.data());
    }
    void printSuccess(string succes_info){
        printf("\033[32m%s\n\033[0m",succes_info.data());
    }
}

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
    else{
        printf("\033[33mInstruction \"%s\" doesn't make sense.\n\033[0m",instruct.data());
    }
    
    return 0;
}

bool config_search(vector<string> &configs,string tfg){
    for(auto config : configs){
        if(config == tfg) return true;
    }
    return false;
}

int init(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    sqlite3 *psql;
    sqlite3_stmt *psqlsmt;
    sqlite3_open("info.db", &psql);
    const char *pzTail;
    try {
        sql::table_create(psql, "server_info", {
            {"name","TEXT"},
            {"tag","TEXT"},
            {"admin_key_sha1","TEXT"},
            {"msqes_ip","TEXT"},
            {"msqes_port","INT"},
            {"msqes_key","TEXT"},
            {"msqes_rsa_public","TEXT"},
        });
        sql::table_create(psql, "sqes_info", {
            {"sqes_ip","TEXT PRIMARY KEY"},
            {"sqes_port","INT"},
            {"sqes_key","TEXT"},
            {"rsa_public","TEXT"},
        });
    } catch (const char *error_info) {
        if(!strcmp(error_info, "fail to create table")){
            if(!config_search(configs, "-f")){
                printf("\033[33mWarning: Have Already run init process.Try configure -f to continue.\n\033[0m");
                return 0;
            }
            else{
                string sql_quote = "DELETE FROM server_info;";
                sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
                int rtn = sqlite3_step(psqlsmt);
                if(rtn == SQLITE_DONE){
                    
                }
                else{
                    const char *error = sqlite3_errmsg(psql);
                    int errorcode =  sqlite3_extended_errcode(psql);
                    printf("\033[31mSQL Error: [%d]%s\n\033[0m",errorcode,error);
                    throw error;
                }
                sqlite3_finalize(psqlsmt);
            }
        }
    }
    
    
    sql::insert_info(psql, &psqlsmt, "server_info", {
        {"name","?1"},
        {"tag","?2"}
    });
    if(setting_file::if_name_illegal(targets[0]));
    else{
        error::printError("Args(name) abnormal.");
    }
    if(setting_file::if_name_illegal(targets[1]));
    else{
        error::printError("Args(tag) abnormal.");
    }
    sqlite3_bind_text(psqlsmt, 1, targets[0].data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(psqlsmt, 2, targets[1].data(), -1, SQLITE_TRANSIENT);
    int rtn = sqlite3_step(psqlsmt);
    if(rtn == SQLITE_DONE){
        
    }
    else throw "sql writes error";
    sqlite3_finalize(psqlsmt);
    sqlite3_close(psql);
    return 0;
}

int set(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    sqlite3 *psql;
    sqlite3_stmt *psqlsmt;
    const char *pzTail;
    if(sqlite3_open("info.db", &psql) == SQLITE_ERROR){
        sql::printError(psql);
    }
    string sql_quote = "SELECT count(*) FROM sqlite_master WHERE name = 'server_info';";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    int if_find = sqlite3_column_int(psqlsmt, 0);
    if(if_find);
    else{
        error::printError("Couldn't do set before init process.");
        return -1;
    }
    sqlite3_finalize(psqlsmt);
    if(targets[0] == "square"){
        sql_quote = "UPDATE server_info SET msqes_ip = ?1, msqes_port = ?2 WHERE rowid = 1;";
        sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
        
        if(!Addr::checkValidIP(targets[1])){
            error::printError("Args(ipaddr) abnomal.");
            sqlite3_finalize(psqlsmt);
            sqlite3_close(psql);
            return -1;
        }
        sqlite3_bind_text(psqlsmt, 1, targets[1].data(), -1, SQLITE_TRANSIENT);
        
        stringstream ss;
        ss<<targets[2];
        int port;
        ss>>port;
        if(port > 0 && port <= 65535);
        else{
            error::printError("Args(port) abnomal.");
            sqlite3_finalize(psqlsmt);
            sqlite3_close(psql);
            return -1;
        }
        sqlite3_bind_int(psqlsmt, 2, port);
        int rtn = sqlite3_step(psqlsmt);
        if(rtn != SQLITE_DONE){
            sql::printError(psql);
        }
        sqlite3_finalize(psqlsmt);
    }
    else if (targets[1] == "key"){
        
    }
    error::printSuccess("Succeed.");
    sqlite3_close(psql);
    return 0;
}

int server(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    initClock();
    setThreadsClock();
    Server nsvr;
    setServerClock(&nsvr, 3);
    while(1) usleep(10000);
    return 0;
}

int update(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    try {
        Proj nproj(targets[0], "netc.proj");
        nproj.UpdateProcess();
    } catch (const char *err_info) {
        printf("\033[31mError: %s\n\033[0m",err_info);
        return -1;
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


