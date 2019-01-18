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
    map<void *, block_info> blocks_list;
public:
    void *b_malloc(uint32_t size){
        void *ptr = malloc(size);
        if(ptr == nullptr) return nullptr;
        blocks_list.insert({ptr,{size,1}});
        return ptr;
    }
    void *b_get(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.lock++;
            return ptr;
        }
        else return nullptr;
    }
    void b_protect(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.pted = true;
        }
        else throw "protect nil value";
    }
    void b_noprotect(void *ptr){
        auto blk = blocks_list.find(ptr);
        if(blk != blocks_list.end()){
            blk->second.pted = false;
        }
        else throw "noprotect nil value";
    }
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
