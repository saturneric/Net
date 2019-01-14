//
//  net.hpp
//  Net
//
//  Created by 胡一兵 on 2019/1/13.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef net_hpp
#define net_hpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <string>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "cpart.h"

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::list;
using std::ifstream;
using std::cout;
using std::endl;

class Addr{
public:
    struct sockaddr_in address;
    socklen_t len;
    Addr(string ip_addr, int port = 0, bool ipv4 = true){
        memset(&address, 0, sizeof(struct sockaddr_in));
        if(ipv4)
            address.sin_family = AF_INET;
        else
            address.sin_family = AF_INET6;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = inet_addr(ip_addr.data());
        len = sizeof(address);
    }
    Addr(struct sockaddr_in saddri){
        memset(&address, 0, sizeof(struct sockaddr_in));
        address = saddri;
    }
    Addr(){
        memset(&address, 0, sizeof(struct sockaddr_in));
    }
    Addr(const Addr &t_addr){
        address = t_addr.address;
        len = t_addr.len;
    }
    socklen_t *sizep(void){
        return &len;
    }
    socklen_t size(void){
        return len;
    }
    void setSize(void){
        len = sizeof(address);
    }
    struct sockaddr *obj(void){
        return (struct sockaddr *) &address;
    }
};

class Socket{
public:
    struct sockaddr_in c_addr;
    Addr addr;
    int nsfd,sfd,port;
    bool server,tcp,ipv4;
    void (*func)(class Socket &,int ,Addr);
    Socket(string ip_addr, int port, bool server = false, bool tcp = true, bool ipv4 = true){
        if(ipv4)
            addr.address.sin_family = AF_INET;
        else
            addr.address.sin_family = AF_INET6;
        addr.address.sin_port = htons(port);
        this->port = port;
        addr.address.sin_addr.s_addr = inet_addr(ip_addr.data());
        addr.setSize();
        this->server = server;
        this->tcp = tcp;
        this->ipv4 = ipv4;
        int TAU = SOCK_STREAM;
        if(!tcp) TAU = SOCK_DGRAM;
        //如果是服务端
        if(server){
            if(ipv4) sfd = socket(AF_INET,TAU,0);
            else sfd = socket(AF_INET6,TAU,0);
            if(!~sfd) throw "fail to get sfd";
            if(!~bind(sfd, addr.obj(), addr.size())) throw "fail to bind";
        }
        else{
            if(ipv4) sfd = socket(PF_INET,TAU,0);
            else sfd = socket(PF_INET6,TAU,0);
            if(tcp && !~connect(sfd,addr.obj(),addr.size()))
                throw "connection fail";
            
        }
    }
    
    ~Socket(){
        close(sfd);
    }
    
    void Listen(int connection, void (*func)(class Socket &,int ,Addr) = NULL){
        if(server && tcp){
            listen(sfd, 10);
            this->func = func;
        }
    }
    
    void Accept(void){
        if(server && tcp){
            socklen_t scaddr = sizeof(struct sockaddr);
            nsfd = accept(sfd,(struct sockaddr *) &c_addr, &scaddr);
            Addr addr(c_addr);
            if(~nsfd){
                if(func != NULL) func(*this,nsfd,addr);
                close(nsfd);
            }
        }
    }
    
    void Send(int t_nsfd, string buff){
        if(tcp){
            ssize_t len = send(t_nsfd,buff.data(),buff.size(),0);
            if(len != buff.size()) throw "size unmatch";
        }
    }
    
    void PacketSend(string buff){
        if(!tcp)
            sendto(sfd, buff.data(), buff.size(), 0, addr.obj(), addr.size());
    }
    
    string PacketRecv(Addr t_addr){
        if(!tcp){
            char buff[BUFSIZ];
            ssize_t tlen = recvfrom(sfd, (void *)buff, BUFSIZ, 0, t_addr.obj(), t_addr.sizep());
            if(tlen > 0){
                buff[tlen] = '\0';
                string str = buff;
                return str;
            }
            else throw "packet receive fail";
        }
        throw "connection is tcp";
    }
    
    string Recv(int t_nsfd){
        if(tcp){
            char buff[BUFSIZ];
            ssize_t len=recv(t_nsfd,buff,BUFSIZ,0);
            if(len > 0){
                buff[len] = '\0';
                string str = buff;
                return str;
            }
            else throw "receive fail";
        }
        throw "connection is udp";
    }
};

