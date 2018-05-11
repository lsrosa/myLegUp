`timescale 1 ns / 1 ns
module tester
  (
   );
   
   reg  clk;
   reg  reset;
   reg  start;
   reg [7:0] SW;
   reg [3:0] KEY;
   reg [1:0] counter;
   wire [7:0] LEDR;
   reg [7:0] 	out;
   wire [31:0] return_val;
   wire        finish;
   reg 				 waitrequest;
   
   top top_inst (
								 .clk (clk),
								 .LEDR (LEDR),
								 .SW (SW),
								 .KEY (KEY),
								 .reset (reset),
								 .start (start),
								 .finish (finish),
								 .waitrequest (waitrequest),
								 .return_val (return_val)
								 );
   

   initial 
     clk = 0;
   always @(clk)
     clk <= #10 ~clk;

   initial begin
      SW = 8'b11111010;
      counter = 2'b00;
   end
   
   initial begin
      @(negedge clk);
      reset <= 1;
      @(negedge clk);
      reset <= 0;
      start <= 1;
      
   end

   initial begin
      waitrequest <= 1;
      @(negedge clk);
      @(negedge clk);
      waitrequest <= 0;
   end   
   
   always@(posedge clk) begin
      counter <= counter + 1;
      if (counter == 2'b11)
				KEY <= 4'b0111;
      else
				KEY <= 4'b1111;
   end

   always@(*) begin
      if (LEDR == 8'b00111011)
				out = LEDR;
      else if (LEDR == 8'b11111100)
				$display("LEDR1=%b LEDR2=%b", out, LEDR);
   end
   
   always@(finish) begin
      if (finish == 1) begin
         $display("At t=%t clk=%b finish=%b", $time, clk, finish);
         $display("Cycles: %d", ($time-50)/20);
         $finish;
      end
   end
   
endmodule 
