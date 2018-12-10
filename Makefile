CC		:= gcc
C_FLAGS := -g -Wall -Wextra -std=c11 -pthread

LIBRARIES	:= -lm

EXECUTABLE	:= main.out

all: $(EXECUTABLE)

clean:
	-$(RM) $(EXECUTABLE)

run: all
	./$(EXECUTABLE)

$(EXECUTABLE): *.c
	$(CC) $(C_FLAGS) $^ -o $@ $(LIBRARIES)
