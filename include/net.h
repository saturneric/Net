//
//  net.h
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef net_h
#define net_h

#include "type.h"

//IP地址管理类
class Addr{
//    IP地址管理结构
    struct sockaddr_in address;
//    IP地址管理结构的大小
    socklen_t addr_size = 0;
public:
    Addr(string ip_addr, int port = 0, bool ipv4 = true);
    Addr(struct sockaddr_in saddri);
    Addr();
    Addr(const Addr &t_addr);
//    获得记录IP地址管理结构的大小的变量本身
    socklen_t *SizeP(void);
//    获得IP地址管理结构大小
    socklen_t Size(void);
//    重新设置IP地址管理结构所对应的端口
    void SetPort(int port);
//    重新设置IP地址管理结构所对应的IP地址
    void SetIP(string ip_addr);
//    IP地址管理结构的大小变量
    void SetSize(void);
//    获得指向IP地址管理结构的指针
    struct sockaddr_in *Obj(void);
//    获得指向IP地址管理结构的指针
    struct sockaddr *RawObj(void);
    void SetIpv4(void);
    void SetIpv6(void);
    static bool checkValidIP(string ipaddr);

};

//服务器套接字类
class SocketServer{
protected:
//    套接字操作柄
    int server_sfd;
//    服务器IP及端口管理类
    Addr server_addr;
//    传输协议参数
    int ipptl;
//    临时缓冲区
    char buff[BUFSIZ];
public :
    SocketServer(int port,bool ipv4){
        server_addr.SetPort(port);
        if(ipv4){
            ipptl = AF_INET;
            server_addr.SetIpv4();
        }
        else{
            ipptl = AF_INET6;
            server_addr.SetIpv6();
        }
    }
    ~SocketServer(){
        close(server_sfd);
    }
//    接受储存简单字符串
    virtual ssize_t Recv(string &str) = 0;
//    接受储存二进制串
    virtual ssize_t RecvRAW(char **p_rdt) = 0;
};

//客户端套接字类
class SocketClient{
protected:
//    目标服务器IP地址及端口管理类
    Addr send_addr;
//    套接字操作柄
    int client_sfd;
//    传输协议参数
    int ipptl;
//    临时缓冲区
    char buff[BUFSIZ];
public :
    SocketClient(string ip,int port,bool ipv4){
        send_addr.SetIP(ip);
        send_addr.SetPort(port);
        if(ipv4){
            ipptl = PF_INET;
            send_addr.SetIpv4();
        }
        else{
            ipptl = PF_INET6;
            send_addr.SetIpv6();
        }
    }
    ~SocketClient(){
        close(client_sfd);
    }
    //    接受储存简单字符串
    virtual void Send(string buff) = 0;
    //    接受储存二进制串
    virtual void SendRAW(char *buff, unsigned long size) = 0;
    //    重新设置发送目的地的端口
    void SetSendPort(int port);
    //    重新设置发送目的地的IP地址
    void SetSendIP(string ip);
};








//TCP服务器套接字类
class SocketTCPCServer:public SocketServer{
//    连接操作柄
    int ctn_sfd;
    void (*func)(class Socket &,int ,Addr);
public:
    SocketTCPCServer(int port):SocketServer(port,true){
//        获得套接字操作柄
        server_sfd = socket(ipptl,SOCK_STREAM, 0);
        if(!~server_sfd) throw "fail to get server sfd";
//        绑定IP地址与端口
        if(!~bind(server_sfd, server_addr.RawObj(), server_addr.Size())) throw "fail to bind";
    }
//    监听端口
    void Listen(int connection, void (*func)(class Socket &,int ,Addr) = NULL);
//    接受连接
    void Accept(void);
//    接收简单字符串数据
    ssize_t Recv(string &str);
};

//TCP客户端套接字类
class SocketTCPClient:public SocketClient{
//    连接操作柄
    int ctn_sfd;
public:
    SocketTCPClient(string ip,int port):SocketClient(ip,port,true){
//        获得套接字操作柄
        client_sfd = socket(ipptl,SOCK_STREAM,0);
        if(!~client_sfd) throw "fail to get client sfd";
//        建立TCP连接
        if(!~connect(client_sfd,send_addr.RawObj(),send_addr.Size())) throw "fail to connect";
    }
//    发送简单字符串数据
    void Send(string str);
};





//UDP服务端套接字类
class SocketUDPServer:public SocketServer{
//    是否设置非阻塞
    bool set_fcntl = false;
public:
    SocketUDPServer(int port):SocketServer(port,true){
//        获得套接字操作柄
        server_sfd = socket(ipptl,SOCK_DGRAM,0);
        if(!~server_sfd) throw "fail to get server sfd";
//        绑定IP地址与端口
        if(!~bind(server_sfd, server_addr.RawObj(), server_addr.Size())) throw "fail to bind";
    }
//    接受储存简单字符串信息的数据包
    ssize_t Recv(string &str);
//    接受储存二进制信息的数据包
    ssize_t RecvRAW(char **p_rdt);
//    设置非阻塞模式
    void UDPSetFCNTL(void);
};

//UDP客户端套接字类
class SocketUDPClient:public SocketClient{
public:
    SocketUDPClient(string ip,int port):SocketClient(ip,port,true){
//        获得套接字操作柄
        client_sfd = socket(ipptl,SOCK_DGRAM,0);
        if(!~client_sfd) throw "fail to get client sfd";
    }
//    发送简单字符串数据
    void Send(string buff);
//    发送一个带有二进制原始信息的数据包
    void SendRAW(char *buff, unsigned long size);
};

#endif /* net_h */
