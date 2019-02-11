//
//  model.cpp
//  Net
//
//  Created by 胡一兵 on 2019/2/8.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "instruct.h"

extern int if_wait;

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

bool config_search(vector<string> &configs,string tfg){
    for(auto config : configs){
        if(config == tfg) return true;
    }
    return false;
}

void getSQEPublicKey(respond *pres,void *args){
    if(pres != nullptr){
        public_key_class *npbc = (public_key_class *)pres->buff;
        sqlite3 *psql = (sqlite3 *)args;
        sqlite3_stmt *psqlsmt;
        const char *pzTail;
        string sql_quote = "update client_info set msqes_rsa_public = ?1 where rowid = 1;";
        sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
        sqlite3_bind_blob(psqlsmt, 1, npbc, sizeof(public_key_class), SQLITE_TRANSIENT);
        sqlite3_step(psqlsmt);
        sqlite3_finalize(psqlsmt);
        if_wait = 0;
    }
    else if_wait = -1;
}

void registerSQECallback(respond *pres,void *args){
    if(pres != nullptr){
        
    }
    else{
        if_wait = -1;
        printf("request timeout.\n");
    }
}


