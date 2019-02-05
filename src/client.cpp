//
//  main.cpp
//  Netc
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "type.h"
#include "sql.h"
#include "net.h"
#include "server.h"
#include "rng.h"

int main(int argc, char *argv[])
{
    try {
        Server BServer(1081,"127.0.0.1",9048);
        
        while (1) {
            request nreq;
            nreq.type = "client-square request";
            nreq.data = "request for public key";
            packet *pnpkt = new packet();
            SQEServer::Request2Packet(*pnpkt, nreq);
            raw_data *pnrwd = new raw_data();
            Server::Packet2Rawdata(*pnpkt, *pnrwd);
            BServer.SignedRawdata(pnrwd, "SPKT");
            BServer.SentRawdata(pnrwd);
            Server::freeRawdataServer(*pnrwd);
            delete pnrwd;
            Server::freePcaketServer(*pnpkt);
            delete pnpkt;
            
        }
        
    } catch (char const *str) {
        printf("%s\n",str);
        return 0;
    }
    
}
