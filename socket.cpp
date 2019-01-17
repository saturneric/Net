//
//  socket.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "net.h"

void SocketClient::SetSendPort(int port){
    send_addr.SetPort(port);
}

void SocketClient::SetSendIP(string ip){
    send_addr.SetIP(ip);
}

ssize_t SocketTCPCServer::Recv(string &str){
    ssize_t len=recv(ctn_sfd,buff,BUFSIZ,0);
    if(len > 0){
        buff[len] = '\0';
        string str = buff;
        return len;
    }
    else return -1;
}

void SocketTCPCServer::Listen(int connection, void (*func)(class Socket &,int ,Addr)){
    listen(server_sfd, 10);
    this->func = func;
}

void SocketTCPClient::Send(string str){
    ssize_t len = send(ctn_sfd,str.data(),str.size(),0);
    if(len != str.size()) throw "size unmatch";
}

ssize_t SocketUDPServer::Recv(string &str){
    ssize_t tlen;
//    非阻塞接收
    if(set_fcntl){
        tlen = recvfrom(server_sfd, buff, BUFSIZ, 0, server_addr.RawObj(), server_addr.SizeP());
//        读取错误
        if(tlen == -1 && errno != EAGAIN) return -1;
//        缓冲区没有信息
        else if(tlen == 0 || (tlen == -1 && errno == EAGAIN)) return 0;
//        成功读取信息
        else{
            str = buff;
            buff[tlen] = '\0';
            return 1;
        }
    }
//    阻塞接收
    else{
        tlen = recvfrom(server_sfd, buff, BUFSIZ, 0, server_addr.RawObj(), server_addr.SizeP());
        if(~tlen){
            str = buff;
            buff[tlen] = '\0';
            return 1;
        }
        else return -1;
    }
}

ssize_t SocketUDPServer::RecvRAW(char **p_rdt){
    ssize_t tlen;
    //            非阻塞输入
    if(set_fcntl){
        tlen = recvfrom(server_sfd, buff, BUFSIZ, 0, server_addr.RawObj(), server_addr.SizeP());
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
        tlen = recvfrom(server_sfd, buff, BUFSIZ, 0, server_addr.RawObj(), server_addr.SizeP());
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

void SocketUDPServer::UDPSetFCNTL(void){
    int flags = fcntl(server_sfd, F_GETFL, 0);
    fcntl(server_sfd, F_SETFL, flags | O_NONBLOCK);
    set_fcntl = true;
}

void SocketUDPClient::Send(string buff){
    sendto(client_sfd, buff.data(), buff.size(), 0, send_addr.RawObj(), send_addr.Size());
}

void SocketUDPClient::SendRAW(char *buff, unsigned long size){
    sendto(client_sfd, buff, size, 0, send_addr.RawObj(), send_addr.Size());
}
