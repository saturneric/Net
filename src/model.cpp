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
	void printRed(string red_info) {
		printf("\033[31m%s\n\033[0m", red_info.data());
	}
	void printInfo(string info) {
		printf("%s\n", info.data());
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

//客户端连接管理守护线程
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

//    检测连接是长连接还是短连接
    bool if_sm = true;
	string dget = "DGET";
    if(Server::CheckRawMsg(buff, size)){
        Server::ProcessSignedRawMsg(buff, size, *pnrwd);
        if(!memcmp(&pnrwd->info, "LCNT", sizeof(uint32_t))){
            if_sm = false;
        }
        else if(!memcmp(&pnrwd->info, "SCNT", sizeof(uint32_t))){
            if_sm = true;
			ntcps.SendRespond(dget);
        }
        else if(!memcmp(&pnrwd->info, "CNTL", sizeof(uint32_t))){
            if_sm = true;
            //printf("Listen Connection From Server\n");
            
            ntcps.CloseConnection();
            pthread_exit(NULL);
        }
        else{
			//断开无效连接
            printf("Connection Illegal.\n");
			delete pnrwd;
			close(pcntl->data_sfd);
			delete pcntl;

            pthread_exit(NULL);
        }
        
    }
    else{
        printf("Connection Illegal.\n");
        delete pnrwd;
        pthread_exit(NULL);
    }
    delete pnrwd;
    
    while (1) {
		if (*pcntl->pif_atv == false) {
			close(pcntl->data_sfd);
			delete pcntl;
			break;
		}
		//区分长连接与短连接
        if(if_sm) size = ntcps.RecvRAW(&buff, t_addr);
        else size = ntcps.RecvRAW_SM(&buff, t_addr);
        
        if(size > 0){
			raw_data *pnrwd = new raw_data();
			packet *nppkt = new packet();
			encrypt_post *pncryp = new encrypt_post();
            if(Server::CheckRawMsg(buff, size)){
                Server::ProcessSignedRawMsg(buff, size, *pnrwd);
				//获得端对端加密报文
                if(!memcmp(&pnrwd->info, "ECYP", sizeof(uint32_t))){
                    Server::Rawdata2Packet(*nppkt, *pnrwd);
                    SQEServer::Packet2Post(*nppkt, *pncryp, pcntl->key);
					//获得注册信息反馈报文
                    if(!memcmp(&pncryp->type, "JRES", sizeof(uint32_t))){
						//自我解析
						pncryp->SelfParse();
                        if(pncryp->edoc["status"].GetString() == string("ok")){
							error::printSuccess("Register Successful.");
							//进入客户端管理终端
							memcpy(pcntl->father_buff,"D_OK", sizeof(uint32_t));
                        }
                    }
					//管理指令连接
					else if (!memcmp(&pnrwd->info, "JCMD", sizeof(uint32_t))) {
						//来自管理员的命令

					}
                }
				//心跳连接
                else if(!memcmp(&pnrwd->info, "BEAT", sizeof(uint32_t))){
                    //printf("Connection Beated.\n");
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
			delete pnrwd;
			delete pncryp;
			delete nppkt;
        }
        usleep(1000);
    }
    
    pthread_exit(NULL);
}


void *clientServiceDeamon(void *arg) {
	connection_listener *pclst = (connection_listener *)arg;
	while (1) {
		if (pclst->if_active == false) {
			break;
		}
		pclst->server_cnt->Accept();
		//构造连接守护子进程
		connection_listener *pncl = new connection_listener();
		pncl->client_addr = pclst->client_addr;
		pncl->data_sfd = pclst->server_cnt->GetDataSFD();
		pncl->key = pclst->key;
		pncl->father_buff = pclst->father_buff;
		pncl->pif_atv = &pclst->if_active;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&pncl->pid, &attr, connectionDeamon, pncl);
		pthread_attr_destroy(&attr);
		usleep(1000);
	}
}

void gets_s(char *buff, uint32_t size) {
	char ch;
	uint32_t i = 0;
	while ((ch = getchar()) != '\n' && i < (size - 1)) {
		buff[i++] = ch;
	}
	buff[i] = '\0';
}