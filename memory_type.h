//
//  memory_type.h
//  Net
//
//  Created by 胡一兵 on 2019/1/19.
//  Copyright © 2019年 Bakantu. All rights reserved.
//

#ifndef memory_type_h
#define memory_type_h

#include <stdint.h>

struct block_info{
    uint32_t size = 0;
    int lock = 0;
    bool pted = false;
    void *pvle = nullptr;
    block_info(uint32_t size,void *pvle){
        this->size = size;
        this->pvle = pvle;
    }
    block_info(){
        
    }
};

#endif /* memory_type_h */
