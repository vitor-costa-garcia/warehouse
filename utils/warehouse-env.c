#include <stdlib.h>
#include "warehouse-env.h"

#define WH_WIDTH 20
#define WH_LENGTH 20

void createWarehouse(Warehouse* wh, int width, int height){
    wh -> warehouse = malloc(sizeof(int)*width*height);
    wh -> width = width;
    wh -> height = height;
    wh -> robotCounter = 0;
};

void addPickupStation(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(x, y)] = PICKUP_STATION;
};

void addDropStation(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(x, y)] = DROP_STATION;
};

void addRechargeStation(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(x, y)] = RECHARGE_STATION;
};

void addObstacle(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(x, y)] = OBSTACLE;
};

void addRobot(Warehouse* wh, int x, int y){
    wh->warehouse[getWhIndex(x, y)] = ROBOT;
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
    return wh->warehouse[getWhIndex(wh, x, y)];
};

int getPolIndex(RobotState state){
    return 1;
}

int* fixInvalidActions(Warehouse* wh, RobotState state, int* actions){
    int* validActions = malloc(sizeof(int)*4);
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

void moveRobot(Warehouse* wh, int robotIndex, int* policy){
    int allActions[][2] = {{0, -1},{0, 1},{-1, 0},{1, 0}};
    int newX, newY;
    float reward;

    Robot robot = wh -> robots[robotIndex];
    if(robot.state.batteryLevel == 0){
        reward = -12;
        robot.available = false;
    }

    if(robot.available){
        int* actions = policy[getPolIndex(robot.state)];
        actions = fixInvalidActions(wh, robot.state, actions);

        int actionTaken = weightedActionChoice(actions, 4);
        free(actions);

        newX = robot.state.x + allActions[actionTaken][0];
        newY = robot.state.y + allActions[actionTaken][1];
        int* newPos = getWarehousePos(wh, newX, newY);

        if(*newPos == OBSTACLE || *newPos == ROBOT){
            reward = -1;

        } else if(*newPos == RECHARGE_STATION){
            reward = (5 - robot.state.batteryLevel)/2;

            robot.available = false;
            robot.state.x = newX; 
            robot.state.y = newY;
            robot.state.batteryLevel = 9; 
        } else if(*newPos == PICKUP_STATION && robot.state.carryingBox == false){
            reward = 1;

            robot.available = false;
            robot.state.x = newX; 
            robot.state.y = newY;
            robot.state.carryingBox = false; 
        } else if(*newPos == DROP_STATION && robot.state.carryingBox == true){
            reward = 3;

            robot.available = false;
            robot.state.x = newX; 
            robot.state.y = newY;
            robot.state.carryingBox = false; 
        } else { //when goes to drop_station without box or pickup_station with box or when walking in path
            reward = -1;

            robot.state.x = newX; 
            robot.state.y = newY;
        }

        if(rand()%10 < 2){ //20% prob decrease battery at each move
            robot.state.batteryLevel--;
        }
    } else {
        reward = -1;
        robot.available = true;
    }
}

int main(){

}