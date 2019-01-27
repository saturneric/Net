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
#include "md5.h"

class Proj;

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


class setting_file{
protected:
    //    检查路径或文件
    void check_paths(string main_path, vector<string> paths){
        for(auto path : paths){
            if(!~access((main_path+path).data(),F_OK)) throw path+" is abnormal";
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
        }while(line.find(key) == string::npos && ifsfile.eof() == false);
        
        if(ifsfile.eof() == true) return false;
        return true;
    }
};


struct cpt_func_args{
    string type;
    int size;
    string key;
};

class Cpt:public setting_file{
    friend Proj;
    vector<CPart> cparts;
    vector<string> src_files;
//    源文件内含有的入口函数
    map<string,vector<string>> funcs;
//    入口函数的输入与输出参数格式
    map<string,vector<cpt_func_args>> fargs_in,fargs_out;
    string path;
    ifstream ifscpt;
    string name;
public:
    
    Cpt(string path, string proj_name){
        ifscpt.open(path);
        string line;
        if(search_key(ifscpt, "cparts")){
//            读取下一段信息
            ifscpt>>line;
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
//            检查工程名
            if(t_name == proj_name) name = t_name;
            else throw "project's name confilct";
//            寻找左大括号
            if(qb_idx == string::npos){
                ifscpt>>line;
                if((qb_idx = line.find("{")) == string::npos) throw "syntax error";
            }
//            源文件描述遍历
            while(search_key(ifscpt, "srcfile")){
                ifscpt>>line;
//                大括号位置
                string::size_type qb_idx = line.find("{");
//                寻找源文件名
                string tsrc_name;
                if(qb_idx == string::npos) tsrc_name = line;
                else tsrc_name = line.substr(0,qb_idx);
                tsrc_name = tsrc_name.substr(1,tsrc_name.size()-2);
//                记录源文件名
                src_files.push_back(tsrc_name);
                funcs.insert({tsrc_name,{}});
//                    寻找左大括号
                if(qb_idx == string::npos){
                    ifscpt>>line;
                    if((qb_idx = line.find("{")) == string::npos) throw "syntax error";
                }
//                入口函数描述遍历
                while(getline(ifscpt,line)){
                    string real_line;
                    for(auto c:line){
                        if(isgraph(c) || c == ' ') real_line.push_back(c);
                    }
                    if(real_line.empty()) continue;
                    if(real_line != "};"){
//                        分离输出参数列表
                        string::size_type dq_l = real_line.find("{");
                        string::size_type dq_r = real_line.find("}");
                        if(dq_l == string::npos || dq_r == string::npos) throw "syntax error";
                        string str_argout = real_line.substr(dq_l+1,dq_r-dq_l-1);
                        vector<cpt_func_args> cfgo = deal_args(str_argout);
//                        分离输入参数列表
                        string::size_type yq_l = real_line.find("(");
                        string::size_type yq_r = real_line.find(")");
                        if(yq_l == string::npos || yq_r == string::npos) throw "syntax error";
                        string str_argin = real_line.substr(yq_l+1,yq_r-yq_l-1);
                        vector<cpt_func_args> cfgi = deal_args(str_argin);
                        
//                        分离入口函数名
                        string func = real_line.substr(dq_r+1,yq_l-dq_r-1);
                        string real_func;
                        for(auto c : func){
                            if(isgraph(c)){
                                real_func.push_back(c);
                                if(!if_illegal(c)) throw "func name has illegal char";
                            }
                        }
//                        添加相关参数
                        fargs_out.insert({real_func,cfgo});
                        fargs_in.insert({real_func,cfgi});
                        
                    }
                    else break;
                }
                
            }
        
        }
        else throw "fail to find key word";
    }
    
    vector<cpt_func_args> deal_args(string args){
        string::size_type lcma_dix = 0;
        string::size_type cma_idx = args.find(",",0);
        vector<cpt_func_args> cfgs;
//        分割逗号
        while(cma_idx != string::npos){
            string arg = args.substr(lcma_dix,cma_idx-lcma_dix);
            deal_arg(arg);
            lcma_dix = cma_idx+1;
            cma_idx = args.find(",",lcma_dix);
            if(cma_idx == string::npos && lcma_dix != string::npos){
                arg = args.substr(lcma_dix,args.size()-lcma_dix);
                cpt_func_args ncfg = deal_arg(arg);
                cfgs.push_back(ncfg);
            }
            
        }
        return cfgs;
    }
    
    cpt_func_args deal_arg(string arg){
        cpt_func_args ncfa;
        std::stringstream ss,sr;
        ss<<arg;
        string key;
        ss>>key;
//        读取数组标号
        string::size_type fq_l = key.find("[");
        string::size_type fq_r = key.find("]");
        int size = 1;
        string type;
        if(fq_l != string::npos && fq_r != string::npos){
            string size_str = key.substr(fq_l+1,fq_r-fq_l-1);
            sr<<size_str;
            sr>>size;
            type = key.substr(0,fq_l);
            
            
        }
        else if (fq_r == string::npos && fq_r == string::npos){
            type = key.substr(0,fq_l);
        }
        else throw "syntax error";
        ncfa.size = size;
        ncfa.type = type;
        ss>>ncfa.key;
        return ncfa;
    }
    
};

class Map: public Cpt{
    
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
//    源文件
    map<string,int> src_files;
//    源文件的MD5
    map<string,string> src_md5;
//    关系描述文件所在的目录
    vector<string> map_paths;
//    模块描述文件所在目录
    vector<string> cpt_paths;
//    模块描述对象
    vector<Cpt *> cpts;
//    关系描述对象
    vector<Map> maps;
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
    void check_paths(string main_path, vector<string> paths){
        for(auto path : paths){
            if(!~access((main_path+path).data(),F_OK)) throw path+" is abnormal";
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
        }while(line.find(key) == string::npos && ifsfile.eof() == false);
        if(ifsfile.eof()) return false;
        return true;
    }
    void build_cpts(void){
        for(auto cptp : cpt_paths){
            Cpt *ncpt = new Cpt(proj_path + cptp, name);
            cpts.push_back(ncpt);
        }
    }
    void check_cpt(void){
        
    }
//    搜寻源文件目录
    void search_src(int idx,string path){
        DIR *pdir = opendir(path.data());
        struct dirent *ptr;
        if(pdir != NULL){
            while ((ptr = readdir(pdir)) != NULL) {
//                如果是文件
                if(ptr->d_type == 8){
                    string file = ptr->d_name;
//                    含有.cpp的文件
                    if(file.find(".cpp") != string::npos){
                        src_files.insert({file,idx});
//                        计算源文件的MD5
                        string md5;
                        ComputeFile(path+"/"+file, md5);
                        src_md5.insert({file,md5});
                    }
                }
            }
        }
        else throw "path is abnormal";
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
        check_paths(proj_path,src_paths);
        check_paths(proj_path,map_paths);
        check_paths(proj_path,cpt_paths);
    }
    
    void SearchPathInfo(void){
        int idx = 0;
//        遍历源文件储存目录
        for(auto path : src_paths) search_src(idx++, proj_path+path);
//        读取cpt文件
        build_cpts();
        
    }
    
    void CheckInfo(void){
        for(auto cpt:cpts){
            for(auto file:cpt->src_files){
                if(src_files.find(file) == src_files.end()) throw "source file not exist";
            }
        }
    }
    
   
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
