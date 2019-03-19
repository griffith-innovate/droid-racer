# makefile
# Griffith Droid Racer 2019

COMPILER = g++
CFLAGS = -g -Wall -Wextra -Werror
OPTIMISATION = -O2

all: clean main
clean:
	rm *.o -f 2> /dev/null
	rm *.out -f 2> /dev/null
	rm *.stackdump -f 2> /dev/null
	rm *.gch -f 2> /dev/null
arduino_controller:
	$(COMPILER) $(CFLAGS) $(OPTIMISATION) arduino_controller.cpp -c -o arduino_controller.o
main: arduino_controller
	$(COMPILER) $(CFLAGS) $(OPTIMISATION) arduino_controller.o main.cpp -o main.o
