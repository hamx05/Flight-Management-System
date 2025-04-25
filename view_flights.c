#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "view_flights.h"
#include "book_flight.h"
static int readers_threads_count=0;

void get_flight_details(flights flight_array[MAX_FLIGHTS])
 {
    pthread_mutex_lock(&read_mutex); //locking while incrementing counter
    
    readers_threads_count++;

    if (readers_threads_count == 1)
         pthread_mutex_lock(&write_mutex);  // First reader locks files from being overwritten while reading

    pthread_mutex_unlock(&read_mutex);  //allowing multiple threads to read.

//------------------------------------------------ reader section (critical section) Starts

    FILE *file=fopen(FLIGHT_FILE_PATH, "r");
    if(!file) 
    {
        perror("Error opening file");
        pthread_mutex_lock(&read_mutex);
        readers_threads_count--;
        if (readers_threads_count == 0)
            pthread_mutex_unlock(&write_mutex);
        pthread_mutex_unlock(&read_mutex);
        return;
    }

    char *line_reader=NULL;
    size_t chars_read=0;
    int flight_no=0;
   
    for(int i=0; i<MAX_FLIGHTS; i++) 
        flight_array[i].seats_booked=NULL;  

    while((getline(&line_reader, &chars_read, file) != -1) &&(flight_no<MAX_FLIGHTS))
     {
        if(strstr(line_reader, "name:")==line_reader)  // if line starts with "name:"
            snprintf(flight_array[flight_no].name, MAX_LENGTH, "%s", line_reader + 6);  //  line_reader + 6 Skips "name: "

        else if(strstr(line_reader, "date:")==line_reader) 
            snprintf(flight_array[flight_no].date, MAX_LENGTH, "%s", line_reader + 6); 

        else if(strstr(line_reader, "time:")==line_reader) 
            snprintf(flight_array[flight_no].time, MAX_LENGTH, "%s", line_reader + 6); 

        else if(strstr(line_reader, "seats_booked:")==line_reader) 
            {
                char *seats_str=line_reader + 13; // Skip "seats_booked: " 
            
                if(seats_str[0]=='\n' || seats_str[0]=='\0') // No seats booked
                {
                    flight_array[flight_no].seat_count=0;
                    flight_array[flight_no].seats_booked=NULL;
                }

                else 
                {
                    int seat_index=0,seat_count=1;

                    char *temp=seats_str;
                    while(*temp != '\0') 
                    {
                        if(*temp==',')
                            seat_count++;
                
                        temp++;
                    }

                    flight_array[flight_no].seats_booked=malloc(seat_count * sizeof(int));
                    if(!flight_array[flight_no].seats_booked) 
                    {
                        perror("Error allocating memory for seats_booked");
                        fclose(file);
                        pthread_mutex_lock(&read_mutex);
                        readers_threads_count--;
                        if (readers_threads_count == 0)
                            pthread_mutex_unlock(&write_mutex);
                        pthread_mutex_unlock(&read_mutex);
                        return;
                    }

                    int i=0;
                    while(seats_str[i] != '\0' && seats_str[i] != '\n') 
                    {
                       
                         while(seats_str[i]==' ' || seats_str[i]==',') // Skip over spaces and commas          
                         i++;

                         if(seats_str[i] >= '0' && seats_str[i] <= '9') 
                        {
                            int seat_number=seats_str[i] - '0'; // Extract the number as an integer
                            i++;
                            
                            while(seats_str[i] >= '0' && seats_str[i] <= '9') //if seat number is multiple digits
                            {
                                seat_number=seat_number * 10 +(seats_str[i] - '0'); 
                                i++;
                            }

                            flight_array[flight_no].seats_booked[seat_index]=seat_number;
                            seat_index++;
                            flight_array[flight_no].seat_count=seat_index;
                        } 
                     }
                }
            }

        if(strstr(line_reader, "*****") != NULL) 
            flight_no++;
        
    }

    fclose(file);
    free(line_reader);

    

//------------------------------------------------ reader section (critical section) Ends

    pthread_mutex_lock(&read_mutex); //lock while decrementing counter
    readers_threads_count--;
    if (readers_threads_count == 0)
        pthread_mutex_unlock(&write_mutex);  // Last reader unlocks writers
    pthread_mutex_unlock(&read_mutex);
}

