//
// Created by pbahr on 30/07/2019.
//

#include "MolflowCLI.h"
#include <iostream>
#include <Worker.h>
#include <MolflowGeometry.h>

MolflowCLI::MolflowCLI(){
    worker = new Worker();
}

int MolflowCLI::RunMolflow(){

    //MolflowGeometry* geom = worker->GetMolflowGeometry();
    //geom.LoadGEO();

    std::cout << "-- This is experimental CLI mode -- " << std::endl;
    int i;
    std::cout << "Please enter an integer value: ";
    std::cin >> i ;

    std::cout << "Worker init: "<< worker->moments.size() << std::endl;
    BuildPipe(10.0, i);

    return 0;
}

void MolflowCLI::BuildPipe(double ratio, int steps) {

    MolflowGeometry *geom = worker->GetMolflowGeometry();

    double R = 1.0;
    double L = ratio * R;
    int step = steps; //Quick Pipe

    std::ostringstream temp;
    temp << "PIPE" << L / R;
    geom->UpdateName(temp.str().c_str());

    //     ResetSimulation(false);
    worker->ResetStatsAndHits(0.05);


    try {
        geom->BuildPipe(L, R, 0, step);
        worker->CalcTotalOutgassing();
        //default values
        worker->wp.enableDecay = false;
        worker->wp.halfLife = 1;
        worker->wp.gasMass = 28;
        worker->ResetMoments();
    }
    catch (Error &e) {
        geom->Clear();
        return;
    }
    //worker.globalHitCache.globalHits.nbDesorbed = 0; //Already done by ResetWorkerStats
    //sprintf(tmp,"L|R %g",L/R);
    worker->SetCurrentFileName("");

    // Send to sub process
    worker->Reload();
    std::cout << "Pipe reloaded ";

}