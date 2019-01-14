//
//  cthread.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "cthread.h"

CThread::CThread(CMap *tp_map):p_map(tp_map){
    for(auto k = p_map->cparts.begin(); k != p_map->cparts.end(); k++){
        vector<void *> args,args_out;
        rargs.insert(pair<string,vector<void *>>((*k).first,args));
        rargs_out.insert(pair<string,vector<void *>>((*k).first,args_out));
    }
    for(auto k = p_map->cparts.begin(); k != p_map->cparts.end(); k++){
        ifsolved.insert(pair<string,bool>((*k).first,false));
    }
}

void CThread::Analyse(void){
    for(auto k = p_map->cparts.begin(); k != p_map->cparts.end(); k++){
        auto cpart_depends = (*k).second->depends;
        if(cpart_depends.size()){
            bool if_ok = true;
            for(auto ditem = cpart_depends.begin(); ditem != cpart_depends.end(); ditem++){
                string name = ditem->t_cpart->name;
                if(!(ifsolved.find(name)->second)){
                    if_ok = false;
                }
            }
            if(if_ok) line.push_back((*k).second);
        }
        else{
            string name = (*k).second->name;
            if(rargs.find(k->second->name)->second.size() == k->second->fargs_in.size()){
                if(ifsolved.find(name)->second == false){
                    line.push_back(k->second);
                }
            }
            
        }
    }
}

void CThread::DoLine(void){
    for(auto pcp = line.begin(); pcp != line.end(); pcp++){
        string name = (*pcp)->name;
        
        vector<void *> args = rargs.find(name)->second;
        vector<void *> &args_out = rargs_out.find(name)->second;
        vector<int> fargs = (*pcp)->fargs_in;
        vector<int> fargs_out = (*pcp)->fargs_out;
        vector<void *> &argso = (*pcp)->args_out;
        (*pcp)->Clear();
        int cout = 0;
        for(auto arg = args.begin(); arg != args.end(); arg++,cout++){
            if(fargs[cout] == INT){
                (*pcp)->addArgsIn<int>(*((int *)(*arg)));
            }
            else if(fargs[cout] == DOUBLE){
                (*pcp)->addArgsIn<double>(*((double *)(*arg)));
            }
        }
        if(!(*pcp)->Run()){
            ifsolved.find(name)->second = true;
            int cout = 0;
            for(auto argo = argso.begin(); argo != argso.end(); argo++,cout++){
                if(fargs_out[cout] == INT){
                    int *p_value = new int(*((int *)(*argo)));
                    args_out.push_back((void *)p_value);
                }
                else if(fargs_out[cout] == DOUBLE){
                    double *p_value = new double(*((double *)(*argo)));
                    args_out.push_back((double *)p_value);
                }
            }
        }
    }
}
