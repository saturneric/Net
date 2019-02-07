//
//  main.cpp
//  Netc
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "instruct.h"

void getSQEPublicKey(respond *pres);

int main(int argc, char *argv[])
{
    try {
        Client nclt(9050);
        request *preq;
        initClock();
        setThreadsClock();
        setClientClock(&nclt,2);
        nclt.NewRequest(&preq, "127.0.0.1", 9048, "client-square request", "request for public key");
        nclt.NewRequestListener(preq, 10, getSQEPublicKey);
        while (1) {
            sleep(10);
        }
        
    } catch (char const *str) {
        printf("%s\n",str);
        return 0;
    }
    
}

void getSQEPublicKey(respond *pres){
    printf("Get Respond.\n");
}
