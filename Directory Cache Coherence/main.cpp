//
//  main.cpp
//  Directory Cache Coherence
//
//  Created by Joseph Kalash on 4/23/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <vector>
#include "mpi.h"

#include "Helper.h"
#include "Processor.h"

#define FILE_PATH "ENTER_DATA_FILE_PATH"

int main(int argc, char * argv[]) {
    
    /*
     Initialize MPI.
     */
    int ierr;
    ierr = MPI_Init (&argc, &argv);
    
    /*
     Get the number of processes.
     */
    int n_processes;
    ierr = MPI_Comm_size ( MPI_COMM_WORLD, &n_processes);
    
    /*
     Get the individual process ID.
     */
    int id;
    ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );
    
    
    //Read instructions from text file
    std::vector<RWInstruction> instructions = Helper::readIntructionsFile(FILE_PATH);
        
    //Create (Quad-core) processors
    //For each processor, pass the set of instructions for it to extract the appropriate set from them
    Processor p(id, instructions);

    //All wait until everything setup
    MPI_Barrier(MPI_COMM_WORLD);
    
    int priority = 0;
    int loopsExecuted = 0;
    
    double start = MPI_Wtime();
    
    /*
     1. Processor initially Idle
     2. Figure out what request to make.
     3. Based on Priority -> Listen for message or perform an action
     4. If did request -> Increment instructionsProcessed & Go back to idle
     5. Barrier to synchronize all processes
     */
    while (loopsExecuted < instructions.size()) {
        
        if (priority == id)
            p.doSomething(priority);
        else p.listenForMessage(priority);
        
        priority = ++priority % n_processes;
        loopsExecuted++;
        
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    double end = MPI_Wtime();
    
    /* Root only */
    if(id == 0)
        cout << "DONE! Time: " << end - start << "s. Processed " << loopsExecuted << " instructions and sent " << 4*loopsExecuted << " messages.\n";
    
    MPI_Finalize();
    
    return 0;
}
