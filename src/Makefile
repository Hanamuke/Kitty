CXX = g++
CFLAGS = -O3 -std=c++1z -Wall -Wextra -Werror -Wfatal-errors -flto -pthread -march=native -fno-exceptions -DNDEBUG
EXTRACFLAGS =
LDFLAGS =
PROG = Kitty
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

all : .depend $(PROG)

$(PROG) : $(OBJ)
	$(CXX) -o $(PROG) $(CFLAGS) $(EXTRACFLAGS) $^ $(LDFLAGS)

%.o:%.cpp
	$(CXX) $(EXTRACFLAGS) $(CFLAGS) -c $<

clean :
	rm -rf *.o
	rm -f .depend

run : $(PROG)
	./$(PROG)

profile :
	make clean
	make EXTRACFLAGS=-fprofile-generate all
	./$(PROG) bench
	make clean
	make EXTRACFLAGS=-fprofile-use all
	rm *.gcda

tidy : $(SRC)
	clang-tidy $^ -checks='modernize-* bugprone-* clang-analyzer-* llvm-* performance-* readability-* hicpp-*' -- -std=c++17

.depend : $(SRC)
	$(CXX) -MM $(CFLAGS) $(EXTRACFLAGS) $^ > .depend

include .depend