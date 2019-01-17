//
//  server.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/16.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef server_h
#define server_h

#include "clock.h"
#include "net.h"
#include "cpart.h"
#include "cthread.h"

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

//通用服务器类
class Server{
public:
    vector<compute_result> cpurs_in;
    vector<packet> packets_in;
    vector<raw_data> rawdata_in;
//    服务器类的接收套接字对象与发送套接字对象
    Socket socket, send_socket;
    int packet_max = 30;
    Server(string ip_addr, int port = 9048, string send_ip_addr = "127.0.0.1",int send_port = 9049);
    
//    重新设置服务器的发送端口
    void SetSendPort(int port);
    
//    重新设置服务器的发送IP地址
    void SetSendIP(string ip_addr);
    
//    将计算结果包转化为结构数据包
    static packet CPURS2Packet(compute_result tcpur);
    
//    将结构数据包转换成二进制串
    static raw_data Packet2Rawdata(packet tpkt);
//    将通用二进制串转换为通用数据包
    static packet Rawdata2Packet(raw_data trdta);
    
//    将结构数据包转化为计算结果包
    static compute_result Packet2CPUR(packet *tpkt);
    
//    释放二进制串占用的空间
    static void freeRawdataServer(struct raw_data trdt);
//    释放通用数据包包占用
    static void freePcaketServer(struct packet tpkt);
//    释放计算结果包占用的空间
    static void freeCPURServer(struct compute_result tcpur);
    
//    给二进制串贴上识别标签
    static void SignedRawdata(struct raw_data *trdt,string info);
    
//    发送已经贴上标签的二进制串
    int SentRawdata(struct raw_data *trdt);
    
//    检查消息串是否为一个贴上标签的二进制串
    static bool CheckRawMsg(char *p_rdt, ssize_t size);
    
//    处理一个已贴上标签的原始二进制串，获得其包含的信息
    static raw_data ProcessSignedRawMsg(char *p_rdt, ssize_t size);
    
    
    
};

//设置服务器守护程序的时钟
void setServerClock(Server *psvr, int clicks);
//服务器守护线程
void *serverDeamon(void *psvr);

#endif /* server_h */
