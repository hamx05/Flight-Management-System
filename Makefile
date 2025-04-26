$(shell mkdir -p build)

OBJ_DIR = build
SRCS = main.c cancellation.c book_flight.c view_flights.c
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)
TARGET = flight_system

$(TARGET): $(OBJS)
	gcc $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: %.c
	gcc -c $< -o $@

SIM_OBJS = $(OBJ_DIR)/simulation.o $(OBJ_DIR)/cancellation.o $(OBJ_DIR)/view_flights.o $(OBJ_DIR)/book_flight.o
SIM_TARGET = sc1

$(SIM_TARGET): $(SIM_OBJS)
	gcc $(SIM_OBJS) -lpthread -o $(SIM_TARGET)

$(OBJ_DIR)/simulation.o: simulation.c
	gcc -c $< -o $@

run: $(TARGET)
	./$(TARGET)

simul: $(SIM_TARGET)
	./$(SIM_TARGET)

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET) $(SIM_TARGET)

