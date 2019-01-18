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

void CPMT(void){
    CMap map("./PCS");
    
    CThread thread(&map);
    thread.Analyse();
    thread.DoLine();
    thread.SetDaemon();
    thread.CancelChildPCS(0);
}

int main(void){
    initClock();
    CPart ncp("./PCS","a.cpp","A");
    void *a = main_pool.bv_malloc<double>(2.0);
    void *b = main_pool.bv_malloc<double>(3.5);
    void *c = main_pool.bv_malloc<int>(5);
    ncp.AddCPArgsIn(a);
    ncp.AddCPArgsIn(b);
    ncp.AddCPArgsIn(c);
    ncp.Run();
    void *oa = ncp.args_out[0];
    printf("%d",*((int *)oa));
    main_pool.b_free(a);
    main_pool.b_free(b);
    main_pool.b_free(c); 
    return 0;
}




