CC := gcc
CFLAGS :=
CPPFLAGS :=
LDFLAGS := -lSDL2
OBJS := mandelbrot.o
TARGETS := mandelbrot

all: $(TARGETS)

mandelbrot: mandelbrot.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

mandelbrot.o: mandelbrot.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	-rm -f $(TARGETS) $(OBJS)
