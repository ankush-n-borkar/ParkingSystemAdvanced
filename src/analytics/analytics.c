#include "analytics.h"
#include <stdio.h>

HeatmapData generate_floor_heatmap(Floor *floor) {
    HeatmapData heatmap;
    heatmap.count = 0;
    heatmap.max_usage = 0;
    heatmap.avg_usage = 0.0;
    
    Graph *g = floor->graph;
    int total_usage = 0;
    
    for (int i = 0; i < g->node_count && heatmap.count < 100; i++) {
        Node *node = &g->nodes[i];
        if (node->type == NODE_SLOT) {
            HeatmapPoint *point = &heatmap.points[heatmap.count];
            
            strcpy(point->slot_id, node->slot_id);
            point->usage_count = node->usage_count;
            point->is_occupied = node->is_occupied;
            point->x = node->x;
            point->y = node->y;
            
            total_usage += node->usage_count;
            
            if (node->usage_count > heatmap.max_usage) {
                heatmap.max_usage = node->usage_count;
            }
            
            heatmap.count++;
        }
    }
    
    /* Calculate intensities */
    if (heatmap.max_usage > 0) {
        for (int i = 0; i < heatmap.count; i++) {
            heatmap.points[i].usage_intensity = 
                (float)heatmap.points[i].usage_count / heatmap.max_usage * 100.0;
        }
    }
    
    if (heatmap.count > 0) {
        heatmap.avg_usage = (float)total_usage / heatmap.count;
    }
    
    return heatmap;
}

void get_peak_usage_slots(Floor *floor, char slot_ids[][20], int *counts, int max_results) {
    Graph *g = floor->graph;
    
    /* Simple selection sort to get top N */
    for (int rank = 0; rank < max_results; rank++) {
        int max_idx = -1;
        int max_count = -1;
        
        for (int i = 0; i < g->node_count; i++) {
            Node *node = &g->nodes[i];
            if (node->type == NODE_SLOT) {
                /* Check if already selected */
                int already_selected = 0;
                for (int j = 0; j < rank; j++) {
                    if (strcmp(slot_ids[j], node->slot_id) == 0) {
                        already_selected = 1;
                        break;
                    }
                }
                
                if (!already_selected && node->usage_count > max_count) {
                    max_count = node->usage_count;
                    max_idx = i;
                }
            }
        }
        
        if (max_idx >= 0) {
            Node *node = &g->nodes[max_idx];
            strcpy(slot_ids[rank], node->slot_id);
            counts[rank] = node->usage_count;
        } else {
            strcpy(slot_ids[rank], "");
            counts[rank] = 0;
        }
    }
}

float calculate_average_occupancy(ParkingSystem *ps) {
    if (ps->total_slots == 0) return 0.0;
    return (float)ps->total_occupied / ps->total_slots * 100.0;
}

void print_analytics_report(ParkingSystem *ps) {
    printf("\n");
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║         PARKING ANALYTICS REPORT              ║\n");
    printf("╚════════════════════════════════════════════════╝\n\n");
    
    printf("Overall System:\n");
    printf("  Current Occupancy: %.1f%%\n", calculate_average_occupancy(ps));
    printf("  Total Vehicles Parked Today: %d\n", ps->vehicles_parked_today);
    printf("  Total Vehicles Exited Today: %d\n", ps->vehicles_exited_today);
    printf("\n");
    
    /* Floor-wise analysis */
    for (int f = 0; f < ps->floor_count; f++) {
        Floor *floor = ps->floors[f];
        
        printf("=== %s (Floor %d) ===\n", floor->name, f);
        printf("Occupancy: %.1f%%\n\n", floor_get_occupancy_rate(floor));
        
        /* Top 5 most used slots */
        char top_slots[5][20];
        int top_counts[5];
        get_peak_usage_slots(floor, top_slots, top_counts, 5);
        
        printf("Top 5 Most Used Slots:\n");
        for (int i = 0; i < 5 && strlen(top_slots[i]) > 0; i++) {
            printf("  %d. %s - Used %d times\n", i+1, top_slots[i], top_counts[i]);
        }
        printf("\n");
        
        /* Heatmap summary */
        HeatmapData heatmap = generate_floor_heatmap(floor);
        printf("Heatmap Analysis:\n");
        printf("  Total Slots: %d\n", heatmap.count);
        printf("  Average Usage: %.1f times\n", heatmap.avg_usage);
        printf("  Peak Usage: %d times\n", heatmap.max_usage);
        printf("\n");
    }
}
