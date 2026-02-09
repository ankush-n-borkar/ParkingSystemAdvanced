#include "utils.h"

void print_header(const char *title) {
    int len = strlen(title);
    printf("\n");
    printf(COLOR_CYAN);
    printf("╔");
    for (int i = 0; i < len + 4; i++) printf("═");
    printf("╗\n");
    printf("║  %s  ║\n", title);
    printf("╚");
    for (int i = 0; i < len + 4; i++) printf("═");
    printf("╝\n");
    printf(COLOR_RESET);
    printf("\n");
}

void print_success(const char *message) {
    printf(COLOR_GREEN "✓ %s" COLOR_RESET "\n", message);
}

void print_error(const char *message) {
    printf(COLOR_RED "✗ %s" COLOR_RESET "\n", message);
}

void print_info(const char *message) {
    printf(COLOR_BLUE "ℹ %s" COLOR_RESET "\n", message);
}

void print_warning(const char *message) {
    printf(COLOR_YELLOW "⚠ %s" COLOR_RESET "\n", message);
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void press_enter_to_continue() {
    printf("\nPress ENTER to continue...");
    getchar();
    getchar();
}

char* get_timestamp() {
    static char buffer[80];
    time_t rawtime;
    struct tm *timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return buffer;
}

int string_to_int(const char *str) {
    return atoi(str);
}

void trim_whitespace(char *str) {
    char *end;
    
    /* Trim leading space */
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    *(end + 1) = '\0';
}