void get_flight_details_for_writer(flights flight_array[MAX_FLIGHTS]) //to be used to a function already in a mutex, if we need ti determine if a seat is booked or not.
    {


       FILE *file=fopen(FLIGHT_FILE_PATH, "r");
       if(!file) 
       {
           perror("Error opening file");
           return;
       }
   
       char *line_reader=NULL;
       size_t chars_read=0;
       int flight_no=0;
      
       for(int i=0; i<MAX_FLIGHTS; i++) 
           flight_array[i].seats_booked=NULL;  
   
       while((getline(&line_reader, &chars_read, file) != -1) &&(flight_no<MAX_FLIGHTS))
        {
           if(strstr(line_reader, "name:")==line_reader)  // if line starts with "name:"
               snprintf(flight_array[flight_no].name, MAX_LENGTH, "%s", line_reader + 6);  //  line_reader + 6 Skips "name: "
   
           else if(strstr(line_reader, "date:")==line_reader) 
               snprintf(flight_array[flight_no].date, MAX_LENGTH, "%s", line_reader + 6); 
   
           else if(strstr(line_reader, "time:")==line_reader) 
               snprintf(flight_array[flight_no].time, MAX_LENGTH, "%s", line_reader + 6); 
   
           else if(strstr(line_reader, "seats_booked:")==line_reader) 
               {
                   char *seats_str=line_reader + 13; // Skip "seats_booked: " 
               
                   if(seats_str[0]=='\n' || seats_str[0]=='\0') // No seats booked
                   {
                       flight_array[flight_no].seat_count=0;
                       flight_array[flight_no].seats_booked=NULL;
                   }
   
                   else 
                   {
                       int seat_index=0,seat_count=1;
   
                       char *temp=seats_str;
                       while(*temp != '\0') 
                       {
                           if(*temp==',')
                               seat_count++;
                   
                           temp++;
                       }
   
                       flight_array[flight_no].seats_booked=malloc(seat_count * sizeof(int));
                       if(!flight_array[flight_no].seats_booked) 
                       {
                           perror("Error allocating memory for seats_booked");
                           fclose(file);
                           return;
                       }
   
                       int i=0;
                       while(seats_str[i] != '\0' && seats_str[i] != '\n') 
                       {
                          
                            while(seats_str[i]==' ' || seats_str[i]==',') // Skip over spaces and commas          
                            i++;
   
                            if(seats_str[i] >= '0' && seats_str[i] <= '9') 
                           {
                               int seat_number=seats_str[i] - '0'; // Extract the number as an integer
                               i++;
                               
                               while(seats_str[i] >= '0' && seats_str[i] <= '9') //if seat number is multiple digits
                               {
                                   seat_number=seat_number * 10 +(seats_str[i] - '0'); 
                                   i++;
                               }
   
                               flight_array[flight_no].seats_booked[seat_index]=seat_number;
                               seat_index++;
                               flight_array[flight_no].seat_count=seat_index;
                           } 
                        }
                   }
               }
   
           if(strstr(line_reader, "*****") != NULL) 
               flight_no++;
           
       }
   
       fclose(file);
       free(line_reader);    

}

int select_flight(flights flight[])
{
    char choice;
    printf("Following Are the flights Avaialable:-\n\n");
    for(int i=0; i<MAX_FLIGHTS; i++) 
          printf("%d) %s", i+1, flight[i].name);

    printf("\n(Note: enter any other character to abort)\nSelect flight: ");
    scanf(" %c",&choice);    

    if((choice-'0')<1 ||(choice-'0')>MAX_FLIGHTS)
    return -1;

    return(choice-'0' -1);
}

int seat_is_booked(int seat, int booked_seats[], int number_of_booked_seats) 
{

    for(int i=0; i<number_of_booked_seats; i++) 
        if(booked_seats[i]==seat)
            return 1;

    return 0;        
}

void print_seat(int i,int seat_taken)
{
    if(seat_taken)
       printf("{  } ");
    else
       printf("{%02d} ", i);

    if(i  % 6==3)
        printf("\t\t");

    if(i % 6==0)
        printf("\n");

    if(i  % 18==0)
        printf("\n");
}

void print_available_seats(int booked_seats[], int number_of_booked_seats) 
{
    if(number_of_booked_seats==0)
       {
          for(int i=1; i <= MAX_SEATS; i++) 
              print_seat(i,0);

          return;
       }

    for(int i=1; i <= MAX_SEATS; i++) 
    {
        if(seat_is_booked(i, booked_seats, number_of_booked_seats)) 
           print_seat(i,1);
        else
            print_seat(i,0);
    }    
        
}

int select_seat(int booked_seats[], int number_of_booked_seats)
{
    int choice;
    char temp;
    printf("\n(Note: enter 0 or any invalid seat number to cancel)\n");

    try_again:
    printf("Which seat to book: ");
    if (scanf(" %d", &choice) != 1) {
        // Invalid input (non-integer)
        while ((temp = getchar()) != '\n' && temp != EOF); // clear buffer
        return -1;
    }
    
    if( choice <1 || choice>MAX_SEATS)
    return -1;

    else if(seat_is_booked(choice, booked_seats, number_of_booked_seats)) 
    {
        printf("Seat %d is already booked. try again\n",choice);
        goto  try_again;
    }

    else   
      return choice;
}

