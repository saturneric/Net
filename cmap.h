//
//  cmap.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cmap_h
#define cmap_h

#include "type.h"
#include "cpart.h"


//计算模块管理对象间的依赖关系管理结构
class cp_depend{
public:
//    指向某计算模块对象
    CPart *f_cpart;
//    记录所依赖的子计算模块对象及其参数信息
    map<CPart *, vector<int> > cdpd;
//    记录其父计算模块对象及其参数信息
    map<CPart *, vector<int> > fdpd;
};

class Proj{
//    计算工程描述文件所在的地址
    string proj_path;
    string proj_file;
//    工程名
    string name;
//    计算工程读入流
    ifstream ifsproj;
//    源文件所在的目录
    vector<string> src_paths;
//    关系描述文件所在的目录
    vector<string> map_paths;
//    模块描述文件所在目录
    vector<string> cpt_paths;
//    动态链接库存放的目录
    string lib_path;
//    判断参数是否为字符串
    bool if_string(string &arg){
        if(arg[0] == '\"' && arg[arg.size()-1] == '\"') return true;
        else return false;
    }
//    判断下一条命令的有无
    bool if_continue(string &arg){
        if(arg.find(",") != string::npos && arg[arg.size()-1] == ',') return true;
        else return false;
    }
//    消去字符串的双引号
    string cut_string(string &arg){
        return arg.substr(1,arg.size()-2);
    }
//    处理描述文件的命令
    void deal_order(string tag, string arg){
        if(tag == "cpt"){
            cpt_paths.push_back(arg);
        }
        else if (tag == "map"){
            map_paths.push_back(arg);
        }
        else if(tag == "src_dir"){
            src_paths.push_back(arg);
        }
        else if(tag == "lib_dir"){
            this->lib_path = arg;
        }
        else throw "syntax error";
    }
protected:
//    检查路径或文件
    void check_paths(vector<string> paths){
        for(auto path : paths){
            if(!~access((proj_path+path).data(),F_OK)) throw path+" is abnormal";
        }
    }
//    检查字符是否合法
    bool if_illegal(char c){
        if(isalnum(c) || c == '_') return true;
        else return false;
    }
//    寻找保留字
    bool search_key(ifstream &ifsfile,string key){
        string line;
        do{
            ifsfile>>line;
        }while(line.find("proj") == string::npos && ifsfile.eof() == false);
        if(ifsfile.eof()) return false;
        return true;
    }
public:
    Proj(string t_projpath, string t_projfile){
//        检查工程描述文件是否可读
        if(!~access(t_projpath.data(), R_OK)) throw "project directory state is abnormal";
        if(proj_path[proj_path.find_last_not_of(" ")] != '/')
            t_projpath += "/";
        proj_path = t_projpath;
        if(!~access((t_projpath+t_projfile).data(), R_OK)) throw "project file state is abnormal";
        
        
        proj_file = t_projfile;
        ifsproj.open(proj_path+proj_file);
        if(ifsproj.good()){
            string line;
//            寻找保留字
            if(!search_key(ifsproj,"proj")) throw "project struct not found";
//            读取下一段信息
            ifsproj>>line;
//            大括号位置
            string::size_type qb_idx = line.find("{");
//            寻找任务工程名
            string t_name;
            if(qb_idx == string::npos) t_name = line;
            else t_name = line.substr(0,qb_idx);
//            检查工程名是否含有非法字符
            for(auto c:t_name){
                if(if_illegal(c));
                else throw "project's name has illegal char";
            }
            name = t_name;
//            寻找左大括号
            if(qb_idx == string::npos){
                ifsproj>>line;
                if((qb_idx = line.find("{")) == string::npos) throw "syntax error";
            }
//            逐行分析语句
            string tag,cma,arg;
            bool if_ctn = false;
            do{
//                读取命令标签
                ifsproj>>tag;
//                读取冒号
                ifsproj>>cma;
//                读取命令变量
                ifsproj>>arg;
//                检查参数是否含有非法字符
                for(auto c:t_name){
                    if(if_illegal(c));
                    else throw " arg has illegal char";
                }
                if(cma != ":") throw "syntax error";
                if((if_ctn = if_continue(arg)) == true){
//                    消掉逗号
                    arg = arg.substr(0,arg.size()-1);
                }
                if(if_string(arg)){
                    deal_order(tag, cut_string(arg));
                }
                else throw "syntax error";
            }while(if_ctn);
            ifsproj>>line;
            if(line != "};") throw "syntax error";
        }
        else throw "fail to open project file";
//        检查目录以及描述文件是否存在
        if(!~access((proj_path+lib_path).data(),W_OK)) throw "lib path is abnormal";
        check_paths(src_paths);
        check_paths(map_paths);
        check_paths(cpt_paths);
    }
   
};

class Cpt:public Proj{
    
};

class Map: public Cpt{
    
};

//计算任务图类
class CMap{
public:
//    计算任务图中包含的的计算模块列表
    map<string,CPart *> cparts;
//    记录计算模块依赖关系图
    map<CPart *, cp_depend> depends;
//    构造函数传入图包所在的目录
    CMap(string proj_path);
//    根据图的表述文件构造计算模块列表
    void BuildCPart(ifstream &map);
//    根据图表述文件中的描述信息，处理并转化为形式输入或输出参数列表
    vector<int> BuidArgs(string &line);
//    根据图的表述文件构造计算模块之间的依赖关系
    void BuildConnection(ifstream &map);
//    根据图描述文件依赖关系描述语句所提供的信息转化为依赖关系结构
//    Depends ReadItem(string item);
    
//    由某个节点递归向下遍历
    static void MapThrough(CPart *pcp,void(*func)(void *,CPart *),void *);
    
};

#endif /* cmap_h */
