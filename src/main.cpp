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


int main(int argc, char *argv[]){
    try{
        Proj nproj("./PCS","pcs.proj");
        nproj.SearchInfo();
        nproj.BuildFuncIndex();
        nproj.CheckCptInfo();
        nproj.CompileUsedSrcFiles();
        nproj.CheckFuncInfo();
        //Cpt ncpt("./PCS/pcs.cpt","CPTest");
        
    }
    catch(char const *error_info){
        printf("%s\n",error_info);
    }
    
    return 0;
}

void wiki_cpart(void){
    CPart ncp("./PCS","./Libs","a.cpp","A");
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
}


