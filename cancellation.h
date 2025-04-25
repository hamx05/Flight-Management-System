#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern pthread_mutex_t read_mutex;
extern pthread_mutex_t write_mutex;

typedef struct {
    char name[100];
    char date[50];
    char time[50];
    int seats_booked[100];
    int num_seats_booked;
} Flight;

typedef struct {
    char username[100];
    char password[100];
    char flight_bookings[100][100];  // Format: "flightX: seat1, seat2, ..."
    int num_bookings;
} User;

int read_flights(Flight flights[], int *num_flights);
int read_users(User users[], int *num_users);
int write_flights(Flight flights[], int num_flights);
int write_users(User users[], int num_users);
int cancel_booking(const char *username, int flight_number, int seat_number);
