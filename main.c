#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "cancellation.h"
#include "view_flights.h"

#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define USERS_FILE "users.txt"
#define SEMAPHORE_NAME "/airline_user_file_sem"

// Global variables
sem_t *user_file_sem;
bool is_running = true;
pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;   // protects readers_threads_count
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;  // ensures one writer at a time

// Function prototypes
bool is_username_unique(const char *username);
bool check_username_unique_internal(const char *username);
bool contains_asterisk(const char *str);
bool contains_invalid(const char *str);
bool signup_user(const char *username, const char *password);
bool login_user(const char *username, const char *password);
void initialize_synchronization();
void cleanup_synchronization();
void handle_signal(int sig);

int main() {
    char choice;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    bool result;
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Initialize semaphores
    initialize_synchronization();
    
    while (is_running) {
        printf("\n===== Airline Reservation System =====\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("(Any other key) Exit\n");
        printf("Enter your choice: ");
        scanf(" %c", &choice);
        getchar(); // Consume newline
        
        switch (choice) {
            case '1':
                printf("\n----- Login -----\n");
                printf("Username: ");
                fgets(username, MAX_USERNAME_LENGTH, stdin);
                username[strcspn(username, "\n")] = 0; // Remove newline
                
                printf("Password: ");
                fgets(password, MAX_PASSWORD_LENGTH, stdin);
                password[strcspn(password, "\n")] = 0; // Remove newline
                
                result = login_user(username, password);
                if (result) {
                    // User successfully logged in
                    printf("\n-----------------------------------------------------------------\n");
                    printf("\tWelcome, %s! You are now logged in.", username);
                    printf("\n-----------------------------------------------------------------\n");

                    char logged_in_choice;
                    bool stay_logged_in = true;
                    
                    while (stay_logged_in) {
                        printf("\n===== User Menu =====\n");
                        printf("1. View Flights\n");
                        printf("2. Cancel Flights\n");
                        printf("(Any other key) Logout\n");
                        take_inner_choice_again:
                        printf("Enter your choice: ");
                        scanf(" %c", &logged_in_choice);
                        
                        switch (logged_in_choice) {
                            case '1':
                                View_flights(username);
                                break;
                            case '2':

                                printf("\n-----------------------------------------------------------------\n");
                                printf("\tFlghts Cancellation section");
                                printf("\n-----------------------------------------------------------------\n\n");
                            take_flight_no_again:    
                                printf("(Enter '0' to navigate back to the User Menu)\n");
                                printf("Enter flight number: ");
                                int flight_number;
                                scanf("%d", &flight_number);
                                getchar();

                                if(flight_number==0)
                                   goto return_to_user_menu;
                                
                                printf("Enter seat number: ");
                                int seat_number;
                                scanf("%d", &seat_number);
                                getchar();
                                
                                int result = cancel_booking(username, flight_number, seat_number);
                                if (result == 0) {
                                    printf("Failed to cancel booking. Please check your flight number and seat number.\n");
                                    goto take_flight_no_again;
                                    
                                }
                                    
                                return_to_user_menu:
                                   printf("\n-----------------------------------------------------------------\n");
                                   printf("\tReturning to User Menu");
                                   printf("\n-----------------------------------------------------------------\n");
                                   break;

                            default:
                            printf("\n-----------------------------------------------------------------\n");
                            printf("\tLogout Successful.");
                            printf("\n-----------------------------------------------------------------\n");
                                stay_logged_in = false;

                        }
                    }
                } else {
                    printf("Login failed. Invalid username or password.\n");
                }
                break;
                
            case '2':
                printf("\n----- Register -----\n");

            take_input_again:
                printf("Username: ");
                fgets(username, MAX_USERNAME_LENGTH, stdin);
                username[strcspn(username, "\n")] = 0; // Remove newline
                
                if (contains_invalid(username)) {
                    goto take_input_again;
                }
                
                if (!is_username_unique(username)) {
                    printf("Error: Username already exists. Please choose a different username.\n");
                    goto take_input_again;
                }
                
                printf("Password: ");
                fgets(password, MAX_PASSWORD_LENGTH, stdin);
                password[strcspn(password, "\n")] = 0; // Remove newline

                if (contains_invalid(password)) {
                    goto take_input_again;
                }
                
                result = signup_user(username, password);
                if (result) {
                    printf("Registration successful! You can now login with your credentials.\n");
                } else {
                    printf("Registration failed. Please try again later.\n");
                }
                break;
                
            default:
                printf("\n-----------------------------------------------------------------\n");
                printf("\tAirline System Exited. Goodbye!");
                printf("\n-----------------------------------------------------------------\n");
                is_running = false;     
                break;

        }
    }
    
    cleanup_synchronization();
    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&write_mutex);
    return 0;
}

