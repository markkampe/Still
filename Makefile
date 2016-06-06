OBJS=tester.o still.o sensors.o simulator.o

tester: $(OBJS)
	g++ -o $@ $(OBJS)

$(OBJS): sensors.h still.h simulator.h Arduino.h

clobber:
	rm -f tester $(OBJS)

clean:
	rm -f $(OBJS)
