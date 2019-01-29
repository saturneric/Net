//
//  cproj.h
//  Net
//
//  Created by 胡一兵 on 2019/1/22.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef cproj_h
#define cproj_h

#include "type.h"
#include "cpart.h"
#include "md5.h"
#include "sql.h"

class Proj;

//配置文件通用方法类
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
    
    int read_file(string path, Byte *buff){
        ifstream ifsf(path.data(),std::ios::binary);
        char tmp[512] = {0}, *idx = buff;
        while (!ifsf.eof()) {
            memset(tmp, 0, 512);
            ifsf.read(tmp, 512);
            memcpy(idx, tmp, 512);
            idx += 512;
        }
        ifsf.close();
        return 0;
    }
};


struct cpt_func_args{
    string type;
    int size;
    string key;
};

//cpt文件管理类
class Cpt:public setting_file{
    friend Proj;
//    Cpt文件中所提到的源文件
    vector<string> src_files;
//    入口函数对应的源文件（入口函数名，源文件名）
    map<string,string> funcs_src;
//    入口函数的输入与输出参数格式(入口函数名，参数列表)
    map<string,vector<cpt_func_args>> fargs_in,fargs_out;
//    Cpt文件路径
    string path;
    ifstream ifscpt;
//    工程名
    string name;
//    处理参数列表
    vector<cpt_func_args> deal_args(string args);
//    处理参数
    cpt_func_args deal_arg(string arg);
public:
//    构造函数
    Cpt(string path, string proj_name);
};

//map文件管理类
class Map: public Cpt{
    
};


//proj文件管理类
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
//    工程对应数据库
    sqlite3 *psql;
//    数据库文件路径
    string db_path;

//    处理描述文件的命令
    void deal_order(string tag, string arg);
//    读取所有涉及的Cpt文件
    void build_cpts(void);
//    检查Cpt文件中描述的源文件是否存在对应实体
    void check_cpt(void);
//    搜寻源文件目录
    void search_src(int idx,string path);
//    将现有信息储存到一个新的数据库中
    void update_db(void);
//    检查涉及到的源文件的MD5与数据库中的是否一致
    void check_src_md5(string db_path);
//    编译目标源文件生成动态链接库
    void build_src(string lib_name,string srcfile_path, string libs_path);
//    创建数据库
    void build_new_db(void);
//    写入项目涉及的源文件信息到数据库中
    void write_src_info(void);
//    写入动态链接库信息到数据库中
    void write_lib_info(void);
//    写入入口函数信息到数据库中
    void write_func_info(void);
//    写入口函数入输入输出参数信息到数据库中
    void write_args_info(void);
//    写入cpt文件信息到数据库中
    void write_cpt_info(void);
//    写入工程描述文件信息到数据库中
    void write_proj_info(void);
//    检查数据库是否正确
    void check_database(void);
public:
//    读取Proj文件
    Proj(string t_projpath, string t_projfile);
//    检查目录以及描述文件是否存在
    void GeneralCheckInfo(void);
//   搜寻源文件搜索目录并读取Cpt文件
    void SearchInfo(void);
//    检查Cpt文件内所描述的源文件是否有对应的实体
    void CheckCptInfo(void);
//    建立计算模块入口函数索引
    void BuildFuncIndex(void);
//    编译涉及到的源文件
    void CompileUsedSrcFiles(void);
//    检查入口函数是否在对应的动态链接库中可被按要求正确解析
    void CheckFuncInfo(void);
//    建立数据库
    void DBProcess(void);
//    获得工程名
    string GetName(void);
    void UpdateProcess(void);
};

#endif /* cproj_h */
