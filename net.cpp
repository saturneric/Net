//
//  net.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "net.hpp"

void process(class Socket &server,int nsfd,struct sockaddr_in c_addr){
    server.Send(nsfd, "ORDER");
}

int main(void){
    char ip[] = "127.0.0.1";
    Socket server("127.0.0.1",9048,true);
    server.Listen(10,process);
    printf("Start to listen\n");
    server.Accept();
    return 0;
}


