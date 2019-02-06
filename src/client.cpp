//
//  main.cpp
//  Netc
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "instruct.h"


int main(int argc, char *argv[])
{
    try {
        Server BServer(9050,"127.0.0.1",9048);
        
        while (1) {
            request nreq;
            nreq.type = "client-square request";
            nreq.data = "request for public key";
            nreq.port = 9050;
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
            Addr taddr;
            char *buff = nullptr;
            if(BServer.socket.RecvRAW(&buff, taddr) > 0){
                printf("Receive: %s\n",buff);
                free(buff);
            }
            
            
            
        }
        
    } catch (char const *str) {
        printf("%s\n",str);
        return 0;
    }
    
}
