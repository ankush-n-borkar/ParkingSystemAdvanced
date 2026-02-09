#include "graph.h"

Graph* graph_create() {
    Graph *g = (Graph*)malloc(sizeof(Graph));
    if (!g) return NULL;
    
    g->node_count = 0;
    g->edge_count = 0;
    
    /* Initialize adjacency lists */
    for (int i = 0; i < MAX_NODES; i++) {
        g->adj_list[i].count = 0;
    }
    
    return g;
}

void graph_destroy(Graph *g) {
    if (g) {
        free(g);
    }
}

int graph_add_node(Graph *g, int floor, NodeType type, float x, float y) {
    if (g->node_count >= MAX_NODES) {
        return -1;
    }
    
    int node_id = g->node_count;
    Node *n = &g->nodes[node_id];
    
    n->id = node_id;
    n->floor = floor;
    n->type = type;
    n->x = x;
    n->y = y;
    n->is_occupied = 0;
    n->slot_type = SLOT_STANDARD;
    strcpy(n->slot_id, "");
    strcpy(n->vehicle_number, "");
    n->occupied_time = 0;
    n->usage_count = 0;
    n->last_used = 0;
    
    g->node_count++;
    return node_id;
}

void graph_add_edge(Graph *g, int from_id, int to_id, int weight, int bidirectional) {
    if (g->edge_count >= MAX_EDGES) {
        return;
    }
    
    /* Add forward edge */
    g->edges[g->edge_count].from_id = from_id;
    g->edges[g->edge_count].to_id = to_id;
    g->edges[g->edge_count].weight = weight;
    g->edges[g->edge_count].is_bidirectional = bidirectional;
    g->edge_count++;
    
    /* Add reverse edge if bidirectional */
    if (bidirectional && g->edge_count < MAX_EDGES) {
        g->edges[g->edge_count].from_id = to_id;
        g->edges[g->edge_count].to_id = from_id;
        g->edges[g->edge_count].weight = weight;
        g->edges[g->edge_count].is_bidirectional = 0;
        g->edge_count++;
    }
}

void graph_build_adjacency_list(Graph *g) {
    /* Reset adjacency lists */
    for (int i = 0; i < g->node_count; i++) {
        g->adj_list[i].count = 0;
    }
    
    /* Build from edges */
    for (int i = 0; i < g->edge_count; i++) {
        Edge *e = &g->edges[i];
        int from = e->from_id;
        
        if (from >= 0 && from < MAX_NODES) {
            AdjacencyList *adj = &g->adj_list[from];
            if (adj->count < MAX_NEIGHBORS) {
                adj->neighbor_ids[adj->count] = e->to_id;
                adj->edge_weights[adj->count] = e->weight;
                adj->count++;
            }
        }
    }
}

Node* graph_get_node(Graph *g, int node_id) {
    if (node_id < 0 || node_id >= g->node_count) {
        return NULL;
    }
    return &g->nodes[node_id];
}

int graph_get_node_by_slot_id(Graph *g, const char *slot_id) {
    for (int i = 0; i < g->node_count; i++) {
        if (g->nodes[i].type == NODE_SLOT && 
            strcmp(g->nodes[i].slot_id, slot_id) == 0) {
            return i;
        }
    }
    return -1;
}

void graph_print_stats(Graph *g) {
    printf("\nGraph Statistics \n");
    printf("Total Nodes: %d\n", g->node_count);
    printf("Total Edges: %d\n", g->edge_count);
    
    int road_count = 0, slot_count = 0, junction_count = 0;
    int ramp_count = 0, entry_count = 0, exit_count = 0;
    
    for (int i = 0; i < g->node_count; i++) {
        switch (g->nodes[i].type) {
            case NODE_ROAD: road_count++; break;
            case NODE_SLOT: slot_count++; break;
            case NODE_JUNCTION: junction_count++; break;
            case NODE_RAMP_UP:
            case NODE_RAMP_DOWN: ramp_count++; break;
            case NODE_ENTRY: entry_count++; break;
            case NODE_EXIT: exit_count++; break;
        }
    }
    
    printf("Roads: %d, Slots: %d, Junctions: %d\n", 
           road_count, slot_count, junction_count);
    printf("Ramps: %d, Entry: %d, Exit: %d\n", 
           ramp_count, entry_count, exit_count);
}
