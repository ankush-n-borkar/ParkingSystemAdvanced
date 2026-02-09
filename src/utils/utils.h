#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Constants */
#define MAX_STRING_LENGTH 256
#define SUCCESS 1
#define FAILURE 0
#define TRUE 1
#define FALSE 0

/* Color codes for terminal output */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"

/* Utility Functions */
void print_header(const char *title);
void print_success(const char *message);
void print_error(const char *message);
void print_info(const char *message);
void print_warning(const char *message);
void clear_screen();
void press_enter_to_continue();
char* get_timestamp();
int string_to_int(const char *str);
void trim_whitespace(char *str);

#endif
