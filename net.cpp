//
//  net.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "net.hpp"

void process(void *arg){
    
}

int main(void){
    char ip[] = "127.0.0.1";
    Socket server("127.0.0.1",9048,true,false);
    printf("Start to listen\n");
    Addr t_addr;
    string str = server.PacketRecv(t_addr);
    if(str == "request token"){
        
    }
    return 0;
}


