#include "file_manager.h"
#include <stdio.h>

int save_parking_state(ParkingSystem *ps, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return FAILURE;
    }
    
    /* Write header */
    fprintf(fp, "# Parking System State\n");
    fprintf(fp, "# Generated: %s\n", get_timestamp());
    fprintf(fp, "FLOORS=%d\n", ps->floor_count);
    fprintf(fp, "\n");
    
    /* Write each floor's slot states */
    for (int f = 0; f < ps->floor_count; f++) {
        Floor *floor = ps->floors[f];
        Graph *g = floor->graph;
        
        fprintf(fp, "[FLOOR_%d]\n", f);
        
        for (int i = 0; i < g->node_count; i++) {
            Node *node = &g->nodes[i];
            if (node->type == NODE_SLOT) {
                fprintf(fp, "%s %d %s %d\n",
                    node->slot_id,
                    node->is_occupied,
                    node->vehicle_number,
                    node->usage_count
                );
            }
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp);
    return SUCCESS;
}

int load_parking_state(ParkingSystem *ps, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return FAILURE; /* File doesn't exist - not an error for first run */
    }
    
    char line[256];
    int current_floor = -1;
    
    while (fgets(line, sizeof(line), fp)) {
        /* Skip comments and empty lines */
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        
        /* Parse floor section */
        if (sscanf(line, "[FLOOR_%d]", &current_floor) == 1) {
            continue;
        }
        
        /* Parse slot state */
        if (current_floor >= 0 && current_floor < ps->floor_count) {
            char slot_id[20], vehicle_number[20];
            int is_occupied, usage_count;
            
            if (sscanf(line, "%s %d %s %d", 
                slot_id, &is_occupied, vehicle_number, &usage_count) == 4) {
                
                Floor *floor = ps->floors[current_floor];
                int node_id = graph_get_node_by_slot_id(floor->graph, slot_id);
                
                if (node_id >= 0) {
                    Node *node = graph_get_node(floor->graph, node_id);
                    node->is_occupied = is_occupied;
                    strcpy(node->vehicle_number, vehicle_number);
                    node->usage_count = usage_count;
                }
            }
        }
    }
    
    fclose(fp);
    
    /* Update all floor counts */
    for (int i = 0; i < ps->floor_count; i++) {
        floor_update_slot_counts(ps->floors[i]);
    }
    parking_system_update_stats(ps);
    
    return SUCCESS;
}

void log_transaction(const char *transaction_type, const char *slot_id, const char *vehicle_number) {
    FILE *fp = fopen("data/transactions.log", "a");
    if (!fp) {
        return;
    }
    
    fprintf(fp, "[%s] %s - Slot: %s, Vehicle: %s\n",
        get_timestamp(),
        transaction_type,
        slot_id,
        vehicle_number
    );
    
    fclose(fp);
}
