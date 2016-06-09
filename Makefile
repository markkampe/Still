OBJS=tester.o controller.o sensors.o simulator.o still.o
HEADERS=sensors.h controller.h still.h brew.h physics.h simulator.h Arduino.h

tester: $(OBJS)
	g++ -o $@ $(OBJS)

$(OBJS): $(HEADERS)

clobber:
	rm -f tester $(OBJS)

clean:
	rm -f $(OBJS)
