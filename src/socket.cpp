//
//  socket.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "memory.h"
#include "net.h"

void SocketClient::SetSendPort(int port){
    send_addr.SetPort(port);
}

void SocketClient::SetSendIP(string ip){
    send_addr.SetIP(ip);
}

ssize_t SocketTCPCServer::Recv(string &str){
    ssize_t bdtas = 0 ,tmp_bdtas;
    while ((tmp_bdtas = read(data_sfd, buff, BUFSIZ)) > 0) {
        str += string(buff,tmp_bdtas);
        bdtas += tmp_bdtas;
    }
    return bdtas;
}

int SocketTCPCServer::Listen(void){
    return listen(server_sfd, 10);
}

void SocketTCPClient::Send(string str){
    ssize_t len = send(client_sfd,str.data(),str.size(),0);
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

ssize_t SocketUDPServer::RecvRAW(char **p_rdt, Addr &taddr){
    ssize_t tlen;
    sockaddr_in tsai;
    socklen_t tsai_size = sizeof(sockaddr);
//    非阻塞读取
    if(set_fcntl){
        tlen = recvfrom(server_sfd, buff, BUFSIZ, 0, (struct sockaddr *)(&tsai), &tsai_size);
        
        //读取错误
        if(tlen == -1 && errno != EAGAIN){
            *p_rdt = nullptr;
            printf("%d",errno);
            perror("recv");
            return -1;
        }
		//缓冲区没有信息
        else if(tlen == 0 || (tlen == -1 && errno == EAGAIN)){
            *p_rdt = nullptr;
            return 0;
        }
		//成功读取信息
        else{
            *p_rdt = (char *)malloc(tlen);
            taddr.SetSockAddr(tsai);
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

ssize_t SocketUDPClient::SendRAW(char *buff, unsigned long size){
    return sendto(client_sfd, buff, size, 0, send_addr.RawObj(), send_addr.Size());
}

ssize_t SocketTCPClient::SendRAW(char *buff, unsigned long size){
    ssize_t send_size = send(client_sfd, buff, size, 0);
    /*if(send_size < 0){
        printf("Error[%u]:",errno);
        perror("send");
    }*/
    return send_size;
}

void SocketClient::SetSendSockAddr(struct sockaddr_in tsi){
    send_addr.SetSockAddr(tsi);
}

void SocketTCPClient::Close(void){
    close(client_sfd);
}

ssize_t SocketTCPCServer::RecvRAW_SM(char **p_rdt, Addr &taddr){
    ssize_t tmp_bdtas = recv(data_sfd, buff, BUFSIZ, 0);
    if (tmp_bdtas > 0) {
        *p_rdt = (char *)malloc(tmp_bdtas);
        memcpy(*p_rdt, buff, tmp_bdtas);
    }
    return tmp_bdtas;
}

ssize_t SocketTCPCServer::RecvRAW(char **p_rdt, Addr &taddr){
    ssize_t bdtas = 0 ,tmp_bdtas;
    *p_rdt = nullptr;
    while ((tmp_bdtas = recv(data_sfd, buff, BUFSIZ, 0)) > 0) {
        if(*p_rdt == nullptr){
            *p_rdt = (char *)malloc(tmp_bdtas);
            memcpy(*p_rdt, buff, tmp_bdtas);
        }
        else{
            *p_rdt = (char *)realloc(*p_rdt, bdtas + tmp_bdtas);
            memcpy(*p_rdt + bdtas, buff, tmp_bdtas);
        }
        bdtas += tmp_bdtas;
    }
    return bdtas;
}

void SocketTCPCServer::Accept(void ){
    data_sfd = accept(server_sfd, client_addr.RawObj(), client_addr.SizeP());
}


void SocketTCPClient::Reconnect(void){
    client_sfd = socket(ipptl,SOCK_STREAM,0);
    if(!~client_sfd) throw "fail to get client sfd";
    if(!~connect(client_sfd,send_addr.RawObj(),send_addr.Size())) throw "fail to connect";
}


void SocketTCPCServer::CloseConnection(void){
    close(data_sfd);
}
void SocketTCPCServer::Close(void) {
	close(server_sfd);
}


void SocketTCPClient::GetRespond(string &str){
    ssize_t size = recv(client_sfd, buff, BUFSIZ, 0);
    if(size > 0){
        str = string(buff,size);
    }
    else str = "";
}

void SocketTCPCServer::SendRespond(string &str){
    send(data_sfd, str.data(), str.size(), 0);
}

ssize_t SocketTCPClient::RecvRAW(char **p_rdt, Addr &taddr){
	ssize_t tmp_bdtas = recv(client_sfd, buff, BUFSIZ, 0);
	if (tmp_bdtas > 0) {
		*p_rdt = (char *)malloc(tmp_bdtas);
		memcpy(*p_rdt, buff, tmp_bdtas);
	}
	return tmp_bdtas;
}


Addr &SocketTCPClient::GetAddr(void){
    return send_addr;
}

void SocketTCPClient::SetAddr(Addr &taddr){
    send_addr = taddr;
}

Addr &SocketTCPCServer::GetAddr(void){
    return server_addr;
}
Addr &SocketTCPCServer::GetClientAddr(void){
    return client_addr;
}


int SocketTCPCServer::GetDataSFD(void){
    return data_sfd;
}

void SocketTCPCServer::SetDataSFD(int tdata_sfd){
    data_sfd = tdata_sfd;
}

void SocketTCPCServer::SetClientAddr(Addr &caddr){
    client_addr = caddr;
}
