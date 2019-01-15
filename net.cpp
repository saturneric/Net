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

void init(void){
    signal(SIGALRM, threadsClock);
    setThreadsClock();
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
    
    while(1){
        thread.Daemon();
    }
    return 0;
}


