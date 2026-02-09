#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "../core/parking_system.h"

/* Heatmap data point */
typedef struct {
    char slot_id[20];
    int usage_count;
    float usage_intensity;
    int is_occupied;
    float x;
    float y;
} HeatmapPoint;

/* Heatmap result */
typedef struct {
    HeatmapPoint points[100];
    int count;
    int max_usage;
    float avg_usage;
} HeatmapData;

/* Generate usage heatmap for a floor */
HeatmapData generate_floor_heatmap(Floor *floor);

/* Get peak usage slots */
void get_peak_usage_slots(Floor *floor, char slot_ids[][20], int *counts, int max_results);

/* Calculate average occupancy over time (simulated) */
float calculate_average_occupancy(ParkingSystem *ps);

/* Generate analytics report */
void print_analytics_report(ParkingSystem *ps);

#endif
