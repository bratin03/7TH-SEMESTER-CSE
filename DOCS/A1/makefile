# Executables
EXES = fullgc incrementalgc generationalgc nogc

# Lua directory and library
LUADIR = ./lua-5.4.7

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm -ldl

# Source and object files
SRCS = $(EXES:%=%.c)
OBJS = $(SRCS:%.c=%.o)

# Rules
all: $(EXES) 

# Build executables
$(EXES): %: %.c $(LUADIR)/src/liblua.a testbench.lua
	$(CC) $(CFLAGS) -o $@ $< $(LUADIR)/src/liblua.a -I$(LUADIR)/src $(LDFLAGS)

# Callgrind analysis
callgrind: $(EXES:%=result/%.callgrind)

$(EXES:%=result/%.callgrind): result/%.callgrind: %
	mkdir -p result
	valgrind --tool=callgrind --callgrind-out-file=$@ ./$<

# Annotate callgrind results
annotate: $(EXES:%=result/%.callgrind.annotate)

$(EXES:%=result/%.callgrind.annotate): %.annotate: %
	callgrind_annotate --tree=calling $< > $@.calling
	callgrind_annotate --inclusive=yes $< > $@.inclusive
	callgrind_annotate --inclusive=no $< > $@.exclusive
	
# Convert callgrind to dot format with filtering and depth limitation
dot: $(EXES:%=result/%.dot)

$(EXES:%=result/%.dot): result/%.dot: result/%.callgrind
	python3 -m gprof2dot -e0.005 -n0.005 -f callgrind -o $@ $<

# Convert dot to high-resolution PDF
pdf: $(EXES:%=result/%.pdf)

$(EXES:%=result/%.pdf): result/%.pdf: result/%.dot
	dot -Tpdf -Gdpi=300 -Gnodesep=0.4 -Granksep=0.7 $< -o $@

# Open callgrind results with kcachegrind
kcachegrind: $(EXES:%=result/%.callgrind)
	kcachegrind $(EXES:%=result/%.callgrind)

# Clean build artifacts except pdf files
clean:
	rm -rf $(EXES) $(OBJS) result/*.callgrind result/*.annotate* result/*.dot
