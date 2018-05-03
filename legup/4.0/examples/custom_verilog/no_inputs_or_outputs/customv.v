// Custom V
// Author: Mathew Hall
// Date: May 7, 2014

`timescale 1 ns / 1 ns
module customAdd
(
	clk,
	clk2x,
	clk1x_follower,
	reset,
	start,
	finish,
	return_val,
	arg_i,
	arg_j
);

   parameter  LEGUP_0 = 1'd0;
   parameter  LEGUP_F_testadd_BB_0_1 = 1'd1;

   input  clk;
   input clk2x;
   input clk1x_follower;
   input reset;
   input start;
   output finish;
   output [31:0] return_val;
   input [31:0] arg_i;
   input [31:0] arg_j;

   reg 	     cur_state;
   reg [31:0] testadd_0_1;
   assign return_val = arg_i + arg_j;
   assign finish = start; 
endmodule 
