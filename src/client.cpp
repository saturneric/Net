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
        Server BServer(1081,"127.0.0.1",9048);
        vector<int> fargs = {1,0,0,1};
        vector<void *>args;
        raw_data rwd;
        rwd.setData("Hello");
        BServer.SignedRawdata(&rwd, "TEXT");
        while (1) {
            BServer.SentRawdata(&rwd);
            usleep(1000);
        }
        
    } catch (char const *str) {
        printf("%s\n",str);
        return 0;
    }
    
}
