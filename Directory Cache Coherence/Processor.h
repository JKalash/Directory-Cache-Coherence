//
//  Processor.h
//  2
//
//  Created by Joseph Kalash on 3/2/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#ifndef Processor_h
#define Processor_h

#include <iostream>
#include <vector>
#include "mpi.h"

#include "Constants.h"
#include "MemoryBlock.h"

using namespace std;

class Processor {
public:
    typedef enum : int {
        Idle,
        Executing
    } ProcState;
private:
    //N-way set associative can be treated as a 2D array (k rows of N columns, k*n = total blocks)
    vector<vector<MemBlock>> cache;
    vector<MemBlock> blockSetForAddress(Address address);
    bool isHit(Address address);
    
    
    void criticalPrint(string msg);
public:
    vector<RWInstruction> instructionsToExecute;
    Processor(int cpuid, std::vector<RWInstruction> instructions);
    ProcState state;
    int cpuid;
    int instructionsProcessed;
    void read(int address);
    void write(int address, char* data);
    void listenForMessage(int root);
    void broadcast(MessageType t, Address a, int iteration);
    bool taskComplete();
    void doSomething(int root);
};

#endif /* Processor_h */
