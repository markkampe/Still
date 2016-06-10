STILL_OBJS=controller.o sensors.o
SIM_OBJS=simulator.o still.o SD.o physics.o
TEST_OBJS=tester.o
OBJS=$(TEST_OBJS) $(STILL_OBJS) $(SIM_OBJS)

STILL_HDRS=sensors.h controller.h
SIM_HDRS=simulator.h still.h brew.h physics.h Arduino.h SD.h
HDRS=$(STILL_HDRS) $(SIM_HDRS)

GRAPHS=sensors.png energy.png
CSVS=sensors.csv energy.csv

GRAPHS: $(CSVS)
	gnuplot test.plot

$(CSVS): tester
	./tester

tester: $(OBJS)
	g++ -o $@ $(OBJS)

$(OBJS): $(HDRS)

clobber:
	rm -f tester $(GRAPHS) $(OBJS) $(CSVS)

clean:
	rm -f $(OBJS) $(CSVS)
