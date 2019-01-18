//
//  cmap.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "memory.h"
#include "cmap.h"


CMap::CMap(string path){
    ifstream map;
    map.open(path+"/pcs.map");
    this->path = path;
//    如果文件存在且打开成功
    if(map.good()){
        string line;
        map>>line;
//        读取计算模块信息
        if(line == "#PCS"){
            BuildCPart(map);
        }
        map>>line;
//        读取计算模块之间的依赖关系信息
        if(line == "#DEPEND"){
            BuildConnection(map);
        }
        
    }
    else throw "fail to open pcs.map";
}

void CMap::BuildCPart(ifstream &map){
    string line;
    map>>line;
//    从初始行开始读取在描述结束符之前的所有行
    while(line!="END"){
//        定位括号，来截取计算模块的名字
        unsigned long qs = line.find('(',0), qe = line.find(')',0);
        string name = line.substr(qs+1,qe-qs-1);
//        定位尖括号来截取计算模块所在源文件的名字
        qs = line.find('<',0);
        qe = line.find('>',0);
        string src_name = line.substr(qs+1,qe-qs-1);
        
//        根据以上信息构造计算管理结构对象
        CPart *ncp = new CPart(path,src_name,name);
//        在列表中加入该对象
        this->cparts.insert(pair<string,CPart *>(name,ncp));
        
//        构造该计算模块的输入与输出形式参数列表
        vector<int>fargs_in,fargs_out;
        map>>line;
        if(line[0] == '>'){
            fargs_in = BuidArgs(line);
        }
        map>>line;
        if(line[0] == '<'){
            fargs_out = BuidArgs(line);
        }
//        设置输入与输出形式参数列表
        ncp->setArgsType(fargs_in, fargs_out);
//        读入下一行
        map>>line;
    }
}

vector<int> CMap::BuidArgs(string &line){
    vector<int> fargs;
    unsigned long qs,qe;
//    定位方括号来获得函数的作用域
    qs = line.find('[',0);
    qe = line.find(']',0);
    unsigned long idx = qs;
    while(idx < qe){
        unsigned long ts,te;
//        截取冒号与逗号之间的类型信息
        ts = line.find(':',idx);
        te = line.find(',',ts);
        if(te == string::npos){
            te = qe;
        }
        string type = line.substr(ts+1,te-ts-1);
//        在形式参数列表中加入类型信息
        if(type == "int")
            fargs.push_back(INT);
        else if(type == "double")
            fargs.push_back(DOUBLE);
        
//        向下移动
        idx = te;
    }
    return fargs;
}


void CMap::BuildConnection(ifstream &map){
    string line;
    map>>line;
//    从初始行开始读取在描述结束符之前的所有行
    while(line != "END"){
//        定位圆括号来截取计算模块的名字
        unsigned long qs = line.find('(',0), qe = line.find(')',0);
        string name = line.substr(qs+1,qe-qs-1);
//        通过计算模块的名字在参数列表中找到对应的计算模块
        CPart *p_ncp = cparts.find(name)->second;
//        定位大括号来截取依赖模块的列表
        unsigned long idx = line.find('{',qe);
        unsigned long ss = line.find('}',idx);
        unsigned long ts, te;
        while(idx+1 < ss){
//            定位圆括号与方括号来截取依赖的计算模块的名字及依赖的相关参数信息
            ts = line.find('(',idx);
            te = line.find(']',ts);
            if(te == string::npos){
                te = ss;
            }
//            在计算模块管理结构中添加构造的依赖信息管理结构
            string item = line.substr(ts,te-ts+1);
            p_ncp->depends.push_back(ReadItem(item));
            //cout<<item<<endl;
//            向下移动
            idx = te+1;
            
        }
        map>>line;
    }
}

Depends CMap::ReadItem(string item){
//    构造依赖关系信息结构
    Depends dep;
//    定位圆括号来截取计算模块的名字
    unsigned long qs = item.find('(',0), qe = item.find(')',qs);
    string name = item.substr(qs+1,qe-qs-1);
//    通过模块名字在模块列表中找到相应模块
    dep.t_cpart = cparts.find(name)->second;
//    定位方括号来截取依赖参数信息
    unsigned long idx = item.find('[',0),ss = item.find(']',idx);
    unsigned long ts, te;
    while(idx < ss){
        ts = idx;
        te = item.find(',',ts+1);
        if(te == string::npos){
            te = ss;
        }
//        将字符串转换为整数
        string arg = item.substr(ts+1,te-ts-1);
        std::stringstream sstr;
        int darg;
        sstr<<arg;
        sstr>>darg;
//        加入依赖参数序号信息
        dep.args.push_back(darg);
//        向下移动
        idx = te;
    }
    return dep;
}

void CMap::MapThrough(CPart *pcp,void (*func)(void *, CPart *),void *args){
//    调用回调函数
    func(args,pcp);
    auto dpds = pcp->depends;
    for(auto i = dpds.begin(); i != dpds.end(); i++){
        MapThrough(i->t_cpart, func, args);
    }
}
