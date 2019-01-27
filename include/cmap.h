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
    vector<string> src_files;
//    入口函数对应的源文件
    map<string,string> funcs_src;
//    入口函数的输入与输出参数格式
    map<string,vector<cpt_func_args>> fargs_in,fargs_out;
    string path;
    ifstream ifscpt;
    string name;
public:
    
    Cpt(string path, string proj_name){
#ifdef DEBUG
        printf("Reading Cpt File %s\n[*]Require Project Name %s\n",path.data(),proj_name.data());
#endif
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
#ifdef DEBUG
            printf("Read Project Name %s\n",t_name.data());
#endif
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
#ifdef DEBUG
                printf("Read Source File Name %s\n",tsrc_name.data());
#endif
//                记录源文件名
                src_files.push_back(tsrc_name);
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
#ifdef DEBUG
                        printf("Read Function Description %s\n",real_line.data());
#endif
//                        分离输出参数列表
                        string::size_type dq_l = real_line.find("{");
                        string::size_type dq_r = real_line.find("}");
                        if(dq_l == string::npos || dq_r == string::npos) throw "syntax error";
                        string str_argout = real_line.substr(dq_l+1,dq_r-dq_l-1);
#ifdef DEBUG
                        printf("Read Args (OUT) Description %s\n",str_argout.data());
#endif
                        vector<cpt_func_args> cfgo = deal_args(str_argout);
//                        分离输入参数列表
                        string::size_type yq_l = real_line.find("(");
                        string::size_type yq_r = real_line.find(")");
                        if(yq_l == string::npos || yq_r == string::npos) throw "syntax error";
                        string str_argin = real_line.substr(yq_l+1,yq_r-yq_l-1);
                        vector<cpt_func_args> cfgi = deal_args(str_argin);
#ifdef DEBUG
                        printf("Read Args (IN) Description %s\n",str_argin.data());
#endif
//                        分离入口函数名
                        string func = real_line.substr(dq_r+1,yq_l-dq_r-1);
                        string real_func;
                        for(auto c : func){
                            if(isgraph(c)){
                                real_func.push_back(c);
                                if(!if_illegal(c)) throw "func name has illegal char";
                            }
                        }
#ifdef DEBUG
                        printf("Read Function Name %s\n",real_func.data());
#endif
//                        记录入口函数名
                        funcs_src.insert({real_func,tsrc_name});
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


class Proj:public setting_file{
//    计算工程所在的目录
    string proj_path;
//    计算工程描述文件名
    string proj_file;
//    工程名
    string name;
//    计算工程读入流
    ifstream ifsproj;
//    源文件所在的目录
    vector<string> src_paths;
//    源文件搜索目录下的所有源文件
    map<string,int> src_files;
//    计算工程所涉及到的源文件
    map<string,int> used_srcfiles;
//    计算工程所涉及到的源文件的MD5
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
//    模块入口函数索引
    map<string, Cpt *> func_index;
//    动态链接库对应的源文件索引
    map<string, string> lib_index;
    
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
    
//    读取所有涉及的Cpt文件
    void build_cpts(void){
        for(auto cptp : cpt_paths){
            Cpt *ncpt = new Cpt(proj_path + cptp, name);
            cpts.push_back(ncpt);
        }
    }
    
//    检查Cpt文件中描述的源文件是否存在对应实体
    void check_cpt(void){
        for(auto cpt:cpts){
#ifdef DEBUG
            printf("Checking Cpt File %p.\n",cpt);
#endif
            for(auto file:cpt->src_files){
                auto src_file = src_files.find(file);
//                如果在索引中没有找到对应的源文件
                if(src_file == src_files.end()){
#ifdef DEBUG
                    printf("Fail To Find Soruce File Related %s\n",file.data());
#endif
                    throw "source file not exist";
                }
                else{
#ifdef DEBUG
                    printf("Succeed In Finding Source File %s\n",file.data());
#endif
                    used_srcfiles.insert({src_file->first,src_file->second});
//                    计算源文件的MD5
                    string md5;
                    string tsrc_path = src_paths[src_file->second];
                    string t_path = proj_path+tsrc_path+"/"+file;
                    ComputeFile(t_path, md5);
                    src_md5.insert({file,md5});
#ifdef DEBUG
                    printf("Computed Source File's MD5 %s\n[*]Source File Path %s\n",md5.data(),t_path.data());
#endif
                }
            }
        }
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
                    }
                }
            }
        }
        else throw "path is abnormal";
    }
    
//    将现有信息储存到一个新的数据库中
    void update_db(void);
    
//    检查涉及到的源文件的MD5与数据库中的是否一致
    void check_src_md5(string db_path);
    
