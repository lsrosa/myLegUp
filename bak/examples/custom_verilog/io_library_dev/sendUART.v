`timescale 1 ns / 1 ns
//`define TX_CLOCK_RATE 10'd521	// 60MHz clock
//`define TX_CLOCK_RATE 10'd564	// 65MHz clock
//`define TX_CLOCK_RATE 10'd347	// 40MHz clock
`define TX_CLOCK_RATE 10'd434 // 50MHz clock
//`define TX_CLOCK_RATE 10'd217
module sendUART
(
	clk,
	reset,
	finish,
	start,
	arg_send_byte,
	UART_TXD

);
input clk,reset,start;
input [7:0] arg_send_byte;
output reg finish;
output UART_TXD;

reg [2:0] TX_state;
parameter S_TX_IDLE = 3'd0, S_TX_start_BIT = 3'd1, S_bitstosend = 3'd2, S_TX_STOP_BIT = 3'd3;

reg [7:0] data_buffer;
reg [3:0] data_count;
reg [9:0] clk_count;
reg writevalue;
assign UART_TXD = writevalue;

always @ (posedge clk or posedge reset) begin
	if (reset) begin
		data_buffer <= 8'h00;
		writevalue <= 1'b1;
		data_count <= 4'h0;
		TX_state <= S_TX_IDLE;
		finish <= 1'b1;
		clk_count <= 10'h000;
	end else begin
			case (TX_state)
			S_TX_IDLE: begin
				writevalue <= 1'b1;
				finish <= 1'b0;
			
				if (start == 1'b1) begin
					// start detected, prepare to send
					data_buffer <= arg_send_byte;
					TX_state <= S_TX_start_BIT;
					clk_count <= 10'h000;
					data_count <= 4'h0;
				end
			end
			S_TX_start_BIT: begin
				// Send the start bit
				writevalue <= 1'b0;
				finish <= 1'b0;
				if (clk_count >= `TX_CLOCK_RATE - 1) begin
					TX_state <= S_bitstosend;
					clk_count <= 10'd0;
				end else begin
					clk_count <= clk_count + 10'd1;
				end
			end
			S_bitstosend: begin
				// Repeat here until the 8 bit data is sent
				writevalue <= data_buffer[0];
				finish <= 1'b0;
				if (clk_count >= `TX_CLOCK_RATE - 1) begin
					clk_count <= 10'd0;
					if (data_count == 4'd7) begin
						TX_state <= S_TX_STOP_BIT;
						data_count <= 4'd0;
					end else begin
						data_count <= data_count + 4'd1;
						data_buffer <= {1'b0, data_buffer[7:1]};
					end
				end else begin
					clk_count <= clk_count + 10'd1;
				end
				
			end
			S_TX_STOP_BIT: begin
				// This is the stop bit
				writevalue <= 1'b1;
				if (clk_count >= `TX_CLOCK_RATE * 2 - 1) begin
					finish <= 1'b1;
					clk_count <= 10'd0;
					TX_state <= S_TX_IDLE;
				end else begin
					clk_count <= clk_count + 10'd1;
				end
				
			end
			default: TX_state <= S_TX_IDLE;
			endcase
	end
end

endmodule 
