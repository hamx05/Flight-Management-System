build: main.o cancellation.o view_flights.o book_flight.o
	gcc main.o cancellation.o view_flights.o book_flight.o -o flight_system

main.o: main.c
	gcc -c main.c

cancellation.o: cancellation.c
	gcc -c cancellation.c

view_flights.o: view_flights.c
	gcc -c view_flights.c

book_flight.o: book_flight.c
	gcc -c book_flight.c

simulation.o: simulation.c
	gcc -c simulation.c
	

run: build
	./flight_system

simul: simulation.o cancellation.o view_flights.o book_flight.o
	gcc simulation.o view_flights.o cancellation.o book_flight.o -lpthread -o sc1
	./sc1

clean:
	rm -f main.o cancellation.o view_flights.o book_flight.o flight_system sc1

