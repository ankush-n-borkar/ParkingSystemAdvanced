#include "json_export.h"
#include <stdio.h>

int export_system_json(ParkingSystem *ps, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return FAILURE;
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"timestamp\": \"%s\",\n", get_timestamp());
    fprintf(fp, "  \"total_floors\": %d,\n", ps->floor_count);
    fprintf(fp, "  \"total_slots\": %d,\n", ps->total_slots);
    fprintf(fp, "  \"total_occupied\": %d,\n", ps->total_occupied);
    fprintf(fp, "  \"occupancy_rate\": %.1f,\n", 
           (float)ps->total_occupied / ps->total_slots * 100.0);
    fprintf(fp, "  \"floors\": [\n");
    
    for (int f = 0; f < ps->floor_count; f++) {
        Floor *floor = ps->floors[f];
        Graph *g = floor->graph;
        
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"floor_number\": %d,\n", f);
        fprintf(fp, "      \"name\": \"%s\",\n", floor->name);
        fprintf(fp, "      \"total_slots\": %d,\n", floor->total_slots);
        fprintf(fp, "      \"occupied_slots\": %d,\n", floor->occupied_slots);
        fprintf(fp, "      \"occupancy_rate\": %.1f,\n", floor_get_occupancy_rate(floor));
        fprintf(fp, "      \"slots\": [\n");
        
        int slot_count = 0;
        for (int i = 0; i < g->node_count; i++) {
            Node *node = &g->nodes[i];
            if (node->type == NODE_SLOT) {
                if (slot_count > 0) fprintf(fp, ",\n");
                
                fprintf(fp, "        {\n");
                fprintf(fp, "          \"id\": \"%s\",\n", node->slot_id);
                fprintf(fp, "          \"type\": \"%s\",\n", 
                       node->slot_type == SLOT_COMPACT ? "compact" :
                       node->slot_type == SLOT_LARGE ? "large" :
                       node->slot_type == SLOT_DISABLED ? "disabled" : "standard");
                fprintf(fp, "          \"is_occupied\": %s,\n", 
                       node->is_occupied ? "true" : "false");
                fprintf(fp, "          \"vehicle_number\": \"%s\",\n", 
                       node->is_occupied ? node->vehicle_number : "");
                fprintf(fp, "          \"usage_count\": %d,\n", node->usage_count);
                fprintf(fp, "          \"x\": %.1f,\n", node->x);
                fprintf(fp, "          \"y\": %.1f\n", node->y);
                fprintf(fp, "        }");
                
                slot_count++;
            }
        }
        
        fprintf(fp, "\n      ]\n");
        fprintf(fp, "    }");
        if (f < ps->floor_count - 1) fprintf(fp, ",");
        fprintf(fp, "\n");
    }
    
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return SUCCESS;
}

int export_recommendations_json(RecommendationResult *result, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return FAILURE;
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"count\": %d,\n", result->count);
    fprintf(fp, "  \"recommendations\": [\n");
    
    for (int i = 0; i < result->count; i++) {
        Recommendation *rec = &result->recommendations[i];
        
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"rank\": %d,\n", i + 1);
        fprintf(fp, "      \"slot_id\": \"%s\",\n", rec->slot_id);
        fprintf(fp, "      \"floor\": %d,\n", rec->floor_number);
        fprintf(fp, "      \"entry_distance\": %d,\n", rec->entry_distance);
        fprintf(fp, "      \"exit_distance\": %d,\n", rec->exit_distance);
        fprintf(fp, "      \"floor_occupancy\": %.1f,\n", rec->floor_occupancy);
        fprintf(fp, "      \"score\": %.2f\n", rec->total_score);
        fprintf(fp, "    }");
        
        if (i < result->count - 1) fprintf(fp, ",");
        fprintf(fp, "\n");
    }
    
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return SUCCESS;
}

int export_analytics_json(ParkingSystem *ps, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return FAILURE;
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"timestamp\": \"%s\",\n", get_timestamp());
    fprintf(fp, "  \"overall_occupancy\": %.1f,\n", 
           (float)ps->total_occupied / ps->total_slots * 100.0);
    fprintf(fp, "  \"floors\": [\n");
    
    for (int f = 0; f < ps->floor_count; f++) {
        Floor *floor = ps->floors[f];
        HeatmapData heatmap = generate_floor_heatmap(floor);
        
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"floor\": %d,\n", f);
        fprintf(fp, "      \"name\": \"%s\",\n", floor->name);
        fprintf(fp, "      \"avg_usage\": %.1f,\n", heatmap.avg_usage);
        fprintf(fp, "      \"max_usage\": %d,\n", heatmap.max_usage);
        fprintf(fp, "      \"heatmap\": [\n");
        
        for (int i = 0; i < heatmap.count; i++) {
            HeatmapPoint *point = &heatmap.points[i];
            
            fprintf(fp, "        {\n");
            fprintf(fp, "          \"slot_id\": \"%s\",\n", point->slot_id);
            fprintf(fp, "          \"usage_count\": %d,\n", point->usage_count);
            fprintf(fp, "          \"intensity\": %.1f,\n", point->usage_intensity);
            fprintf(fp, "          \"is_occupied\": %s\n", 
                   point->is_occupied ? "true" : "false");
            fprintf(fp, "        }");
            
            if (i < heatmap.count - 1) fprintf(fp, ",");
            fprintf(fp, "\n");
        }
        
        fprintf(fp, "      ]\n");
        fprintf(fp, "    }");
        
        if (f < ps->floor_count - 1) fprintf(fp, ",");
        fprintf(fp, "\n");
    }
    
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return SUCCESS;
}
