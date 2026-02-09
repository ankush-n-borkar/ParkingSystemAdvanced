#ifndef PARKING_SYSTEM_H
#define PARKING_SYSTEM_H

#include "floor.h"

#define MAX_FLOORS 5

/* Parking system structure */
typedef struct {
    Floor *floors[MAX_FLOORS];
    int floor_count;
    
    int total_slots;
    int total_occupied;
    
    time_t system_start_time;
    int vehicles_parked_today;
    int vehicles_exited_today;
} ParkingSystem;

/* System operations */
ParkingSystem* parking_system_create();
void parking_system_destroy(ParkingSystem *ps);
int parking_system_add_floor(ParkingSystem *ps, const char *name, const char *config_file);
void parking_system_update_stats(ParkingSystem *ps);
void parking_system_print_overview(ParkingSystem *ps);
Floor* parking_system_get_floor(ParkingSystem *ps, int floor_number);

#endif
