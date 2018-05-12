// Assign Switches to LEDs
// Author: Mathew Hall
// Date: May 8, 2014

`timescale 1 ns / 1 ns
module assignSwitchesToLEDs
(
	clk,
	clk2x,
	clk1x_follower,
	reset,
	start,
        SW,
        LEDR,
        KEY,
	finish
);

input  clk;
input  clk2x;
input  clk1x_follower;
input  reset;
input  start;
input [5:0] SW;
input [3:0] KEY;
output [5:0] LEDR;
output reg finish;   

   parameter [1:0] idle = 2'b00;
   parameter [1:0] run = 2'b01;
   parameter [1:0] finishKeyPushed = 2'b10;
   parameter [1:0] finished = 2'b11;

   reg [1:0] state;

   always @(posedge clk or posedge reset) begin
      if (reset) begin
	 state <= idle;
      end
      else begin
	 case (state)
	   idle: if (start) state <= run;
	   run: if (~KEY[3]) state <= finishKeyPushed;
	   finishKeyPushed: if (KEY[3]) state <= finished;
	   finished: state <= idle;
	 endcase
      end
   end

   always @(*) begin
      if (state != finished)
	finish <= 0;
      else
	finish <= 1;
   end

   assign LEDR[5:0] = SW[5:0] + 1'b1;
   
endmodule 


module assignSwitchesToLEDsInverted
(
	clk,
	clk2x,
	clk1x_follower,
	reset,
	start,
        SW,
        KEY,
        LEDR,
	finish
);

input  clk;
input  clk2x;
input  clk1x_follower;
input  reset;
input  start;
input [7:2] SW;
input [3:0] KEY;
output [7:2] LEDR;
output reg finish;   

   parameter [1:0] idle = 2'b00;
   parameter [1:0] run = 2'b01;
   parameter [1:0] finishKeyPushed = 2'b10;
   parameter [1:0] finished = 2'b11;
   
   reg [1:0] state;

   always @(posedge clk or posedge reset) begin
      if (reset) begin
	 state <= idle;
      end
      else begin
	 case (state)
	   idle: if (start) state <= run;
	   run: if (~KEY[3]) state <= finishKeyPushed;
	   finishKeyPushed: if (KEY[3]) state <= finished;
	   finished: state <= idle;
	 endcase
      end
   end

   always @(*) begin
      if (state != finished)
	finish <= 0;
      else
	finish <= 1;
   end

   assign LEDR[7:2] = SW[7:2] + 1'b1;
   
endmodule
