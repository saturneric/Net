//
//  net.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef net_hpp
#define net_hpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <string>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include "cpart.h"

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::list;
using std::ifstream;
using std::cout;
using std::endl;


class Addr{
public:
    struct sockaddr_in address;
    socklen_t len;
    Addr(string ip_addr, int port = 0, bool ipv4 = true){
        memset(&address, 0, sizeof(struct sockaddr_in));
        if(ipv4)
            address.sin_family = AF_INET;
        else
            address.sin_family = AF_INET6;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = inet_addr(ip_addr.data());
        len = sizeof(address);
    }
    Addr(struct sockaddr_in saddri){
        memset(&address, 0, sizeof(struct sockaddr_in));
        address = saddri;
    }
    Addr(){
        memset(&address, 0, sizeof(struct sockaddr_in));
    }
    Addr(const Addr &t_addr){
        address = t_addr.address;
        len = t_addr.len;
    }
    socklen_t *sizep(void){
        return &len;
    }
    socklen_t size(void){
        return len;
    }
    void setSize(void){
        len = sizeof(address);
    }
    struct sockaddr *obj(void){
        return (struct sockaddr *) &address;
    }
};

class Socket{
public:
    struct sockaddr_in c_addr;
    Addr addr;
    int nsfd,sfd,port;
    bool server,tcp,ipv4,set_fcntl = false;
    void (*func)(class Socket &,int ,Addr);
    Socket(string ip_addr, int port, bool server = false, bool tcp = true, bool ipv4 = true){
        if(ipv4)
            addr.address.sin_family = AF_INET;
        else
            addr.address.sin_family = AF_INET6;
        addr.address.sin_port = htons(port);
        this->port = port;
        addr.address.sin_addr.s_addr = inet_addr(ip_addr.data());
        addr.setSize();
        this->server = server;
        this->tcp = tcp;
        this->ipv4 = ipv4;
        int TAU = SOCK_STREAM;
        if(!tcp) TAU = SOCK_DGRAM;
        //如果是服务端
        if(server){
            if(ipv4) sfd = socket(AF_INET,TAU,0);
            else sfd = socket(AF_INET6,TAU,0);
            if(!~sfd) throw "fail to get sfd";
            if(!~bind(sfd, addr.obj(), addr.size())) throw "fail to bind";
        }
        else{
            if(ipv4) sfd = socket(PF_INET,TAU,0);
            else sfd = socket(PF_INET6,TAU,0);
            if(tcp && !~connect(sfd,addr.obj(),addr.size()))
                throw "connection fail";
            
        }
    }
    
    ~Socket(){
        close(sfd);
    }
    
    void Listen(int connection, void (*func)(class Socket &,int ,Addr) = NULL){
        if(server && tcp){
            listen(sfd, 10);
            this->func = func;
        }
    }
    
    void Accept(void){
        if(server && tcp){
            socklen_t scaddr = sizeof(struct sockaddr);
            nsfd = accept(sfd,(struct sockaddr *) &c_addr, &scaddr);
            Addr addr(c_addr);
            if(~nsfd){
                if(func != NULL) func(*this,nsfd,addr);
                close(nsfd);
            }
        }
    }
    
    void Send(int t_nsfd, string buff){
        if(tcp){
            ssize_t len = send(t_nsfd,buff.data(),buff.size(),0);
            if(len != buff.size()) throw "size unmatch";
        }
    }
    
    void PacketSend(string buff){
        if(!tcp)
            sendto(sfd, buff.data(), buff.size(), 0, addr.obj(), addr.size());
    }
    
//    发送一段二进制信息
    void PacketSendRAW(char *buff, unsigned long size){
        if(!tcp)
            sendto(sfd, buff, size, 0, addr.obj(), addr.size());
    }
    
//    接受储存字符串信息的UDP包
    int PacketRecv(Addr &t_addr, string &str){
        if(!tcp){
            char buff[BUFSIZ];
            ssize_t tlen;
//            非阻塞输入
            if(set_fcntl){
                tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
    //            读取错误
                if(tlen == -1 && errno != EAGAIN){
                    str = "";
                    return -1;
                }
    //            缓冲区没有信息
                else if(tlen == 0 || (tlen == -1 && errno == EAGAIN)){
                    str = "";
                    return 0;
                }
    //            成功读取信息
                else{
                    str = buff;
                    buff[tlen] = '\0';
                    return 1;
                }
            }
            else{
                tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
                if(~tlen){
                    str = buff;
                    buff[tlen] = '\0';
                    return 1;
                }
                else{
                    str = "";
                    return -1;
                }
                
            }
        }
        throw "connection is tcp";
    }
    
//    接受储存二进制信息的UDP包
    ssize_t PacketRecvRAW(Addr &t_addr, char *p_rdt){
        if(!tcp){
            char buff[BUFSIZ];
            ssize_t tlen;
            //            非阻塞输入
            if(set_fcntl){
                tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
                //            读取错误
                if(tlen == -1 && errno != EAGAIN){
                    p_rdt = nullptr;
                    return -1;
                }
                //            缓冲区没有信息
                else if(tlen == 0 || (tlen == -1 && errno == EAGAIN)){
                    p_rdt = nullptr;
                    return 0;
                }
                //            成功读取信息
                else{
                    p_rdt = (char *)malloc(tlen);
                    memcpy(p_rdt, buff, tlen);
                    return tlen;
                }
            }
            else{
                tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
                if(~tlen){
                    p_rdt = (char *)malloc(tlen);
                    memcpy(p_rdt, buff, tlen);
                    return tlen;
                }
                else{
                    p_rdt = nullptr;
                    return -1;
                }
                
            }
        }
        throw "connection is tcp";
    }
    
    unsigned long IfHasPacket(void){
        
        return 0;
    }
    
    void UDPSetFCNTL(void){
        if(!tcp){
            int flags = fcntl(sfd, F_GETFL, 0);
            fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
            set_fcntl = true;
        }
    }
    
    string Recv(int t_nsfd){
        if(tcp){
            char buff[BUFSIZ];
            ssize_t len=recv(t_nsfd,buff,BUFSIZ,0);
            if(len > 0){
                buff[len] = '\0';
                string str = buff;
                return str;
            }
            else throw "receive fail";
        }
        throw "connection is udp";
    }
};

struct pcs_result{
    char *name[16];
    uint32_t in_size;
    char *in_buff;
    uint32_t out_size;
    char *out_buff;
};

//设置全局线程时钟
void setThreadsClock(void);
//时钟滴答调用函数
void threadsClock(int);


#endif /* net_hpp */
