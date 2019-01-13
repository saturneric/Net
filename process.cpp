//
//  process.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "cpart.h"

PCSFUNC_DEFINE(process){
    int a = GET_ARGS(int), b = GET_ARGS(int);
    printf("%d\n",a+b);
    ADD_ARGS(int, a+b);
    return 0;
}

