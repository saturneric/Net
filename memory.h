//
//  memory.h
//  Net
//
//  Created by 胡一兵 on 2019/1/18.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef memory_h
#define memory_h

#include "type.h"



struct block_info{
    uint32_t size = 0;
    int lock = 0;
    bool pted = false;
};

class BlocksPool{
//    内存块表
    map<void *, block_info> blocks_list;
public:
//    声明某内存块
    void *b_malloc(uint32_t size){
        void *ptr = malloc(size);
        if(ptr == nullptr) return nullptr;
        blocks_list.insert({ptr,{size,1}});
        return ptr;
    }
//    标记使用某内存块
    void *b_get(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.lock++;
            return ptr;
        }
        else return nullptr;
    }
//    标记保护某内存块
    void b_protect(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.pted = true;
        }
        else throw "protect nil value";
    }
//    标记不再保护某内存块
    void b_noprotect(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.pted = false;
        }
        else throw "noprotect nil value";
    }
//    标记不再使用某内存块
    void b_free(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end() && blk->second.pted == false){
            if(blk->second.lock - 1 == 0){
                free(blk->first);
                blocks_list.erase(blk);
            }
            else blk->second.lock--;
        }
    }
};

extern BlocksPool main_pool;

#endif /* memory_h */
