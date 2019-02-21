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
        string resjson = string(pres->buff,pres->buff_size);
        Document resdoc;
        resdoc.Parse(resjson.data());
        string status = resdoc["status"].GetString();
        if(status == "ok"){
            printf("Register succeed.\n");
            if_wait = 0;
        }
        else{
            printf("Register Fail.\n");
            if_wait = -1;
        }
    }
    else{
        if_wait = -1;
        printf("Request timeout.\n");
    }
}

void *connectionDeamon(void *args){
    connection_listener * pcntl = (connection_listener *)args;
    string first_data;
    //printf("Start Listen Connection From Server.\n");
    char *buff = nullptr;
    Addr t_addr;
    ssize_t size = 0;
    SocketTCPCServer ntcps;
    ntcps.SetDataSFD(pcntl->data_sfd);
    ntcps.SetClientAddr(pcntl->client_addr);
//    获得连接的类型是长链还是断链
    size = ntcps.RecvRAW_SM(&buff, t_addr);
    raw_data *pnrwd = new raw_data();
//    长连接还是短连接
    bool if_sm = true;
    if(Server::CheckRawMsg(buff, size)){
        Server::ProcessSignedRawMsg(buff, size, *pnrwd);
        if(!memcmp(&pnrwd->info, "LCNT", sizeof(uint32_t))){
            if_sm = false;
            printf("Long Connection From Server\n");
        }
        else if(!memcmp(&pnrwd->info, "SCNT", sizeof(uint32_t))){
            if_sm = true;
            printf("Short Connection From Server\n");
        }
        else if(!memcmp(&pnrwd->info, "CNTL", sizeof(uint32_t))){
            if_sm = true;
            //printf("Listen Connection From Server\n");
            
            ntcps.CloseConnection();
            pthread_exit(NULL);
        }
        else{
            printf("Connection illegal\n");
            delete pnrwd;
            pthread_exit(NULL);
        }
        
    }
    else{
        printf("Connection illegal\n");
        delete pnrwd;
        pthread_exit(NULL);
    }
    delete pnrwd;
    
    while (1) {
        if(if_sm) size = ntcps.RecvRAW(&buff, t_addr);
        else size = ntcps.RecvRAW_SM(&buff, t_addr);
        raw_data *pnrwd = new raw_data();
        packet *nppkt = new packet();
        encrypt_post *pncryp = new encrypt_post();
        if(size > 0){
            if(Server::CheckRawMsg(buff, size)){
                Server::ProcessSignedRawMsg(buff, size, *pnrwd);
                if(!memcmp(&pnrwd->info, "ECYP", sizeof(uint32_t))){
                    Server::Rawdata2Packet(*nppkt, *pnrwd);
                    SQEServer::Packet2Post(*nppkt, *pncryp, pcntl->key);
                    if(!memcmp(&pncryp->type, "JRES", sizeof(uint32_t))){
                        string jres_str = string(pncryp->buff,pncryp->buff_size);
                        Document ndoc;
                        ndoc.Parse(jres_str.data());
                        if(ndoc["status"].GetString() == string("ok")){
                            printf("Register Successful.\n");
                        }
                    }
                }
                else if(!memcmp(&pnrwd->info, "BEAT", sizeof(uint32_t))){
                    //printf("Connection Beated.\n");
                }
                else if(!memcmp(&pnrwd->info, "SCMD", sizeof(uint32_t))){
//                    来自管理员的命令
                    
                }
                Server::freeRawdataServer(*pnrwd);
                Server::freePcaketServer(*nppkt);
            }
            else if(size < 0){
                //printf("Lost Connection From Server.\n");
                delete pnrwd;
                delete pncryp;
                delete nppkt;
                delete pcntl;
                break;
            }
            free(buff);
        }
        delete pnrwd;
        delete pncryp;
        delete nppkt;
        usleep(10000);
    }
    
    pthread_exit(NULL);
}
