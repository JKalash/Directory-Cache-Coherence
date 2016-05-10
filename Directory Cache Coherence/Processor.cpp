//
//  Processor.cpp
//  2
//
//  Created by Joseph Kalash on 2/29/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#include "Processor.h"
#include "Constants.h"
#include <math.h>
#include <cstdlib>

using namespace std;

Processor::Processor(int cpuid, std::vector<RWInstruction> instructions) {
    this->state = Idle;
    this->cpuid = cpuid;
    this->instructionsProcessed = 0;
    
    for(int i = 0; i < K_SETS; ++i) {
        vector<MemBlock> set(N_SET_ASSOCIATIVE, MemBlock());
        this->cache.push_back(set);
    }
    
    //Extract all instructions that need to be executed by this processor
    for(int i = 0; i < instructions.size(); ++i)
        if(instructions[i].cpuid == this->cpuid)
            this->instructionsToExecute.push_back(instructions[i]);
}

//Returns NULL when it's a MISS.
vector<MemBlock> Processor::blockSetForAddress(Address address) {
    
    Address index = MemBlock::getIndexFromAddress(address);
    return this->cache[index];
}

bool Processor::isHit(Address address) {
    
    Address copy = address;
    Address addressTag = copy >> (BLOCK_BYTE_OFFSET_SIZE + BLOCK_INDEX_SIZE);
    vector<MemBlock> set = blockSetForAddress(address);
    
    for(int i = 0; i < set.size(); ++i) {
        if (set[i].getBlockTag() == addressTag)
            return true;
    }
    
    return false;
}

bool Processor::taskComplete() {
    return instructionsProcessed >= instructionsToExecute.size();
}

void Processor::doSomething(int root) {
    if(instructionsProcessed == this->instructionsToExecute.size()) {
        broadcast(None, 0x0, 0x0);
        return;
    }
    
    this->state = Executing;
    RWInstruction nextInstruction = this->instructionsToExecute[instructionsProcessed];
    if(nextInstruction.isRead)
        this->read(nextInstruction.address);
    else this->write(nextInstruction.address, 0x0);
    this->instructionsProcessed++;
    this->state = Idle;
}

void Processor::broadcast(MessageType t, Address a, int iterator) {
    
    /* BCast ONLY on the first iteration */
    if (iterator > 0) return;
    
    vector<unsigned int> dataToSend;
    dataToSend.push_back(t);
    dataToSend.push_back(a);
    
    /* Send to all other processes
     Do not use Bcast as there is no guarantee that this method will be called before
     Other processed corresponding call to a Bcast */
    for(int i = 0 ; i < 4; i++)
        if(i != cpuid)
            MPI_Send(&dataToSend[0], 2, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);
}

