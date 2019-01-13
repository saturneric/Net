//
//  net.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef net_hpp
#define net_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

using std::string;

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
    bool server,tcp,ipv4;
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
    
    string PacketRecv(Addr t_addr){
        if(!tcp){
            char buff[BUFSIZ];
            ssize_t tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
            if(tlen > 0){
                buff[tlen] = '\0';
                string str = buff;
                return str;
            }
            else throw "packet receive fail";
        }
        throw "connection is tcp";
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

class UProcess{
public:
    Addr addr;
    int token;
    UProcess(Addr t_addr, int token){
        addr = t_addr;
        this->token = token;
    }
};

#endif /* net_hpp */
