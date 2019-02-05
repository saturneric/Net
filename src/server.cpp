//
//  server.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/16.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "memory.h"
#include "server.h"

extern list<clock_register> clocks_list;
pthread_mutex_t mutex,mutex_rp,mutex_pktreq;

void setServerClock(Server *psvr, int clicks){
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_rp, NULL);
//    注册数据接收守护时钟
    clock_register *pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks;
    pncr->rawclick = clicks;
    pncr->func = serverDeamon;
    pncr->arg = (void *)psvr;
    newClock(pncr);
    
//    注册数据处理守护时钟
    pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks*2;
    pncr->rawclick = clicks*2;
    pncr->func = dataProcessorDeamon;
    pncr->arg = (void *)psvr;
    newClock(pncr);
}

void setServerClockForSquare(SQEServer *psvr, int clicks){
    setServerClock(psvr, clicks);
    pthread_mutex_init(&mutex_pktreq, NULL);
//    注册标准数据包处理守护时钟
    clock_register *pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks*2+3;
    pncr->rawclick = clicks*2;
    pncr->func = packetProcessorDeamonForSquare;
    pncr->arg = (void *)psvr;
    newClock(pncr);
}

Server::Server(int port, string send_ip,int send_port):socket(port),send_socket(send_ip,send_port){
    socket.UDPSetFCNTL();
}

void Server::SetSendPort(int port){
    send_socket.SetSendPort(port);
}

void Server::SetSendIP(string ip_addr){
    send_socket.SetSendIP(ip_addr);
}

//    将计算结果包转化为结构数据包
packet CNodeServer::CPURS2Packet(compute_result tcpur){
    packet rawpkt;
    rawpkt.type = 0;
    int count = 0;
    //        写入计算模块名字
    rawpkt.buffs.push_back({tcpur.name.size(),(void *)tcpur.name.data()});
    
    //        写入输入参数个数
    int *p_value = (int *)malloc(sizeof(uint32_t));
    *p_value = (int)tcpur.args_in->size();
    rawpkt.buffs.push_back({sizeof(uint32_t),p_value});
    //        写入输入参数
    vector<int> &fargs_in = *(tcpur.fargs_in);
    for(auto i = tcpur.args_in->begin(); i != tcpur.args_in->end(); i++,count++){
        if(fargs_in[count] == INT){
            int *p_value = (int *)malloc(sizeof(int));
            *p_value = *((int *)(*i));
            rawpkt.buffs.push_back({sizeof(int),p_value});
        }
        else if(fargs_in[count] == DOUBLE){
            double *p_value = (double *)malloc(sizeof(double));
            *p_value = *((double *)(*i));
            rawpkt.buffs.push_back({sizeof(double),p_value});
        }
    }
    //        写入输入参数个数
    p_value = (int *)malloc(sizeof(uint32_t));
    *p_value = (int)tcpur.args_out->size();
    rawpkt.buffs.push_back({sizeof(uint32_t),p_value});
    //        写入输出参数
    count = 0;
    vector<int> &fargs_out = *(tcpur.fargs_out);
    for(auto i = tcpur.args_out->begin(); i != tcpur.args_out->end(); i++,count++){
        if(fargs_out[count] == INT){
            int *p_value = (int *)malloc(sizeof(int));
            *p_value = *((int *)(*i));
            rawpkt.buffs.push_back({sizeof(int),p_value});
        }
        else if(fargs_out[count] == DOUBLE){
            double *p_value = (double *)malloc(sizeof(double));
            *p_value = *((double *)(*i));
            rawpkt.buffs.push_back({sizeof(double),p_value});
        }
    }
    return rawpkt;
}

void Server::Packet2Rawdata(packet &tpkt, raw_data &rdt){
    char *data = (char *)malloc(BUFSIZ);
    memset(data, 0, BUFSIZ);
    rdt.data = data;
    char *idx = data;
    string fdata;
//        写入包ID信息
    memcpy(idx, &tpkt.type, sizeof(uint32_t));
    idx += sizeof(uint32_t);
    for(auto i = tpkt.buffs.begin(); i != tpkt.buffs.end(); i++){
//            写入数据块大小信息
        memcpy(idx, &(*i).first, sizeof(uint32_t));
        idx += sizeof(uint32_t);
//            写入数据块信息
        memcpy(idx, (*i).second, (*i).first);
        idx += (*i).first;
    }
    rdt.size = idx - data;
}

void Server::Rawdata2Packet(packet &tpkt, raw_data &trdt){
    char *idx = trdt.data;
//        数据包ID
    uint32_t uint;
    memcpy(&tpkt.type, idx, sizeof(uint32_t));
    idx += sizeof(uint32_t);
//        数据包主体
    while(idx - trdt.data < trdt.size){
        memcpy(&uint, idx, sizeof(uint32_t));
        idx += sizeof(uint32_t);
        void *data = malloc(uint);
        memcpy(data, idx, uint);
        idx += uint;
        tpkt.AddBuff(data, uint);
        free(data);
    }
}

