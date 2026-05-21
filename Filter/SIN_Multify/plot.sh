#!/bin/bash
#-p, --persist lets plot windows survive after main gnuplot program exits.
#-e "command list" executes the requested commands before loading the next input file.
gnuplot -persist \
	-e "set term wxt; \
	set title 'SIN Waveform'; \
	set xrange [0:4095]; set yrange [-200:200]; \
	set xlabel 'Time/Samples'; set ylabel 'Voltage'; \
	set label 'Peak:12345' at 100,1000 tc rgb 'blue' center; \
	plot 'sin_50kHz.csv' using 1:2 with linespoints pointsize 0.2 title '50KHz sin', \
	'sin_1MHz.csv' using 1:2 with linespoints pointsize 0.2 title '1MHz sin', \
	'sin_modulated.csv' using 1:2 with linespoints pointsize 0.2 title '50kHz * 1MHz', \
	'sin_demodulated.csv' using 1:2 with linespoints pointsize 0.2 title 'Demodulated', \
	'sin_lpf.csv' using 1:2 with linespoints pointsize 0.2 title 'LPF' "
