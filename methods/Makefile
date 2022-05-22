# ------------------------------------------------------------------------------
#  Makefile
# ------------------------------------------------------------------------------
OBJS=pri_queue.o util.o qalsh.o srp_lsh.o cone_tree.o block.o baseline.o \
	h2_alsh.o h2_simpfer.o sa_simpfer.o sah.o armips.o main.o

CXX=g++ -std=c++17
# CXX=g++-8 -std=c++17
OMP=-fopenmp -lpthread
OPT=-w -O3

# ------------------------------------------------------------------------------
#  Compile with C++17 and OpenMP
# ------------------------------------------------------------------------------
all:$(OBJS)
	$(CXX) $(OMP) $(OPT) -o rmips $(OBJS)

%.o: %.cc
	$(CXX) $(OMP) -c $(OPT) -o $@ $<

clean:
	-rm $(OBJS) rmips
