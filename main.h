// main.h
#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

// Constants
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define USERS_FILE "users.txt"
#define SEMAPHORE_NAME "/airline_user_file_sem"

// External variables (declared in main.c)
extern sem_t *user_file_sem;
extern bool is_running;
extern pthread_mutex_t read_mutex;
extern pthread_mutex_t write_mutex;

// Function Prototypes
void handle_signal(int sig);
void initialize_synchronization();
void cleanup_synchronization();
bool is_username_unique(const char *username);
bool check_username_unique_internal(const char *username);
bool signup_user(const char *username, const char *password);
bool login_user(const char *username, const char *password);
bool contains_asterisk(const char *str);
bool contains_invalid(const char *str);

#endif // MAIN_H