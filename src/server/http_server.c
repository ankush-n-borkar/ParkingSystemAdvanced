#include "http_server.h"
#include "../io/json_export.h"
#include "../algorithms/recommendation.h"
#include "../io/file_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

HTTPServer* http_server_create(ParkingSystem *ps) {
    HTTPServer *server = (HTTPServer*)malloc(sizeof(HTTPServer));
    server->parking_system = ps;
    server->is_running = 0;
    server->server_socket = -1;
    
    #ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            printf("WSAStartup failed\n");
            free(server);
            return NULL;
        }
    #endif
    
    return server;
}

void send_cors_headers(int client_socket) {
    const char *cors_headers = 
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n";
    send(client_socket, cors_headers, strlen(cors_headers), 0);
}

void send_response(int client_socket, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];
    
    snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status, content_type, strlen(body), body
    );
    
    send(client_socket, response, strlen(response), 0);
}

void handle_get_state(HTTPServer *server, int client_socket) {
    /* Generate JSON response */
    char json[BUFFER_SIZE * 2];
    ParkingSystem *ps = server->parking_system;
    
    int offset = 0;
    offset += snprintf(json + offset, sizeof(json) - offset,
        "{\n"
        "  \"timestamp\": \"%s\",\n"
        "  \"total_floors\": %d,\n"
        "  \"total_slots\": %d,\n"
        "  \"total_occupied\": %d,\n"
        "  \"occupancy_rate\": %.1f,\n"
        "  \"floors\": [\n",
        get_timestamp(),
        ps->floor_count,
        ps->total_slots,
        ps->total_occupied,
        ps->total_slots > 0 ? (float)ps->total_occupied / ps->total_slots * 100.0 : 0.0
    );
    
    for (int f = 0; f < ps->floor_count; f++) {
        Floor *floor = ps->floors[f];
        Graph *g = floor->graph;
        
        offset += snprintf(json + offset, sizeof(json) - offset,
            "    {\n"
            "      \"floor_number\": %d,\n"
            "      \"name\": \"%s\",\n"
            "      \"total_slots\": %d,\n"
            "      \"occupied_slots\": %d,\n"
            "      \"occupancy_rate\": %.1f,\n"
            "      \"slots\": [\n",
            f, floor->name, floor->total_slots, floor->occupied_slots,
            floor_get_occupancy_rate(floor)
        );
        
        int slot_count = 0;
        for (int i = 0; i < g->node_count; i++) {
            Node *node = &g->nodes[i];
            if (node->type == NODE_SLOT) {
                if (slot_count > 0) offset += snprintf(json + offset, sizeof(json) - offset, ",\n");
                
                const char *type_str = "standard";
                if (node->slot_type == SLOT_COMPACT) type_str = "compact";
                else if (node->slot_type == SLOT_LARGE) type_str = "large";
                else if (node->slot_type == SLOT_DISABLED) type_str = "disabled";
                
                offset += snprintf(json + offset, sizeof(json) - offset,
                    "        {\"id\":\"%s\",\"type\":\"%s\",\"is_occupied\":%s,"
                    "\"vehicle_number\":\"%s\",\"usage_count\":%d,\"x\":%.1f,\"y\":%.1f}",
                    node->slot_id, type_str,
                    node->is_occupied ? "true" : "false",
                    node->is_occupied ? node->vehicle_number : "",
                    node->usage_count, node->x, node->y
                );
                slot_count++;
            }
        }
        
        offset += snprintf(json + offset, sizeof(json) - offset, "\n      ]\n    }");
        if (f < ps->floor_count - 1) offset += snprintf(json + offset, sizeof(json) - offset, ",");
        offset += snprintf(json + offset, sizeof(json) - offset, "\n");
    }
    
    offset += snprintf(json + offset, sizeof(json) - offset, "  ]\n}\n");
    
    send_response(client_socket, "200 OK", "application/json", json);
}

/* Helper to send JSON responses */
void send_json_response(int client_socket, int status_code, const char *json_body) {
    const char *status_text;
    
    switch (status_code) {
        case 200: status_text = "200 OK"; break;
        case 400: status_text = "400 Bad Request"; break;
        case 404: status_text = "404 Not Found"; break;
        case 500: status_text = "500 Internal Server Error"; break;
        default: status_text = "200 OK"; break;
    }
    
    send_response(client_socket, status_text, "application/json", json_body);
}


