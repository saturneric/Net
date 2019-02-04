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
#include "rsa.h"

class Server;

//外来数据包解析结构
struct compute_result{
    string name;
    vector<void *> *args_in;
    vector<void *> *args_out;
    vector<int> *fargs_in;
    vector<int> *fargs_out;
};

//请求数据包
struct request {
    uint64_t r_id;
    string type;
    string data;
};

//通用数据包类
class packet{
public:
//    数据包类型
    unsigned int type;
//    记录块的大小及内容所在的内存地址
    vector<pair<unsigned int, void *>> buffs;
};

//带标签的二进制串管理结构
class raw_data{
public:
//    二进制串
    char *data = NULL;
    unsigned long size = 0;
//    标签
    uint32_t head, tail;
    uint32_t info;
//    信息串
    char *msg = NULL;
    unsigned long msg_size = 0;
//    用简单字符串直接出适合
    void setData(string str){
        data = (char *)malloc(str.size());
        size = str.size();
        memcpy(data, str.data(),str.size());
    }
};

struct server_info{
    string tag;
    string name;
    string msqes_ip;
    int msqes_prot;
    string key;
};

//通用服务器类
class Server{
protected:
//    缓存通用数据包
    list<packet *> packets_in;
//    缓存带标签的二进制串管理结构
    list<raw_data *> rawdata_in;
    struct server_info tsi;
    sqlite3 *psql;
public:
//    服务器类的接收套接字对象与发送套接字对象
    SocketUDPServer socket;
    SocketUDPClient send_socket;
    int packet_max = 30;
    Server(int port = 9048, string send_ip = "127.0.0.1",int send_port = 9049);
    
//    重新设置服务器的发送端口
    void SetSendPort(int port);
//    重新设置服务器的发送IP地址
    void SetSendIP(string ip_addr);
//    将结构数据包转换成二进制串
    static void Packet2Rawdata(packet &tpkt, raw_data &rdt);
//    将通用二进制串转换为通用数据包
    static packet Rawdata2Packet(raw_data trdta);
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
    static void ProcessSignedRawMsg(char *p_rdt, ssize_t size, raw_data &rdt);
//    服务器守护线程
    friend void *serverDeamon(void *psvr);
//    处理RawData
    void ProcessRawData(void);
    
    
};

//计算节点服务器类
class CNodeServer:public Server{
    vector<compute_result> cpurs_in;
public:
//    将计算结果包转化为结构数据包
    static packet CPURS2Packet(compute_result tcpur);
//    将结构数据包转化为计算结果包
    static compute_result Packet2CPUR(packet *tpkt);
};

class SQEServer:public Server{
protected:
//    请求数据包
    list<request> req_list;
//    服务器公私钥
    public_key_class pkc;
    private_key_class prc;
public:
    SQEServer(void);
};

//设置服务器守护程序的时钟
void setServerClock(Server *psvr, int clicks);
//服务器守护线程
void *serverDeamon(void *psvr);

#endif /* server_h */
