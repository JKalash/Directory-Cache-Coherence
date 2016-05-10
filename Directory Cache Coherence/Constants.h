//
//  Constants.h
//  2
//
//  Created by Joseph Kalash on 4/26/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#ifndef Constants_h
#define Constants_h

#define MEM_TOTAL_CAPACITY 64*1024
#define N_SET_ASSOCIATIVE 2
#define ADDRESS_SIZE 32
#define MEM_BLOCK_SIZE 64
#define TOTAL_CACHE_BLOCKS (MEM_TOTAL_CAPACITY/MEM_BLOCK_SIZE)
#define K_SETS (TOTAL_CACHE_BLOCKS/N_SET_ASSOCIATIVE)
#define BLOCK_BYTE_OFFSET_SIZE (int)log2(MEM_BLOCK_SIZE)
#define BLOCK_INDEX_SIZE (int)log2(K_SETS)
#define BLOCK_TAG_SIZE ADDRESS_SIZE - BLOCK_INDEX_SIZE - BLOCK_BYTE_OFFSET_SIZE

#include "MemoryBlock.h"

typedef MemBlock::MemAddress Address;

typedef enum : unsigned int {
    None,
    ReadMiss,
    WriteMiss,
    Invalidate,
} MessageType;

//A Message needs to hold information about its type as well as the memory address we're dealing with
struct Message {
    MessageType type;
    Address address;
    Message() {
        address = -1;
    }
    Message(Address a, MessageType t) {
        address = a;
        type = t;
    }
};

struct RWInstruction {
    int cpuid;
    bool isRead;
    Address address;
};

#endif /* Constants_h */
