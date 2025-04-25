#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "cancellation.h"

extern pthread_mutex_t read_mutex;
extern pthread_mutex_t write_mutex;

#define MAX_LINE_LENGTH 256
#define MAX_FLIGHTS 100
#define MAX_USERS 100

int read_flights(Flight flights[], int *num_flights) {
    FILE *file = fopen("flights.txt", "r");
    if (!file) {
        printf("Error opening flights.txt\n");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int current_flight = -1;
    *num_flights = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "*** flight")) {
            current_flight++;
            *num_flights = current_flight + 1;
            flights[current_flight].num_seats_booked = 0;
        } else if (strstr(line, "name:")) {
            strcpy(flights[current_flight].name, line + 6);
            flights[current_flight].name[strlen(flights[current_flight].name) - 1] = '\0';
        } else if (strstr(line, "date:")) {
            strcpy(flights[current_flight].date, line + 6);
            flights[current_flight].date[strlen(flights[current_flight].date) - 1] = '\0';
        } else if (strstr(line, "time:")) {
            strcpy(flights[current_flight].time, line + 6);
            flights[current_flight].time[strlen(flights[current_flight].time) - 1] = '\0';
        } else if (strstr(line, "seats_booked:")) {
            char *seats = line + 13;
            // Skip any leading spaces
            while (*seats == ' ') seats++;
            
            // Only process seats if there are any
            if (*seats != '\n' && *seats != '\0') {
                char *seat = strtok(seats, ",");
                while (seat) {
                    while (*seat == ' ') seat++;
                    flights[current_flight].seats_booked[flights[current_flight].num_seats_booked++] = atoi(seat);
                    seat = strtok(NULL, ",");
                }
            }
        }
    }

    fclose(file);
    return 1;
}

int read_users(User users[], int *num_users) {
    FILE *file = fopen("users.txt", "r");
    if (!file) {
        printf("Error opening users.txt\n");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int current_user = -1;
    *num_users = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "*** ")) {
            current_user++;
            *num_users = current_user + 1;
            users[current_user].num_bookings = 0;
            strcpy(users[current_user].username, line + 4);
            users[current_user].username[strlen(users[current_user].username) - 5] = '\0';
        } else if (strstr(line, "password:")) {
            strcpy(users[current_user].password, line + 10);
            users[current_user].password[strlen(users[current_user].password) - 1] = '\0';
        } else if (strstr(line, "flight")) {
            strcpy(users[current_user].flight_bookings[users[current_user].num_bookings], line);
            users[current_user].flight_bookings[users[current_user].num_bookings][strlen(users[current_user].flight_bookings[users[current_user].num_bookings]) - 1] = '\0';
            users[current_user].num_bookings++;
        }
    }

    fclose(file);
    return 1;
}

int write_flights(Flight flights[], int num_flights) {
    FILE *file = fopen("flights.txt", "w");
    if (!file) {
        printf("Error opening flights.txt for writing\n");
        return 0;
    }

    for (int i = 0; i < num_flights; i++) {
        fprintf(file, "*** flight%d ***\n", i + 1);
        fprintf(file, "name: %s\n", flights[i].name);
        fprintf(file, "date: %s\n", flights[i].date);
        fprintf(file, "time: %s\n", flights[i].time);
        
        // Handle seats_booked line
        if (flights[i].num_seats_booked == 0) {
            fprintf(file, "seats_booked:\n");
        } else {
            fprintf(file, "seats_booked: ");
            for (int j = 0; j < flights[i].num_seats_booked; j++) {
                fprintf(file, "%d", flights[i].seats_booked[j]);
                if (j < flights[i].num_seats_booked - 1) {
                    fprintf(file, ", ");
                }
            }
            fprintf(file, "\n");
        }
        
        fprintf(file, "\n*****\n\n");
    }

    fclose(file);
    return 1;
}

int write_users(User users[], int num_users) {
    FILE *file = fopen("users.txt", "w");
    if (!file) {
        printf("Error opening users.txt for writing\n");
        return 0;
    }

    for (int i = 0; i < num_users; i++) {
        fprintf(file, "*** %s ***\n", users[i].username);
        fprintf(file, "password: %s\n", users[i].password);
        
        for (int j = 0; j < users[i].num_bookings; j++) {
            fprintf(file, "%s\n", users[i].flight_bookings[j]);
        }
        
        fprintf(file, "*****\n\n");
    }

    fclose(file);
    return 1;
}

