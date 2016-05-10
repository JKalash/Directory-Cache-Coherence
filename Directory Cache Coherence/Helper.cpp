//
//  Helper.cpp
//  2
//
//  Created by Joseph Kalash on 3/6/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#include "Helper.h"

std::vector<RWInstruction> Helper::readIntructionsFile(std::string filePath) {
        
    std::ifstream textfile(filePath);
    std::vector<RWInstruction> instructions;
    
    int cpuid; Address address; std::string read;
        
    while(textfile >> cpuid >> address >> read) {
        
        RWInstruction instruction = RWInstruction();
        
        instruction.address = address;
        instruction.cpuid = cpuid;
        if (read == "r")
            instruction.isRead = true;
        else instruction.isRead = false;
        
        instructions.push_back(instruction);
    }
    
    return instructions;
}

bool Helper::taskComplete(std::vector<Processor>& processors) {
    
    //Remove any Processor whose task is complete
    for(int i = 0; i < processors.size(); i++)
        if(processors[i].taskComplete())
            processors.erase(processors.begin()+i);
    
    return  processors.size() == 0;
}

bool Helper::isProcessorPriority(int cpuid, const std::vector<Processor> processors) {
    
    //If ANY of the CPUs is not IDLE, return false
    //This will prevent two CPUs from being counted as highest priorty
    //If there is a big delay between one thread checking then doing something before another thread
    //Even starts checking
    for(int i = 0; i < processors.size(); ++i)
        if(processors[i].state != Processor::Idle)
            return false;
    
    //Figure out the minimum number of instructions processed amond all processors
    int minInstructionsProcessed = INT_MAX;
    for(int i = 0; i < processors.size(); ++i)
        if(processors[i].instructionsProcessed < minInstructionsProcessed)
            minInstructionsProcessed = processors[i].instructionsProcessed;
    
    
    int highestprioritycpu = 0;
    //The highest priority CPU is the first one we encounter who
    //1. Task not complete
    //2. number of instructions processed <= min instructions processed
    
    while(processors[highestprioritycpu].instructionsProcessed > minInstructionsProcessed && highestprioritycpu < (processors.size()-1))
        highestprioritycpu++;
    
    return  cpuid == processors[highestprioritycpu].cpuid;
}