//
//  clock.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "type.h"
#include "clock.h"

#define CLOCKESE 30

list<clock_register> clocks_list;
map<uint32_t,clock_thread_info *> clocks_thread_map;
list<uint32_t> clock_thread_finished;
static struct itimerval oitrl, itrl;

static uint64_t clock_erase = CLOCKESE;


void initClock(void){
    signal(SIGALRM, threadsClock);
    setThreadsClock();
}

//设置全局线程时钟
void setThreadsClock(void){
    itrl.it_interval.tv_sec = 0;
    itrl.it_interval.tv_usec = 50000;
    itrl.it_value.tv_sec = 0;
    itrl.it_value.tv_usec = 50000;
    setitimer(ITIMER_REAL, &itrl, &oitrl);
}

void newClock(clock_register ncr){
    clocks_list.push_back(ncr);
}

//时钟滴答调用函数
void threadsClock(int n){
//    删除到期时钟
    if(clock_erase == 0){
        for(auto i = clocks_list.begin(); i != clocks_list.end();){
            if(i->click == -1)  i = clocks_list.erase(i);
            else i++;
        }
//        重设总滴答数
        clock_erase = CLOCKESE;
    }
    else clock_erase--;
    
    for(auto tid : clock_thread_finished){
        clock_thread_info *tcti = clocks_thread_map.find(tid)->second;
        pthread_join(tcti->pht,NULL);
        pthread_detach(tcti->pht);
        clocks_thread_map.erase(clocks_thread_map.find(tid));
//        如果时钟需要重置
        if(tcti->if_reset){
            clock_register ncr = *tcti->pcr;
            ncr.click = ncr.rawclick;
            newClock(ncr);
        }
        delete tcti;
    }
    clock_thread_finished.clear();
    
//    处理时钟列表
    for(auto &clock : clocks_list){
        if(clock.click == 0){
            if(clock.if_thread){
                clock_thread_info *pncti = new clock_thread_info();
                pncti->args = clock.arg;
                pncti->pcr = &clock;
                pncti->tid = (uint32_t)clocks_thread_map.size()+1;
                clocks_thread_map.insert({pncti->tid,pncti});
                pthread_create(&pncti->pht, NULL, clock.func, pncti);
            }
            else{
                clock.func(clock.arg);
            }
//            标记时钟到期
            clock.click = -1;
        }
        else if(clock.click > 0){
            clock.click--;
        }
    }
    
}

void clockThreadFinish(uint32_t tid){
    clock_thread_finished.push_back(tid);
}

