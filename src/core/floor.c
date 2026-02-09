#include "floor.h"

Floor* floor_create(int floor_number, const char *name) {
    Floor *floor = (Floor*)malloc(sizeof(Floor));
    if (!floor) return NULL;
    
    floor->floor_number = floor_number;
    floor->graph = graph_create();
    floor->entry_node_id = -1;
    floor->exit_node_id = -1;
    floor->total_slots = 0;
    floor->occupied_slots = 0;
    strncpy(floor->name, name, 49);
    floor->name[49] = '\0';
    
    return floor;
}

void floor_destroy(Floor *floor) {
    if (floor) {
        graph_destroy(floor->graph);
        free(floor);
    }
}

int floor_load_from_config(Floor *floor, const char *config_file) {
    FILE *fp = fopen(config_file, "r");
    if (!fp) {
        printf("Error: Cannot open config file: %s\n", config_file);
        return FAILURE;
    }
    
    char line[MAX_STRING_LENGTH];
    int section = 0; /* 0=nodes, 1=edges */
    
    while (fgets(line, sizeof(line), fp)) {
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip empty lines and comments */
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }
        
        /* Section markers */
        if (strcmp(line, "[NODES]") == 0) {
            section = 0;
            continue;
        }
        if (strcmp(line, "[EDGES]") == 0) {
            section = 1;
            continue;
        }
        
        if (section == 0) {
            /* Parse node: type x y [slot_id] [slot_type] */
            char type_str[20], slot_id[20] = "", slot_type_str[20] = "";
            float x, y;
            int parsed = sscanf(line, "%s %f %f %s %s", 
                               type_str, &x, &y, slot_id, slot_type_str);
            
            if (parsed < 3) continue;
            
            NodeType type;
            if (strcmp(type_str, "ROAD") == 0) type = NODE_ROAD;
            else if (strcmp(type_str, "JUNCTION") == 0) type = NODE_JUNCTION;
            else if (strcmp(type_str, "SLOT") == 0) type = NODE_SLOT;
            else if (strcmp(type_str, "RAMP_UP") == 0) type = NODE_RAMP_UP;
            else if (strcmp(type_str, "RAMP_DOWN") == 0) type = NODE_RAMP_DOWN;
            else if (strcmp(type_str, "ENTRY") == 0) type = NODE_ENTRY;
            else if (strcmp(type_str, "EXIT") == 0) type = NODE_EXIT;
            else continue;
            
            int node_id = graph_add_node(floor->graph, floor->floor_number, type, x, y);
            Node *node = graph_get_node(floor->graph, node_id);
            
            if (type == NODE_SLOT && strlen(slot_id) > 0) {
                snprintf(node->slot_id, sizeof(node->slot_id), "F%d-%s", 
                        floor->floor_number, slot_id);
                
                if (strlen(slot_type_str) > 0) {
                    if (strcmp(slot_type_str, "COMPACT") == 0) 
                        node->slot_type = SLOT_COMPACT;
                    else if (strcmp(slot_type_str, "LARGE") == 0) 
                        node->slot_type = SLOT_LARGE;
                    else if (strcmp(slot_type_str, "DISABLED") == 0) 
                        node->slot_type = SLOT_DISABLED;
                }
                
                floor->total_slots++;
            }
            
            if (type == NODE_ENTRY) {
                floor->entry_node_id = node_id;
            }
            if (type == NODE_EXIT) {
                floor->exit_node_id = node_id;
            }
        }
        else if (section == 1) {
            /* Parse edge: from_id to_id weight [bidirectional] */
            /* NOTE: We IGNORE the weight from config and calculate Manhattan distance */
            int from, to, config_weight, bidirectional = 1;
            int parsed = sscanf(line, "%d %d %d %d", &from, &to, &config_weight, &bidirectional);
            
            if (parsed >= 3) {
                /* Get the two nodes */
                Node *node_from = graph_get_node(floor->graph, from);
                Node *node_to = graph_get_node(floor->graph, to);
                
                if (node_from && node_to) {
                    /* Calculate Manhattan distance: |x2-x1| + |y2-y1| */
                    int dx = (int)(node_to->x - node_from->x);
                    int dy = (int)(node_to->y - node_from->y);
                    int manhattan_distance = abs(dx) + abs(dy);
                    
                    /* Use calculated distance as weight */
                    int actual_weight = manhattan_distance > 0 ? manhattan_distance : 1;
                    
                    printf("DEBUG: Edge %d->%d: from(%.0f,%.0f) to(%.0f,%.0f) distance=%d\n",
                           from, to, node_from->x, node_from->y, 
                           node_to->x, node_to->y, actual_weight);
                    
                    graph_add_edge(floor->graph, from, to, actual_weight, bidirectional);
                } else {
                    printf("WARNING: Invalid edge %d -> %d (nodes not found)\n", from, to);
                }
            }
        }
    }
    
    fclose(fp);
    graph_build_adjacency_list(floor->graph);
    floor_update_slot_counts(floor);
    
    return SUCCESS;
}


void floor_update_slot_counts(Floor *floor) {
    floor->total_slots = 0;
    floor->occupied_slots = 0;
    
    for (int i = 0; i < floor->graph->node_count; i++) {
        Node *node = &floor->graph->nodes[i];
        if (node->type == NODE_SLOT) {
            floor->total_slots++;
            if (node->is_occupied) {
                floor->occupied_slots++;
            }
        }
    }
}

float floor_get_occupancy_rate(Floor *floor) {
    if (floor->total_slots == 0) return 0.0;
    return (float)floor->occupied_slots / floor->total_slots * 100.0;
}

void floor_print_status(Floor *floor) {
    printf("\n--- %s (Floor %d) ---\n", floor->name, floor->floor_number);
    printf("Slots: %d / %d occupied (%.1f%%)\n", 
           floor->occupied_slots, floor->total_slots, 
           floor_get_occupancy_rate(floor));
    printf("Entry Node: %d, Exit Node: %d\n", 
           floor->entry_node_id, floor->exit_node_id);
}

int floor_occupy_slot(Floor *floor, const char *slot_id, const char *vehicle_number) {
    int node_id = graph_get_node_by_slot_id(floor->graph, slot_id);
    if (node_id < 0) return FAILURE;
    
    Node *node = graph_get_node(floor->graph, node_id);
    if (node->is_occupied) return FAILURE;
    
    node->is_occupied = 1;
    strncpy(node->vehicle_number, vehicle_number, 19);
    node->vehicle_number[19] = '\0';
    node->occupied_time = time(NULL);
    node->usage_count++;
    node->last_used = time(NULL);
    
    floor->occupied_slots++;
    
    return SUCCESS;
}

int floor_free_slot(Floor *floor, const char *slot_id) {
    int node_id = graph_get_node_by_slot_id(floor->graph, slot_id);
    if (node_id < 0) return FAILURE;
    
    Node *node = graph_get_node(floor->graph, node_id);
    if (!node->is_occupied) return FAILURE;
    
    node->is_occupied = 0;
    strcpy(node->vehicle_number, "");
    node->occupied_time = 0;
    
    floor->occupied_slots--;
    
    return SUCCESS;
}
