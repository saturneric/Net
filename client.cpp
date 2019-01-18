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
        CNodeServer client(9049,"127.0.0.1",9048);
        vector<int> fargs = {1,0,0,1};
        vector<void *>args;
        CPart::addArg<double>(&args, 12.63);
        CPart::addArg<int>(&args, 10);
        CPart::addArg<int>(&args, 6);
        CPart::addArg<double>(&args, 8.2);
        struct compute_result cpur = {"Test",&args,&args,&fargs,&fargs};
        packet pkt =  client.CPURS2Packet(cpur);
        raw_data rwd =  client.Packet2Rawdata(pkt);
        client.SignedRawdata(&rwd, "RSTR");
        while (1) {
            client.SentRawdata(&rwd);
            usleep(1000);
        }
        
    } catch (char const *str) {
        printf("%s\n",str);
        return 0;
    }
    
}
