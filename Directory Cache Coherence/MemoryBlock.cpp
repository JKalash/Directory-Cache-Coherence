//
//  MemoryBlock.cpp
//  2
//
//  Created by Joseph Kalash on 4/26/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#include "MemoryBlock.h"
#include "Constants.h"
#include <math.h>
#include <cstdlib>

Address MemBlock::tagFromAddress(MemAddress address) {
    MemAddress copy = address;
    return copy >> (BLOCK_BYTE_OFFSET_SIZE + BLOCK_INDEX_SIZE);
}

std::string MemBlock::stateName() {
    switch(state) {
        case Invalidated: return "Invalid";
        case Shared: return "Shared";
        case Exclusive: return "Exclusive";
    }
    return "";
}

bool MemBlock::isHit(MemAddress address) {
    const MemAddress tag = this->tagFromAddress(address);
    return tag == this->getBlockTag() && this->state != Invalidated;
}

MemBlock::MemBlock() {
    this->state = Invalidated;
    this->address = 0x0;
}

MemBlock::MemBlock(int address) {
    MemBlock();
    this->address = address;
}

Address MemBlock::getFullAddress() {return address;}
Address MemBlock::getBlockAddress() { MemAddress copy = address; return copy >> N_SET_ASSOCIATIVE;}
Address MemBlock::getBlockIndex() { return this->getIndexFromAddress(address); }
Address MemBlock::getBlockTag() {return tagFromAddress(this->address);}
MemBlock::MemState MemBlock::getState() {return this->state;}
void MemBlock::setState(MemState newState) {this->state = newState;}

void MemBlock::setBlockAddress(MemAddress address) {this->address = address;}

Address MemBlock::getIndexFromAddress(MemAddress address) { return (address << (BLOCK_TAG_SIZE)) >> (BLOCK_BYTE_OFFSET_SIZE + BLOCK_TAG_SIZE); }