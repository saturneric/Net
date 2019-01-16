//
//  server.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/16.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef server_h
#define server_h

#include "net.h"

class Server;

//外来数据包解析结构
struct compute_result{
    string name;
    vector<void *> *args_in;
    vector<void *> *args_out;
    vector<int> *fargs_in;
    vector<int> *fargs_out;
};

struct server_clock{
    Server *psvr;
    int click;
};

//原始数据包
struct packet{
    unsigned int type;
    vector<pair<unsigned int, void *>> buffs;
};

//二进制原始串及其信息标签
struct raw_data{
    char *data = NULL;
    unsigned long size = 0;
    uint32_t head, tail;
    uint32_t info;
    char *msg = NULL;
    unsigned long msg_size = 0;
    
    void setData(string str){
        data = (char *)malloc(str.size()+1);
        size = str.size()+1;
        memcpy(data, str.data(),str.size());
        data[str.size()+1] = '\0';
    }
};

//设置服务器守护程序的时钟
void setServerClock(Server *psvr, int clicks);

class Server{
public:
    vector<compute_result> cpurs_in;
    vector<packet> packets_in;
    vector<string> rawstr_in;
    Socket socket, send_socket;
    int packet_max = 30;
    Server(string ip_addr, int port = 9048, string send_ip_addr = "127.0.0.1",int send_port = 9049):socket(ip_addr,port,true,false),send_socket(send_ip_addr,send_port,false,false){
        socket.UDPSetFCNTL();
    }
    
//    重新设置服务器的发送端口
    void SetSendPort(int port){
        send_socket.SetSendPort(port);
    }
    
//    重新设置服务器的发送IP地址
    void SetSendIP(string ip_addr){
        send_socket.SetSendIP(ip_addr);
    }
    
    void Deamon(void){
        //cout<<"Server Deamon Checked."<<endl;
        Addr f_addr;
        
        int prm = packet_max;
        ssize_t tlen;
        char *str = nullptr;
        printf("Checking Packet.\n");
        do{
            tlen = socket.PacketRecvRAW(f_addr,&str);
            if(tlen > 0){
                cout<<"Get."<<endl;
                if(CheckRawMsg(str, tlen)){
                    cout<<"Signed Raw Data."<<endl;
                    ProcessSignedRawMsg(str, tlen);
                }
                rawstr_in.push_back(str);
            }
            else{
                
            }
        }while (tlen && prm-- > 0);
        setServerClock(this, 2);
    }
//    将计算结果包转化为结构数据包
    packet CPURS2Packet(compute_result tcpur){
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
    
//    将结构数据包转换成原始二进制串
    raw_data Packet2Rawdata(packet tpkt){
        raw_data rdta;
        char *data = (char *)malloc(BUFSIZ);
        memset(data, 0, BUFSIZ);
        rdta.data = data;
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
            memcpy(idx, &(*i).second, (*i).first);
            idx += (*i).first;
        }
        rdta.size = idx - data;
        return rdta;
    }
    
//    将二进制串信息转换为结构数据包
    packet Rawdata2Packet(raw_data trdta){
        packet pkt;
        char *idx = trdta.data;
//        数据包ID
        uint32_t uint;
        memcpy(&pkt.type, idx, sizeof(uint32_t));
        idx += sizeof(uint32_t);
//        数据包主体
        while(idx - trdta.data < trdta.size){
            memcpy(&uint, idx, sizeof(uint32_t));
            idx += sizeof(uint32_t);
            void *data = malloc(uint);
            memcpy(data, idx, uint);
            idx += uint;
            pkt.buffs.push_back({uint,data});
        }
        return pkt;
    }
    
//    将结构数据包转化为计算结果包
    compute_result Packet2CPUR(packet *tpkt){
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
    
    void freeRawdataServer(struct raw_data trdt){
        free(trdt.data);
        if(trdt.msg != NULL) free(trdt.msg);
    }
    
    void freePcaketServer(struct packet tpkt){
        for(auto i = tpkt.buffs.begin(); i != tpkt.buffs.end(); i++)
            free(i->second);
        delete &tpkt.buffs;
    }
    
    void freeCPURServer(struct compute_result tcpur){
//        释放输入参数容器所占用的所有内存
        for(auto i = tcpur.args_in->begin(); i != tcpur.args_in->end(); i++)
            free(*i);
        delete tcpur.args_in;
        
//        释放输出参数容器所占用的所有内存
        for(auto i = tcpur.args_out->begin(); i != tcpur.args_out->end(); i++)
            free(*i);
        delete tcpur.args_out;
    }
    
//    为原始二进制串打上信息标签
    void SignedRawdata(struct raw_data *trdt,string info){
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
    
//    发送已经打上标签的原始二进制串
    int SentRawdata(struct raw_data *trdt){
        send_socket.PacketSendRAW(trdt->msg, trdt->msg_size);
        return 0;
    }
    
//    检查二进制信息是否为一个打上标签的原始二进制串
    bool CheckRawMsg(char *p_rdt, ssize_t size){
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
    
//    处理一个已打上标签的原始二进制串，获得其中储存的信息
    raw_data ProcessSignedRawMsg(char *p_rdt, ssize_t size){
        raw_data trdt;
        trdt.data = (char *)malloc(size-3*sizeof(uint32_t));
        memcpy(&trdt.info, p_rdt+sizeof(uint32_t), sizeof(uint32_t));
        memcpy(trdt.data, p_rdt+sizeof(uint32_t)*2, size-3*sizeof(uint32_t));
        printf("Data:%s\n",trdt.data);
        return trdt;
    }
    
};

#endif /* server_h */
