OBJS=tester.o controller.o sensors.o simulator.o still.o
HEADERS=sensors.h controller.h still.h brew.h physics.h simulator.h Arduino.h

test.png:	tester
	./tester > test.csv
	gnuplot -e "infile='test.csv'" -e "outfile='test.png'" test.plot 

tester: $(OBJS)
	g++ -o $@ $(OBJS)

$(OBJS): $(HEADERS)

clobber:
	rm -f tester test.png test.csv $(OBJS) 

clean:
	rm -f $(OBJS) test.csv
