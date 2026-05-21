#!/bin/bash
#-p, --persist lets plot windows survive after main gnuplot program exits.
#-e "command list" executes the requested commands before loading the next input file.
gnuplot -persist \
	-e "set term wxt; \
	set title 'SIN Waveform'; \
	set xrange [0:4095]; set yrange [-20:20]; \
	set xlabel 'Time/Samples'; set ylabel 'Voltage'; \
	set label 'Peak:12345' at 100,1000 tc rgb 'blue' center; \
	plot 'sin_wave.csv' using 1:2 with linespoints pointsize 2 title '50KHz SIN', \
	'sin_wave_offset.csv' using 1:2 with linespoints pointsize 2 title '50KHz SIN With Offset', \
	'sin_wave_multify.csv' using 1:2 with linespoints pointsize 2 title '50KHz SIN Multify x2' "
