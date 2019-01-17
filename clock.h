//
//  clock.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef clock_h
#define clock_h

#include "type.h"

//时钟管理结构
struct clock_register{
    void *(*func)(void *);
    bool if_thread;
    int click;
};

//初始化全局时钟
void initClock(void);
//设置全局线程时钟
void setThreadsClock(void);
//时钟滴答调用函数
void threadsClock(int);


#endif /* clock_h */
