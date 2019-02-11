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
extern rng::rng64 rand64;
extern rng::rng128 rand128;



pthread_mutex_t mutex,mutex_rp,mutex_pktreq,mutex_sndpkt,mutex_box,mutex_cltreg;

void setServerClock(Server *psvr, int clicks){
    if(!clicks) throw "clock clicks error";
    
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_rp, NULL);
    pthread_mutex_init(&mutex_sndpkt, NULL);
    pthread_mutex_init(&mutex_box, NULL);
//    注册数据接收时钟
    clock_register *pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks;
    pncr->rawclick = clicks;
    pncr->func = serverDeamon;
    pncr->arg = (void *)psvr;
    newClock(pncr);
    
//    注册数据处理时钟
    pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks*2;
    pncr->rawclick = clicks*2;
    pncr->func = dataProcessorDeamon;
    pncr->arg = (void *)psvr;
    newClock(pncr);
    
//    注册标准数据包发送时钟
    pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks*2;
    pncr->rawclick = clicks/1.5+1;
    pncr->func = sendPacketProcessorDeamonForSquare;
    pncr->arg = (void *)psvr;
    newClock(pncr);
    
//    UDP分包监听管理时钟
    pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks+1;
    pncr->rawclick = clicks/1.5+1;
    pncr->func = boxProcessorDeamon;
    pncr->arg = (void *)psvr;
    newClock(pncr);
    
//    UDP分包监听清理管理时钟
    pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = (clicks+1)*5;
    pncr->rawclick = clicks*2.5+1;
    pncr->func = boxsCleaningProcessorDeamon;
    pncr->arg = (void *)psvr;
    newClock(pncr);
}

