`timescale 1ns/1ps
//testbench 

module ZUART_Tx_tb;

//Dump FSDB file for Verdi.
initial begin
    $fsdbDumpfile("My.fsdb");
    //$fsdbDumpvars(0,"ZUART_Tx_tb","+all");
    $fsdbDumpvars(0,"ZUART_Tx_tb.myUART_Tx","+all");
end

//Two methods to generate clock.
reg clk;
//Method 1.
initial clk=0;
always #10 clk=~clk;

//Method 2.
// initial begin
//     clk=0;
//     forever #5 clk=~clk;
// end

reg rst_n;
initial begin
    rst_n=0; 
    #100;
    rst_n=1;
end

reg [7:0] tx_data;
wire tx_done;
wire tx_txd;
ZUART_Tx #(.Freq_divider(12)) myUART_Tx
(
	.iClk(clk),
	.iRst_N(rst_n),
	.iData(tx_data),

	//pull down iEn to start transmition until pulse done oDone was issued.
	.iEn(1),
	.oDone(tx_done),
	.oTxD(tx_txd)
);

//update tx data after each done.
reg [7:0] cntFSM;
always @(posedge clk or negedge rst_n)
if(!rst_n) begin
    cntFSM<=0; 
    tx_data<=8'h19;
end
else begin
    case(cntFSM)
    0: 
        if(tx_done) begin tx_data<=8'h87; cntFSM<=cntFSM+1; end
    1: 
        if(tx_done) begin tx_data<=8'h09; cntFSM<=cntFSM+1; end
    2:
        if(tx_done) begin tx_data<=8'h01; cntFSM<=cntFSM+1; end
    3:
        if(tx_done) begin 
            $display("simulation ends here: %t\n",$time);
            $finish;
        end
    endcase
end

//multiplication testing.
integer fd;
initial begin
    fd=$fopen("mydata.dat","r");
    if(fd==0) begin
        $display("can't open mydata.dat,quit.\n");
        $finish;
    end
end

reg [7:0] adc1;
reg [7:0] adc2;
reg [15:0] adc_multiple;
reg [7:0] cntFSM2;
always @(posedge clk or negedge rst_n)
if(!rst_n) begin
    adc1<=0; adc2<=0; cntFSM2<=0; 
end
else begin
    case(cntFSM2) 
    0: //check if reaches end of file.
        if($feof(fd)) begin cntFSM2<=4; end
        else begin cntFSM2<=cntFSM2+1; end
    1: //read from file.
        //xx xx 
        //xx xx 
        //xx xx
        begin $fscanf(fd,"%h %h\n",adc1,adc2); adc_multiple<=0; cntFSM2<=cntFSM2+1; end
    2:
        begin adc_multiple<=adc1*adc2; cntFSM2<=cntFSM2+1; end
    3:
        begin 
            $display("%d * %d = %d\n",adc1,adc2,adc_multiple);
            cntFSM2<=cntFSM2-3;
        end
    4:
        begin $fclose(fd); $display("reaches end of file.\n"); cntFSM2<=cntFSM2+1; end
    5:
        begin cntFSM2<=cntFSM2; end
    endcase
end
// initial begin 
//     tx_data=8'h19;
//     #100;
//     tx_data=8'h87;
//     #100;
//     tx_data=8'h09;
//     #100; 
//     tx_data=8'h01;

//     //Pause simulation at 500ns for debugging.
//     #100000;
//     $display("Simulation paused at time %t, inspect waves now.",$time);
//     //$stop(1);

//     //Simulation can be resumed from here if supported by the tool.
//     #500000;
//     $finish;

// end


endmodule
