//
//  main.cpp
//  Netc
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//


#include "net.h"
#include "server.h"

int main(int argc, char *argv[])
{
    
    try {
        Socket client("127.0.0.1",9048,false,false);
        while (1) {
            client.PacketSend("Hello");
            usleep(50000);
        }
        
    } catch (char const *str) {
        printf("%s\n",str);
        return 0;
    }
    
}
