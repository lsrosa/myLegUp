module printToLEDs(
		   clk,
		   clk2x,
		   clk1x_follower,
		   start,
		   reset,
		   arg_pattern,
		   finish,
		   LEDR
		   );
   input clk, clk2x, clk1x_follower;
   input start, reset;
   input [31:0] arg_pattern;
   output reg finish;
   output reg [17:0] LEDR;

   reg [25:0] 	     counter;
   wire [25:0] 	     counter_max; 	     
   reg 		     counting;

   assign counter_max = 25000000;

   always @(posedge clk) begin
      if (start) begin
	 LEDR = arg_pattern[17:0];
	 counting = 1;
	 counter = 1;

      end
      else if (counter < counter_max) begin
	 counter = counter + 1;
      end
      else if (counting) begin
	 finish = 1;
	 counting = 0;
      end
      else begin
	 finish = 0;
      end
   end
endmodule		   
