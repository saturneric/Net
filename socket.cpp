//
//  socket.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "net.h"


Socket::Socket(string ip_addr, int port, bool server, bool tcp, bool ipv4){
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

Socket::~Socket(){
    close(sfd);
}

void Socket::Listen(int connection, void (*func)(class Socket &,int ,Addr)){
    if(server && tcp){
        listen(sfd, 10);
        this->func = func;
    }
}

void Socket::Accept(void){
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

void Socket::Send(int t_nsfd, string buff){
    if(tcp){
        ssize_t len = send(t_nsfd,buff.data(),buff.size(),0);
        if(len != buff.size()) throw "size unmatch";
    }
}

void Socket::PacketSend(string buff){
    if(!tcp)
        sendto(sfd, buff.data(), buff.size(), 0, addr.obj(), addr.size());
}

void Socket::SetSendPort(int port){
    if(!server){
        addr.SetPort(port);
    }
}

void Socket::SetSendIP(string ip_addr){
    if(!server){
        addr.SetIP(ip_addr);
    }
}
void Socket::PacketSendRAW(char *buff, unsigned long size){
    if(!tcp)
        sendto(sfd, buff, size, 0, addr.obj(), addr.size());
}

int Socket::PacketRecv(Addr &t_addr, string &str){
    if(!tcp){
        ssize_t tlen;
        //            非阻塞输入
        if(set_fcntl){
            tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
            //                读取错误
            if(tlen == -1 && errno != EAGAIN){
                str = "";
                return -1;
            }
            //                缓冲区没有信息
            else if(tlen == 0 || (tlen == -1 && errno == EAGAIN)){
                str = "";
                return 0;
            }
            //                成功读取信息
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

ssize_t Socket::PacketRecvRAW(Addr &t_addr, char **p_rdt){
    if(!tcp){
        ssize_t tlen;
        //            非阻塞输入
        if(set_fcntl){
            tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
            //            读取错误
            if(tlen == -1 && errno != EAGAIN){
                *p_rdt = nullptr;
                return -1;
            }
            //            缓冲区没有信息
            else if(tlen == 0 || (tlen == -1 && errno == EAGAIN)){
                *p_rdt = nullptr;
                return 0;
            }
            //            成功读取信息
            else{
                *p_rdt = (char *)malloc(tlen);
                memcpy(*p_rdt, buff, tlen);
                return tlen;
            }
        }
        else{
            tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
            if(~tlen){
                *p_rdt = (char *)malloc(tlen);
                memcpy(p_rdt, buff, tlen);
                return tlen;
            }
            else{
                *p_rdt = nullptr;
                return -1;
            }
            
        }
    }
    throw "connection is tcp";
}

void Socket::UDPSetFCNTL(void){
    if(!tcp){
        int flags = fcntl(sfd, F_GETFL, 0);
        fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
        set_fcntl = true;
    }
}

string Socket::Recv(int t_nsfd){
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
