
CFLAGS ?= -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pedantic-errors -std=c++23 -ggdb -I./vendor/raylib-cpp-5.0.2/include

LDFLAGS ?= -lraylib -L./vendor/raylib-cpp-5.0.2/include -lGL -lm -lpthread -ldl -lrt -lX11

main: main.cc
	g++ $(CFLAGS) main.cc $(LDFLAGS) -o main 

.PHONY: run
run: main
	./main

.PHONY: clean
clean:
	rm main

