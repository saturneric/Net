//
//  clock.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "type.h"
#include "clock.h"

#define CLOCKESE 15

list<clock_register> clocks_list;
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

//时钟滴答调用函数
void threadsClock(int n){
//    删除到期时钟
    if(clock_erase == 0){
        for(auto i = clocks_list.begin(); i != clocks_list.end();){
            if(i->click == 0)  i = clocks_list.erase(i);
            else i++;
        }
//        重设总滴答数
        clock_erase = CLOCKESE;
    }
    else clock_erase--;
//    处理时钟列表
    for(auto &clock : clocks_list){
        if(clock.click == 0){
            if(clock.if_thread){
                pthread_t ptd = 0;
                pthread_create(&ptd, NULL, clock.func, NULL);
            }
            else{
                clock.func(NULL);
            }
//            标记时钟到期
            clock.click = -1;
        }
        else if(clock.click > 0){
            clock.click--;
        }
    }
    
}