void setServerClockForSquare(SQEServer *psvr, int clicks){
    setServerClock(psvr, clicks);
    pthread_mutex_init(&mutex_pktreq, NULL);
    pthread_mutex_init(&mutex_cltreg, NULL);
//    注册标准数据包处理守护时钟
    clock_register *pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks*2+3;
    pncr->rawclick = clicks*2;
    pncr->func = packetProcessorDeamonForSquare;
    pncr->arg = (void *)psvr;
    newClock(pncr);
    
//    注册请求处理守护时钟
    pncr = new clock_register();
    pncr->if_thread = true;
    pncr->if_reset = true;
    pncr->click = clicks*2+7;
    pncr->rawclick = clicks/2;
    pncr->func = requestProcessorDeamonForSquare;
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

//将计算结果包转化为结构数据包
packet CNodeServer::CPURS2Packet(compute_result tcpur){
    packet rawpkt;
    rawpkt.type = 0;
    int count = 0;
//    写入计算模块名字
    rawpkt.buffs.push_back({tcpur.name.size(),(void *)tcpur.name.data()});
    
//    写入输入参数个数
    int *p_value = (int *)malloc(sizeof(uint32_t));
    *p_value = (int)tcpur.args_in->size();
    rawpkt.buffs.push_back({sizeof(uint32_t),p_value});
//    写入输入参数
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
//    写入输入参数个数
    p_value = (int *)malloc(sizeof(uint32_t));
    *p_value = (int)tcpur.args_out->size();
    rawpkt.buffs.push_back({sizeof(uint32_t),p_value});
//    写入输出参数
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
    unsigned char *data = (unsigned char *)malloc(BUFSIZ);
    memset(data, 0, BUFSIZ);
    rdt.data = data;
    unsigned char *idx = data;
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
    unsigned char *idx = trdt.data;
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

void Client::SendRawData(raw_data *trdt){
//    对大包进行拆分发送
    if(trdt->msg_size > 256){
        uint64_t aidx = 0,bidx = 0;
        int64_t alls = trdt->msg_size;
        uint64_t tmp_cnt = (trdt->msg_size/256), tmp_idx = 0;
        while(bidx < trdt->msg_size-1){
            alls -= 256;
            if(alls > 256) bidx = aidx+255;
            else bidx = trdt->msg_size-1;
//            构造UDP分包
            net_box nnb;
            nnb.b_id = trdt->r_id;
            nnb.cnt = tmp_cnt;
            nnb.idx = tmp_idx;
            UByte *f_byte = (UByte *)&trdt->msg[aidx];
            nnb.set(f_byte,bidx-aidx+1);
            
            nnb.build();
            send_socket.SendRAW((Byte *)nnb.send_data, nnb.sdt_size);
            aidx = bidx+1;
            tmp_idx++;
        }
    }
    else send_socket.SendRAW(trdt->msg, trdt->msg_size);
}

int Server::SentRawdata(struct raw_data *trdt){
//    对大包进行拆分发送
    if(trdt->msg_size > 256){
        uint64_t aidx = 0,bidx = 0;
        int64_t alls = trdt->msg_size;
        uint64_t tmp_cnt = (trdt->msg_size/256), tmp_idx = 0;
        while(bidx < trdt->msg_size-1){
            alls -= 256;
            if(alls > 256) bidx = aidx+255;
            else bidx = trdt->msg_size-1;
//            构造UDP分包
            net_box nnb;
            nnb.b_id = trdt->r_id;
            nnb.cnt = tmp_cnt;
            nnb.idx = tmp_idx;
            UByte *f_byte = (UByte *)&trdt->msg[aidx];
            nnb.set(f_byte,bidx-aidx+1);
            
            nnb.build();
            send_socket.SendRAW((Byte *)nnb.send_data, nnb.sdt_size);
            aidx = bidx+1;
            tmp_idx++;
        }
    }
    else send_socket.SendRAW(trdt->msg, trdt->msg_size);
    return 0;
}

void net_box::FreeNetBox(void){
    if(data != nullptr) free(data);
    if(send_data != nullptr) free(send_data);
}

net_box::~net_box(){
    
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

bool Server::CheckNetBox(char *p_nb, ssize_t size){
    uint32_t head, tail;
    Byte *idx = p_nb;
    memcpy(&head, "NETB", sizeof(uint32_t));
    memcpy(&tail, "BTEN", sizeof(uint32_t));
    if(!memcmp(idx, &head, sizeof(uint32_t))){
        idx += size-sizeof(uint32_t);
        if(!memcmp(idx, &tail, sizeof(uint32_t))) return true;
        else return false;
    }
    else return false;
}

void Server::ProcessSignedRawMsg(char *p_rdt, ssize_t size, raw_data &rdt){
    rdt.data = (UByte *)malloc(size-3*sizeof(uint32_t));
    if(rdt.data == nullptr) throw "malloc error";
    memcpy(&rdt.head, p_rdt, sizeof(uint32_t));
    memcpy(&rdt.info, p_rdt+sizeof(uint32_t), sizeof(uint32_t));
    memcpy(rdt.data, p_rdt+sizeof(uint32_t)*2, size-3*sizeof(uint32_t));
    memcpy(&rdt.tail, p_rdt+size-sizeof(uint32_t), sizeof(uint32_t));
    rdt.size = size-3*sizeof(uint32_t);
}

void Server::DecryptRSARawMsg(raw_data &rdt, private_key_class &pkc){
    UByte *p_data = rdt.data;
    rdt.data = rsa_decrypt((const uint64_t *) p_data, rdt.size, &pkc);
    rdt.size /= 8;
    free(p_data);
}

void Server::EncryptRSARawMsg(raw_data &rdt, public_key_class &pkc){
    UByte *p_data = rdt.data;
    rdt.data = (UByte *)rsa_encrypt((const UByte *)p_data, rdt.size, &pkc);
    rdt.size *= 8;
    free(p_data);
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
//            记录UDP分包
            if(Server::CheckNetBox(str, tlen)){
                net_box *pnbx = new net_box();
                Server::ProcessNetBox(*pnbx, str);
                auto pnbxl_itr = psvr->boxls.end();
                
                if((pnbxl_itr = psvr->boxls.find(pnbx->b_id)) != psvr->boxls.end()){
                    box_listener *pnbxl = pnbxl_itr->second;
                    if (pthread_mutex_lock(&mutex_box) != 0) throw "lock error";
                    if(pnbxl->boxs[pnbx->idx] == nullptr && pnbxl->nbn < pnbxl->cnt){
                        pnbxl->boxs[pnbx->idx] = pnbx;
                        pnbxl->clicks += 5;
                        pnbxl->nbn++;
                    }
                    else{
                        pnbx->FreeNetBox();
                        delete pnbx;
                    }
                    pthread_mutex_unlock(&mutex_box);
                }
                else{
                    box_listener *pnbxl = new box_listener();
                    pnbxl->cnt = pnbx->cnt;
                    pnbxl->boxs = (net_box **) malloc(sizeof(net_box *) * pnbxl->cnt);
                    memset(pnbxl->boxs, 0, sizeof(net_box *) * pnbxl->cnt);
                    pnbxl->boxs[pnbx->idx] = pnbx;
                    pnbxl->b_id = pnbx->b_id;
                    pnbxl->clicks = 15;
                    pnbxl->nbn = 1;
                    if (pthread_mutex_lock(&mutex_box) != 0) throw "lock error";
                    psvr->boxls.insert({pnbxl->b_id,pnbxl});
                    pthread_mutex_unlock(&mutex_box);
                }
                
                
            }
//            记录UDP包
            else if(Server::CheckRawMsg(str, tlen)){
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

void box_listener::free_boxs(void){
    for(auto i = 0; i < cnt; i++){
        if(boxs[i] != nullptr){
            boxs[i]->FreeNetBox();
            delete boxs[i];
        }
    }
    free(boxs);
}

void *boxProcessorDeamon(void *pvcti){
    clock_thread_info *pcti = (clock_thread_info *) pvcti;
    Server *psvr = (Server *) pcti->args;
    if (pthread_mutex_lock(&mutex_box) != 0) throw "lock error";
//    加锁
    for(auto boxl_pair : psvr->boxls){
        box_listener *pboxl = boxl_pair.second;
        if(pboxl->clicks < 0) continue;
        if(pboxl->cnt == pboxl->nbn){
            raw_data *pnrdt = new raw_data();
            pboxl->TogtRawData(*pnrdt);
            pnrdt->r_id = pboxl->b_id;
            psvr->rawdata_in.push_back(pnrdt);
            pboxl->clicks = -1;
            pboxl->free_boxs();
        }
        else{
            if(pboxl->clicks > 0) pboxl->clicks--;
            else if(pboxl->clicks == 0){
                pboxl->free_boxs();
                pboxl->clicks = -1;
            }
        }
        
    }
//    清除到期的监听
    for(auto i = psvr->boxls.begin(); i != psvr->boxls.end();){
        if(i->second->clicks == -1){
            delete i->second;
            i = psvr->boxls.erase(i);
        }
        
        else i++;
    }
//    解锁
    pthread_mutex_unlock(&mutex_box);
    clockThreadFinish(pcti->tid);
    pthread_exit(NULL);
}

void *boxsCleaningProcessorDeamon(void *pvcti){
    clock_thread_info *pcti = (clock_thread_info *) pvcti;
    Server *psvr = (Server *) pcti->args;
    psvr->CleaningBoxs();
    clockThreadFinish(pcti->tid);
    pthread_exit(NULL);
}

void Server::CleaningBoxs(void){
    if(pthread_mutex_lock(&mutex_box) != 0) throw "lock error";
    //    清除所有到期的监听
    for(auto i = boxls.begin(); i != boxls.end();){
        if(i->second->clicks == -1){
            delete i->second;
             i = boxls.erase(i);
        }
        else i++;
    }
    pthread_mutex_unlock(&mutex_box);
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

void *requestProcessorDeamonForSquare(void *pvcti){
    clock_thread_info *pcti = (clock_thread_info *) pvcti;
    SQEServer *psvr = (SQEServer *) pcti->args;
    psvr->ProcessRequset();
    clockThreadFinish(pcti->tid);
    pthread_exit(NULL);
}

void *sendPacketProcessorDeamonForSquare(void *pvcti){
    clock_thread_info *pcti = (clock_thread_info *) pvcti;
    SQEServer *psvr = (SQEServer *) pcti->args;
    
    psvr->ProcessSendPackets();
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
//            标记未加密
            pnpkt->if_encrypt = false;
            Rawdata2Packet(*pnpkt,*prdt);
            pnpkt->address = prdt->address;
            packets_in.push_back(pnpkt);
//            解锁
            pthread_mutex_unlock(&mutex_rp);
        }
//        编码加密包
        else if(!memcmp(&prdt->info, "RPKT", sizeof(uint32_t))){
            if (pthread_mutex_lock(&mutex_rp) != 0) throw "lock error";
            packet *pnpkt = new packet();
//            标记数据已被加密
            pnpkt->if_encrypt = true;
            Server::DecryptRSARawMsg(*prdt, prc);
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
//    一次性最大处理个数
    int prm = 2048;
//    加锁
    if (pthread_mutex_lock(&mutex_rp) != 0) throw "lock error";
    for(auto &ppkt : packets_in){
        if(ppkt == nullptr) continue;
        if(prm-- == 0) break;
        if(ppkt->type == REQUSET_TYPE){
            if(pthread_mutex_lock(&mutex_pktreq) != 0) throw "lock error";
            request *pnreq = new request();
            Packet2Request(*ppkt, *pnreq);
            pnreq->t_addr.SetSockAddr(ppkt->address);
            req_list.push_back(pnreq);
            pthread_mutex_unlock(&mutex_pktreq);
        }
        if(ppkt->type == ENCRYPT_POST_TYPE){
            encrypt_post *ecpst = new encrypt_post();
            GetPostInfo(*ppkt, *ecpst);
            auto tgtclt = client_lst.find(ecpst->client_id);
            if(tgtclt != client_lst.end()){
                client_register *pclr = tgtclt->second;
                Packet2Post(*ppkt, *ecpst, pclr->key);
            }
        }
        freePcaketServer(*ppkt);
        delete ppkt;
        ppkt = nullptr;
    }
    packets_in.remove_if([](auto &ppkt){return ppkt == nullptr;});
//    解锁
    pthread_mutex_unlock(&mutex_rp);
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
        req.r_id = *(uint64_t *)pkt.buffs[0].second;
        uint64_t type_size = *(uint64_t *)pkt.buffs[1].second;
        req.type = string((const char *)pkt.buffs[2].second,type_size);
        uint64_t data_size = *(uint64_t *)pkt.buffs[3].second;
        req.data = string((const char *)pkt.buffs[4].second,data_size);
        req.t_addr = Addr(*(struct sockaddr_in *)pkt.buffs[5].second);
        req.recv_port = *(uint32_t *)pkt.buffs[6].second;
    }
}

void SQEServer::Request2Packet(packet &pkt, request &req){
    pkt.address = *req.t_addr.Obj();
//    请求的类型标识号
    pkt.type = REQUSET_TYPE;
    pkt.AddBuff((void *)&req.r_id, sizeof(uint64_t));//0
    uint64_t type_size = req.type.size();
    pkt.AddBuff((void *)&type_size, sizeof(uint64_t));//1
    pkt.AddBuff((void *)req.type.data(), (uint32_t)req.type.size());//2
    uint64_t data_size = req.data.size();
    pkt.AddBuff((void *)&data_size, sizeof(uint64_t));//3
    pkt.AddBuff((void *)req.data.data(), (uint32_t)req.data.size());//4
    pkt.AddBuff((void *)req.t_addr.Obj(), sizeof(struct sockaddr_in));//5
    pkt.AddBuff((void *)&req.recv_port, sizeof(uint32_t));//6
}

void packet::AddBuff(const void *pbuff, uint32_t size){
    void *pnbuff = malloc(size);
    memcpy(pnbuff, pbuff, size);
    buffs.push_back({size,pnbuff});
}

void SQEServer::ProcessRequset(void){
    //printf("BL: %3lu RW: %4lu PKT: %4lu REQ: %4lu PKTS: %5lu\n",boxls.size(),rawdata_in.size(),packets_in.size(),req_list.size(),packets_out.size());
//    一次性最大处理数
    int prm = 2048;
    if(pthread_mutex_lock(&mutex_pktreq) != 0) throw "lock error";
    for(auto &preq : req_list){
        if(preq == nullptr) continue;
        if(prm-- == 0) break;
        if(preq->type == "client-square request"){
            if(preq->data == "request for public key"){
                respond *pnr = new respond();
                pnr->r_id = preq->r_id;
                pnr->SetBuff((Byte *)&pkc, sizeof(public_key_class));
                pnr->type = "square public key";
                pnr->t_addr = preq->t_addr;
                pnr->t_addr.SetPort(preq->recv_port);
                packet *pnpkt = new packet();
                Respond2Packet(*pnpkt, *pnr);
                delete pnr;
                if(pthread_mutex_lock(&mutex_sndpkt) != 0) throw "lock error";
                packets_out.push_back(pnpkt);
                pthread_mutex_unlock(&mutex_sndpkt);
            }
        }
        else if(preq->type == "client-register request"){
//            解析JSON结构
            preq->req_doc.Parse(preq->data.data());
            Document &jdoc = preq->req_doc;
            if(1){
                client_register *pclr = new client_register();
                pclr->client_id = rand64();
                uint8_t *pkey = (uint8_t *) pclr->key.key;
                uint32_t idx = 0;
                for(auto &kitem : jdoc["key"].GetArray())
                    pkey[idx++] = kitem.GetInt();
                
                pclr->name = jdoc["name"].GetString();
                pclr->tag = jdoc["tag"].GetString();
                if(pthread_mutex_lock(&mutex_cltreg) != 0) throw "lock error";
                client_lst.insert({pclr->client_id,pclr});
                pthread_mutex_unlock(&mutex_cltreg);
            }
            
            
        }
        delete preq;
        preq = nullptr;
    }
    req_list.remove_if([](auto &preq){return preq == nullptr;});
    pthread_mutex_unlock(&mutex_pktreq);
}

void SQEServer::Packet2Respond(packet &pkt, respond &res){
    res.r_id = *(uint64_t *)pkt.buffs[0].second;
    res.t_addr.SetSockAddr(*(struct sockaddr_in *)pkt.buffs[1].second);
    res.type = (const char *)pkt.buffs[2].second;
    res.buff_size = pkt.buffs[3].first;
    res.buff = (Byte *)malloc(res.buff_size);
    memcpy(res.buff,pkt.buffs[3].second,res.buff_size);
}

void SQEServer::Respond2Packet(packet &pkt, respond &res){
    pkt.type = RESPOND_TYPE;
    pkt.address = *res.t_addr.Obj();
    pkt.AddBuff((void *) &res.r_id, sizeof(rng::rng64));
    pkt.AddBuff((void *) res.t_addr.Obj(), sizeof(sockaddr_in));
    pkt.AddBuff((void *) res.type.data(), (uint32_t)res.type.size());
    pkt.AddBuff((void *) res.buff, res.buff_size);
}

request::request(){
    r_id = rand64();
}

void respond::SetBuff(Byte *buff, uint32_t size){
    void *nbuff = malloc(size);
    memcpy(nbuff, buff, size);
    this->buff = (Byte *)nbuff;
    this->buff_size = size;
}

packet::~packet(){
    
}

respond::~respond(){
    if(buff != nullptr) free(buff);
}

void Server::ProcessSendPackets(void){
//    一次性最大处理个数
    int prm = 512;
    if(pthread_mutex_lock(&mutex_sndpkt) != 0) throw "lock error";
    for(auto &ppkt : packets_out){
        if(ppkt == nullptr) continue;
        if(prm-- == 0) break;
        raw_data nrwd;
        Packet2Rawdata(*ppkt, nrwd);
        SignedRawdata(&nrwd, "SPKT");
        send_socket.SetSendSockAddr(ppkt->address);
        SentRawdata(&nrwd);
        freeRawdataServer(nrwd);
        freePcaketServer(*ppkt);
        delete ppkt;
        ppkt = nullptr;
    }
    packets_out.remove_if([](auto ppkt){return ppkt == nullptr;});
    pthread_mutex_unlock(&mutex_sndpkt);
}

aes_key256::aes_key256(){
    for (int i = 0; i < 4; i++){
        key[i] = rand64();
    }
}


const uint8_t *aes_key256::GetKey(void){
    return (const uint8_t *)&key;
}


void SQEServer::Post2Packet(packet &pkt, encrypt_post &pst, aes_key256 &key){
    pkt.type = ENCRYPT_POST_TYPE;
    Addr taddr(pst.ip,pst.port);
    pkt.address = *taddr.Obj();
    pkt.AddBuff(&pst.client_id, sizeof(uint64_t));//0
    pkt.AddBuff(&pst.p_id, sizeof(uint64_t));//1
    pkt.AddBuff((const void *)pst.ip.data(), (uint32_t)pst.ip.size());//2
    pkt.AddBuff((void *)&pst.port, sizeof(uint32_t));//3
    string MD5_HEX;
    MD5EncryptEasy(MD5_HEX, pst.buff, pst.buff_size);
    
//    加密数据
    AES_ctx naes;
    AES_init_ctx(&naes, key.GetKey());
    AES_CBC_encrypt_buffer(&naes, (uint8_t *)pst.buff, pst.buff_size);
    pkt.AddBuff(pst.buff, pst.buff_size);//4
    pkt.AddBuff((void *)MD5_HEX.data(), (uint32_t)MD5_HEX.size());//5
}

void SQEServer::GetPostInfo(packet &pkt, encrypt_post &pst){
    pst.client_id = *(uint64_t *)pkt.buffs[0].second;
    pst.p_id = *(uint64_t *)pkt.buffs[1].second;
    pst.ip = (const char *)pkt.buffs[2].second;
    pst.port = *(uint32_t *)pkt.buffs[3].second;
}

void SQEServer::Packet2Post(packet &pkt, encrypt_post &pst, aes_key256 &key){
    pst.client_id = *(uint64_t *)pkt.buffs[0].second;
    pst.p_id = *(uint64_t *)pkt.buffs[1].second;
    pst.ip = (const char *)pkt.buffs[2].second;
    pst.port = *(uint32_t *)pkt.buffs[3].second;
    pst.buff_size = pkt.buffs[4].first;
    string TMD5,MD5;
    MD5 = (const char *)pkt.buffs[5].second;
    uint8_t *t_data = (uint8_t *)malloc(pst.buff_size);
    memcpy(t_data, pkt.buffs[4].second, pst.buff_size);
//    解密数据
    AES_ctx naes;
    AES_init_ctx(&naes, key.GetKey());
    AES_CBC_decrypt_buffer(&naes, t_data, pst.buff_size);
    MD5EncryptEasy(MD5, (Byte *)t_data, pst.buff_size);
//    校验数据
    if(TMD5 != MD5) throw "key error";
    pst.buff = (Byte *)t_data;
}

void Client::SetAESKey(aes_key256 &key){
    post_key = key;
}

void request::Json2Data(){
    Writer<StringBuffer> writer(doc_str);
    req_doc.Accept(writer);
    data = doc_str.GetString();
}

void request::JsonParse(string data_from){
    if(req_doc.Parse(data_from.data()).HasParseError()) throw "fail to parse into json";
}

void net_box::set(void *pbuff, uint16_t pbsize){
    data = (UByte *)malloc(pbsize);
    data_size = pbsize;
    memcpy(data, pbuff, pbsize);
}

void net_box::build(void){
    send_data = (UByte *) malloc(data_size+2*sizeof(uint32_t)+3*sizeof(uint16_t)+sizeof(uint64_t));
    UByte *tidx = send_data;
    memcpy(&head, "NETB", sizeof(uint32_t));
    memcpy(&tail, "BTEN", sizeof(uint32_t));

    memcpy(tidx, &head, sizeof(uint32_t));
    tidx += sizeof(uint32_t);
    memcpy(tidx, &this->b_id, sizeof(uint64_t));
    tidx += sizeof(uint64_t);
    memcpy(tidx, &this->idx, sizeof(uint16_t));
    tidx += sizeof(uint16_t);
    memcpy(tidx, &this->cnt, sizeof(uint16_t));
    tidx += sizeof(uint16_t);
    memcpy(tidx, &this->data_size, sizeof(uint16_t));
    tidx += sizeof(uint16_t);
    memcpy(tidx, data, data_size);
    tidx += data_size;
    memcpy(tidx, &tail, sizeof(uint32_t));
    sdt_size = data_size+2*sizeof(uint32_t)+3*sizeof(uint16_t)+sizeof(uint64_t);
}

void Server::ProcessNetBox(net_box &tnb, Byte *p_data){
    Byte *idx = p_data;
    memcpy(&tnb.head, idx, sizeof(uint32_t));
    idx += sizeof(uint32_t);
    memcpy(&tnb.b_id, idx, sizeof(uint64_t));
    idx += sizeof(uint64_t);
    memcpy(&tnb.idx, idx, sizeof(uint16_t));
    idx += sizeof(uint16_t);
    memcpy(&tnb.cnt, idx, sizeof(uint16_t));
    idx += sizeof(uint16_t);
    memcpy(&tnb.data_size, idx, sizeof(uint16_t));
    idx += sizeof(uint16_t);
    tnb.data = (UByte *) malloc(tnb.data_size);
    memcpy(tnb.data, idx, tnb.data_size);
    idx += tnb.data_size;
    memcpy(&tnb.tail, idx, sizeof(uint32_t));
}

net_box::net_box(){
    
}

raw_data::raw_data(){
   r_id = rand64();
}

void box_listener::TogtRawData(raw_data &trdt){
    uint32_t msg_size = 0;
    for(int i = 0; i < cnt; i++){
        net_box *pnb = boxs[i];
        msg_size += pnb->data_size;
    }
    Byte *pbyt = (Byte *)malloc(msg_size);
    Byte *idx = pbyt;
    for(int i = 0; i < cnt; i++){
        net_box *pnb = boxs[i];
        memcpy(idx, pnb->data, pnb->data_size);
        idx += (uint64_t)pnb->data_size;
    }
    if(Server::CheckRawMsg(pbyt, msg_size)){
        Server::ProcessSignedRawMsg(pbyt, msg_size, trdt);
    }
    free(pbyt);
}
