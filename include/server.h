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
#include "sqlite3.h"
#include "rsa.h"
#include "rng.h"

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
//    匹配id
    rng::rng64 r_id;
//    类型
    string type;
//    数据
    string data;
//    接收端口
    uint32_t recv_port;
//    标记是否为加密请求
    bool if_encrypt;
    Addr t_addr;
    request();
};

//加密端对端报文
struct encrypt_post{
//    明文部分
//    注册客户端id
    rng::rng64 client_id;
//    目标ip
    string ip;
//    目标端口
    int port;
    
//    加密部分
//    匹配id
    rng::rng64 p_id;
//    类型
    uint32_t type;
//    内容
    Byte *buff;
};

//回复数据包
struct respond {
    rng::rng64 r_id;
    string type;
    Byte *buff = nullptr;
    uint32_t buff_size;
    Addr t_addr;
    void SetBuff(Byte *buff, uint32_t size);
    ~respond();
};

//通用数据包类
class packet{
public:
//    数据包类型
    unsigned int type;
    struct sockaddr_in address;
//    记录块的大小及内容所在的内存地址
    vector<pair<unsigned int, void *>> buffs;
    void AddBuff(void *pbuff, uint32_t size);
    bool if_encrypt = false;
    ~packet();
};

//注册客户端管理
struct client_register{
//    客户端id
    rng::rng64 client_id;
//    通信密钥
    rng::rng128 key;
    
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
//    来源ip地址
    struct sockaddr_in address;
//    用简单字符串直接出适合
    void setData(string str){
        data = (char *)malloc(str.size());
        size = str.size();
        memcpy(data, str.data(),str.size());
    }
};

//请求监听管理结构
struct request_listener{
    void (*callback)(respond *,void *args);
    request *p_req;
    uint32_t timeout;
    uint32_t clicks;
    raw_data trwd;
    bool active;
    void *args;
    ~request_listener();
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
//    输出的数据包列表
    list<packet *> packets_out;
    struct server_info tsi;
    sqlite3 *psql;
//    服务器公私钥
    public_key_class pkc;
    private_key_class prc;
public:
//    服务器类的接收套接字对象与发送套接字对象
    SocketUDPServer socket;
    SocketUDPClient send_socket;
    int packet_max = 1024;
    Server(int port = 9048, string send_ip = "127.0.0.1",int send_port = 9049);
    
//    重新设置服务器的发送端口
    void SetSendPort(int port);
//    重新设置服务器的发送IP地址
    void SetSendIP(string ip_addr);
//    将结构数据包转换成二进制串
    static void Packet2Rawdata(packet &tpkt, raw_data &rdt);
//    将通用二进制串转换为通用数据包
    static void Rawdata2Packet(packet &tpkt, raw_data &trdt);
//    释放二进制串占用的空间
    static void freeRawdataServer(struct raw_data &trdt);
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
//    解码已加密的原始二进制串
    void DecryptRSARawMsg(raw_data &rdt, private_key_class &pkc);
//    编码原始二进制串
    void EncryptRSARawMsg(raw_data &rdt, public_key_class &pkc);
//    服务器守护线程
    friend void *serverDeamon(void *psvr);
//    处理RawData
    void ProcessRawData(void);
    void ProcessSendPackets(void);
    
    
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
    list<request *> req_list;
//    注册客户端管理
    list<client_register *> client_lst;
//    加密端对端报文
    list<encrypt_post *>post_lst;
public:
    SQEServer(int port = 9048);
    void ProcessPacket(void);
    void ProcessRequset(void);
    static void Packet2Request(packet &pkt, request &req);
    static void Request2Packet(packet &pkt, request &req);
    static void Respond2Packet(packet &pkt, respond &res);
    static void Packet2Respond(packet &pkt, respond &res);
    
    static void Post2Packet(packet &pkt, encrypt_post &pst, rng::rng128 key);
    static void Packet2Post(packet &pkt, encrypt_post &pst, rng::rng128 key);
};

class Client{
//    请求监听列表
    list<request_listener *> req_lst;
//    回复处理列表
    list<respond *> res_lst;
//    请求监听端口
    uint32_t listen_port;
    SocketUDPServer socket;
    SocketUDPClient send_socket;
//    广场服务器通信公钥
    public_key_class sqe_pbc;
public:
//    构造函数(send_port指的是发送的目标端口)
    Client(int port = 9050, string send_ip = "127.0.0.1",int send_port = 9049);
//    处理请求监听
    void ProcessRequestListener(void);
//    新的请求
    void NewRequest(request **ppreq,string send_ip,int send_port,string type, string data);
//    新的请求监听
    void NewRequestListener(request *preq, int timeout, void *args, void (*callback)(respond *, void *));
//    设置公钥
    void SetPublicKey(public_key_class &t_pbc);
//    友元回复接受守护进程
    friend void *clientRespondDeamon(void *);
};

//设置服务器守护线程的时钟
void setServerClock(Server *psvr, int clicks);
//设置广场服务器守护线程的时钟
void setServerClockForSquare(SQEServer *psvr, int clicks);
//服务器接收数据包守护线程
void *serverDeamon(void *psvr);
//服务器处理原始数据守护线程
void *dataProcessorDeamon(void *pvcti);
//广场服务器处理数据包守护线程
void *packetProcessorDeamonForSquare(void *pvcti);
//广场服务器处理请求守护线程
void *requestProcessorDeamonForSquare(void *pvcti);
//服务器发送数据包守护线程
void *sendPacketProcessorDeamonForSquare(void *pvcti);


//设置客户端请求监听守护时钟
void setClientClock(Client *pclient,int clicks);
//客户端请求监听守护线程
void *clientRequestDeamon(void *pvclt);
//客户端回复接收守护线程
void *clientRespondDeamon(void *pvclt);
//客户端待机守护线程
void *clientWaitDeamon(void *pvclt);

#endif /* server_h */
