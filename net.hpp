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
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

using std::string;

class Socket{
public:
    struct sockaddr_in c_addr, addr;
    int nsfd,sfd,port;
    bool server;
    void (*func)(class Socket &,int ,struct sockaddr_in);
    Socket(string ip_addr, int port, bool server = false, bool tcp = true, bool ipv4 = true){
        memset(&addr, 0, sizeof(struct sockaddr_in));
        if(ipv4)
            addr.sin_family = AF_INET;
        else
            addr.sin_family = AF_INET6;
        addr.sin_port = htons(port);
        this->port = port;
        addr.sin_addr.s_addr = inet_addr(ip_addr.data());
        this->server = server;
        if(server){
            if(ipv4) sfd = socket(AF_INET,SOCK_STREAM,0);
            else sfd = socket(AF_INET6,SOCK_STREAM,0);
            if(!~sfd) throw "fail to get sfd";
            
            if(!~bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr))) throw "fail to bind";
        }
        else{
            if(ipv4) sfd = socket(PF_INET,SOCK_STREAM,0);
            else sfd = socket(PF_INET6,SOCK_STREAM,0);
            if(!~connect(sfd,(struct sockaddr *)&addr,sizeof(struct sockaddr)))
                throw "connection fail";
            
        }
    }
    
    ~Socket(){
        close(sfd);
    }
    
    void Listen(int connection, void (*func)(class Socket &,int ,struct sockaddr_in) = NULL){
        if(server){
            listen(sfd, 10);
            this->func = func;
        }
    }
    
    void Accept(void){
        if(server){
            socklen_t scaddr = sizeof(struct sockaddr);
            nsfd = accept(sfd,(struct sockaddr *) &c_addr, &scaddr);
            if(~nsfd){
                if(func != NULL) func(*this,nsfd,c_addr);
                close(nsfd);
            }
        }
    }
    
    
    
    void Send(int t_nsfd, string buff){
        ssize_t len = send(t_nsfd,buff.data(),buff.size(),0);
        if(len != buff.size()) throw "size unmatch";
    }
    
    string Recv(int t_nsfd){
        char buff[BUFSIZ];
        ssize_t len=recv(t_nsfd,buff,BUFSIZ,0);
        if(len > 0){
            buff[len] = '\0';
            string str = buff;
            return str;
        }
        else throw "receive fail";
        
    }
};

#endif /* net_hpp */
