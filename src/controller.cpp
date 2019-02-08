//
//  controller.cpp
//  Net
//
//  Created by 胡一兵 on 2019/2/8.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "instruct.h"

extern string PRIME_SOURCE_FILE;

//线程阻塞开关
int if_wait = 1;

int init(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    sqlite3 *psql;
    sqlite3_stmt *psqlsmt;
    sqlite3_open("info.db", &psql);
    const char *pzTail;
    if(targets[0] == "server"){
        sql::table_create(psql, "server_info", {
            {"sqes_public","NONE"},
            {"sqes_private","NONE"},
            {"key_sha1","TEXT"}
        });
        sql::insert_info(psql, &psqlsmt, "server_info", {
            {"sqes_public","?1"},
            {"sqes_private","?2"},
            {"key_sha1","?3"},
        });
        struct public_key_class npbkc;
        struct private_key_class nprkc;
        rsa_gen_keys(&npbkc, &nprkc, PRIME_SOURCE_FILE);
        sqlite3_bind_blob(psqlsmt, 1, &npbkc, sizeof(public_key_class), SQLITE_TRANSIENT);
        sqlite3_bind_blob(psqlsmt, 2, &nprkc, sizeof(private_key_class), SQLITE_TRANSIENT);
        if(targets[1].size() < 6) error::printWarning("Key is too weak.");
        string sha1_hex;
        SHA1_Easy(sha1_hex, targets[1], targets.size());
        sqlite3_bind_text(psqlsmt, 3, sha1_hex.data(), -1, SQLITE_TRANSIENT);
        
        if(sqlite3_step(psqlsmt) != SQLITE_DONE){
            sql::printError(psql);
        }
        sqlite3_finalize(psqlsmt);
        error::printSuccess("Succeed.");
        sqlite3_close(psql);
        return 0;
        
    }
    else{
        try {
            sql::table_create(psql, "client_info", {
                {"name","TEXT"},
                {"tag","TEXT"},
                {"admin_key_sha1","TEXT"},
                {"msqes_ip","TEXT"},
                {"msqes_port","INT"},
                {"msqes_key","TEXT"},
                {"msqes_rsa_public","NONE"},
            });
            sql::table_create(psql, "sqes_info", {
                {"sqes_ip","TEXT PRIMARY KEY"},
                {"sqes_port","INT"},
                {"sqes_key","TEXT"},
                {"rsa_public","NONE"},
            });
        } catch (const char *error_info) {
            if(!strcmp(error_info, "fail to create table")){
                if(!config_search(configs, "-f")){
                    printf("\033[33mWarning: Have Already run init process.Try configure -f to continue.\n\033[0m");
                    return 0;
                }
                else{
                    string sql_quote = "DELETE FROM client_info;";
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
        
    }
    
    
    sql::insert_info(psql, &psqlsmt, "client_info", {
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
    if(targets.size() < 2){
        error::printError("Args error.");
        return -1;
    }
    sqlite3 *psql;
    sqlite3_stmt *psqlsmt;
    const char *pzTail;
    if(sqlite3_open("info.db", &psql) == SQLITE_ERROR){
        sql::printError(psql);
        return -1;
    }
    string sql_quote = "SELECT count(*) FROM sqlite_master WHERE name = 'client_info';";
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
        sql_quote = "UPDATE client_info SET msqes_ip = ?1, msqes_port = ?2 WHERE rowid = 1;";
        sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
        
        if(!Addr::checkValidIP(targets[1])){
            error::printError("Args(ipaddr) is abnomal.");
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
            error::printError("Args(port) is abnomal.");
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
    else if (targets[0] == "key"){
        if(targets[1] == "admin"){
            string hexresult;
            SHA1_Easy(hexresult, targets[2], targets[2].size());
            if(targets[1].size() < 6){
                error::printWarning("Key is too weak.");
            }
            sql_quote = "UPDATE client_info SET admin_key_sha1 = ?1 WHERE rowid = 1;";
            sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
            sqlite3_bind_text(psqlsmt, 1, hexresult.data(), -1, SQLITE_TRANSIENT);
            if(sqlite3_step(psqlsmt) != SQLITE_DONE){
                sql::printError(psql);
            }
            sqlite3_finalize(psqlsmt);
        }
        else if(targets[1] == "square"){
            sql_quote = "UPDATE client_info SET msqes_key = ?1 WHERE rowid = 1;";
            sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
            sqlite3_bind_text(psqlsmt, 1, targets[2].data(), -1, SQLITE_TRANSIENT);
            if(sqlite3_step(psqlsmt) != SQLITE_DONE){
                sql::printError(psql);
            }
            sqlite3_finalize(psqlsmt);
        }
        else{
            error::printError("Args(type) is abnormal.");
            return -1;
        }
    }
    error::printSuccess("Succeed.");
    sqlite3_close(psql);
    return 0;
}

int server(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    initClock();
    setThreadsClock();
    if(targets.size() == 0){
        Server nsvr;
        setServerClock(&nsvr, 3);
    }
    else{
        if(targets[0] == "square"){
            SQEServer nsvr;
            setServerClockForSquare(&nsvr, 3);
        }
    }
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

int client(string instruct, vector<string> &configs, vector<string> &lconfigs, vector<string> &targets){
    sqlite3 *psql;
    sqlite3_stmt *psqlsmt;
    const char *pzTail;
    if(sqlite3_open("info.db", &psql) == SQLITE_ERROR){
        sql::printError(psql);
        return -1;
    }
//    初始化时钟
    initClock();
    setThreadsClock();
    
//    建立客户端
    Client nclt(9050);
    setClientClock(&nclt, 3);
    request *preq;
    
//    获得主广场服务器的通信公钥
    string sql_quote = "select count(*) from client_info where rowid = 1 and msqes_rsa_public is null;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    int if_null = sqlite3_column_int(psqlsmt, 0);
    sqlite3_finalize(psqlsmt);
    
//    获得主广场服务器的ip地址及其通信端口
    string msqe_ip;
    int msqe_port;
    sql_quote = "select msqes_ip,msqes_port from client_info where rowid = 1;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    msqe_ip = (const char *)sqlite3_column_text(psqlsmt, 0);
    msqe_port = sqlite3_column_int(psqlsmt, 1);
    sqlite3_finalize(psqlsmt);
    
//    如果本地没有主广场服务器的公钥
    if(if_null){
        nclt.NewRequest(&preq, msqe_ip, msqe_port, "client-square request", "request for public key");
        nclt.NewRequestListener(preq, 30, psql, getSQEPublicKey);
        while (if_wait == 1) {
            sleep(10);
        }
        if(!if_wait){
#ifdef DEBUG
            printf("Succeed In Getting Rsa Public Key From SQEServer.\n");
#endif
        }
        else{
#ifdef DEBUG
            printf("Error In Getting Rsa Public Key From SQEServer.\n");
#endif
            throw "connection error";
            return -1;
        }
    }
//    获得与广场服务器的通信的公钥
    sql_quote = "select msqes_rsa_public from client_info where rowid = 1;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    sqlite3_step(psqlsmt);
    public_key_class *ppbc = (public_key_class *)sqlite3_column_blob(psqlsmt, 0);
    nclt.SetPublicKey(*ppbc);
    sqlite3_finalize(psqlsmt);
    
//    已获得主广场服务器的密钥，进行启动客户端守护进程前的准备工作
    nclt.NewRequest(&preq, msqe_ip, msqe_port, "client-register request", "");
    nclt.NewRequestListener(preq, 30, psql, getSQEPublicKey);
    
    
    
    return 0;
}