#include <stdio.h>
#include "core/parking_system.h"
#include "server/http_server.h"
#include "io/file_manager.h"
#include "utils/utils.h"

#define STATE_FILE "data/parking_state.txt"

int main() {
    print_header("MULTI-FLOOR PARKING SYSTEM - SERVER MODE");
    
    printf("Initializing system...\n\n");
    
    /* Create parking system */
    ParkingSystem *ps = parking_system_create();
    if (!ps) {
        print_error("Failed to create parking system");
        return 1;
    }
    
    /* Load floors */
    print_info("Loading Floor 0 (Ground Floor)...");
    if (parking_system_add_floor(ps, "Ground Floor", "config/floor0.config") == SUCCESS) {
        print_success("Floor 0 loaded");
    }
    
    print_info("Loading Floor 1 (First Floor)...");
    if (parking_system_add_floor(ps, "First Floor", "config/floor1.config") == SUCCESS) {
        print_success("Floor 1 loaded");
    }
    
    /* Load saved state */
    print_info("Loading saved state...");
    if (load_parking_state(ps, STATE_FILE) == SUCCESS) {
        print_success("State restored");
    } else {
        print_info("Starting with fresh state");
    }
    
    /* Show current status */
    printf("\n");
    parking_system_print_overview(ps);
    
    /* Start HTTP server */
    HTTPServer *server = http_server_create(ps);
    if (!server) {
        print_error("Failed to create HTTP server");
        parking_system_destroy(ps);
        return 1;
    }
    
    http_server_start(server);
    
    /* Cleanup */
    http_server_stop(server);
    parking_system_destroy(ps);
    
    return 0;
}
