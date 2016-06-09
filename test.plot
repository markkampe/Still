#!/usr/bin/gnuplot test.plot test

# output to a file
set term png
set output outfile

set key bottom left
set style data lines

set title "Still Test Run"

set xlabel "time(mm:ss)"
set xdata time
set timefmt "%s"

set ylabel "temp(F)"
set yrange [0:220]

set y2label "percent"
set y2range[0:110]
set ytics nomirror
set y2tics

set datafile sep ','

plot infile using 1:3 title "heat" axes x1y2 lc rgb "red", \
     infile using 1:4 title "Kettle" lc rgb "orange", \
     infile using 1:5 title "Ambient" lc rgb "blue", \
     infile using 1:6 title "Condenser" lc rgb "violet", \
     infile using 1:7 title "Alcohol" lc rgb "green" axes x1y2

# and plot the power on the right axis
