//
//  MemoryBlock.h
//  2
//
//  Created by Joseph Kalash on 4/26/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#ifndef MemoryBlock_h
#define MemoryBlock_h

#include <string>

class MemBlock {
public:
    typedef enum : int {
        Invalidated,
        Shared,
        Exclusive
    } MemState;
    
    typedef unsigned int MemAddress;
private:
    MemAddress address;
    MemState state;
    
    //char* data;
    
    MemAddress tagFromAddress(MemAddress address);
    
public:
    MemBlock();
    MemBlock(int address);
    MemAddress getFullAddress();
    MemAddress getBlockAddress();
    MemAddress getBlockIndex();
    MemAddress getBlockTag();
    MemState getState();
    void setState(MemState newState);
    
    void setBlockAddress(MemAddress address);
    
    bool isHit(MemAddress address);
    
    std::string stateName();
    
    static MemAddress getIndexFromAddress(MemAddress address);
};



#endif /* MemoryBlock_h */
