#!/bin/bash
gnuplot -persist \
	-e "set term wxt; \
	set title 'ASK / OOK Waveforms'; \
	set xrange [0:2047]; set yrange [-200:200]; \
	set xlabel 'Sample'; set ylabel 'Amplitude'; \
	plot 'sin_message.csv' using 1:2 with lines title '50 kHz message', \
	'ook_modulated.csv' using 1:2 with lines title 'OOK modulated', \
	'ook_demodulated.csv' using 1:2 with lines title 'OOK demodulated', \
	'ask_modulated.csv' using 1:2 with lines title 'ASK modulated', \
	'ask_demodulated.csv' using 1:2 with lines title 'ASK demodulated' "
