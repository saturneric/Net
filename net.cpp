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
    CMap map("./PCS");
    CThread thread(&map);
    thread.AddArgs<int>("B", 4);
    thread.AddArgs<double>("B", 8);
    thread.Analyse();
    thread.DoLine();
    cout<<thread.rargs_out.find("B")->second.size()<<endl;
    
    return 0;
}


