//
//  Helper.hpp
//  2
//
//  Created by Joseph Kalash on 3/6/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#ifndef Helper_h
#define Helper_h

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "Constants.h"
#include "Processor.h"

//Collection of methods useful to our simulation implementation

class Helper {
public:
    static std::vector<RWInstruction> readIntructionsFile(std::string filePath);
    static bool taskComplete(std::vector<Processor>& processors);
    static bool isProcessorPriority(int cpuid, const std::vector<Processor> processors);
};

#endif /* Helper_hpp */
