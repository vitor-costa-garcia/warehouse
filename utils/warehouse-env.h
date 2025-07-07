#ifndef WAREHOUSE_ENV_H
#define WAREHOUSE_ENV_H

#include <stdbool.h>

#define MAX_ROBOTS 10
#define GRID_WIDTH 20
#define GRID_HEIGHT 20

#define PATH 0
#define OBSTACLE 1
#define ROBOT 2
#define PICKUP_STATION 3
#define DROP_STATION 4
#define RECHARGE_STATION 5

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Action; //Possible actions

typedef struct {
    int x;            // [0 - (GRID_WIDTH - 1)]
    int y;            // [0 - (GRID_HEIGHT - 1)]
    int batteryLevel; // [0 - 9]
    bool carryingBox; // [0, 1]
} RobotState; //Describes robot state

typedef struct{
    bool available;
    RobotState state;
} Robot; //Describes robot. can be unavailable if picking up object or out of battery

typedef struct{
    int* warehouse;
    int width;
    int height;
    Robot robots[MAX_ROBOTS];
    int robotCounter;
} Warehouse; //Warehouse map

void createWarehouse(Warehouse* wh, int width, int height){};
int getWhIndex(){};
int* getWarehousePos(Warehouse* wh, int x, int y){};
void addPickupStation(Warehouse* wh, int x, int y){};
void addDropStation(Warehouse* wh, int x, int y){};
void addRobot(Warehouse* wh, int x, int y){};
void addRechargeStation(Warehouse* wh, int x, int y){};
void addObstacle(Warehouse wh, int x, int y){};

#endif
