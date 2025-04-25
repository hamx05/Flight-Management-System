#ifndef BOOK_FLIGHT_H
#define BOOK_FLIGHT_H

#include <stdbool.h>
#include <semaphore.h>

// Function to set the current username for booking operations
void set_current_username(const char *username);

// Function to get the current username
const char* get_current_username();

// Function to update user.txt with booked flight information
bool update_user_txt(const char *username, int flight_no, int seat_no);

// Function to book a flight (integrated with view_flights.c)
void update_user_flight_record(const char *username, int flight_no, int seat_no);

// Function to initialize book flight functionality
void initialize_book_flight();

// Function to cleanup book flight resources
void cleanup_book_flight();

#endif // BOOK_FLIGHT_H