//    编译目标源文件生成动态链接库
    void build_src(string lib_name,string srcfile_path, string libs_path){
        string build_command = "g++ -fPIC -shared -std=c++11 -o "+libs_path+"/"+lib_name+" "+srcfile_path+" >gcc_build.log 2>&1";
#ifdef DEBUG
        printf("Build Command %s\n",build_command.data());
#endif
        int rtn = system(build_command.data());
//    检测命令执行情况
        if(rtn != -1 && rtn != 127){
            struct stat statbuff;
            stat("gcc_build.log", &statbuff);
            unsigned long file_size = statbuff.st_size;
//            检测命令重定向输出文件的大小，判断编译是否出现了错误
            if(file_size == 0){
#ifdef DEBUG
                printf("Succeed In Compiling File %s\n",srcfile_path.data());
#endif
            }
            else{
#ifdef DEBUG
                printf("Caught Errors or Warrnings In Compiling File %s\n",srcfile_path.data());
#endif
                throw "compile error";
            }
        }
        else throw "fail to build lib file";
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
    
//   搜寻源文件搜索目录并读取Cpt文件
    void SearchInfo(void){
        int idx = 0;
//        遍历源文件储存目录
        for(auto path : src_paths) search_src(idx++, proj_path+path);
//        读取cpt文件
        build_cpts();
        
    }
//    检查Cpt文件内所描述的源文件是否有对应的实体
    void CheckCptInfo(void){
        check_cpt();
    }
    
//    建立计算模块入口函数索引
    void BuildFuncIndex(void){
//        对应cpt文件
        for(auto pcpt : cpts){
//            对应源文件
            for(auto func: pcpt->funcs_src){
//                对应的计算模块入口函数
                func_index.insert({func.first,pcpt});
#ifdef DEBUG
                printf("Building Func Index %s (%p).\n",func.first.data(),pcpt);
#endif
            }
        }
    }
    
//    编译涉及到的源文件
    void CompileUsedSrcFiles(void){
        for(auto src : used_srcfiles){
            string tsrc_path = src_paths[src.second];
            string tlib_name = "lib_"+src.first+".so";
            string t_path = proj_path+tsrc_path+"/"+src.first;
            string tlib_path = proj_path+lib_path;
#ifdef DEBUG
            printf("Compling Used Source File %s\n[*]Libname %s\n[*]Source File Path %s\n[*]Lib Path %s\n",src.first.data(),tlib_name.data(),t_path.data(),tlib_path.data());
#endif
            build_src(tlib_name,t_path,tlib_path);
            lib_index.insert({src.first,tlib_name});
        }
    }
    
//    检查入口函数是否在对应的动态链接库中可被按要求正确解析
    void CheckFuncInfo(void){
        for(auto func : func_index){
            string src_file = func.second->funcs_src.find(func.first)->second;
            string tlib_name = lib_index.find(src_file)->second;
            string tlib_path = proj_path+lib_path+"/"+tlib_name;
#ifdef DEBUG
            printf("Checking Func %s\n[*]Lib File %s\n[*]Source File %s\n",func.first.data(),tlib_path.data(),src_file.data());
            
#endif
//            动态链接库操作柄
            void *lib_handle;
//            入口函数操作柄
            PCSFUNC func_handle;
//            传入传出参数列表操作柄
            vector<block_info> *args_handle;
            //    获得动态链接库的操作柄
            lib_handle = dlopen(tlib_path.data(), RTLD_NOW | RTLD_LOCAL);
            if(lib_handle == nullptr){
#ifdef DEBUG
                printf("Fail To Get Lib File Handle\n");
                
#endif
                throw "can not open library";
            }
#ifdef DEBUG
            printf("Succeed In Getting Lib File Handle %p\n",lib_handle);
            
#endif
            //    获得该模块的入口
            func_handle = (PCSFUNC) dlsym(lib_handle, func.first.data());
            if(func_handle == nullptr){
#ifdef DEBUG
                printf("Fail To Get Func Handle\n");
                
#endif
                throw "can not get func "+func.first;
            }
#ifdef DEBUG
            printf("Succeed In Getting Func Handle %p\n",func_handle);
            
#endif
            //    获得向该模块传入参数的操作柄
            args_handle = (vector<block_info> *) dlsym(lib_handle, ("__"+func.first+"_args_in").data());
            if(args_handle == nullptr){
#ifdef DEBUG
                printf("Fail To Get Args (IN) Handle\n");
                
#endif
                throw "can not get the HANDLE to PUSH args";
            }
#ifdef DEBUG
            printf("Succeed In Getting Args (IN) Handle %p\n",args_handle);
            
#endif
            //    获得获取该模块传出参数的操作柄
            args_handle = (vector<block_info> *) dlsym(lib_handle, ("__"+func.first+"_args_out").data());
            if(args_handle == nullptr){
#ifdef DEBUG
                printf("Fail To Get Args (OUT) Handle\n");
                
#endif
                throw "can not get the HANDLE to GET args";
            }
#ifdef DEBUG
            printf("Succeed In Getting Args (OUT) Handle %p\n",args_handle);
            
#endif
            dlclose(lib_handle);
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
//    构造函数传入计算工程管理类
    CMap(class Proj &);
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