void Processor::listenForMessage(int root) {
    
    /*
     Our Message is a struct holding an int (message type enum) and an address (unsigned long)
     use a vector to send and recv a message
     */
    
    vector<unsigned int> dataToRecv;
    dataToRecv.resize(2);
    MPI_Status status;
    MPI_Recv(&dataToRecv[0], 2, MPI_UNSIGNED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    int count = 0;
    MPI_Get_count(&status, MPI_INT, &count);
    if (count < 2) return;
    
    MessageType type = static_cast<MessageType>(dataToRecv[0]);
    Address address = dataToRecv[1];
    
    //Perform necessary action based on received message
    if (address == -1 || type == None) return;
    
    //Start by going to the proper cache -> Index
    Address index = MemBlock::getIndexFromAddress(address);
    
    string msgType = "READ MISS";
    if(type == WriteMiss) msgType = "WRITE MISS";
    else if(type == Invalidate) msgType = "INVALIDATE";
    
    criticalPrint("CPU " + std::to_string(this->cpuid) + " received a " + msgType + ". ");
    
    for(int i = 0; i < cache[index].size(); ++i) {
            
        string oldState = cache[index][i].stateName();
        string newState = "";
        
        if (!cache[index][i].isHit(address)) {
            //Invalid case. if Read Miss -> Become shared else become exlusive
                
            if(type == ReadMiss)
                cache[index][i].setState(MemBlock::Shared);
                
            else cache[index][i].setState(MemBlock::Exclusive);
        }
            
        else if (cache[index][i].getState() == MemBlock::Shared) {
            //Block is shared.
            if(type == ReadMiss) {
                //Still Shared Do nothing
            }
            //Write Miss on shared block. Need to send invalidate to all sharers and write back block. Becomes exclusive
            else cache[index][i].setState(MemBlock::Exclusive);
            
            broadcast(None, address, i);
        }
            
        else {
            //Block is exclusive
            if(type == ReadMiss)
                cache[index][i].setState(MemBlock::Shared);
            else cache[index][i].setState(MemBlock::Exclusive);
        }
            
        newState = cache[index][i].stateName();
            
        
        criticalPrint(oldState + " block " + std::to_string(address) + " -> " +  newState + ". ");
            
        if(cache[index][i].isHit(address)) break;
    }
    
    criticalPrint("\n");
}

void Processor::read(int address) {
    
    //Start by going to the proper cache -> Index
    //Update my block to hold the same tag as the one I'm trying to write
    Address index = MemBlock::getIndexFromAddress(address);
    
    bool isHit = false;
    string oldName1 = this->cache[index][0].stateName();
    string oldName2 = this->cache[index][1].stateName();
    
    for(int i = 0; i < this->cache[index].size(); ++i) {
        
        if(!this->cache[index][i].isHit(address)) {
            //Read Miss
            switch (this->cache[index][i].getState()) {
                case MemBlock::Invalidated:
                    broadcast(ReadMiss, address, i);
                    this->cache[index][i].setState(MemBlock::Shared);
                    break;
                case MemBlock::Shared:
                    /* Do Nothing */
                    broadcast(None, address, i);
                    break;
                case MemBlock::Exclusive:
                    //**WB Cache block** (NOT NEEDED in this simulation)
                    broadcast(None, address, i);
                    this->cache[index][i].setState(MemBlock::Shared);
                    break;
            }
        
            //Update my block to hold the same tag as the one I'm trying to write
            this->cache[index][i].setBlockAddress(address);
            continue;
        }
    
        //Read Hit does not require anything to be done.
        isHit = true;

       
        criticalPrint("CPU " +  std::to_string(this->cpuid) + ": READ HIT on " + this->cache[index][i].stateName() + " block " + std::to_string(address) + ".\n");
        
        broadcast(None, address, i);
        break;
    }
    
    if(!isHit)
        criticalPrint("CPU " + std::to_string(this->cpuid) + ": READ MISS on " + std::to_string(address) + ". Old States: [" + oldName1 + "-"+ oldName2+ "]. New States: [" + this->cache[index][0].stateName() + "-" + this->cache[index][1].stateName() + "].\n");
}

void Processor::write(int address, char* data) {
    
    Address index = MemBlock::getIndexFromAddress(address);
    
    bool isHit = false;
    string oldName1 = this->cache[index][0].stateName();
    string oldName2 = this->cache[index][1].stateName();
    
    for(int i = 0; i < this->cache[index].size(); ++i) {
        
        if(!this->cache[index][i].isHit(address)) {
            //Write Miss
            switch (this->cache[index][i].getState()) {
                case MemBlock::Invalidated:
                    broadcast(WriteMiss, address, i);
                    this->cache[index][i].setState(MemBlock::Exclusive);
                    break;
                case MemBlock::Shared:
                    broadcast(WriteMiss, address, i);
                    this->cache[index][i].setState(MemBlock::Exclusive);
                    break;
                case MemBlock::Exclusive:
                    //**WB Cache block** (NOT NEEDED in this simulation)
                    broadcast(None, address, i);
                    break;
            }
            
            //Update my block to hold the same tag as the one I'm trying to write
            this->cache[index][i].setBlockAddress(address);
            
            continue;
        }
        
    
        //Write Hit. If shared, go to exclusive & send invalidate message else nothing to do
        isHit = true;
        string oldName = this->cache[index][i].stateName();
        if(this->cache[index][i].getState() == MemBlock::Shared) {
            this->cache[index][i].setState(MemBlock::Exclusive);
            broadcast(Invalidate, address, i);
        }
        else broadcast(None, address, i);

        /* CRITICAL PRINT */
        criticalPrint("CPU " +  std::to_string(this->cpuid) + ": WRITE HIT on " + this->cache[index][i].stateName() + " block " + std::to_string(address) + ".\n");
        break;
    }
    
    if(!isHit) {
        /* CRITICAL PRINT */
        criticalPrint("CPU " + std::to_string(this->cpuid) + ": WRITE MISS on " + std::to_string(address) + ". Old States: [" + oldName1 + "-"+ oldName2+ "]. New States: [" + this->cache[index][0].stateName() + "-" + this->cache[index][1].stateName() + "].\n");
    }
}

void Processor::criticalPrint(string msg) { cout << msg; }