compute_result CNodeServer::Packet2CPUR(packet *tpkt){
    compute_result tcpur;
    tcpur.args_in = new vector<void *>();
    tcpur.args_out = new vector<void *>();
    if(tpkt->type == 0){
        int nargs_in = *(int *)(tpkt->buffs[0].second);
        int nargs_out = *(int *)(tpkt->buffs[nargs_in+1].second);
        //            转化输入参数
        for(int i = 0; i < nargs_in; i++){
            (*tcpur.args_in)[i] = malloc(tpkt->buffs[i+1].first);
            memcpy((*tcpur.args_in)[i], tpkt->buffs[i+1].second, tpkt->buffs[i+1].first);
        }
        for(int i = nargs_in+1; i < nargs_in+nargs_out+2; i++){
            (*tcpur.args_out)[i] = malloc(tpkt->buffs[i+1].first);
            memcpy((*tcpur.args_out)[i], tpkt->buffs[i+1].second, tpkt->buffs[i+1].first);
        }
    }
    return tcpur;
}


void Server::freeRawdataServer(struct raw_data &trdt){
    free(trdt.data);
    if(trdt.msg != NULL) free(trdt.msg);
}

void Server::freePcaketServer(struct packet tpkt){
    for(auto i = tpkt.buffs.begin(); i != tpkt.buffs.end(); i++)
        free(i->second);
}

void Server::freeCPURServer(struct compute_result tcpur){
    //        释放输入参数容器所占用的所有内存
    for(auto i = tcpur.args_in->begin(); i != tcpur.args_in->end(); i++)
        free(*i);
    delete tcpur.args_in;
    
    //        释放输出参数容器所占用的所有内存
    for(auto i = tcpur.args_out->begin(); i != tcpur.args_out->end(); i++)
        free(*i);
    delete tcpur.args_out;
}

void Server::SignedRawdata(struct raw_data *trdt,string info){
    //        填充标签信息
    memcpy(&trdt->head, "NETC", sizeof(uint32_t));
    memcpy(&trdt->tail, "CTEN", sizeof(uint32_t));
    memcpy(&trdt->info, info.data(), sizeof(uint32_t));
    //        整合信息
    char *msg = (char *)malloc(sizeof(uint32_t) * 3 + trdt->size);
    trdt->msg_size = sizeof(uint32_t) * 3 + trdt->size;
    char *idx = msg;
    memcpy(idx, &trdt->head, sizeof(uint32_t));
    idx += sizeof(uint32_t);
    memcpy(idx, &trdt->info, sizeof(uint32_t));
    idx += sizeof(uint32_t);
    memcpy(idx, trdt->data, trdt->size);
    idx += trdt->size;
    memcpy(idx, &trdt->tail, sizeof(uint32_t));
    trdt->msg = msg;
    
}

int Server::SentRawdata(struct raw_data *trdt){
    send_socket.SendRAW(trdt->msg, trdt->msg_size);
    return 0;
}

bool Server::CheckRawMsg(char *p_rdt, ssize_t size){
    uint32_t head, tail;
    char *idx = p_rdt;
    memcpy(&head, "NETC", sizeof(uint32_t));
    memcpy(&tail, "CTEN", sizeof(uint32_t));
    if(!memcmp(idx, &head, sizeof(uint32_t))){
        idx += size-sizeof(uint32_t);
        if(!memcmp(idx, &tail, sizeof(uint32_t))) return true;
        else return false;
    }
    else return false;
}

void Server::ProcessSignedRawMsg(char *p_rdt, ssize_t size, raw_data &rdt){
    rdt.data = (char *)malloc(size-3*sizeof(uint32_t));
    memcpy(&rdt.info, p_rdt+sizeof(uint32_t), sizeof(uint32_t));
    memcpy(rdt.data, p_rdt+sizeof(uint32_t)*2, size-3*sizeof(uint32_t));
    rdt.size = size-3*sizeof(uint32_t);
}

void *serverDeamon(void *pvcti){
    clock_thread_info *pcti = (clock_thread_info *) pvcti;
    Server *psvr = (Server *) pcti->args;
    //cout<<"Server Deamon Checked."<<endl;
    Addr f_addr;
    
    int prm = psvr->packet_max;
    ssize_t tlen;
    char *str = nullptr;
    Addr taddr;
    do{
//        加锁
        if (pthread_mutex_lock(&mutex) != 0) throw "lock error";
        tlen = psvr->socket.RecvRAW(&str,taddr);
        if(tlen > 0){
//            记录有效数据包
            
            if(Server::CheckRawMsg(str, tlen)){
                raw_data *ptrdt = new raw_data();
                Server::ProcessSignedRawMsg(str, tlen, *ptrdt);
                ptrdt->address = *(struct sockaddr_in *)taddr.RawObj();
                psvr->rawdata_in.push_back(ptrdt);
                
            }
        }
        free(str);
//        解锁
        pthread_mutex_unlock(&mutex);
    }while (tlen && prm-- > 0);
    
    clockThreadFinish(pcti->tid);
    pthread_exit(NULL);
}

void *dataProcessorDeamon(void *pvcti){
    clock_thread_info *pcti = (clock_thread_info *) pvcti;
    Server *psvr = (Server *) pcti->args;
    psvr->ProcessRawData();
    clockThreadFinish(pcti->tid);
    pthread_exit(NULL);
}