void handle_signal(int sig) {
    printf("\nReceived signal %d. Cleaning up and exiting...\n", sig);
    is_running = false;
    cleanup_synchronization();
    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&write_mutex);
    exit(0);
}

void initialize_synchronization() {
    // Create named semaphore for cross-process synchronization
    user_file_sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 1);
    if (user_file_sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }
    
    // Create users.txt file if it doesn't exist
    FILE *file = fopen(USERS_FILE, "a+");
    if (file) {
        fclose(file);
    } else {
        perror("Failed to access users file");
        exit(EXIT_FAILURE);
    }
}

void cleanup_synchronization() {
    // Close and unlink semaphore
    if (user_file_sem != SEM_FAILED) {
        sem_close(user_file_sem);
        sem_unlink(SEMAPHORE_NAME); // This removes the semaphore from the system
    }
}

bool contains_asterisk(const char *str) 
{
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '*') {
            printf("Error: Credentials cannot contain '*' character.\n");
            return true;
        }
    }
    return false;
}

bool contains_invalid(const char *str)
{
    if(strlen(str)==0)
    {
        printf("Credentials can not be empty.\n");
        return true;
    }
    else if(strlen(str)>20)
    {
        printf("Credentials can not exceed 20 characters.\n");
        return true;
    }

    else return contains_asterisk(str);
} 


// This function acquires the semaphore itself
bool is_username_unique(const char *username) {
    bool is_unique;
    
    // Wait for semaphore
    sem_wait(user_file_sem);
    
    // Use the internal function that doesn't acquire the semaphore
    is_unique = check_username_unique_internal(username);
    
    // Release semaphore
    sem_post(user_file_sem);
    
    return is_unique;
}

// Internal function that assumes the semaphore is already acquired
bool check_username_unique_internal(const char *username) {
    FILE *file;
    char line[100];
    char expected_line[100];
    bool is_unique = true;
    
    // Construct the string to search for
    snprintf(expected_line, sizeof(expected_line), "*** %s ***", username);
    
    file = fopen(USERS_FILE, "r");
    if (!file) {
        // If file doesn't exist, username is unique
        return true;
    }
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        if (strcmp(line, expected_line) == 0) {
            // Username found, not unique
            is_unique = false;
            break;
        }
    }
    
    fclose(file);
    return is_unique;
}

bool signup_user(const char *username, const char *password) {
    FILE *file;
    bool success = false;
    
    // Wait for semaphore
    sem_wait(user_file_sem);
    
    // Do a final check to make sure username is still unique
    // Using the internal function that doesn't try to acquire the semaphore again
    if (!check_username_unique_internal(username)) {
        printf("Error: Username was taken while processing. Please try another username.\n");
        sem_post(user_file_sem);
        return false;
    }
    
    // Open file in append mode
    file = fopen(USERS_FILE, "a");
    if (file) {
        // Format: *** username ***\npassword: password\n*****\n
        fprintf(file, "*** %s ***\n", username);
        fprintf(file, "password: %s\n", password);
        fprintf(file, "*****\n\n");
        
        fclose(file);
        success = true;
    }
    
    // Release semaphore
    sem_post(user_file_sem);
    
    return success;
}

bool login_user(const char *username, const char *password) {
    FILE *file;
    char line[100];
    char expected_username[100];
    char expected_password[100];
    bool found_username = false;
    bool password_matched = false;
    
    // Construct the strings to search for
    snprintf(expected_username, sizeof(expected_username), "*** %s ***", username);
    snprintf(expected_password, sizeof(expected_password), "password: %s", password);
    
    // Wait for semaphore
    sem_wait(user_file_sem);
    
    file = fopen(USERS_FILE, "r");
    if (!file) {
        sem_post(user_file_sem);
        return false;
    }
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        if (!found_username) {
            if (strcmp(line, expected_username) == 0) {
                found_username = true;
            }
        } else if (strcmp(line, expected_password) == 0) {
            password_matched = true;
            break;
        } else if (strcmp(line, "*****") == 0) {
            // We've reached the end of this user's entry without finding the password
            found_username = false;
        }
    }
    
    fclose(file);
    
    // Release semaphore
    sem_post(user_file_sem);
    
    return found_username && password_matched;
}