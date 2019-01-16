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
        Server client("127.0.0.1",9049,"127.0.0.1",9048);
        while (1) {
            raw_data trdt;
            trdt.setData("Hello");
            client.SignedRawdata(&trdt, "RSTR");
            client.SentRawdata(&trdt);
            usleep(5000);
        }
        
    } catch (char const *str) {
        printf("%s\n",str);
        return 0;
    }
    
}
