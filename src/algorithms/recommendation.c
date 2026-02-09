#include "recommendation.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


/* Check if vehicle fits in slot */
static int vehicle_fits_slot(VehicleType vehicle, SlotType slot) {
    if (slot == SLOT_DISABLED) {
        return 0; /* Reserved for disabled parking */
    }
    
    if (vehicle == VEHICLE_COMPACT) {
        return 1; /* Compact fits anywhere */
    }
    
    if (vehicle == VEHICLE_STANDARD) {
        return slot != SLOT_COMPACT;
    }
    
    if (vehicle == VEHICLE_LARGE) {
        return slot == SLOT_LARGE;
    }
    
    return 0;
}


/* Calculate multi-criteria score */
static float calculate_score(
    int entry_distance,
    int exit_distance,
    float floor_occupancy
) {
    /* HEAVILY prioritize entry distance for true BFS shortest path */
    float weight_entry = 0.80;  // 80% weight on entry distance
    float weight_exit = 0.15;   // 15% weight on exit distance
    float weight_occupancy = 0.05; // 5% weight on occupancy
    
    /* Normalize distances (inverse - shorter is better) */
    /* Use exponential decay to heavily penalize longer distances */
    float entry_score = 1000.0 / (1.0 + entry_distance * entry_distance);
    float exit_score = 100.0 / (1.0 + exit_distance);
    
    /* Occupancy score (lower occupancy is slightly better) */
    float occupancy_score = 100.0 - floor_occupancy;
    
    /* Combined score */
    float total = (entry_score * weight_entry) +
                  (exit_score * weight_exit) +
                  (occupancy_score * weight_occupancy);
    
    return total;
}



RecommendationResult generate_recommendations(
    ParkingSystem *ps,
    VehicleType vehicle_type,
    int max_results
) {
    RecommendationResult result;
    result.count = 0;
    
    if (max_results > MAX_RECOMMENDATIONS) {
        max_results = MAX_RECOMMENDATIONS;
    }
    
    /* COLLECT ALL available slots from ALL floors BEFORE sorting */
    typedef struct {
        Recommendation rec;
        int valid;
    } TempRec;
    
    TempRec all_slots[MAX_RECOMMENDATIONS * 2]; // Double size to ensure we get all floors
    int total_found = 0;
    
    /* Collect from all floors */
    for (int f = 0; f < ps->floor_count; f++) {
        Floor *floor = ps->floors[f];
        Graph *g = floor->graph;
        
        printf("DEBUG: Scanning floor %d, total nodes: %d\n", f, g->node_count);
        
        /* Find available slots */
        for (int i = 0; i < g->node_count; i++) {
            Node *node = &g->nodes[i];
            
            if (node->type == NODE_SLOT && 
                !node->is_occupied &&
                vehicle_fits_slot(vehicle_type, node->slot_type)) {
                
                /* Calculate distances */
                int entry_dist = get_distance_bfs(g, floor->entry_node_id, i);
                int exit_dist = get_distance_bfs(g, i, floor->exit_node_id);
                
                if (entry_dist == INFINITY_DIST || exit_dist == INFINITY_DIST) {
                    printf("DEBUG: Slot %s unreachable (entry=%d, exit=%d)\n", 
                           node->slot_id, entry_dist, exit_dist);
                    continue;
                }
                
                printf("DEBUG: Found slot %s on floor %d, entry_dist=%d\n", 
                       node->slot_id, f, entry_dist);
                
                /* Create recommendation */
                Recommendation *rec = &all_slots[total_found].rec;
                rec->floor_number = f;
                rec->node_id = i;
                strcpy(rec->slot_id, node->slot_id);
                rec->slot_type = node->slot_type;
                rec->entry_distance = entry_dist;
                rec->exit_distance = exit_dist;
                rec->floor_occupancy = floor_get_occupancy_rate(floor);
                
                /* Calculate score - PRIORITIZE ENTRY DISTANCE */
                rec->total_score = calculate_score(
                    entry_dist,
                    exit_dist,
                    rec->floor_occupancy
                );
                
                /* Get path */
                rec->path_from_entry = find_path_bfs(g, floor->entry_node_id, i);
                
                all_slots[total_found].valid = 1;
                total_found++;
                
                if (total_found >= MAX_RECOMMENDATIONS * 2) {
                    break;
                }
            }
        }
        
        if (total_found >= MAX_RECOMMENDATIONS * 2) {
            break;
        }
    }
    
    printf("DEBUG: Total slots found: %d\n", total_found);
    
    /* Sort ALL slots with multi-level tiebreakers */
    for (int i = 0; i < total_found - 1; i++) {
        for (int j = 0; j < total_found - i - 1; j++) {
            int should_swap = 0;
            
            /* Primary: Compare scores (higher is better) */
            float score_diff = all_slots[j].rec.total_score - all_slots[j + 1].rec.total_score;
            
            if (score_diff < -0.01) {
                /* j has lower score than j+1, swap */
                should_swap = 1;
            }
            /* Tiebreaker 1: If scores are equal (within 0.01), compare entry distance */
            else if (fabs(score_diff) < 0.01) {
                if (all_slots[j].rec.entry_distance > all_slots[j + 1].rec.entry_distance) {
                    /* j is farther from entry, swap */
                    should_swap = 1;
                }
                /* Tiebreaker 2: If distances are also equal, prefer lower slot ID */
                else if (all_slots[j].rec.entry_distance == all_slots[j + 1].rec.entry_distance) {
                    /* Lexicographic comparison: "F0-R4S1" < "F0-R4S6" */
                    if (strcmp(all_slots[j].rec.slot_id, all_slots[j + 1].rec.slot_id) > 0) {
                        should_swap = 1;
                    }
                }
            }
            
            if (should_swap) {
                TempRec temp = all_slots[j];
                all_slots[j] = all_slots[j + 1];
                all_slots[j + 1] = temp;
            }
        }
    }
    
    /* Take top N results */
    int count = total_found < max_results ? total_found : max_results;
    for (int i = 0; i < count; i++) {
        result.recommendations[i] = all_slots[i].rec;
        result.count++;
    }
    
    return result;
}



void print_recommendations(RecommendationResult *result) {
    if (result->count == 0) {
        printf("\nNo available slots found.\n");
        return;
    }
    
    printf("\n=== TOP RECOMMENDATIONS ===\n\n");
    
    for (int i = 0; i < result->count; i++) {
        Recommendation *rec = &result->recommendations[i];
        
        printf("%d. Slot: %s (Floor %d)\n", 
               i + 1, rec->slot_id, rec->floor_number);
        printf("   Type: ");
        switch (rec->slot_type) {
            case SLOT_COMPACT: printf("Compact"); break;
            case SLOT_STANDARD: printf("Standard"); break;
            case SLOT_LARGE: printf("Large"); break;
            case SLOT_DISABLED: printf("Disabled"); break;
        }
        printf("\n");
        printf("   Distance from Entry: %d units\n", rec->entry_distance);
        printf("   Distance to Exit: %d units\n", rec->exit_distance);
        printf("   Floor Occupancy: %.1f%%\n", rec->floor_occupancy);
        printf("   Score: %.2f\n", rec->total_score);
        printf("\n");
    }
}


Recommendation* get_best_recommendation(RecommendationResult *result) {
    if (result->count == 0) {
        return NULL;
    }
    return &result->recommendations[0];
}
