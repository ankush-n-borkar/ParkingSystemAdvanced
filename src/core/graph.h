#ifndef GRAPH_H
#define GRAPH_H

#include "../utils/utils.h"

#define MAX_NODES 500
#define MAX_EDGES 2000
#define MAX_NEIGHBORS 20

/* Node Types */
typedef enum {
    NODE_ROAD,
    NODE_JUNCTION,
    NODE_SLOT,
    NODE_RAMP_UP,
    NODE_RAMP_DOWN,
    NODE_ENTRY,
    NODE_EXIT
} NodeType;

/* Slot Types */
typedef enum {
    SLOT_STANDARD,
    SLOT_COMPACT,
    SLOT_LARGE,
    SLOT_DISABLED
} SlotType;

/* Vehicle Types */
typedef enum {
    VEHICLE_COMPACT,
    VEHICLE_STANDARD,
    VEHICLE_LARGE
} VehicleType;

/* Node structure */
typedef struct {
    int id;
    int floor;
    NodeType type;
    
    /* Position (for visualization) */
    float x;
    float y;
    
    /* Slot-specific data */
    int is_occupied;
    SlotType slot_type;
    char slot_id[20];
    char vehicle_number[20];
    time_t occupied_time;
    
    /* Analytics */
    int usage_count;
    time_t last_used;
} Node;

/* Edge structure */
typedef struct {
    int from_id;
    int to_id;
    int weight;
    int is_bidirectional;
} Edge;

/* Adjacency list */
typedef struct {
    int neighbor_ids[MAX_NEIGHBORS];
    int edge_weights[MAX_NEIGHBORS];
    int count;
} AdjacencyList;

/* Graph structure */
typedef struct {
    Node nodes[MAX_NODES];
    int node_count;
    
    Edge edges[MAX_EDGES];
    int edge_count;
    
    AdjacencyList adj_list[MAX_NODES];
} Graph;

/* Graph operations */
Graph* graph_create();
void graph_destroy(Graph *g);
int graph_add_node(Graph *g, int floor, NodeType type, float x, float y);
void graph_add_edge(Graph *g, int from_id, int to_id, int weight, int bidirectional);
void graph_build_adjacency_list(Graph *g);
Node* graph_get_node(Graph *g, int node_id);
int graph_get_node_by_slot_id(Graph *g, const char *slot_id);
void graph_print_stats(Graph *g);

#endif
