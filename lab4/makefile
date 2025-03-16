CC = gcc
CFLAGS = -c -fPIC
LDFLAGS = -shared
DLFLAGS = -ldl
RM = rm -f

GCF_SRCS = libgcf_euclid.c libgcf_naive.c
SQUARE_SRCS = libpi_leibniz.c libpi_wallace.c
PROGRAMS = program1 program2

all: $(PROGRAMS)

libgcfeuclid.so: libgcf_euclid.o
	$(CC) $(LDFLAGS) -o $@ $^

libgcfnaive.so: libgcf_naive.o
	$(CC) $(LDFLAGS) -o $@ $^

libpileibniz.so: libpi_leibniz.o
	$(CC) $(LDFLAGS) -o $@ $^

libpiwallace.so: libpi_wallace.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

program1: program1.c libgcfeuclid.so libpileibniz.so
	$(CC) -o $@ $< -L. -lgcfeuclid -lpileibniz

program2: program2.c libgcfeuclid.so libpileibniz.so
	$(CC) -o $@ $< $(DLFLAGS)

run1: program1
	LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH ./program1

run2: program2
	LD_LIBRARY_PATH=$(PWD):$$LD_LIBRARY_PATH ./program2

clean:
	$(RM) *.o *.so $(PROGRAMS)
