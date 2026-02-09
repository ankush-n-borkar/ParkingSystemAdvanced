#ifndef FLOOR_H
#define FLOOR_H

#include "graph.h"

/* Floor structure */
typedef struct {
    int floor_number;
    Graph *graph;
    
    int entry_node_id;
    int exit_node_id;
    
    int total_slots;
    int occupied_slots;
    
    char name[50];
} Floor;

/* Floor operations */
Floor* floor_create(int floor_number, const char *name);
void floor_destroy(Floor *floor);
int floor_load_from_config(Floor *floor, const char *config_file);
void floor_update_slot_counts(Floor *floor);
float floor_get_occupancy_rate(Floor *floor);
void floor_print_status(Floor *floor);
int floor_occupy_slot(Floor *floor, const char *slot_id, const char *vehicle_number);
int floor_free_slot(Floor *floor, const char *slot_id);

#endif
