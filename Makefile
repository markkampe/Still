OBJS=tester.o still.o simulator.o
tester: $(OBJS)
	g++ -o $@ $(OBJS)

clobber:
	rm -f tester $(OBJS)

clean:
	rm -f $(OBJS)
