#include "utils/warehouse-env.c"
#include <stdlib.h>
#include <stdio.h>

void qLearningWarehouse(Warehouse* wh, float* qTable, float* policy, float a, float epsilon, unsigned int n){
    float* qTable = createQTable();
    Step* episode = generateSteps(wh, policy, n);
    Step* trash;

    while(episode){
        trash = episode;

        // Fill here

        episode = episode -> next;
        free(trash);
    }
}