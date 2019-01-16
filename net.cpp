//
//  net.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "net.h"
#include "cpart.h"
#include "cmap.h"
#include "cthread.h"

extern list<CThread *> daemon_list;
extern list<server_clock> server_list;
static struct itimerval oitrl, itrl;

void init(void){
    signal(SIGALRM, threadsClock);
    setThreadsClock();
}

//设置全局线程时钟
void setThreadsClock(void){
    itrl.it_interval.tv_sec = 0;
    itrl.it_interval.tv_usec = 500000;
    itrl.it_value.tv_sec = 0;
    itrl.it_value.tv_usec = 500000;
    setitimer(ITIMER_REAL, &itrl, &oitrl);
}
//时钟滴答调用函数
void threadsClock(int n){
    //printf("Clock click.\n");
    for(auto i = daemon_list.begin(); i != daemon_list.end(); i++){
        (*i)->Daemon();
    }
//    服务器守护程序
    for(auto i = server_list.begin(); i != server_list.end(); i++){
        
        if(--(*i).click == 0){
            (*i).psvr->Deamon();
        }
    }
    for(auto i = server_list.begin(); i != server_list.end();){
        if((*i).click == 0){
            i = server_list.erase(i);
        }
        else i++;
    }
    
    daemon_list.clear();
}

int main(void){
    init();
    Server srvr("127.0.0.1");
    //srvr.Deamon();
    vector<int> fargs = {1,0,0,1};
    vector<void *>args;
    CPart::addArg<double>(&args, 12.63);
    CPart::addArg<int>(&args, 10);
    CPart::addArg<int>(&args, 6);
    CPart::addArg<double>(&args, 8.2);
//    输入过程
    struct compute_result cpur = {"Test",&args,&args,&fargs,&fargs};
    packet pkt =  srvr.CPURS2Packet(cpur);
    raw_data rwd =  srvr.Packet2Rawdata(pkt);
    
//    输出过程
    srvr.Rawdata2Packet(rwd);
    
    while(1){
        sleep(100);
    }
    return 0;
}

void CPMT(void){
    CMap map("./PCS");
    CThread thread(&map);
    thread.AddArgs<int>("B", 4);
    thread.AddArgs<double>("B", 9.0);
    thread.AddArgs<int>("C", 1.0);
    thread.AddArgs<double>("C", 3.0);
    thread.Analyse();
    thread.DoLine();
    thread.SetDaemon();
    thread.CancelChildPCS(0);
}


