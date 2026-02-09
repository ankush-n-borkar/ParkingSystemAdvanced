#include "pathfinding.h"
#include <stdio.h>

PathResult find_path_bfs(Graph *g, int start_node, int end_node) {
    PathResult result;
    result.path_length = 0;
    result.total_distance = INFINITY_DIST;
    result.is_valid = 0;
    
    if (start_node < 0 || start_node >= g->node_count ||
        end_node < 0 || end_node >= g->node_count) {
        return result;
    }
    
    /* BFS setup */
    Queue q;
    queue_init(&q);
    
    int visited[MAX_NODES] = {0};
    int distance[MAX_NODES];
    int parent[MAX_NODES];
    
    for (int i = 0; i < MAX_NODES; i++) {
        distance[i] = INFINITY_DIST;
        parent[i] = -1;
    }
    
    /* Start BFS */
    queue_enqueue(&q, start_node, 0);
    visited[start_node] = 1;
    distance[start_node] = 0;
    
    while (!queue_is_empty(&q)) {
        QueueElement elem = queue_dequeue(&q);
        int current = elem.node_id;
        
        /* Found target */
        if (current == end_node) {
            break;
        }
        
        /* Explore neighbors */
        AdjacencyList *adj = &g->adj_list[current];
        for (int i = 0; i < adj->count; i++) {
            int neighbor = adj->neighbor_ids[i];
            int weight = adj->edge_weights[i];
            
            if (!visited[neighbor]) {
                visited[neighbor] = 1;
                distance[neighbor] = distance[current] + weight;
                parent[neighbor] = current;
                queue_enqueue(&q, neighbor, distance[neighbor]);
            }
        }
    }
    
    /* Reconstruct path */
    if (distance[end_node] == INFINITY_DIST) {
        return result; /* No path found */
    }
    
    int path_temp[MAX_PATH_LENGTH];
    int path_len = 0;
    int current = end_node;
    
    while (current != -1 && path_len < MAX_PATH_LENGTH) {
        path_temp[path_len++] = current;
        current = parent[current];
    }
    
    /* Reverse path */
    for (int i = 0; i < path_len; i++) {
        result.path[i] = path_temp[path_len - 1 - i];
    }
    
    result.path_length = path_len;
    result.total_distance = distance[end_node];
    result.is_valid = 1;
    
    return result;
}

int get_distance_bfs(Graph *g, int start_node, int end_node) {
    PathResult result = find_path_bfs(g, start_node, end_node);
    return result.is_valid ? result.total_distance : INFINITY_DIST;
}

int find_reachable_slots(Graph *g, int entry_node, int *slot_nodes, int *distances, int max_slots) {
    int count = 0;
    
    for (int i = 0; i < g->node_count && count < max_slots; i++) {
        Node *node = &g->nodes[i];
        if (node->type == NODE_SLOT && !node->is_occupied) {
            int dist = get_distance_bfs(g, entry_node, i);
            if (dist != INFINITY_DIST) {
                slot_nodes[count] = i;
                distances[count] = dist;
                count++;
            }
        }
    }
    
    return count;
}

void print_path(Graph *g, PathResult *path) {
    if (!path->is_valid) {
        printf("No valid path\n");
        return;
    }
    
    printf("Path (length=%d, distance=%d): ", path->path_length, path->total_distance);
    for (int i = 0; i < path->path_length; i++) {
        Node *node = graph_get_node(g, path->path[i]);
        if (node->type == NODE_SLOT) {
            printf("%s", node->slot_id);
        } else {
            printf("Node%d", path->path[i]);
        }
        if (i < path->path_length - 1) printf(" -> ");
    }
    printf("\n");
}

/* Wrapper function for HTTP server - converts PathResult to simple array */
int bfs_shortest_path(Graph *g, int start, int end, int *path, int max_path_len) {
    PathResult result = find_path_bfs(g, start, end);
    
    if (!result.is_valid || result.path_length == 0) {
        return 0;
    }
    
    /* Copy path to output array */
    int len = result.path_length;
    if (len > max_path_len) {
        len = max_path_len;
    }
    
    for (int i = 0; i < len; i++) {
        path[i] = result.path[i];
    }
    
    return len;
}