void *packetProcessorDeamonForSquare(void *pvcti){
    clock_thread_info *pcti = (clock_thread_info *) pvcti;
    SQEServer *psvr = (SQEServer *) pcti->args;
    psvr->ProcessPacket();
    clockThreadFinish(pcti->tid);
    pthread_exit(NULL);
}

void *requsetProcessorDeamonForSquare(void *pvcti){
    clock_thread_info *pcti = (clock_thread_info *) pvcti;
    SQEServer *psvr = (SQEServer *) pcti->args;
    psvr->ProcessPacket();
    clockThreadFinish(pcti->tid);
    pthread_exit(NULL);
}

void Server::ProcessRawData(void){
//    一次性最大处理个数
    int prm = 2048;
//    加锁
    if (pthread_mutex_lock(&mutex) != 0) throw "lock error";
    for(auto &prdt : rawdata_in){
        if(prdt == nullptr) continue;
        if(prm-- == 0) break;

        if(!memcmp(&prdt->info, "SPKT", sizeof(uint32_t))){
//            加锁
            if (pthread_mutex_lock(&mutex_rp) != 0) throw "lock error";
            packet *pnpkt = new packet();
            Rawdata2Packet(*pnpkt,*prdt);
            pnpkt->address = prdt->address;
            packets_in.push_back(pnpkt);
//            解锁
            pthread_mutex_unlock(&mutex_rp);
        }
        else{
            
        }
        freeRawdataServer(*prdt);
        delete prdt;
        prdt = nullptr;

    }
//    解锁
    pthread_mutex_unlock(&mutex);
    
    if (pthread_mutex_lock(&mutex) != 0) throw "lock error";
    rawdata_in.remove_if([](auto prdt){return prdt == nullptr;});
    pthread_mutex_unlock(&mutex);
}

void SQEServer::ProcessPacket(void){
    printf("RW: %lu PKT: %lu REQ: %lu\n",rawdata_in.size(),packets_in.size(),req_list.size());
//    一次性最大处理个数
    int prm = 2048;
//    加锁
    if (pthread_mutex_lock(&mutex_rp) != 0) throw "lock error";
    for(auto &ppkt : packets_in){
        if(ppkt == nullptr) continue;
        if(prm-- == 0) break;
        if(ppkt->type == REQUSET_TYPE){
            request *pnreq = new request();
            Packet2Request(*ppkt, *pnreq);
            req_list.push_back(pnreq);
        }
        freePcaketServer(*ppkt);
        delete ppkt;
        ppkt = nullptr;
    }
//    解锁
    pthread_mutex_unlock(&mutex_rp);
    packets_in.remove_if([](auto &ppkt){return ppkt == nullptr;});
}

SQEServer::SQEServer(int port):Server(port){
    if(sqlite3_open("info.db", &psql) == SQLITE_ERROR){
        sql::printError(psql);
        throw "database is abnormal";
    }
    sqlite3_stmt *psqlsmt;
    const char *pzTail;
//    从数据库获得服务器的公私钥
    string sql_quote = "select sqes_public,sqes_private from server_info where rowid = 1;";
    sqlite3_prepare(psql, sql_quote.data(), -1, &psqlsmt, &pzTail);
    if(sqlite3_step(psqlsmt) != SQLITE_ROW){
        sql::printError(psql);
        throw "database is abnormal";
    }
    Byte *tbyt = (Byte *)sqlite3_column_blob(psqlsmt, 0);
    memcpy(&pkc, tbyt, sizeof(public_key_class));
    
    tbyt = (Byte *)sqlite3_column_blob(psqlsmt, 1);
    memcpy(&prc, tbyt, sizeof(private_key_class));
    
    sqlite3_finalize(psqlsmt);
}

void SQEServer::Packet2Request(packet &pkt, request &req){
    if(pkt.type == REQUSET_TYPE){
        req.r_id = *(uint32_t *)pkt.buffs[0].second;
        req.type = (const char *)pkt.buffs[1].second;
        req.data = (const char *)pkt.buffs[2].second;
        req.t_addr = Addr(*(struct sockaddr_in *)pkt.buffs[3].second);
    }
}

void SQEServer::Request2Packet(packet &pkt, request &req){
    pkt.address = *req.t_addr.Obj();
//    请求的类型标识号
    pkt.type = REQUSET_TYPE;
    pkt.AddBuff((void *)&req.r_id, sizeof(req.r_id));
    pkt.AddBuff((void *)req.type.data(), (uint32_t)req.type.size());
    pkt.AddBuff((void *)req.data.data(), (uint32_t)req.data.size());
    pkt.AddBuff((void *)req.t_addr.Obj(), sizeof(struct sockaddr_in));
}

void packet::AddBuff(void *pbuff, uint32_t size){
    void *pnbuff = malloc(size);
    memcpy(pnbuff, pbuff, size);
    buffs.push_back({size,pnbuff});
}

void Server::ProcessRequset(void){
    
}


request::request(){
    r_id = rng::tsc_seed{}();
}


packet::~packet(){
    
}
