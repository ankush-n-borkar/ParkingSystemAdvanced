#include "parking_system.h"

ParkingSystem* parking_system_create() {
    ParkingSystem *ps = (ParkingSystem*)malloc(sizeof(ParkingSystem));
    if (!ps) return NULL;
    
    ps->floor_count = 0;
    ps->total_slots = 0;
    ps->total_occupied = 0;
    ps->system_start_time = time(NULL);
    ps->vehicles_parked_today = 0;
    ps->vehicles_exited_today = 0;
    
    for (int i = 0; i < MAX_FLOORS; i++) {
        ps->floors[i] = NULL;
    }
    
    return ps;
}

void parking_system_destroy(ParkingSystem *ps) {
    if (!ps) return;
    
    for (int i = 0; i < ps->floor_count; i++) {
        floor_destroy(ps->floors[i]);
    }
    
    free(ps);
}

int parking_system_add_floor(ParkingSystem *ps, const char *name, const char *config_file) {
    if (ps->floor_count >= MAX_FLOORS) {
        return FAILURE;
    }
    
    Floor *floor = floor_create(ps->floor_count, name);
    if (!floor) return FAILURE;
    
    if (floor_load_from_config(floor, config_file) == FAILURE) {
        floor_destroy(floor);
        return FAILURE;
    }
    
    ps->floors[ps->floor_count] = floor;
    ps->floor_count++;
    
    parking_system_update_stats(ps);
    
    return SUCCESS;
}

void parking_system_update_stats(ParkingSystem *ps) {
    ps->total_slots = 0;
    ps->total_occupied = 0;
    
    for (int i = 0; i < ps->floor_count; i++) {
        floor_update_slot_counts(ps->floors[i]);
        ps->total_slots += ps->floors[i]->total_slots;
        ps->total_occupied += ps->floors[i]->occupied_slots;
    }
}

void parking_system_print_overview(ParkingSystem *ps) {
    printf("\n");
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║   MULTI-FLOOR PARKING MANAGEMENT SYSTEM       ║\n");
    printf("╚════════════════════════════════════════════════╝\n\n");
    
    printf("System Overview:\n");
    printf("  Total Floors: %d\n", ps->floor_count);
    printf("  Total Slots: %d\n", ps->total_slots);
    printf("  Occupied: %d\n", ps->total_occupied);
    printf("  Available: %d\n", ps->total_slots - ps->total_occupied);
    printf("  Occupancy Rate: %.1f%%\n", 
           (ps->total_slots > 0) ? (float)ps->total_occupied / ps->total_slots * 100.0 : 0.0);
    
    printf("\n");
    for (int i = 0; i < ps->floor_count; i++) {
        floor_print_status(ps->floors[i]);
    }
    printf("\n");
}

Floor* parking_system_get_floor(ParkingSystem *ps, int floor_number) {
    if (floor_number < 0 || floor_number >= ps->floor_count) {
        return NULL;
    }
    return ps->floors[floor_number];
}
