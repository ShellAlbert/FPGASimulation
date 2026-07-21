#!/bin/bash
gnuplot -persist \
	-e "set term wxt; \
	set title 'ASK Modulation (50 kHz on 2 MHz carrier)'; \
	set xrange [0:4095]; set yrange [-20:120]; \
	set xlabel 'Sample'; set ylabel 'Amplitude'; \
	plot 'sin_50kHz.csv' using 1:2 with lines title '50 kHz message', \
	'ask_envelope.csv' using 1:2 with lines title 'ASK envelope', \
	'sin_2MHz.csv' using 1:2 with lines title '2 MHz carrier', \
	'ask_modulated.csv' using 1:2 with lines title 'ASK modulated' "
