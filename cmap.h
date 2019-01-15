//
//  cmap.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cmap_h
#define cmap_h

#include "cpart.h"
#include <map>
#include <fstream>
#include <sstream>

using std::map;
using std::ifstream;
using std::pair;

//计算任务无环有向图管理
class CMap{
public:
//    图中包含的的计算模块列表
    map<string,CPart *> cparts;
//    图目录所在的目录
    string path;
//    构造函数传入图包所在的目录
    CMap(string path);
//    根据图的表述文件构造计算模块列表
    void BuildCPart(ifstream &map);
//    根据图表述文件中的描述信息，处理并转化为形式输入或输出参数列表
    vector<int> BuidArgs(string &line);
//    根据图的表述文件构造计算模块之间的依赖关系
    void BuildConnection(ifstream &map);
//    根据图描述文件依赖关系描述语句所提供的信息转化为依赖关系结构
    Depends ReadItem(string item);
    
//    由某个节点递归向下遍历
    static void MapThrough(CPart *pcp,void(*func)(void *,CPart *),void *);
    
};

#endif /* cmap_h */
