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
   reg [31:0] 	character_persist;

   assign return_val = arg_character;
   
   always @(posedge clk or posedge reset) begin
      if (reset) begin
	 state <= idle;
	 character_persist <= 0;
      end
      else begin
	 case (state)
	   idle: if (start) begin
	      state <= send_byte_0;
	      character_persist <= arg_character;
	   end
	   send_byte_0: if (~start) state <= sending_byte_0;
	   sending_byte_0: if (UART_RESPONSE[0]) state <= send_byte_1;
	   send_byte_1: state <= sending_byte_1;
	   sending_byte_1: if (UART_RESPONSE[0]) state <= send_byte_2;
	   send_byte_2: state <= sending_byte_2;
	   sending_byte_2: if (UART_RESPONSE[0]) state <= send_byte_3;
	   send_byte_3: state <= sending_byte_3;
	   sending_byte_3: if (UART_RESPONSE[0]) state <= finished; 
	   finished: state <= idle;
	 endcase
      end
   end
   
   always @(*) begin
      finish = 0;
      UART_BYTE_OUT = 0;
      UART_START_SEND = 0;
      case (state)
	send_byte_0: begin
	   UART_BYTE_OUT = character_persist[7:0];
	   UART_START_SEND = 1;
	end
	sending_byte_0: begin
	   UART_BYTE_OUT = character_persist[7:0];
	end
	send_byte_1: begin
	   UART_BYTE_OUT = character_persist[15:8];
	   UART_START_SEND = 1;
	end
	sending_byte_1: UART_BYTE_OUT = character_persist[15:8];
	send_byte_2: begin
	   UART_BYTE_OUT = character_persist[23:16];
	   UART_START_SEND = 1;
	end
	sending_byte_2: UART_BYTE_OUT = character_persist[23:16];
	send_byte_3: begin
	   UART_BYTE_OUT = character_persist[31:24];
	   UART_START_SEND = 1;
	end
	sending_byte_3: UART_BYTE_OUT = character_persist[31:24];
	finished: finish = 1;
      endcase
   end

   assign LEDR[17:10] = character_persist[31:24] | character_persist[23:16] | character_persist[15:8] | character_persist[7:0];
   assign LEDR[0] = 1;
   assign LEDR[9:6] = state;
   

endmodule 


module boardGetChar
  (
   clk,
   clk2x,
   clk1x_follower,
   reset,
   start,
   UART_BYTE_IN,
   UART_START_RECEIVE,
   UART_RESPONSE,
   LEDR,
   KEY,
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
   input [7:0] 	 UART_BYTE_IN;
   output reg 	 UART_START_RECEIVE;
   input [1:0] 	 UART_RESPONSE;
   output reg 	 finish;
   output reg [31:0] return_val;
   
   parameter [2:0] idle = 3'b000;
   parameter [2:0] receive_byte_0 = 3'b001;
   parameter [2:0] receiving_byte_0 = 3'b010;
   parameter [2:0] set_return = 3'b011;
   parameter [2:0] finished = 3'b100;

   reg [2:0] 	state;
   reg [12:0] 	counter;

   always @(posedge clk or posedge reset) begin
      if (reset) begin
	 state <= idle;
	 return_val <= 0;
	 counter <= 1;
      end
      else begin
	 case (state)
	   idle: if (start & ~finish & ~UART_RESPONSE[1] & ~UART_START_RECEIVE) state <= receive_byte_0;
	   receive_byte_0: state <= receiving_byte_0;
	   receiving_byte_0: if (UART_RESPONSE[1]) state <= set_return;
	   set_return: begin
	      if (~UART_RESPONSE[1] & counter == 0) state <= finished;
	      return_val[7:0] <= UART_BYTE_IN;
	      return_val[31:8] <= 0;
	      counter <= counter + 1;
	   end
	   finished: state <= idle;
	 endcase
      end
   end

   always @(*) begin
      UART_START_RECEIVE = 0;
      finish = 0;
      case (state)
	idle: finish = 0;
	receive_byte_0: UART_START_RECEIVE = 1;
	receiving_byte_0: UART_START_RECEIVE = 0;
	finished: finish = 1;
      endcase
   end
      
   assign LEDR[17:10] = return_val;
   assign LEDR[9:7] = state;
   assign LEDR[6] = finish;
   assign LEDR[5] = start;
   assign LEDR[4:3] = UART_RESPONSE;
   
   
endmodule 


