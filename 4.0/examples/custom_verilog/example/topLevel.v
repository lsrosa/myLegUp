// Top Level I/O
// Author: Mathew Hall
// Date: May 23, 2014

`include "sendUART.v"
`include "receiveUART.v"
module io_top
  (
   CLOCK_50,
   UART_TXD,
   UART_RXD,
   KEY,
   LEDR,
   LEDG
   );
   
   input CLOCK_50;
   input [3:0] KEY;
   output [17:0] LEDR;
   output [3:0]  LEDG;
   
   output      UART_TXD;
   input       UART_RXD;

   wire [7:0] UART_BYTE_OUT;
   wire       UART_START_SEND;
   wire [1:0] UART_RESPONSE;

   wire [7:0] UART_BYTE_IN;
   wire       UART_START_RECEIVE;
   
   wire        memory_controller_waitrequest;
   wire        memory_controller_enable_a;
   wire [`MEMORY_CONTROLLER_ADDR_SIZE-1:0] memory_controller_address_a;
   wire 				   memory_controller_write_enable_a;
   wire [`MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_in_a;
   wire [1:0] 				   memory_controller_size_a;
   wire [`MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_out_a;
   
   wire 				   memory_controller_enable_b;
   wire [`MEMORY_CONTROLLER_ADDR_SIZE-1:0] memory_controller_address_b;
   wire 				   memory_controller_write_enable_b;
   wire [`MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_in_b;
   wire [1:0] 				   memory_controller_size_b;
   wire [`MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_out_b;

   wire clk = CLOCK_50;
   wire reset = ~KEY[0];
   wire  go = ~KEY[1];

   reg [31:0] return_val_reg;
   
   assign memory_controller_waitrequest = 0;
   
   memory_controller memory_controller_inst (
					     .clk( clk ),
					     .memory_controller_enable_a( memory_controller_enable_a ),
					     .memory_controller_enable_b( memory_controller_enable_b ),
					     .memory_controller_address_a( memory_controller_address_a ),
					     .memory_controller_address_b( memory_controller_address_b ),
					     .memory_controller_write_enable_a( memory_controller_write_enable_a ),
					     .memory_controller_write_enable_b( memory_controller_write_enable_b ),
					     .memory_controller_in_a( memory_controller_in_a ),
					     .memory_controller_in_b( memory_controller_in_b ),
					     .memory_controller_size_a( memory_controller_size_a ),
					     .memory_controller_size_b( memory_controller_size_b ),
					     .memory_controller_waitrequest( memory_controller_waitrequest ),
					     .memory_controller_out_reg_a( memory_controller_out_a ),
					     .memory_controller_out_reg_b( memory_controller_out_b )
					     );
   
   main main_inst(
		  .clk( clk ),
		  .clk2x( clk2x ),
		  .clk1x_follower( clk1x_follower ),
		  .reset( reset ),
		  .start( start ),
		  .KEY( KEY ),
		  .LEDR( LEDR ),
		  .finish( finish ),
		  .return_val( return_val ),
		  .UART_BYTE_OUT( UART_BYTE_OUT ),
		  .UART_START_SEND( UART_START_SEND ),
		  .UART_RESPONSE( UART_RESPONSE ),
		  .UART_BYTE_IN( UART_BYTE_IN ),
		  .UART_START_RECEIVE( UART_START_RECEIVE ),
		  .memory_controller_address_a( memory_controller_address_a ),
		  .memory_controller_address_b( memory_controller_address_b ),
		  .memory_controller_enable_a( memory_controller_enable_a ),
		  .memory_controller_enable_b( memory_controller_enable_b ),
		  .memory_controller_write_enable_a( memory_controller_write_enable_a ),
		  .memory_controller_write_enable_b( memory_controller_write_enable_b ),
		  .memory_controller_waitrequest( memory_controller_waitrequest ),
		  .memory_controller_in_a( memory_controller_in_a ),
		  .memory_controller_in_b( memory_controller_in_b ),
		  .memory_controller_size_a( memory_controller_size_a ),
		  .memory_controller_size_b( memory_controller_size_b ),
		  .memory_controller_out_a( memory_controller_out_a ),
		  .memory_controller_out_b( memory_controller_out_b )
		  );

   sendUART uartOut(
		   .clk( clk ),
		   .reset( reset ),
		   .finish( UART_RESPONSE[0] ),
		   .start( UART_START_SEND ),
		   .arg_send_byte( UART_BYTE_OUT ),
		   .UART_TXD( UART_TXD )
		   );

   receiveUART uartIn(
		      .clk( clk ),
		      .reset( reset ),
		      .finish( UART_RESPONSE[0] ),
		      .start( UART_START_RECEIVE ),
		      .return( UART_BYTE_IN ),
		      .UART_RXD( UART_RXD )
		      );
   


   parameter s_WAIT = 3'b001, s_START = 3'b010, s_EXE = 3'b011,
                s_DONE = 3'b100;

    // state registers
    reg [3:0] y_Q, Y_D;

    assign LEDG[3:0] = y_Q;

    // next state
    always @(*)
    begin
        case (y_Q)
            s_WAIT: if (go) Y_D = s_START; else Y_D = y_Q;

            s_START: Y_D = s_EXE;

            s_EXE: if (!finish) Y_D = s_EXE; else Y_D = s_DONE;

            s_DONE: Y_D = s_DONE;

            default: Y_D = 3'bxxx;
        endcase
    end

    // current state
    always @(posedge clk)
    begin
        if (reset) // synchronous clear
            y_Q <= s_WAIT;
        else
            y_Q <= Y_D;
    end

    always @(posedge clk)
        if (y_Q == s_EXE && finish)
            return_val_reg <= return_val;
        else if (y_Q == s_DONE)
            return_val_reg <= return_val_reg;
        else
            return_val_reg <= 0;


    assign start = (y_Q == s_START);
   
endmodule
