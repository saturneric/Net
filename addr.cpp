//
//  addr.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "net.h"

Addr::Addr(string ip_addr, int port, bool ipv4){
    memset(&address, 0, sizeof(struct sockaddr_in));
    if(ipv4)
        address.sin_family = AF_INET;
    else
        address.sin_family = AF_INET6;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ip_addr.data());
    addr_size = sizeof(address);
}

Addr::Addr(struct sockaddr_in saddri){
    memset(&address, 0, sizeof(struct sockaddr_in));
    address = saddri;
}

Addr::Addr(){
    memset(&address, 0, sizeof(struct sockaddr_in));
}

Addr::Addr(const Addr &t_addr){
    address = t_addr.address;
    addr_size = t_addr.addr_size;
}


socklen_t *Addr::sizep(void){
    return &addr_size;
}

socklen_t Addr::size(void){
    return addr_size;
}

void Addr::SetPort(int port){
    address.sin_port = htons(port);
    addr_size = sizeof(address);
}

void Addr::SetIP(string ip_addr){
    address.sin_addr.s_addr = inet_addr(ip_addr.data());
    addr_size = sizeof(address);
}

struct sockaddr *Addr::obj(void){
    return (struct sockaddr *) &address;
}
