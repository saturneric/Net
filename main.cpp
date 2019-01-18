//
//  net.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "type.h"
#include "memory.h"
#include "clock.h"
#include "net.h"
#include "cpart.h"
#include "cmap.h"
#include "cthread.h"



int main(void){
    initClock();
    Server srvr(9048);
    //srvr.Deamon();
    vector<int> fargs = {1,0,0,1};
    vector<void *>args;
    CPart::addArg<double>(&args, 12.63);
    CPart::addArg<int>(&args, 10);
    CPart::addArg<int>(&args, 6);
    CPart::addArg<double>(&args, 8.2);
//    输入过程
   /* struct compute_result cpur = {"Test",&args,&args,&fargs,&fargs};
    packet pkt =  srvr.CPURS2Packet(cpur);
    raw_data rwd =  srvr.Packet2Rawdata(pkt);*/
    
//    输出过程
    //srvr.Rawdata2Packet(rwd);
    //srvr.Deamon(NULL);
    while(1){
        sleep(100);
    }
    return 0;
}

void CPMT(void){
    CMap map("./PCS");
    CThread thread(&map);
    thread.Analyse();
    thread.DoLine();
    thread.SetDaemon();
    thread.CancelChildPCS(0);
}