void book_given_seat(int seat_no, int flight_number)
{

    int seat_booked_just_now=0;
    pthread_mutex_lock(&write_mutex);

    flights flights_details[MAX_FLIGHTS];
    get_flight_details_for_writer(flights_details);

    if(seat_is_booked(seat_no, flights_details[flight_number-1].seats_booked, flights_details[flight_number-1].seat_count))
    {
        printf("Seat is already Booked! cant book it twice!\n");
        pthread_mutex_unlock(&write_mutex);
        return;
    }

    /*locking reading the file with writer mutex too, because if we read a file while its about to be manipulated, 
    another program might read it before the changes manipulate it, not saving the first change. (TOCTOU bugs (Time of Check to Time of Use))*/

//------------------------------------------------ Writer section (critical section) Starts


    FILE *file=fopen(FLIGHT_FILE_PATH, "r");
    if(!file)
    {
        perror("Error opening file");
        pthread_mutex_unlock(&write_mutex);
        return;
    }

    char lines[100][200];
    int total_lines=0;

    while(fgets(lines[total_lines], sizeof(lines[0]), file))
        total_lines++;

    fclose(file);
      


    char flight_heading[20];
    sprintf(flight_heading, "*** flight%d ***", flight_number);

    int i;
    for(i=0; i<total_lines; i++)
    {
        if(strstr(lines[i], flight_heading)) //found *** flightn *** heading
            break;
    }

    while(i<total_lines && !strstr(lines[i], "seats_booked:"))
        i++;


    char *line=lines[i]; //the line to update

    // Check if this is the first seat(because then we dont add ,)
    if(strchr(line, ':') &&(strchr(line, ':')[1]=='\n' || strchr(line, ':')[1]=='\0'))
        sprintf(lines[i], "seats_booked: %d\n", seat_no);

    else
    {
        line[strcspn(line, "\n")]=0; // remove newline
        sprintf(lines[i], "%s, %d\n", line, seat_no);
    }
     

    file=fopen(FLIGHT_FILE_PATH, "w");
    if(!file)
    {
        perror("Error writing to file");
        pthread_mutex_unlock(&write_mutex);
        return;
    }

    for(int j=0; j<total_lines; j++)
        fputs(lines[j], file);

    fclose(file);

//------------------------------------------------ writer section (critical section) Ends
    seat_booked_just_now=1;
    pthread_mutex_unlock(&write_mutex);

    const char* username = get_current_username();
    if (strlen(username) > 0) {
        update_user_txt(username, flight_number, seat_no);
    }

    if(!seat_booked_just_now)
        printf("Seat is already Booked! Cant book it twice!\n");
    
    else
        printf("Seat %d booked successfully for flight %d.\n", seat_no, flight_number);
}

void View_flights(const char* username) //this is the function that will be called in main
{
    printf("\n-----------------------------------------------------------------\n");
    printf("\tView Flights section");
    printf("\n-----------------------------------------------------------------\n\n");

    flights flights_details[MAX_FLIGHTS];
    Diplay_flights_again:
    get_flight_details(flights_details);
    int flight_no=select_flight(flights_details);

    if(flight_no==-1)
    {
        for(int i=0; i<MAX_FLIGHTS; i++) 
           free(flights_details[i].seats_booked);

            printf("\n-----------------------------------------------------------------\n");
            printf("\tAborted Flight Selection: Returning to User Menu");
            printf("\n-----------------------------------------------------------------\n");
        return;
    }

    printf("\n--------------------------------------------------\n");
    printf("\t\tFlight Information:\n");
    printf("Flight no: %d\n", flight_no + 1);
    printf("Route: %s", flights_details[flight_no].name);
    printf("Date: %s", flights_details[flight_no].date);
    printf("Time: %s", flights_details[flight_no].time);

  display_seats_again:

    printf("\nSeats Available:-\n\n");
    print_available_seats(flights_details[flight_no].seats_booked,flights_details[flight_no].seat_count);
    printf("\n--------------------------------------------------\n\n");

    int seat_to_book=select_seat(flights_details[flight_no].seats_booked,flights_details[flight_no].seat_count);
    if(seat_to_book==-1)
    {
        printf("\n-----------------------------------------------------------------\n");
        printf("\tAborted Seat Booking: Returning to Flights section");
        printf("\n-----------------------------------------------------------------\n\n");
        goto Diplay_flights_again;
    }
    else 
       book_given_seat(seat_to_book,flight_no+1);
       update_user_flight_record(username,flight_no+1,seat_to_book);
       get_flight_details(flights_details);
       goto display_seats_again;

}

/* here is how it would be expected called in main 
int main() 
{
    View_flights();
    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&write_mutex);
    return 0;
} */
