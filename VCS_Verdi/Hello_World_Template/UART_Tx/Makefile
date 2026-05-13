#Makefile for VCS Compilation and Verdi Debugging.
#Step1: make clean
#Step2: make compile
#Step3: run "./simv -gui=verdi"  single-step debug mode.
#Step4: In Verdi, select Menu Simulation"->"Run/Continue F5"
#Step5: Open Source Editor, right click register names and select "Add to waveform"(Ctrl+W)
#In Single debug mode, we should press F5 continuously to see the full waveform.


#This file tells VCS which files to compile.
#Using a file list is cleaner than passing every files on the command line.
FILE_LIST = filelist.f

DEST_NAME = simv

#VCS Compilation Options.
VCS_OPS = -full64 \
	-sverilog \
	-debug_access+all \
	-lca \
	-kdb \
	-timescale=1ns/1ps \
	-l compile.log \
	-o $(DEST_NAME)
	
.PHONY: compile clean

compile:
	vcs $(VCS_OPS) -f $(FILE_LIST)


clean:
	rm -rvf csrc simv.daidir *.log $(DEST_NAME)
	rm -rf *.fsdb
	rm -rvf verdiLog


