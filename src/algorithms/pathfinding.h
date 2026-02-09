#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "../core/graph.h"
#include "../utils/queue.h"

#define MAX_PATH_LENGTH 100
#define INFINITY_DIST 999999

/* Path result structure */
typedef struct {
    int path[MAX_PATH_LENGTH];
    int path_length;
    int total_distance;
    int is_valid;
} PathResult;

/* BFS pathfinding */
PathResult find_path_bfs(Graph *g, int start_node, int end_node);

/* Get distance between two nodes */
int get_distance_bfs(Graph *g, int start_node, int end_node);

/* Find all reachable slots from entry */
int find_reachable_slots(Graph *g, int entry_node, int *slot_nodes, int *distances, int max_slots);

/* Print path for debugging */
void print_path(Graph *g, PathResult *path);

/* Wrapper for HTTP server compatibility */
int bfs_shortest_path(Graph *g, int start, int end, int *path, int max_path_len);


#endif