void handle_post_park(HTTPServer *server, int client_socket, const char *body) {
    printf("DEBUG: Received POST /api/park\n");
    printf("DEBUG: Body: %s\n", body);
    
    if (!body || strlen(body) == 0) {
        send_json_response(client_socket, 400, "{\"status\":\"error\",\"message\":\"Empty request body\"}");
        return;
    }

    char slot_id[32] = "";
    char vehicle_number[32] = "";
    int vehicle_type = 1;
    int floor_num = 0;
    
    // Improved JSON parsing
    const char *slot_ptr = strstr(body, "\"slot_id\"");
    const char *vehicle_ptr = strstr(body, "\"vehicle_number\"");
    const char *type_ptr = strstr(body, "\"vehicle_type\"");
    
    // Parse slot_id
    if (slot_ptr) {
        const char *colon = strchr(slot_ptr, ':');
        if (colon) {
            colon++;
            while (*colon == ' ' || *colon == '\t') colon++;
            if (*colon == '"') {
                colon++;
                const char *end_quote = strchr(colon, '"');
                if (end_quote) {
                    int len = end_quote - colon;
                    if (len > 0 && len < 31) {
                        strncpy(slot_id, colon, len);
                        slot_id[len] = '\0';
                    }
                }
            }
        }
    }
    
    // Parse vehicle_number
    if (vehicle_ptr) {
        const char *colon = strchr(vehicle_ptr, ':');
        if (colon) {
            colon++;
            while (*colon == ' ' || *colon == '\t') colon++;
            if (*colon == '"') {
                colon++;
                const char *end_quote = strchr(colon, '"');
                if (end_quote) {
                    int len = end_quote - colon;
                    if (len > 0 && len < 31) {
                        strncpy(vehicle_number, colon, len);
                        vehicle_number[len] = '\0';
                    }
                }
            }
        }
    }
    
    // Parse vehicle_type
    if (type_ptr) {
        const char *colon = strchr(type_ptr, ':');
        if (colon) {
            colon++;
            while (*colon == ' ' || *colon == '\t') colon++;
            vehicle_type = atoi(colon);
        }
    }
    
    printf("DEBUG: Parsed - slot_id='%s', vehicle_number='%s', vehicle_type=%d\n", 
           slot_id, vehicle_number, vehicle_type);
    
    // Validate inputs
    if (strlen(slot_id) == 0 || strlen(vehicle_number) == 0) {
        char error[256];
        snprintf(error, sizeof(error), 
                "{\"status\":\"error\",\"message\":\"Missing required fields. Got slot_id='%s', vehicle_number='%s'\"}",
                slot_id, vehicle_number);
        send_json_response(client_socket, 400, error);
        return;
    }
    
    // Extract floor number from slot_id (format: F0-R1S1)
    if (slot_id[0] == 'F' && strlen(slot_id) > 1) {
        floor_num = slot_id[1] - '0';
    }
    
    printf("DEBUG: Floor number: %d\n", floor_num);
    
    if (floor_num < 0 || floor_num >= server->parking_system->floor_count) {
        send_json_response(client_socket, 400, "{\"status\":\"error\",\"message\":\"Invalid floor\"}");
        return;
    }
    
    Floor *floor = server->parking_system->floors[floor_num];
    
    // Find slot node
    int node_id = graph_get_node_by_slot_id(floor->graph, slot_id);
    printf("DEBUG: Node ID for slot %s: %d\n", slot_id, node_id);
    
    if (node_id < 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "{\"status\":\"error\",\"message\":\"Slot not found: %s\"}", slot_id);
        send_json_response(client_socket, 404, error_msg);
        return;
    }
    
    Node *node = graph_get_node(floor->graph, node_id);
    if (!node) {
        send_json_response(client_socket, 500, "{\"status\":\"error\",\"message\":\"Internal error: node is null\"}");
        return;
    }
    
    if (node->is_occupied) {
        send_json_response(client_socket, 400, "{\"status\":\"error\",\"message\":\"Slot is already occupied\"}");
        return;
    }
    
    // Occupy slot
    printf("DEBUG: Attempting to occupy slot %s with vehicle %s\n", slot_id, vehicle_number);
    
    if (floor_occupy_slot(floor, slot_id, vehicle_number) == FAILURE) {
        send_json_response(client_socket, 500, "{\"status\":\"error\",\"message\":\"Failed to occupy slot\"}");
        return;
    }
    
    // ✅ FIX: Update system statistics immediately after occupation
    parking_system_update_stats(server->parking_system);
    
    // Calculate path using BFS
    PathResult path = find_path_bfs(floor->graph, floor->entry_node_id, node_id);
    
    printf("DEBUG: Path calculated - distance: %d\n", path.total_distance);
    
    // Build response
    char response[1024];
    snprintf(response, sizeof(response),
        "{\"status\":\"success\",\"message\":\"Vehicle parked successfully\","
        "\"slot\":\"%s\",\"floor\":%d,\"vehicle_number\":\"%s\",\"path_length\":%d}",
        slot_id, floor_num, vehicle_number, path.total_distance);
    
    send_json_response(client_socket, 200, response);
    
    // Save state to file
    save_parking_state(server->parking_system, "data/parking_state.txt");
    
    printf("✓ Vehicle %s parked at %s\n", vehicle_number, slot_id);
}




