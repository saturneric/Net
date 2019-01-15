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
extern list<Server *> server_list;
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
    for(auto i = daemon_list.begin(); i != daemon_list.end(); i++){
        (*i)->Daemon();
    }
    daemon_list.clear();
}

int main(void){
    init();
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
    while(1){
        sleep(100);
    }
    return 0;
}


