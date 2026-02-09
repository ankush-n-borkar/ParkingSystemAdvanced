#ifndef JSON_EXPORT_H
#define JSON_EXPORT_H

#include "../core/parking_system.h"
#include "../algorithms/recommendation.h"
#include "../analytics/analytics.h"

/* Export system state to JSON */
int export_system_json(ParkingSystem *ps, const char *filename);

/* Export recommendations to JSON */
int export_recommendations_json(RecommendationResult *result, const char *filename);

/* Export analytics to JSON */
int export_analytics_json(ParkingSystem *ps, const char *filename);

#endif