void handle_post_exit(HTTPServer *server, int client_socket, const char *body) {
    /* Parse JSON: {"slot_id": "F0-A1"} */
    char slot_id[20] = "";
    
    const char *slot_pos = strstr(body, "\"slot_id\":");
    if (slot_pos) {
        sscanf(slot_pos, "\"slot_id\":\"%19[^\"]\"", slot_id);
    }
    
    if (strlen(slot_id) == 0) {
        send_response(client_socket, "400 Bad Request", "application/json",
            "{\"status\":\"error\",\"message\":\"Slot ID required\"}");
        return;
    }
    
    /* Find and free slot */
    ParkingSystem *ps = server->parking_system;
    int found = 0;
    
    for (int f = 0; f < ps->floor_count; f++) {
        Floor *floor = ps->floors[f];
        int node_id = graph_get_node_by_slot_id(floor->graph, slot_id);
        
        if (node_id >= 0) {
            Node *node = graph_get_node(floor->graph, node_id);
            
            if (!node->is_occupied) {
                send_response(client_socket, "200 OK", "application/json",
                    "{\"status\":\"error\",\"message\":\"Slot is already empty\"}");
                return;
            }
            
            char vehicle_num[20];
            strcpy(vehicle_num, node->vehicle_number);
            
            if (floor_free_slot(floor, slot_id) == SUCCESS) {
                parking_system_update_stats(ps);
                save_parking_state(ps, "data/parking_state.txt");
                log_transaction("EXIT", slot_id, vehicle_num);
                
                char response[256];
                snprintf(response, sizeof(response),
                    "{\"status\":\"success\",\"vehicle\":\"%s\",\"slot\":\"%s\"}",
                    vehicle_num, slot_id
                );
                
                send_response(client_socket, "200 OK", "application/json", response);
                printf("✓ Vehicle %s exited from %s\n", vehicle_num, slot_id);
                found = 1;
            }
            break;
        }
    }
    
    if (!found) {
        send_response(client_socket, "200 OK", "application/json",
            "{\"status\":\"error\",\"message\":\"Slot not found\"}");
    }
}

void handle_get_analytics(HTTPServer *server, int client_socket) {
    /* Export analytics and send */
    export_analytics_json(server->parking_system, "data/analytics.json");
    
    FILE *fp = fopen("data/analytics.json", "r");
    if (!fp) {
        send_response(client_socket, "500 Internal Server Error", "application/json",
            "{\"error\":\"Failed to generate analytics\"}");
        return;
    }
    
    char buffer[BUFFER_SIZE * 2];
    size_t bytes = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[bytes] = '\0';
    fclose(fp);
    
    send_response(client_socket, "200 OK", "application/json", buffer);
}

