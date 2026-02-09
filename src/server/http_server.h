#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "../core/parking_system.h"

#define PORT 8080
#define BUFFER_SIZE 8192

/* HTTP Server */
typedef struct {
    int server_socket;
    int is_running;
    ParkingSystem *parking_system;
} HTTPServer;

/* Start HTTP server */
HTTPServer* http_server_create(ParkingSystem *ps);
void http_server_start(HTTPServer *server);
void http_server_stop(HTTPServer *server);

/* Request handlers */
void handle_client(HTTPServer *server, int client_socket);
void send_response(int client_socket, const char *status, const char *content_type, const char *body);
void send_cors_headers(int client_socket);

#endif
