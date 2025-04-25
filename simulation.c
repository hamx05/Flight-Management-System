#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include "view_flights.h"
#include "book_flight.h"
#include "cancellation.h"

int flight_no = 5,seat_no=19;

sem_t *user_file_sem;
bool is_running = true;
pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER; // protects readers_threads_count
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER; // ensures one writer at a time

void* try_booking(void* args)
{
    flights flights_details[MAX_FLIGHTS];
    int id = *((int*)args);
    printf("Thread %d: trying to book seat %d on flight %d...\n", id,seat_no,flight_no);
    book_given_seat(seat_no, flight_no);
    update_user_flight_record("hammad",flight_no,seat_no);
    return NULL;
}

void* try_cancel(void* args)
{
    flights flights_details[MAX_FLIGHTS];
    int id = *((int*)args);
    printf("Thread %d: trying to cancel seat %d on flight %d...\n", id,seat_no,flight_no);
    cancel_booking("hammad",flight_no, seat_no);
    return NULL;
}

int main() {
    user_file_sem = sem_open("/user_file_sem", O_CREAT, 0644, 1);
    if (user_file_sem == SEM_FAILED) {
        perror("Failed to create semaphore");
        return 1; 
    }
    
    pthread_t tid[4];
    int id[] = {1,2,3,4};
    
    printf("\n-----------------------------------------------------------------\n");
    printf("\tSimulating Scenario: 2 threads try to book the same seat");
    printf("\n-----------------------------------------------------------------\n\n");
    
    pthread_create(&tid[0], NULL, try_booking, &id[0]);
    pthread_create(&tid[1], NULL, try_booking, &id[1]);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    printf("\n-----------------------------------------------------------------\n");
    printf("\tSimulation finished.");
    printf("\n-----------------------------------------------------------------\n\n");

    sleep(2);

    printf("\n-----------------------------------------------------------------\n");
    printf("\tSimulating Scenario: 2 threads try to cancel the same seat");
    printf("\n-----------------------------------------------------------------\n\n");

    pthread_create(&tid[2], NULL, try_cancel, &id[2]);
    pthread_create(&tid[3], NULL, try_cancel, &id[3]);
    pthread_join(tid[2], NULL);
    pthread_join(tid[3], NULL);

    printf("\n-----------------------------------------------------------------\n");
    printf("\tSimulation finished.");
    printf("\n-----------------------------------------------------------------\n\n");
    
    // Clean up
    sem_close(user_file_sem);
    sem_unlink("/user_file_sem");
    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&write_mutex);
    return 0;
}