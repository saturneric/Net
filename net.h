//
//  net.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef net_hpp
#define net_hpp

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
    socklen_t *sizep(void);
//    获得IP地址管理结构大小
    socklen_t size(void);
//    重新设置IP地址管理结构所对应的端口
    void SetPort(int port);
//    重新设置IP地址管理结构所对应的IP地址
    void SetIP(string ip_addr);
//    获得指向IP地址管理结构的指针
    struct sockaddr *obj(void);
};

//套接字管理类
class Socket{
    struct sockaddr_in c_addr;
    Addr addr;
    int nsfd,sfd,port;
//    属性记录变量
    bool server,tcp,ipv4,set_fcntl = false;
//    缓冲区
    char buff[BUFSIZ];
    void (*func)(class Socket &,int ,Addr);
public:
    Socket(string ip_addr, int port, bool server = false, bool tcp = true, bool ipv4 = true);
    ~Socket();
//    TCP连接模式下进行端口监听
    void Listen(int connection, void (*func)(class Socket &,int ,Addr) = NULL);
//    TCP链接模式下接受连接
    void Accept(void);
//    TCP模式下发送简单字符串数据
    void Send(int t_nsfd, string buff);
//    TCP模式下接收简单字符串数据
    string Recv(int t_nsfd);
    
//    UDP模式下发送简单字符串数据
    void PacketSend(string buff);
//    UDP模式下发送一个带有二进制原始信息的数据包
    void PacketSendRAW(char *buff, unsigned long size);
//    UDP模式下接受储存简单字符串信息的数据包
    int PacketRecv(Addr &t_addr, string &str);
//    UDP模式下接受储存二进制信息的数据包
    ssize_t PacketRecvRAW(Addr &t_addr, char **p_rdt);
//    UDP模式下设置非阻塞模式
    void UDPSetFCNTL(void);
    
//    重新设置客户端模式下的发送目的地的端口
    void SetSendPort(int port);
//    重新设置客户端模式下的发送目的地的IP地址
    void SetSendIP(string ip_addr);
    
};


#endif /* net_hpp */