void handle_client(HTTPServer *server, int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        close(client_socket);
        return;
    }
    
    buffer[bytes_received] = '\0';
    
    /* Parse HTTP request */
    char method[10], path[256];
    sscanf(buffer, "%s %s", method, path);
    
    printf("→ %s %s\n", method, path);
    
    /* Handle OPTIONS (CORS preflight) */
    if (strcmp(method, "OPTIONS") == 0) {
        send_response(client_socket, "200 OK", "text/plain", "");
        close(client_socket);
        return;
    }
    
    /* Route requests */
        /* Route requests */
/* Route requests */
if (strcmp(method, "GET") == 0) {
    if (strcmp(path, "/api/state") == 0) {
        handle_get_state(server, client_socket);
    }
    else if (strcmp(path, "/api/analytics") == 0) {
        handle_get_analytics(server, client_socket);
    }
    else if (strncmp(path, "/api/recommend", 14) == 0) {
        char *query = strchr(path, '?');
        handle_get_recommendation(server, client_socket, query ? query + 1 : NULL);
    }
    else if (strncmp(path, "/api/path", 9) == 0) {
        char *query = strchr(path, '?');
        handle_get_path(server, client_socket, query ? query + 1 : NULL);
    }
    else {
        send_response(client_socket, "404 Not Found", "text/plain", "Not Found");
    }
}
else if (strcmp(method, "POST") == 0) {
    /* Find start of body */
    char *body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4; /* skip the \r\n\r\n */
    } else {
        body = "";
    }

    if (strcmp(path, "/api/park") == 0) {
        handle_post_park(server, client_socket, body);
    }
    else if (strcmp(path, "/api/exit") == 0) {
        handle_post_exit(server, client_socket, body);
    }
    else {
        send_response(client_socket, "404 Not Found", "text/plain", "Not Found");
    }
}
else {
    send_response(client_socket, "405 Method Not Allowed", "text/plain", "Method Not Allowed");
}

close(client_socket);

}

void http_server_start(HTTPServer *server) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    /* Create socket */
    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket < 0) {
        printf("Socket creation failed\n");
        return;
    }
    
    /* Set socket options */
    int opt = 1;
    setsockopt(server->server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    /* Bind */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server->server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Bind failed\n");
        close(server->server_socket);
        return;
    }
    
    /* Listen */
    if (listen(server->server_socket, 10) < 0) {
        printf("Listen failed\n");
        close(server->server_socket);
        return;
    }
    
    server->is_running = 1;
    
    printf("\n");
    print_header("HTTP SERVER STARTED");
    printf("\n");
    printf("✓ Server running on http://localhost:%d\n", PORT);
    printf("✓ Open http://localhost:%d/frontend/index.html in your browser\n", PORT);
    printf("\n");
    printf("API Endpoints:\n");
    printf("  GET  /api/state      - Get parking state\n");
    printf("  GET  /api/analytics  - Get analytics\n");
    printf("  GET  /api/recommend  - Get slot recommendations (BFS)\n");
    printf("  GET  /api/path       - Get path to slot\n");
    printf("  POST /api/park       - Park vehicle\n");
    printf("  POST /api/exit       - Exit vehicle\n");

    printf("\n");
    printf("Press Ctrl+C to stop...\n\n");
    
    /* Accept loop */
    while (server->is_running) {
        int client_socket = accept(server->server_socket, 
                                   (struct sockaddr*)&client_addr, 
                                   &client_len);
        
        if (client_socket < 0) continue;
        
        handle_client(server, client_socket);
    }
}

void http_server_stop(HTTPServer *server) {
    server->is_running = 0;
    if (server->server_socket >= 0) {
        close(server->server_socket);
    }
    
    #ifdef _WIN32
        WSACleanup();
    #endif
    
    free(server);
}


