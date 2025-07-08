#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "warehouse-env.h"

#define WH_WIDTH 20
#define WH_HEIGHT 20
#define N_STATES WH_HEIGHT*WH_WIDTH*2*10 //length width carrying_flag battery
#define N_ACTIONS 5 //up down left right

void createWarehouse(Warehouse* wh, int width, int height){
    wh -> warehouse = malloc(sizeof(int)*width*height);
    memset(wh->warehouse, 0, sizeof(int)*width*height);
    wh -> width = width;
    wh -> height = height;
    wh -> robotCounter = 0;
};

void addPickupStation(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(wh, x, y)] = PICKUP_STATION;
};

void addDropStation(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(wh, x, y)] = DROP_STATION;
};

void addRechargeStation(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(wh, x, y)] = RECHARGE_STATION;
};

void addObstacle(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(wh, x, y)] = OBSTACLE;
};

void addRobot(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(wh, x, y)] = ROBOT;
    int rI = wh -> robotCounter;
    wh -> robots[rI].available = true;
    wh -> robots[rI].state.batteryLevel = 9;
    wh -> robots[rI].state.carryingBox = false;
    wh -> robots[rI].state.x = x;
    wh -> robots[rI].state.y = y;
    wh -> robotCounter++;
};

int getWhIndex(Warehouse* wh, int x, int y){
    return x + wh->width*y;
};

int* getWarehousePos(Warehouse* wh, int x, int y){
    return &wh->warehouse[getWhIndex(wh, x, y)];
};

void showWarehouse(Warehouse* wh){
    for(int i = 0; i < wh->height; i++){
        for(int j = 0; j < wh->width; j++){
            printf("%d ", wh->warehouse[getWhIndex(wh, j, i)]);
        }
        printf("\n");
    }
};

int getPolIndex(RobotState state){
    return (((state.x * WH_HEIGHT + state.y) * 2 + state.carryingBox) * 10 + state.batteryLevel) * N_ACTIONS;
}

float* createRandomPolicy(){
    float* rPolicy = malloc(sizeof(float)*N_STATES*N_ACTIONS);

    for(int i = 0; i < N_STATES*N_ACTIONS; i++){
        rPolicy[i] = 1.0f/(float)N_ACTIONS;
    }

    return rPolicy;
}

float* createQTable(){
    float* qtable = malloc(sizeof(float)*N_STATES*N_ACTIONS);
    memset(qtable, 0, sizeof(float)*N_STATES*N_ACTIONS);

    return qtable;
}

float* fixInvalidActions(Warehouse* wh, RobotState state, float* actions){
    float* validActions = malloc(sizeof(int)*4);
    int allActions[][2] = {{0, -1},{0, 1},{-1, 0},{1, 0}};

    for(int i = 0; i < 4; i++){
        int newX = state.x + allActions[i][0];
        int newY = state.y + allActions[i][1];

        validActions[i] = actions[i];

        if(newX < 0 || newX >= wh -> width){
            validActions[i] = 0;
        };
        if(newY < 0 || newY >= wh -> height){
            validActions[i] = 0;
        };
    }

    return validActions;
}

int weightedActionChoice(float* actions, int nA){
    int i = 0;
    float* weightedActions = malloc(sizeof(float)*nA);
    float total = 0;

    for(i = 0; i < nA; i++){
        weightedActions[i] = actions[i] * (float)(i+1);
        total += weightedActions[i];
    }

    float r = ((float)rand() / (float)RAND_MAX) * total;

    float cumulative = 0;
    for (int i = 0; i < nA; i++) {
        cumulative += weightedActions[i];
        if (r <= cumulative) {
            free(weightedActions);
            return i;
        }
    }

    free(weightedActions);
    return nA - 1;
}

