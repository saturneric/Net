//
//  cthread.cpp
//  Net
//
//  Created by 胡一兵 on 2019/1/14.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#include "cthread.h"

static struct itimerval oitrl, itrl;

CThread::CThread(CMap *tp_map):p_map(tp_map),idxtid(0){
    lpcs.if_als = false;
//    构造空的传入与传出参数列表
    for(auto k = p_map->cparts.begin(); k != p_map->cparts.end(); k++){
        vector<void *> args,args_out;
        rargs.insert(pair<string,vector<void *>>((*k).first,args));
        rargs_out.insert(pair<string,vector<void *>>((*k).first,args_out));
    }
//    构造任务进度列表
    for(auto k = p_map->cparts.begin(); k != p_map->cparts.end(); k++){
        ifsolved.insert(pair<string,bool>((*k).first,false));
    }
}

CThread::~CThread(){
    for(auto item = rargs.begin(); item != rargs.end(); item++){
        int count = 0;
        vector<int> fargs = p_map->cparts.find(item->first)->second->fargs_in;
        for(auto litem = item->second.begin(); litem != item->second.end(); litem++,count++){
            if(fargs[count] == INT){
                delete (int *)(*litem);
                
            }
            else if (fargs[count] == DOUBLE){
                delete (double *)(*litem);
            }
        }
    }
    for(auto item = rargs_out.begin(); item != rargs_out.end(); item++){
        int count = 0;
        vector<int> fargs = p_map->cparts.find(item->first)->second->fargs_out;
        for(auto litem = item->second.begin(); litem != item->second.end(); litem++,count++){
            if((*litem) != nullptr){
                if(fargs[count] == INT){
                    delete (int *)(*litem);
                    
                }
                else if (fargs[count] == DOUBLE){
                    delete (double *)(*litem);
                }
            }
        }
    }
}

void CThread::Analyse(void){
//    如果上一个并行执行进程还没有退出
    if(lpcs.if_als == true){
        //还没想好怎么做
    }
    for(auto k = p_map->cparts.begin(); k != p_map->cparts.end(); k++){
        auto cpart_depends = (*k).second->depends;
//        如果计算模块已经执行则跳过
        if(ifsolved.find(k->second->name)->second) continue;
//        如果该计算模块含有依赖模块
        if(cpart_depends.size()){
            bool if_ok = true;
            for(auto ditem = cpart_depends.begin(); ditem != cpart_depends.end(); ditem++){
                string name = ditem->t_cpart->name;
//                如果依赖模块还没有被调用过
                if(!(ifsolved.find(name)->second)){
                    if_ok = false;
                    break;
                }
            }
            if(if_ok){
                int count = 0;
                vector<int> s_fargs_in = k->second->fargs_in;
                for(auto ditem = cpart_depends.begin(); ditem != cpart_depends.end(); ditem++){
                    vector<int> args = ditem->args;
//                        输入参数列表
                    vector<void *> &args_in = rargs.find(k->second->name)->second;
//                        输出形式参数列表
                    vector<int> f_fargs_out = ditem->t_cpart->fargs_out;
//                        输出参数列表
                    vector<void *> args_out = rargs_out.find(ditem->t_cpart->name)->second;
//                        检查传入传出参数的类型是否匹配
                    for(auto itm = args.begin(); itm != args.end();itm++){
                        if(s_fargs_in[count++] != f_fargs_out[*itm]) throw "type conflict";
//                            重新分配内存
                        if(f_fargs_out[*itm] == INT){
                            CPart::addArg<int>(&args_in, *((int *)(args_out[*itm])));
                        }
                        else if(f_fargs_out[*itm] == DOUBLE){
                            CPart::addArg<double>(&args_in, *((double *)(args_out[*itm])));
                        }
                    }
                        
                }
                lpcs.line.push_back((*k).second);
            }
        }
//        如果该计算模块没有依赖模块
        else{
            string name = (*k).second->name;
            if(rargs.find(k->second->name)->second.size() == k->second->fargs_in.size()){
//                如果该模块还没有被调用
                if(ifsolved.find(name)->second == false){
                    lpcs.line.push_back(k->second);
                }
            }
            
        }
    }
}

