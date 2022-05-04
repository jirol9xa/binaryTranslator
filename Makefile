CC = g++
C_FLAGS = -c
I_FLAGS = -I include/
DEBUG_FLAGS = -fsanitize=address,leak,undefined -Wall -g

build:  main.o translator.o reader.o 
	$(CC) -no-pie main.o translator.o reader.o -o trans  $(DEBUG_FLAGS)
clear:
	rm -rf *.o


main.o:	src/main.cpp
	$(CC) $(C_FLAGS) src/main.cpp       $(I_FLAGS)
translator.o:   src/translator.cpp
	$(CC) $(C_FLAGS) src/translator.cpp $(I_FLAGS)
reader.o:       src/reader.cpp
	$(CC) $(C_FLAGS) src/reader.cpp     $(I_FLAGS)