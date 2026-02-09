#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include "../core/parking_system.h"
#include "pathfinding.h"

#define MAX_RECOMMENDATIONS 10

/* Recommendation structure */
typedef struct {
    int floor_number;
    int node_id;
    char slot_id[20];
    SlotType slot_type;
    
    int entry_distance;
    int exit_distance;
    float floor_occupancy;
    
    float total_score;
    
    PathResult path_from_entry;
} Recommendation;

/* Recommendation result */
typedef struct {
    Recommendation recommendations[MAX_RECOMMENDATIONS];
    int count;
} RecommendationResult;

/* Generate recommendations */
RecommendationResult generate_recommendations(
    ParkingSystem *ps,
    VehicleType vehicle_type,
    int max_results
);

/* Print recommendations */
void print_recommendations(RecommendationResult *result);

/* Get best recommendation */
Recommendation* get_best_recommendation(RecommendationResult *result);

#endif