class CMap{
public:
    map<string,CPart *> cparts;
    string path;
    CMap(string path){
        ifstream map;
        map.open(path+"/pcs.map");
        this->path = path;
        if(map.good()){
            string line;
            map>>line;
            if(line == "#PCS"){
                BuildCPart(map);
            }
            map>>line;
            if(line == "#DEPEND"){
                BuildConnection(map);
            }
            
        }
    }
    void BuildCPart(ifstream &map){
        string line;
        map>>line;
        while(line!="END"){
            unsigned long qs = line.find('(',0), qe = line.find(')',0);
            string name = line.substr(qs+1,qe-qs-1);
            qs = line.find('<',0);
            qe = line.find('>',0);
            string src_name = line.substr(qs+1,qe-qs-1);
            
            //std::cout<<name<<" "<<src_name<<std::endl;
            CPart *ncp = new CPart(path,src_name,name);
            this->cparts.insert(pair<string,CPart *>(name,ncp));
            
            vector<int>fargs_in,fargs_out;
            map>>line;
            if(line[0] == '>'){
                fargs_in = BuidArgs(line);
            }
            map>>line;
            if(line[0] == '<'){
                fargs_out = BuidArgs(line);
            }
            ncp->setArgsType(fargs_in, fargs_out);
            map>>line;
        }
    }
    
    vector<int> BuidArgs(string &line){
        vector<int> fargs;
        unsigned long qs,qe;
        qs = line.find('[',0);
        qe = line.find(']',0);
        unsigned long idx = qs;
        while(idx < qe){
            unsigned long ts,te;
            ts = line.find(':',idx);
            te = line.find(',',ts);
            if(te == string::npos){
                te = qe;
            }
            string type = line.substr(ts+1,te-ts-1);
            if(type == "int")
                fargs.push_back(INT);
            else if(type == "double")
                fargs.push_back(DOUBLE);
            
            idx = te;
        }
        return fargs;
    }
    
    void BuildConnection(ifstream &map){
        string line;
        map>>line;
        while(line != "END"){
            unsigned long qs = line.find('(',0), qe = line.find(')',0);
            string name = line.substr(qs+1,qe-qs-1);
            CPart *p_ncp = cparts.find(name)->second;
            unsigned long idx = line.find('{',qe);
            unsigned long ss = line.find('}',idx);
            unsigned long ts, te;
            while(idx+1 < ss){
                ts = line.find('(',idx);
                te = line.find(']',ts);
                if(te == string::npos){
                    te = ss;
                }
                string item = line.substr(ts,te-ts+1);
                //cout<<item<<endl;
                idx = te+1;
                p_ncp->depends.push_back(ReadItem(item));
            }
            map>>line;
        }
        
    }
    
    Depends ReadItem(string item){
        Depends dep;
        unsigned long qs = item.find('(',0), qe = item.find(')',qs);
        string name = item.substr(qs+1,qe-qs-1);
        dep.t_cpart = cparts.find(name)->second;
        unsigned long idx = item.find('[',0),ss = item.find(']',idx);
        unsigned long ts, te;
        while(idx < ss){
            ts = idx;
            te = item.find(',',ts+1);
            if(te == string::npos){
                te = ss;
            }
            string arg = item.substr(ts+1,te-ts-1);
            std::stringstream sstr;
            int darg;
            sstr<<arg;
            sstr>>darg;
            dep.args.push_back(darg);
            idx = te;
        }
        return dep;
    }
    
};

class CThread{
public:
    CMap *p_map;
    list<CPart *> line;
    map<string,vector<void *>> rargs;
    map<string,vector<void *>> rargs_out;
    map<string,bool> ifsolved;
    CThread(CMap *tp_map):p_map(tp_map){
        for(auto k = p_map->cparts.begin(); k != p_map->cparts.end(); k++){
            vector<void *> args,args_out;
            rargs.insert(pair<string,vector<void *>>((*k).first,args));
            rargs_out.insert(pair<string,vector<void *>>((*k).first,args_out));
        }
        for(auto k = p_map->cparts.begin(); k != p_map->cparts.end(); k++){
            ifsolved.insert(pair<string,bool>((*k).first,false));
        }
    }
    template<class T>
    void AddArgs(string name, T value){
        auto k = rargs.find(name);
        T *p_value = new T();
        *p_value = value;
        (*k).second.push_back((void *)p_value);
    }
    void Analyse(void){
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
                        cout<<"ADD "<<k->second->name<<endl;
                    }
                }
                
            }
        }
    }
    void DoLine(void){
        for(auto pcp = line.begin(); pcp != line.end(); pcp++){
            string name = (*pcp)->name;
            
            cout<<(*pcp)->name<<" "<<(*pcp)->libname<<endl;
            
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
};

struct pcs_result{
    char *name[16];
    uint32_t in_size;
    char *in_buff;
    uint32_t out_size;
    char *out_buff;
};

#endif /* net_hpp */