/* NEW FUNCTION 1: Get recommendation with BFS distances */
/* NEW FUNCTION 1: Get recommendation with BFS distances */
void handle_get_recommendation(HTTPServer *server, int client_socket, const char *query_string) {
    /* Parse vehicle_type and floor from query string */
    int vehicle_type = 1; // Default standard
    int target_floor = -1; // -1 means all floors
    
    if (query_string) {
        if (strstr(query_string, "vehicle_type=")) {
            sscanf(strstr(query_string, "vehicle_type="), "vehicle_type=%d", &vehicle_type);
        }
        if (strstr(query_string, "floor=")) {
            sscanf(strstr(query_string, "floor="), "floor=%d", &target_floor);
        }
    }
    
    printf("DEBUG: Recommendation - vehicle_type=%d, floor=%d\n", vehicle_type, target_floor);
    
    /* Generate recommendations using BFS */
    RecommendationResult result = generate_recommendations(
        server->parking_system,
        (VehicleType)vehicle_type,
        10 // Get more recommendations initially
    );
    
    /* ✅ FIX: STRICT floor filtering - NO FALLBACK */
    if (target_floor >= 0 && target_floor < server->parking_system->floor_count) {
        RecommendationResult filtered;
        filtered.count = 0;
        
        for (int i = 0; i < result.count; i++) {
            if (result.recommendations[i].floor_number == target_floor) {
                filtered.recommendations[filtered.count++] = result.recommendations[i];
                if (filtered.count >= 5) break;
            }
        }
        
        /* ✅ USE FILTERED RESULT - even if count is 0 */
        result = filtered;
        printf("DEBUG: Filtered to %d slots on floor %d\n", result.count, target_floor);
    }
    
    if (result.count == 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            "{\"status\":\"error\",\"message\":\"No available slots%s\",\"recommendations\":[]}",
            target_floor >= 0 ? " on selected floor" : "");
        send_response(client_socket, "200 OK", "application/json", error_msg);
        return;
    }
    
    /* Build JSON response */
    char response[4096];
    int offset = 0;
    
    offset += snprintf(response + offset, sizeof(response) - offset,
        "{\"status\":\"success\",\"recommendations\":[");
    
    for (int i = 0; i < result.count; i++) {
        Recommendation *rec = &result.recommendations[i];
        
        offset += snprintf(response + offset, sizeof(response) - offset,
            "%s{\"slot_id\":\"%s\",\"floor\":%d,\"entry_distance\":%d,\"score\":%.2f}",
            i > 0 ? "," : "",
            rec->slot_id,
            rec->floor_number,
            rec->entry_distance,
            rec->total_score
        );
    }
    
    offset += snprintf(response + offset, sizeof(response) - offset, "]}");
    
    send_response(client_socket, "200 OK", "application/json", response);
}



