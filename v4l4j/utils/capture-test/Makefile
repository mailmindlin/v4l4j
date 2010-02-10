all: capture-test

capture-test: v4l.o main.o
	gcc v4l.o main.o -o capture-test

clean:
	rm -Rf *.o *~ capture-test