void CThread::DoLine(void){
    for(auto pcp = lpcs.line.begin(); pcp != lpcs.line.end(); pcp++){
        string name = (*pcp)->name;
        
        vector<void *> args = rargs.find(name)->second;
        vector<int> fargs = (*pcp)->fargs_in;
        vector<int> fargs_out = (*pcp)->fargs_out;
        
        unsigned long ntid = idxtid++;
        pthread_t npdt = 0;
//        创建新线程
        struct thread_args *pt_ta = new struct thread_args({ntid,this,(*pcp),-1});
        if(pthread_create(&npdt,NULL,&CThread::NewThread,(void *)(pt_ta))){
            throw "fail to create thread";
        }
        lpcs.threads.insert({ntid,npdt});
    }

}

void CThread::SetDaemon(void){
    
}

void CThread::Daemon(void){
    //    等待线程返回
    for(auto i = lpcs.child_finished.begin(); i != lpcs.child_finished.end(); i++){
        unsigned long tid = (*i)->tid;
        pthread_t cpdt = lpcs.threads.find(tid)->second;
        struct thread_args *rpv = nullptr;
        pthread_join(cpdt, (void **)&rpv);
        //        根据返回值处理计算任务状态
        if(rpv->rtn == SUCCESS){
            //            标识该计算模块中计算任务的状态
            ifsolved.find(rpv->pcp->name)->second = true;
        }
        else{
            
        }
        //        释放线程资源
        pthread_detach(cpdt);
        //        在列表中销户证实宣告线程程结束
        lpcs.threads.erase(tid);
        
        printf("TID: %lu Deleted.\n",tid);
        delete rpv;
    }
    lpcs.child_finished.clear();
    if(lpcs.threads.size() > 0){
        
    }
}

void CThread::ChildThreadFSH(struct thread_args *pta){
    CThread *pct = pta->pct;
    (pct->lpcs).child_finished.push_back(pta);
    printf("Called TID %lu.\n",pta->tid);
}

void *CThread::NewThread(void *pv){
    struct thread_args *pta = (struct thread_args *)pv;
    printf("Calling TID %lu.\n",pta->tid);
//    准备输入参数
    PrepareArgsIn(pta->pct,pta->pcp);
    if(pta->pcp->Run() == SUCCESS){
//        处理输出参数
        GetArgsOut(pta->pct,pta->pcp);
        pta->rtn = SUCCESS;
    }
    else{
        pta->rtn = FAIL;
    }
    CThread::ChildThreadFSH(pta);
    pthread_exit(pv);
}

void CThread::PrepareArgsIn(CThread *pct,CPart *pcp){
//    读入实际计算进程参数列表中的输入参数
    vector<void *> args = pct->rargs.find(pcp->name)->second;
//    获得输入参数格式
    vector<int> fargs = pcp->fargs_in;
//    清空历史数据
    pcp->Clear();
//    传入输入参数
    int cout = 0;
    for(auto arg = args.begin(); arg != args.end(); arg++,cout++){
        if(fargs[cout] == INT){
            pcp->addArgsIn<int>(*((int *)(*arg)));
        }
        else if(fargs[cout] == DOUBLE){
            pcp->addArgsIn<double>(*((double *)(*arg)));
        }
    }
}


void CThread::GetArgsOut(CThread *pct,CPart *pcp){
//    获得输出参数格式
    vector<int> fargs_out = pcp->fargs_out;
//    获得计算模块中的输出参数列表
    vector<void *> &argso = pcp->args_out;
//    获得实际计算进程输出参数储存列表
    vector<void *> &args_out = pct->rargs_out.find(pcp->name)->second;
    
//    处理输出
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

//设置全局线程时钟
void setThreadsClock(void){
    itrl.it_interval.tv_sec = 0;
    itrl.it_interval.tv_usec = 500000;
    itrl.it_value.tv_sec = 0;
    itrl.it_value.tv_usec = 500000;
    setitimer(ITIMER_REAL, &itrl, &oitrl);
}
//时钟滴答调用函数
void threadsClock(int n){
    printf("Clock click.\n");
}