void handle_get_path(HTTPServer *server, int client_socket, const char *query_string) {
    char slot_id[20] = "";
    
    if (query_string && strstr(query_string, "slot_id=")) {
        sscanf(strstr(query_string, "slot_id="), "slot_id=%19[^&]", slot_id);
    }
    
    if (strlen(slot_id) == 0) {
        send_response(client_socket, "400 Bad Request", "application/json",
            "{\"status\":\"error\",\"message\":\"slot_id required\"}");
        return;
    }
    
    /* Find slot in which floor */
    ParkingSystem *ps = server->parking_system;
    Floor *target_floor = NULL;
    int slot_node = -1;
    
    for (int f = 0; f < ps->floor_count; f++) {
        Floor *floor = ps->floors[f];
        slot_node = graph_get_node_by_slot_id(floor->graph, slot_id);
        if (slot_node >= 0) {
            target_floor = floor;
            break;
        }
    }
    
    if (!target_floor || slot_node < 0) {
        send_response(client_socket, "404 Not Found", "application/json",
            "{\"status\":\"error\",\"message\":\"Slot not found\"}");
        return;
    }
    
    /* Calculate BFS path */
    Graph *graph = target_floor->graph;
    int entry_node = target_floor->entry_node_id;
    
    int path[100];
    int path_length = bfs_shortest_path(graph, entry_node, slot_node, path, 100);
    
    if (path_length == 0) {
        send_response(client_socket, "404 Not Found", "application/json",
            "{\"status\":\"error\",\"message\":\"No path found\"}");
        return;
    }
    
    /* Get destination slot info */
    Node *dest_slot = graph_get_node(graph, slot_node);
    int dest_x = (int)dest_slot->x;
    int dest_y = (int)dest_slot->y;
    
    /* Build path JSON - clean waypoints only */
    char path_json[4096] = "[";
    int json_offset = 1;
    int waypoint_count = 0;
    int last_x = -1, last_y = -1;
    
    for (int i = 0; i < path_length; i++) {
        Node *node = graph_get_node(graph, path[i]);
        if (!node) continue;
        
        int include_node = 0;
        
        /* Always include ENTRY */
        if (node->type == NODE_ENTRY) {
            include_node = 1;
        }
        /* Include ROADS at direction changes */
        else if (node->type == NODE_ROAD || node->type == NODE_JUNCTION) {
            if (i == 0 || i == path_length - 1) {
                include_node = 1;
            } else {
                Node *prev = graph_get_node(graph, path[i-1]);
                Node *next = graph_get_node(graph, path[i+1]);
                
                if (prev && next) {
                    int dx_prev = (int)node->x - (int)prev->x;
                    int dy_prev = (int)node->y - (int)prev->y;
                    int dx_next = (int)next->x - (int)node->x;
                    int dy_next = (int)next->y - (int)node->y;
                    
                    if ((dx_prev != 0 && dx_next == 0) || 
                        (dy_prev != 0 && dy_next == 0) ||
                        (dx_prev == 0 && dx_next != 0) || 
                        (dy_prev == 0 && dy_next != 0)) {
                        include_node = 1;
                    }
                }
            }
        }
        /* Only include FINAL slot */
        else if (node->type == NODE_SLOT && i == path_length - 1) {
            include_node = 1;
        }
        
        if (include_node) {
            int gridX = (int)node->x;
            int gridY = (int)node->y;
            
            /* Before adding final slot, check if we need horizontal road waypoints */
            if (node->type == NODE_SLOT && i == path_length - 1) {
                /* If last waypoint was on vertical road (x=0) and slot is not at x=0 */
                if (last_x == 0 && gridX > 0) {
                    int road_y;
                    
                    /* Determine which horizontal road based on slot Y */
                    if (gridY == 1) road_y = 2;      // Row 1: slot at y=1, road at y=2
                    else if (gridY == 3) road_y = 2;  // Row 2: slot at y=3, road at y=2
                    else if (gridY == 5) road_y = 6;  // Row 3: slot at y=5, road at y=6
                    else if (gridY == 7) road_y = 6;  // Row 4: slot at y=7, road at y=6
                    else road_y = gridY;
                    
                    /* Add horizontal road ENTRY point at (1, road_y) */
                    if (waypoint_count > 0) {
                        json_offset += snprintf(path_json + json_offset, 
                                               sizeof(path_json) - json_offset, ",");
                    }
                    json_offset += snprintf(path_json + json_offset, 
                                           sizeof(path_json) - json_offset,
                                           "{\"x\":1,\"y\":%d}", road_y);
                    waypoint_count++;
                    
                    /* If slot is further than x=1, add turn point at (slot_x, road_y) */
                    if (gridX > 1) {
                        json_offset += snprintf(path_json + json_offset, 
                                               sizeof(path_json) - json_offset,
                                               ",{\"x\":%d,\"y\":%d}", gridX, road_y);
                        waypoint_count++;
                    }
                    
                    last_x = gridX;
                    last_y = road_y;
                }
            }
            
            /* Add the node */
            if (waypoint_count > 0) {
                json_offset += snprintf(path_json + json_offset, 
                                       sizeof(path_json) - json_offset, ",");
            }
            json_offset += snprintf(path_json + json_offset, 
                                   sizeof(path_json) - json_offset,
                                   "{\"x\":%d,\"y\":%d}", gridX, gridY);
            waypoint_count++;
            last_x = gridX;
            last_y = gridY;
        }
    }
    
    json_offset += snprintf(path_json + json_offset, 
                           sizeof(path_json) - json_offset, "]");
    
    /* Build response */
    char response[8192];
    snprintf(response, sizeof(response),
        "{\"status\":\"success\",\"slot_id\":\"%s\",\"path_length\":%d,\"path\":%s}",
        slot_id, path_length, path_json);
    
    send_response(client_socket, "200 OK", "application/json", response);
}