// Function to cancel a flight booking
int cancel_booking(const char *username, int flight_number, int seat_number) 
{

    Flight flights[MAX_FLIGHTS];
    User users[MAX_USERS];
    int num_flights = 0, num_users = 0;
    int flight_index = flight_number - 1;  // Convert to 0-based index
    int user_index = -1;
    int booking_index = -1;
    int seat_index = -1;
    char flight_id[20];

    // Validate flight number
    if (flight_number < 1) {
        printf("Invalid flight number\n");
        return 0;
    }

    // Acquire read mutex to read files
    pthread_mutex_lock(&read_mutex);
    
    if (!read_flights(flights, &num_flights) || !read_users(users, &num_users)) {
        pthread_mutex_unlock(&read_mutex);
        return 0;
    }

    // Validate flight number against available flights
    if (flight_index >= num_flights) {
        pthread_mutex_unlock(&read_mutex);
        printf("You havent booked flight number %d! \n", flight_number);
        return 0;
    }

    sprintf(flight_id, "flight%d", flight_number);

    // Find the user
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        pthread_mutex_unlock(&read_mutex);
        printf("User not found\n");
        return 0;
    }

    // Check if user has booked this flight
    for (int i = 0; i < users[user_index].num_bookings; i++) {
        if (strstr(users[user_index].flight_bookings[i], flight_id)) {
            booking_index = i;
            break;
        }
    }

    if (booking_index == -1) {
        pthread_mutex_unlock(&read_mutex);
        printf("User has not booked flight %d\n", flight_number);
        return 0;
    }

    // Check if the seat is booked by this user
    for (int i = 0; i < flights[flight_index].num_seats_booked; i++) {
        if (flights[flight_index].seats_booked[i] == seat_number) {
            char *seats = strstr(users[user_index].flight_bookings[booking_index], ":") + 1;
            char *current = seats;
            bool seat_booked_by_user = false;
            
            while (*current) {
                while (*current == ' ') current++;
                
                // Parse the seat number
                int current_seat = 0;
                while (*current >= '0' && *current <= '9') {
                    current_seat = current_seat * 10 + (*current - '0');
                    current++;
                }
                
                while (*current == ' ' || *current == ',') current++;
                
                if (current_seat == seat_number) {
                    seat_booked_by_user = true;
                    break;
                }
            }
            
            if (seat_booked_by_user) {
                seat_index = i;
                break;
            }
        }
    }

    if (seat_index == -1) {
        pthread_mutex_unlock(&read_mutex);
        printf("Seat %d is not booked by you on flight %d\n", seat_number, flight_number);
        return 0;
    }

    // Release read mutex and acquire write mutex
    pthread_mutex_unlock(&read_mutex);
    pthread_mutex_lock(&write_mutex);

    // Remove the seat from the flight
    for (int i = seat_index; i < flights[flight_index].num_seats_booked - 1; i++) {
        flights[flight_index].seats_booked[i] = flights[flight_index].seats_booked[i + 1];
    }
    flights[flight_index].num_seats_booked--;

    // Update the user's booking
    char new_booking[100] = "";
    char *seats = strstr(users[user_index].flight_bookings[booking_index], ":") + 1;
    char *current = seats;
    int first = 1;

    while (*current) {
        while (*current == ' ') current++;
        
        int current_seat = 0;
        while (*current >= '0' && *current <= '9') {
            current_seat = current_seat * 10 + (*current - '0');
            current++;
        }
        
        while (*current == ' ' || *current == ',') current++;
        
        // Add the seat if it's not the one being cancelled
        if (current_seat != 0 && current_seat != seat_number) {
            if (!first) {
                strcat(new_booking, ", ");
            }
            char seat_str[10];
            sprintf(seat_str, "%d", current_seat);
            strcat(new_booking, seat_str);
            first = 0;
        }
    }

    if (strlen(new_booking) > 0) {
        sprintf(users[user_index].flight_bookings[booking_index], "%s: %s", flight_id, new_booking);
    } else {
        // Remove the booking if no seats left
        for (int i = booking_index; i < users[user_index].num_bookings - 1; i++) {
            strcpy(users[user_index].flight_bookings[i], users[user_index].flight_bookings[i + 1]);
        }
        users[user_index].num_bookings--;
    }

    // Write the updated data back to files
    if (!write_flights(flights, num_flights) || !write_users(users, num_users)) {
        pthread_mutex_unlock(&write_mutex);
        return 0;
    }

    pthread_mutex_unlock(&write_mutex);
    printf("Booking cancelled successfully!\n");
    return 1;
}