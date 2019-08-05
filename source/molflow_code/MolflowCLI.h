//
// Created by pbahr on 30/07/2019.
//

#ifndef MOLFLOW_MOLFLOWCLI_H
#define MOLFLOW_MOLFLOWCLI_H


#include <Worker.h>

class MolflowCLI {
public:
    Worker* worker;

    MolflowCLI();
    int RunMolflow();
    void BuildPipe(double ratio, int steps);
};


#endif //MOLFLOW_MOLFLOWCLI_H
