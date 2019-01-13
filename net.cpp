//
//  net.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "net.h"
#include "cpart.h"




int main(void){
    CPart ncp("process");
    ncp.setArgsType({0,0}, {0});
    ncp.addArgsInt(2);
    ncp.addArgsInt(5);
    ncp.Run();
    
    return 0;
}


