//
//  CProj.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/22.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "cproj.h"

Cpt::Cpt(string path, string proj_name){
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

vector<cpt_func_args> Cpt::deal_args(string args){
    string::size_type lcma_dix = 0;
    string::size_type cma_idx = args.find(",",0);
    vector<cpt_func_args> cfgs;
    
    if(cma_idx == string::npos){
        string real_args;
        for(auto c : args){
            if(isgraph(c)){
                real_args.push_back(c);
                if(!if_illegal(c)) throw "func name has illegal char";
            }
        }
        if(real_args.size() > 3){
            cpt_func_args ncfg = deal_arg(args);
            cfgs.push_back(ncfg);
        }
    }
    else{
        //            分割逗号
        while(cma_idx != string::npos){
            string arg = args.substr(lcma_dix,cma_idx-lcma_dix);
            cpt_func_args ncfg;
            ncfg = deal_arg(arg);
            cfgs.push_back(ncfg);
            lcma_dix = cma_idx+1;
            cma_idx = args.find(",",lcma_dix);
            if(cma_idx == string::npos && lcma_dix != string::npos){
                arg = args.substr(lcma_dix,args.size()-lcma_dix);
                ncfg = deal_arg(arg);
                cfgs.push_back(ncfg);
            }
        }
    }
    return cfgs;
}

cpt_func_args Cpt::deal_arg(string arg){
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
