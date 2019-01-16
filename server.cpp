//
//  server.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/16.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "server.h"

list<server_clock> server_list;

void setServerClock(Server *psvr, int clicks){
    server_list.push_back({psvr,clicks});
}
