#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include "book_flight.h"
#include "view_flights.h"

#define USERS_FILE "users.txt"
#define MAX_LINE_LENGTH 256
#define TEMP_FILE "temp_users.txt"

// External variables from main.c/login_module.c
extern sem_t *user_file_sem;
extern pthread_mutex_t read_mutex;
extern pthread_mutex_t write_mutex;

// Global variable to store current username
static char current_username[50] = "";

// Set the current username for the session
void set_current_username(const char *username) {
    strncpy(current_username, username, sizeof(current_username) - 1);
    current_username[sizeof(current_username) - 1] = '\0'; // Ensure null termination
}

// Get the current username
const char* get_current_username() {
    return current_username;
}

// Function to update user.txt to include the booked flight and seat
bool update_user_txt(const char *username, int flight_no, int seat_no) {
    if (strlen(username) == 0) {
        printf("Error: No user is currently logged in.\n");
        return false;
    }

    FILE *file;
    FILE *temp;
    char line[MAX_LINE_LENGTH];
    char user_header[100];
    bool found_user = false;
    bool flight_entry_found = false;
    bool success = false;
    
    // Construct the search string for the username
    snprintf(user_header, sizeof(user_header), "*** %s ***", username);
    
    // Wait for semaphore to ensure exclusive access to users.txt
    sem_wait(user_file_sem);
    
    // Open the original file for reading
    file = fopen(USERS_FILE, "r");
    if (!file) {
        perror("Error opening users file");
        sem_post(user_file_sem);
        return false;
    }
    
    // Create a temporary file for writing
    temp = fopen(TEMP_FILE, "w");
    if (!temp) {
        perror("Error creating temporary file");
        fclose(file);
        sem_post(user_file_sem);
        return false;
    }
    
    // Process the file line by line
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            fputs(line, temp);
            fputs("\n", temp);
        } else {
            fputs(line, temp);
        }
        
        // Check if we found the user entry
        if (strcmp(line, user_header) == 0) {
            found_user = true;
            
            // Read the next line (password)
            if (!fgets(line, MAX_LINE_LENGTH, file)) {
                break;
            }
            fputs(line, temp);
            
            // Check the following lines for flight entries until we hit the separator
            char flight_entry[20];
            snprintf(flight_entry, sizeof(flight_entry), "flight%d:", flight_no);
            
            char peek_line[MAX_LINE_LENGTH];
            long pos = ftell(file);  // Store current position
            
            // Look for the specific flight entry
            while (fgets(peek_line, MAX_LINE_LENGTH, file)) {
                // Remove newline for comparison
                size_t len = strlen(peek_line);
                if (len > 0 && peek_line[len-1] == '\n') {
                    peek_line[len-1] = '\0';
                }
                
                // If we hit the end marker, we're done with this user
                if (strcmp(peek_line, "*****") == 0) {
                    // Before writing the separator, add our new flight entry if it wasn't found
                    if (!flight_entry_found) {
                        fprintf(temp, "flight%d: %d\n", flight_no, seat_no);
                    }
                    // Write the separator and continue
                    fprintf(temp, "*****\n");
                    break;
                }
                
                // Check if this line contains our flight entry
                if (strncmp(peek_line, flight_entry, strlen(flight_entry)) == 0) {
                    // Extract current seat list
                    char *seat_list = strchr(peek_line, ':');
                    bool already_exists = false;
                
                    if (seat_list != NULL) {
                        seat_list++; // move past ':'
                        char updated_line[100] = {0};
                        snprintf(updated_line, sizeof(updated_line), "flight%d:", flight_no);
                
                        char *token = strtok(seat_list, ", ");
                        while (token != NULL) {
                            int seat = atoi(token);
                            if (seat == seat_no) {
                                already_exists = true;
                            }
                            snprintf(updated_line + strlen(updated_line), sizeof(updated_line) - strlen(updated_line), " %d,", seat);
                            token = strtok(NULL, ", ");
                        }
                
                        if (!already_exists) {
                            snprintf(updated_line + strlen(updated_line), sizeof(updated_line) - strlen(updated_line), " %d", seat_no);
                        } else {
                            // Remove trailing comma if exists
                            size_t len = strlen(updated_line);
                            if (updated_line[len - 1] == ',') {
                                updated_line[len - 1] = '\0';
                            }
                        }
                
                        fprintf(temp, "%s\n", updated_line);
                        flight_entry_found = true;
                    }
                }
                else {
                    // Just write the line as is
                    fprintf(temp, "%s\n", peek_line);
                }
            }
            
            // If we didn't find the flight entry and didn't hit the separator,
            // add the new entry before the separator
            if (!flight_entry_found && strcmp(peek_line, "*****") != 0) {
                fprintf(temp, "flight%d: %d\n", flight_no, seat_no);
                fprintf(temp, "*****\n");
            }
            
            success = true;
        }
    }
    
    // Close both files
    fclose(file);
    fclose(temp);
    
    // Replace the original file with the temporary file
    if (success) {
        if (remove(USERS_FILE) != 0) {
            perror("Error removing original file");
            sem_post(user_file_sem);
            return false;
        }
        
        if (rename(TEMP_FILE, USERS_FILE) != 0) {
            perror("Error renaming temporary file");
            sem_post(user_file_sem);
            return false;
        }
    } else {
        // Clean up temporary file if operation was not successful
        remove(TEMP_FILE);
    }
    
    // Release the semaphore
    sem_post(user_file_sem);
    
    return success;
}

// used in view_flights.c
// clones user.txt as temp.txt, adds the seat number user booked for himself, deletes users.txt and rename temp.txt to user.txt
void update_user_flight_record(const char *username, int flight_no, int seat_no) {
    // -------------------------- CRITICAL SECTION STARTED: Lock for writing --------------------------
    pthread_mutex_lock(&write_mutex);
    
    // Update user file with new booking
    if (strlen(username) > 0) {
        if (!update_user_txt(username, flight_no, seat_no)) {
            printf("Failed to update user record for flight booking.\n");
        }
    } else {
        printf("Error: No user is logged in. Cannot update booking record.\n");
    }

    // -------------------------- CRITICAL SECTION ENDED: Unlock for writing --------------------------
    pthread_mutex_unlock(&write_mutex);
}

// Initialize function - nothing to initialize here
void initialize_book_flight() {
    // Clear the current username at startup
    memset(current_username, 0, sizeof(current_username));
}


