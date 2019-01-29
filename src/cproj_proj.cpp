//
//  cproj_proj.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/29.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "cproj.h"

//处理描述文件的命令
void Proj::deal_order(string tag, string arg){
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

//    检查Cpt文件中描述的源文件是否存在对应实体
void Proj::check_cpt(void){
    for(auto cpt:cpts){
#ifdef DEBUG
        printf("Checking Cpt File %p.\n",cpt);
#endif
        for(auto file:cpt->src_files){
            auto src_file = src_files.find(file);
//                如果在索引中没有找到对应的源文件
            if(src_file == src_files.end()){
#ifdef DEBUG
                printf("[Error]Fail To Find Soruce File Related %s\n",file.data());
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

//    读取所有涉及的Cpt文件
void Proj::build_cpts(void){
    for(auto cptp : cpt_paths){
        Cpt *ncpt = new Cpt(proj_path + cptp, name);
        cpts.push_back(ncpt);
    }
}

//    搜寻源文件搜索目录
void Proj::search_src(int idx,string path){
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

//    编译目标源文件生成动态链接库
void Proj::build_src(string lib_name,string srcfile_path, string libs_path){
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
//        检测命令重定向输出文件的大小，判断编译是否出现了错误
        if(file_size == 0){
#ifdef DEBUG
            printf("Succeed In Compiling File %s\n",srcfile_path.data());
#endif
        }
        else{
#ifdef DEBUG
            printf("[Error]Caught Errors or Warrnings In Compiling File %s\n",srcfile_path.data());
#endif
            throw "compile error";
        }
    }
    else throw "fail to build lib file";
}

//    创建数据库
void Proj::build_new_db(void){
    string db_dir = proj_path+"dbs";
    if(mkdir(db_dir.data(), S_IRWXU | S_IRWXG | S_IRWXO) == -1){
        if(!~access(db_dir.data(), X_OK)){
#ifdef DEBUG
            printf("[Error]Caught Error In Creating Directory dbs\n");
#endif
            throw "fail to make dir dbs";
        }
    }
    db_path = proj_path+"dbs/"+name+".db";
    int sqlrtn = sqlite3_open(db_path.data(), &psql);
    if(!sqlrtn){
//        创建数据库表
//        记录源文件信息
        sql::table_create(psql, "srcfiles", {
//            ID
            {"id","INT  PRIMARY KEY  NOT NULL"},
//            文件名
            {"name","TEXT NOT NULL"},
//            文件路径
            {"path","TEXT NOT NULL"},
//            文件内容
            {"content","NONE"},
//            文件MD5
            {"md5","TEXT NOT NULL"}
        });
//        记录入口函数信息
        sql::table_create(psql, "functions", {
//            ID
            {"id","INT  PRIMARY KEY  NOT NULL"},
//            入口函数名
            {"name","TEXT NOT NULL"},
//            入口函数所在的源文件在数据库中对应的ID
            {"srcfile_id","INT NOT NULL"},
//            入口函数所在的动态链接库在数据库中对应的ID
            {"libfile_id","INT NOT NULL"}
        });
//        记录cpt文件信息
        sql::table_create(psql, "cptfiles", {
//            ID
            {"id","INT  PRIMARY KEY  NOT NULL"},
//            cpt文件完整路径
            {"path","TEXT NOT NULL"},
//            cpt文件内容
            {"content","NONE"},
//            cpt文件MD5
            {"md5","TEXT NOT NULL"}
        });
//        记录cpt文件信息
        sql::table_create(psql, "projfile", {
//            工程名
            {"project_name","TEXT NOT NULL"},
//            cpt文件内容
            {"content","NONE"},
//            cpt文件MD5
            {"md5","TEXT NOT NULL"}
        });
        
//        记录动态链接库信息
        sql::table_create(psql, "libfiles", {
//            ID
            {"id","INT  PRIMARY KEY  NOT NULL"},
//            动态链接库文件名
            {"name","TEXT NOT NULL"},
//            动态链接库文件内容
            {"content","NONE"},
        });
    }
    else{
#ifdef DEBUG
        printf("[Error] Fail To Create DB File %s\n",db_path.data());
#endif
        throw "fail to create db file";
    }
    
}

//写入项目涉及的源文件信息到数据库中
void Proj::write_src_info(void){
    int idx = 0;
    sqlite3_stmt *psqlsmt;
//    编译SQL语句
    sql::insert_info(psql, &psqlsmt, "srcfiles", {
        {"id","?1"},
        {"name","?2"},
        {"path","?3"},
        {"md5","?4"}
    });
#ifdef DEBUG
    printf("Writing Srcfiles Information Into Database.\n");
#endif
    for(auto src:used_srcfiles){
//        获取源文件的MD5
        string md5 = src_md5.find(src.first)->second;
//        链接数据
        sqlite3_bind_int(psqlsmt, 1, idx++);
        sqlite3_bind_text(psqlsmt, 2, src.first.data(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(psqlsmt, 3, src_paths[src.second].data(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(psqlsmt, 4, md5.data(), -1, SQLITE_TRANSIENT);
//        执行SQL语句
        int rtn = sqlite3_step(psqlsmt);
        if(rtn == SQLITE_OK || rtn == SQLITE_DONE){
#ifdef DEBUG
            printf("[*]Succeed In Writing Srcfiles Information %s\n",src.first.data());
#endif
        }
        else{
#ifdef DEBUG
            printf("[*]Failed to Write Srcfiles Information %s\n",src.first.data());
#endif
        }
        sqlite3_reset(psqlsmt);
        sqlite3_clear_bindings(psqlsmt);
    }
}

//写入动态链接库信息到数据库中
void Proj::write_lib_info(void){
    int idx = 0;
    sqlite3_stmt *psqlsmt;
    sql::insert_info(psql, &psqlsmt, "libfiles", {
        {"id","?1"},
        {"name","?2"}
    });
#ifdef DEBUG
    printf("Writing Libfiles Information Into Database.\n");
#endif
    for(auto lib : lib_index){
//        链接数据
        sqlite3_bind_int(psqlsmt, 1, idx++);
        sqlite3_bind_text(psqlsmt, 2, lib.second.data(), -1, SQLITE_TRANSIENT);
//        执行SQL语句
        int rtn = sqlite3_step(psqlsmt);
        if(rtn == SQLITE_OK || rtn == SQLITE_DONE){
#ifdef DEBUG
            printf("[*]Succeed In Writing Libfile Information %s\n",lib.second.data());
#endif
        }
        else{
#ifdef DEBUG
            printf("[*]Failed to Write Libfile Information %s\n",lib.second.data());
#endif
        }
        sqlite3_reset(psqlsmt);
        sqlite3_clear_bindings(psqlsmt);
    }
}

//    写入入口函数信息到数据库中
void Proj::write_func_info(void){
    int idx = 0;
    sqlite3_stmt *psqlsmt;
    sql::insert_info(psql, &psqlsmt, "functions", {
        {"id","?1"},
        {"name","?2"},
        {"srcfile_id","?3"},
        {"libfile_id","?4"}
    });
#ifdef DEBUG
    printf("Writing Functions Information Into Database.\n");
#endif
    for(auto func : func_index){
        string src_file = func.second->funcs_src.find(func.first)->second;
        string lib_file = lib_index.find(src_file)->second;
        string sql_quote = "SELECT id FROM srcfiles WHERE name = "+sql::string_type(src_file)+";";
//        查找源文件信息在数据库中的ID
        SQLCallBack *psqlcb =  sql::sql_exec(psql, sql_quote);
        if(psqlcb->size != 1){
#ifdef DEBUG
            printf("[Error]Database Data Is Abnormal.\n");
#endif
            throw "database abnormal";
        }
        std::stringstream ss;
        ss<<psqlcb->items[0].argv[0];
        int srcfile_id = -1;
        ss>>srcfile_id;
        if(psqlcb->items[0].colnum[0] != "id" || srcfile_id == -1){
#ifdef DEBUG
            printf("[Error]Database Data Is Abnormal In Table srcfiles\n");
#endif
            throw "database abnormal";
        }
        delete psqlcb;
//        查找动态链接库信息在数据库中的ID
        sql_quote = "SELECT id FROM libfiles WHERE name = "+sql::string_type(lib_file)+";";
        psqlcb =  sql::sql_exec(psql, sql_quote);
        if(psqlcb->size != 1){
#ifdef DEBUG
            printf("[Error]Database Data Is Abnormal In Table libfiles\n");
#endif
            throw "database abnormal";
        }
        std::stringstream ssl;
        ssl<<psqlcb->items[0].argv[0];
        int libfile_id = -1;
        ssl>>libfile_id;
        if(psqlcb->items[0].colnum[0] != "id" || libfile_id == -1){
#ifdef DEBUG
            printf("[Error]Database Data Is Abnormal In Table libfiles\n");
#endif
            throw "database abnormal";
        }
        delete psqlcb;
//        链接数据
        sqlite3_bind_int(psqlsmt, 1, idx++);
        sqlite3_bind_text(psqlsmt, 2, func.first.data(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(psqlsmt, 3, srcfile_id);
        sqlite3_bind_int(psqlsmt, 4, libfile_id);
        
//        执行SQL语句
        int rtn = sqlite3_step(psqlsmt);
        if(rtn == SQLITE_OK || rtn == SQLITE_DONE){
#ifdef DEBUG
            printf("[*]Succeed In Writing Function Information %s\n",func.first.data());
#endif
        }
        else{
#ifdef DEBUG
            printf("[*]Failed to Write Function Information %s\n",func.first.data());
#endif
        }
        sqlite3_reset(psqlsmt);
        sqlite3_clear_bindings(psqlsmt);
    }
}

//写入口函数入输入输出参数信息到数据库中
void Proj::write_args_info(void){
#ifdef DEBUG
    printf("Start to Write Function Parameter Information Into Database.\n");
#endif
    for(auto func : func_index){
        vector<cpt_func_args> *pfin = &func.second->fargs_in.find(func.first)->second;
        vector<cpt_func_args> *pfout = &func.second->fargs_out.find(func.first)->second;
//        创建对应的储存表
        sql::table_create(psql, func.first, {
//            ID
            {"id","INT  PRIMARY KEY  NOT NULL"},
//            数据类型
            {"type","TEXT  NOT NULL"},
//            顺序序号
            {"idx","INT  NOT NULL"},
//            输入或输出标识(0输入，1输出)
            {"io","INT  NOT NULL"},
//            标签名
            {"key","TEXT"},
//            数据单元个数
            {"size","INT  NOT NULL"}
        });
        
        sqlite3_stmt *psqlsmt;
        
//        生成并编译SQL语句
        sql::insert_info(psql, &psqlsmt, func.first, {
            {"id","?6"},
            {"type","?1"},
            {"idx","?2"},
            {"io","?3"},
            {"key","?4"},
            {"size","?5"}
        });
        
        int idx = 0, sidx = 0;
#ifdef DEBUG
        printf("Writing Function Parameter (IN) Information Into Database (%s)\n",func.first.data());
#endif
//        写入输入参数
        for(auto arg : *pfin){
//            连接数据
            sqlite3_bind_text(psqlsmt, 1, arg.type.data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(psqlsmt, 2, idx++);
            sqlite3_bind_int(psqlsmt, 3, 0);
            sqlite3_bind_text(psqlsmt,4, arg.key.data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(psqlsmt, 5, arg.size);
            sqlite3_bind_int(psqlsmt, 6, sidx++);
//            执行SQL语句
            int rtn = sqlite3_step(psqlsmt);
            if(rtn == SQLITE_OK || rtn == SQLITE_DONE){
#ifdef DEBUG
                printf("[*]Succeed In Writing Function Parameter Information %s\n",func.first.data());
#endif
            }
            else{
#ifdef DEBUG
                printf("[*]Failed to Write Function Parameter Information %s\n",func.first.data());
#endif
            }
            sqlite3_reset(psqlsmt);
            sqlite3_clear_bindings(psqlsmt);
        }
        
#ifdef DEBUG
        printf("Writing Function Parameter (OUT) Information Into Database (%s)\n",func.first.data());
#endif
        idx = 0;
//        写入输入参数
        for(auto arg : *pfout){
//            连接数据
            sqlite3_bind_text(psqlsmt, 1, arg.type.data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(psqlsmt, 2, idx++);
            sqlite3_bind_int(psqlsmt, 3, 1);
            sqlite3_bind_text(psqlsmt,4, arg.key.data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(psqlsmt, 5, arg.size);
            sqlite3_bind_int(psqlsmt, 6, sidx++);
            
//            执行SQL语句
            int rtn = sqlite3_step(psqlsmt);
            if(rtn == SQLITE_OK || rtn == SQLITE_DONE){
#ifdef DEBUG
                printf("[*]Succeed In Writing Function Parameter Information %s\n",func.first.data());
#endif
            }
            else{
#ifdef DEBUG
                printf("[*]Failed to Write Function Parameter Information %s\n",func.first.data());
#endif
            }
            sqlite3_reset(psqlsmt);
            sqlite3_clear_bindings(psqlsmt);
        }
    }
}

void Proj::write_cpt_info(void){
    struct stat tstat;
    sqlite3_stmt *psqlsmt;
    sql::insert_info(psql, &psqlsmt, "cptfiles", {
        {"id","?1"},
        {"path","?2"},
        {"md5","?3"},
        {"content","?4"}
    });
    int idx = 0;
    for(auto cpt : cpt_paths){
        string treal_path = proj_path+cpt;
        if(!~stat(treal_path.data(), &tstat)){
#ifdef DEBUG
            printf("Error: Error In Getting Cpt File %s\n",treal_path.data());
#endif
            throw "cpt file not exist";
        };
        char *buff = (char *)malloc(tstat.st_size);
        read_file(treal_path, buff);
        sqlite3_bind_int(psqlsmt, 1, idx++);
        string md5;
        ComputeFile(treal_path.data(), md5);
        sqlite3_bind_text(psqlsmt, 2, cpt.data(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(psqlsmt, 3, md5.data(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(psqlsmt, 4, buff, (int)tstat.st_size, SQLITE_TRANSIENT);
        free(buff);
//            执行SQL语句
        int rtn = sqlite3_step(psqlsmt);
        if(rtn == SQLITE_OK || rtn == SQLITE_DONE){
#ifdef DEBUG
            printf("[*]Succeed In Writing Cpt File Information %s\n",treal_path.data());
#endif
        }
        else{
#ifdef DEBUG
            printf("[*]Failed to Write Cpt File Information %s\n",treal_path.data());
#endif
        }
        sqlite3_reset(psqlsmt);
        sqlite3_clear_bindings(psqlsmt);
    }
}
//写入工程描述文件信息到数据库中
void Proj::write_proj_info(void){
    struct stat tstat;
    sqlite3_stmt *psqlsmt;
    sql::insert_info(psql, &psqlsmt, "projfile", {
        {"project_name","?1"},
        {"content","?2"},
        {"md5","?3"}
    });
    if(!~stat((proj_path+"netc.proj").data(), &tstat)){
#ifdef DEBUG
        printf("Error: Error In Getting Project File netc.proj\n");
#endif
        throw "project file not exist";
    };
    Byte *buff = (Byte *)malloc(tstat.st_size);
    read_file(proj_path+"netc.proj", buff);
    sqlite3_bind_text(psqlsmt, 1, name.data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(psqlsmt, 2, buff, (int)tstat.st_size, SQLITE_TRANSIENT);
    string md5;
    ComputeFile(proj_path+"netc.proj", md5);
    sqlite3_bind_text(psqlsmt, 3, md5.data(), -1, SQLITE_TRANSIENT);
    free(buff);
//    执行SQL语句
    int rtn = sqlite3_step(psqlsmt);
    if(rtn == SQLITE_OK || rtn == SQLITE_DONE){
#ifdef DEBUG
        printf("[*]Succeed In Writing Project File Information netc.proj\n");
#endif
    }
    else{
#ifdef DEBUG
        printf("[*]Failed to Write Project File Information netc.proj\n");
#endif
    }
    sqlite3_reset(psqlsmt);
    sqlite3_clear_bindings(psqlsmt);
    
}


Proj::Proj(string t_projpath, string t_projfile){
//    检查工程描述文件是否可读
    if(!~access(t_projpath.data(), R_OK)) throw "project directory state is abnormal";
    if(proj_path[proj_path.find_last_not_of(" ")] != '/')
        t_projpath += "/";
    proj_path = t_projpath;
    if(!~access((t_projpath+t_projfile).data(), R_OK)) throw "project file state is abnormal";
    
    
    proj_file = t_projfile;
    ifsproj.open(proj_path+proj_file);
    if(ifsproj.good()){
        string line;
//        寻找保留字
        if(!search_key(ifsproj,"proj")) throw "project struct not found";
//        读取下一段信息
        ifsproj>>line;
//        大括号位置
        string::size_type qb_idx = line.find("{");
//        寻找任务工程名
        string t_name;
        if(qb_idx == string::npos) t_name = line;
        else t_name = line.substr(0,qb_idx);
//        检查工程名是否含有非法字符
        for(auto c:t_name){
            if(if_illegal(c));
            else throw "project's name has illegal char";
        }
        name = t_name;
//        寻找左大括号
        if(qb_idx == string::npos){
            ifsproj>>line;
            if((qb_idx = line.find("{")) == string::npos) throw "syntax error";
        }
//        逐行分析语句
        string tag,cma,arg;
        bool if_ctn = false;
        do{
//            读取命令标签
            ifsproj>>tag;
//            读取冒号
            ifsproj>>cma;
//            读取命令变量
            ifsproj>>arg;
//            检查参数是否含有非法字符
            for(auto c:t_name){
                if(if_illegal(c));
                else throw " arg has illegal char";
            }
            if(cma != ":") throw "syntax error";
            if((if_ctn = if_continue(arg)) == true){
//                消掉逗号
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

}

//检查目录以及描述文件是否存在
void Proj::GeneralCheckInfo(void){

    if(!~access((proj_path+lib_path).data(),W_OK)) throw "lib path is abnormal";
    check_paths(proj_path,src_paths);
    check_paths(proj_path,map_paths);
    check_paths(proj_path,cpt_paths);
}

//搜寻源文件搜索目录并读取Cpt文件
void Proj::SearchInfo(void){
    int idx = 0;
    //        遍历源文件储存目录
    for(auto path : src_paths) search_src(idx++, proj_path+path);
    //        读取cpt文件
    build_cpts();
    
}

//检查Cpt文件内所描述的源文件是否有对应的实体
void Proj::CheckCptInfo(void){
    check_cpt();
}

//建立计算模块入口函数索引
void Proj::BuildFuncIndex(void){
//    对应cpt文件
    for(auto pcpt : cpts){
//        对应源文件
        for(auto func: pcpt->funcs_src){
//            对应的计算模块入口函数
            func_index.insert({func.first,pcpt});
#ifdef DEBUG
            printf("Building Func Index %s (%p).\n",func.first.data(),pcpt);
#endif
        }
    }
}

//编译涉及到的源文件
void Proj::CompileUsedSrcFiles(void){
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

//检查入口函数是否在对应的动态链接库中可被按要求正确解析
void Proj::CheckFuncInfo(void){
    for(auto func : func_index){
//        找到入口函数所在的源文件
        string src_file = func.second->funcs_src.find(func.first)->second;
//        找到入口函数所在的动态链接库
        string tlib_name = lib_index.find(src_file)->second;
        string tlib_path = proj_path+lib_path+"/"+tlib_name;
#ifdef DEBUG
        printf("Checking Func %s\n[*]Lib File %s\n[*]Source File %s\n",func.first.data(),tlib_path.data(),src_file.data());
        
#endif
//        动态链接库操作柄
        void *lib_handle;
//        入口函数操作柄
        PCSFUNC func_handle;
//        传入传出参数列表操作柄
        vector<block_info> *args_handle;
//        获得动态链接库的操作柄
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
//        获得该模块的入口
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
//        获得向该模块传入参数的操作柄
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
//        获得获取该模块传出参数的操作柄
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

//建立数据库
void Proj::DBProcess(void){
//    建立新的数据库
    build_new_db();
//    写入源文件信息
    write_src_info();
//    写入动态链接库信息
    write_lib_info();
//    写入入口函数信息
    write_func_info();
//    写入参数信息
    write_args_info();
//    写入cpt文件信息
    write_cpt_info();
//    写入工程文件信息
    write_proj_info();
}


//获得工程名
string Proj::GetName(void){
    return name;
}