Step* moveRobot(Warehouse* wh, int robotIndex, float* policy){
    int allActions[][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}, {0, 0}};
    int newX, newY;
    float reward;
    Step* step = malloc(sizeof(Step));
    step -> robotID = robotIndex;

    Robot* robot = &(wh -> robots[robotIndex]);

    step -> state.x = robot -> state.x;
    step -> state.y = robot -> state.y;
    step -> state.carryingBox = robot -> state.carryingBox;
    step -> state.batteryLevel = robot -> state.batteryLevel;
    step->next = NULL;

    if(robot -> state.batteryLevel == 0){
        step->reward = -10;
        step->next_state.x = robot->state.x;
        step->next_state.y = robot->state.y;
        step->next_state.carryingBox = false; //if battery runs out, loses the box
        step->next_state.batteryLevel = 9;
        step->action = 4;
        robot -> state.batteryLevel = 9;
        robot -> available = false;
    }

    else if(robot -> available){
        float* actions = &policy[getPolIndex(robot -> state)];
        actions = fixInvalidActions(wh, robot -> state, actions);

        int actionTaken = weightedActionChoice(actions, 4);
        step->action = actionTaken;
        free(actions);

        newX = robot -> state.x + allActions[actionTaken][0];
        newY = robot -> state.y + allActions[actionTaken][1];
    
        int newPos = wh->warehouse[getWhIndex(wh, newX, newY)];

        if(rand()%10 == 0){ //10% prob decrease battery at each move
            robot -> state.batteryLevel--;
        }

        if(newPos == OBSTACLE || newPos == ROBOT){ //cant move to an obstacle or other robot
            step->reward = -1;
            step->next_state.x = robot->state.x;
            step->next_state.y = robot->state.y;
            step->next_state.carryingBox = robot->state.carryingBox;
            step->next_state.batteryLevel = robot->state.batteryLevel;

        } else if(newPos == RECHARGE_STATION){
            step->reward = (4 - robot -> state.batteryLevel)/2;

            robot -> available = false;
            // robot -> state.x = newX; 
            // robot -> state.y = newY;
            robot -> state.batteryLevel = 9;

        } else if(newPos == PICKUP_STATION && robot -> state.carryingBox == false){ //pickup box
            step->reward = 1;

            robot -> available = false;
            // robot -> state.x = newX; 
            // robot -> state.y = newY;
            robot -> state.carryingBox = false;
            
        } else if(newPos == DROP_STATION && robot -> state.carryingBox == true){ //drops box
            step->reward = 3;

            robot -> available = false;
            // robot -> state.x = newX; 
            // robot -> state.y = newY;
            robot -> state.carryingBox = false; 
        } else { //when goes to drop_station without box or pickup_station with box or when walking in path
            step->reward = -1;

            wh->warehouse[getWhIndex(wh, robot->state.x, robot->state.y)] = 0;

            robot -> state.x = newX; 
            robot -> state.y = newY;

            wh->warehouse[getWhIndex(wh, newX, newY)] = 2;
        }
    } else {
        step->reward = -1;
        robot -> available = true;
    }

    step->next_state.x = robot->state.x;
    step->next_state.y = robot->state.y;
    step->next_state.carryingBox = robot->state.carryingBox;
    step->next_state.batteryLevel = robot->state.batteryLevel;
    return step;
}

void showStep(Step* step){
    Step* tempStep = step;
    while(tempStep){
        printf("id:%d x:%d y:%d cb:%d b:%d nx:%d ny:%d ncb:%d nb:%d act:%d r:%f\n",tempStep->robotID,
                                                  tempStep->state.x,tempStep->state.y,tempStep->state.carryingBox,tempStep->state.batteryLevel,
                                                  tempStep->next_state.x,tempStep->next_state.y,tempStep->next_state.carryingBox,tempStep->next_state.batteryLevel,
                                                  tempStep->action,
                                                  tempStep->reward
              );


        tempStep = tempStep -> next;
    }
    free(step);
}

Step* generateSteps(Warehouse* wh, float* pol, int n) {
    if (wh->robotCounter > 0) {
        Step* head = moveRobot(wh, 0, pol);
        Step* current = head;
        
        for (int i = 1; i < n; i++) {
            current->next = moveRobot(wh, i%wh->robotCounter, pol);
            current = current->next;
        }

        return head;
    }

    return NULL;
}

int main(){
    // RobotState s;
    // s.x=19; s.y=19; s.carryingBox=1; s.batteryLevel=9;
    // int a = getPolIndex(s);
    // printf("%d\n",a);

    // float* pol = createRandomPolicy();
    // int i;

    // Warehouse wh;
    // createWarehouse(&wh, 20, 20);
    // addPickupStation(&wh, 0, 1);
    // addDropStation(&wh, 17, 19);

    // for(i = 0; i < 5; i++){
    //     addRobot(&wh, 0, 7+i);
    // }

    // Step* episode = generateSteps(&wh, pol, 10000);
    // showStep(episode);

    // showWarehouse(&wh);

    // free(pol);
    // free(wh.warehouse);
    // free(&wh);
}