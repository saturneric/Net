//
//  type.h
//  Net
//
//  Created by 胡一兵 on 2019/1/17.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef type_h
#define type_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <algorithm>
#include <vector>
#include <list>
#include <string>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include<sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <dirent.h>
#include <memory.h>
#include <sqlite3.h>


using std::string;
using std::vector;
using std::map;
using std::pair;
using std::list;
using std::ifstream;
using std::cout;
using std::endl;
using std::stringstream;

typedef char Byte;

#define REQUSET_TYPE 100
#define RESPOND_TYPE 101
#define ENCRYPT_POST_TYPE 101

#endif /* type_h */
