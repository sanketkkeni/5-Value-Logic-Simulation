CFLAGS = -x c++ 
LIBFLAGS = -L /home/home4/pmilder/ese549/tools/lib -lfl
OPTLEVEL = -O3
SRCPP = main.cc ClassGate.cc ClassCircuit.cc 
SRCC = lex.yy.c parse_bench.tab.c

all: bison flex
	g++ $(CFLAGS) $(SRCC) $(SRCPP) $(LIBFLAGS) -o logicsim $(OPTLEVEL)

debug: bison flex
	g++ $(CFLAGS) $(SRCC) $(SRCPP) $(LIBFLAGS) -o logicsim -g

bison:
	/home/home4/pmilder/ese549/tools/bin/bison -d parse_bench.y

flex:
	/home/home4/pmilder/ese549/tools/bin/flex parse_bench.l

clean:
	rm -rf parse_bench.tab.c parse_bench.tab.h lex.yy.c logicsim

