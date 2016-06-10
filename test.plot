#!/usr/bin/gnuplot test.plot test

# output to a file
set term png size 1024

set key center center
set style data lines

set title "Sensor Readings"

set xlabel "time(min)"
set xdata time
set timefmt "%H%M%S"
set format x "%M"

set ylabel "Temp(F)"
set yrange [0:220]

set y2label "percent"
set y2range[0:110]
set ytics nomirror
set y2tics

# all input is CSV
set datafile sep ','

# sensor input scaling functions
degF(x) = (x/4)
degC(t) = (((x/4 - 32)/9) * 5)
alc(p) = (p/10)

# plot the sensor values
infile = "sensors.csv"
set output "sensors.png"
plot infile using 1:(degF($4)) title "Kettle" lc rgb "orange", \
     infile using 1:(degF($5)) title "Ambient" lc rgb "blue", \
     infile using 1:(degF($6)) title "Condenser" lc rgb "violet", \
     infile using 1:(alc($7)) title "Alcohol(%)" lc rgb "green" axes x1y2, \
     infile using 1:3 title "heat(%)" axes x1y2 lc rgb "red"

# plot the energy simulations
set output "energy.png"
set title "Energy Transfer"

set ylabel "Watts"
set logscale y
set yrange [.5:15000]
# make sure we have no zero values
logSafe(v) = ((v <= 0.01) ? 0.01 : v)

unset y2label
unset y2range
unset y2tics

infile = "energy.csv"
plot infile using 1:(logSafe($2)) title "heat->kettle" lc rgb "red", \
     infile using 1:(logSafe($3)) title "kettle->air"  lc rgb "blue", \
     infile using 1:(logSafe($4)) title "kettle->boil" lc rgb "green", \
     infile using 1:(logSafe($5)) title "cond->air"   lc rgb "orange"
