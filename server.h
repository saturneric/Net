//
//  server.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/16.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef server_h
#define server_h

#include "net.h"

class Server;

static list<Server *> server_list;

//外来数据包解析结构
struct compute_result{
    string name;
    vector<void *> *args_in;
    vector<void *> *args_out;
};

//原始数据包
struct packet{
    unsigned int type;
    vector<pair<unsigned int, void *>> buffs;
};

class Server{
    vector<compute_result> cpurs;
    vector<packet> packets;
    Socket socket;
    Server(string ip_addr):socket(ip_addr,9048,true,false){
    }
    void Deamon(void){
        socket.PacketRecv(<#Addr t_addr#>)
    }
};

#endif /* server_h */
