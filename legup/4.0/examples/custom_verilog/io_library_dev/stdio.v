// Verilog Standard I/O
// Author: Mathew Hall
// Date: May 23, 2014

module boardPutChar
  (
   clk,
   clk2x,
   clk1x_follower,
   reset,
   start,
   UART_BYTE_OUT,
   UART_START_SEND,
   UART_RESPONSE,
   LEDR,
   KEY,
   arg_character,
   finish,
   return_val
   );
   
   input  clk;
   input  clk2x;
   input  clk1x_follower;
   input  reset;
   input  start;
   input [3:0] KEY;
   output [17:0] LEDR;
   output reg [7:0] UART_BYTE_OUT;
   output reg	    UART_START_SEND;
   input [1:0] 	    UART_RESPONSE;
   input [31:0]     arg_character;
   output reg 	    finish;
   output [31:0]    return_val;
   
   parameter [3:0] idle = 4'h0;
   parameter [3:0] run = 4'h1;
   parameter [3:0] send_byte_0 = 4'h3;
   parameter [3:0] sending_byte_0 = 4'h4;
   parameter [3:0] send_byte_1 = 4'h5;
   parameter [3:0] sending_byte_1 = 4'h6;
   parameter [3:0] send_byte_2 = 4'h7;
   parameter [3:0] sending_byte_2 = 4'h8;
   parameter [3:0] send_byte_3 = 4'h9;
   parameter [3:0] sending_byte_3 = 4'ha;
   parameter [3:0] finished = 4'hb;
   
   reg [3:0] 	state;

   assign return_val = arg_character;
   
   always @(posedge clk or posedge reset) begin
      if (reset) begin
	 state <= idle;
      end
      else begin
	 case (state)
	   idle: if (start) state <= send_byte_0;
	   send_byte_0: if (~start) state <= sending_byte_0;
	   sending_byte_0: if (UART_RESPONSE[0]) state <= send_byte_1;
	   send_byte_1: state <= sending_byte_1;
	   sending_byte_1: if (UART_RESPONSE[0]) state <= send_byte_2;
	   send_byte_2: state <= sending_byte_2;
	   sending_byte_2: if (UART_RESPONSE[0]) state <= send_byte_3;
	   send_byte_3: state <= sending_byte_3;
	   sending_byte_3: if (UART_RESPONSE[0]) state <= finished; 
	   finished: state <= finished;
	 endcase
      end
   end
   
   always @(*) begin
      finish = 0;
      case (state)
	send_byte_0: begin
	   UART_BYTE_OUT = 8'd65;//arg_character[7:0];
	   UART_START_SEND = 1;
	end
	send_byte_1: begin
	   UART_BYTE_OUT = 8'd65;	//arg_character[15:8];
	   UART_START_SEND = 1;
	end
	send_byte_2: begin
	   UART_BYTE_OUT = 8'd65;	//arg_character[23:16];
	   UART_START_SEND = 1;
	end
	send_byte_3: begin
	   UART_BYTE_OUT = 8'd65;	//arg_character[31:24];
	   UART_START_SEND = 1;
	end
	finished: begin
	   finish = 1;
	   UART_START_SEND = 0;
	end
	default: UART_START_SEND = 0;
      endcase
   end

   assign LEDR[11] = UART_RESPONSE;
   assign LEDR[12] = UART_START_SEND;
   assign LEDR[13] = start;
   assign LEDR[17:14] = state;

endmodule 


