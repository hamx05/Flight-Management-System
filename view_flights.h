#ifndef VIEW_FLIGHTS_H
#define VIEW_FLIGHTS_H

#define MAX_FLIGHTS 5
#define MAX_LENGTH 100
#define MAX_SEATS 60
#define FLIGHT_FILE_PATH "flights.txt"

#include <pthread.h>       //Will not work on windows
#include <stdbool.h>

extern pthread_mutex_t read_mutex;
extern pthread_mutex_t write_mutex;


typedef struct 
{
    char name[MAX_LENGTH];
    char date[MAX_LENGTH];
    char time[MAX_LENGTH];
    int seat_count;
    int *seats_booked; 
} flights;

// Reader functions
void get_flight_details(flights flight_array[MAX_FLIGHTS]);
void get_flight_details_for_writer(flights flight_array[MAX_FLIGHTS]);
int select_flight(flights flight[]);
int seat_is_booked(int seat, int booked_seats[], int number_of_booked_seats);
void print_seat(int i, int seat_taken);
void print_available_seats(int booked_seats[], int number_of_booked_seats);
int select_seat(int booked_seats[], int number_of_booked_seats);

// Writer functions
void book_given_seat(int seat_no, int flight_number);

// Main user flow
void View_flights();

#endif // VIEW_FLIGHTS_H
