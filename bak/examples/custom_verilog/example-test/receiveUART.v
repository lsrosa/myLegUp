`timescale 1 ns / 1 ns
//`define RX_CLOCK_RATE 10'd521	// 60MHz clock
`define RX_CLOCK_RATE 10'd564	// 65MHz clock
//`define RX_CLOCK_RATE 10'd347	// 40MHz clock
//`define RX_CLOCK_RATE 10'd434
//`define RX_CLOCK_RATE 10'd217
module receiveUART
(
	clk,
	reset,
	start,
	finish,
	return_val,
	UART_RXD
);
input clk,reset,start,UART_RXD;
output reg [7:0] return_val;
output finish;

parameter S_RX_IDLE = 3'd0, S_RX_STARTED = 3'd1, S_RX_SYNC = 3'd2, S_RX_ASSEMBLE_DATA = 3'd3, S_RX_STOP_BIT = 3'd4;

reg [2:0] RX_state;
reg finish_reg;
reg [7:0] data_buffer;
reg [9:0] clock_count;
reg [2:0] data_count;
reg UART_INPUT;

assign finish = finish_reg;
// UART RX Logic
always @ (posedge clk or posedge reset) begin
	if (reset) begin
		data_buffer <= 8'h00;
		return_val <= 8'h00;
		clock_count <= 10'h000;
		data_count <= 3'h0;
		RX_state <= S_RX_IDLE;
		UART_INPUT <= 1'b0;
		finish_reg <= 1'b0;
		
	end else begin
		// Synchronize the asynch signal
		UART_INPUT <= UART_RXD;

		case (RX_state)
		S_RX_IDLE : begin
			finish_reg <= 1'b0;
			if (start) begin
				RX_state <= S_RX_STARTED;
			end
		end
		S_RX_STARTED: begin
			finish_reg <= 1'b0;
			// Uart receiver is startd
			if (UART_INPUT == 1'b0) begin
				// Start bit detected
				RX_state <= S_RX_SYNC;
				clock_count <= 10'h000;
				data_count <= 3'h0;
			end
		end
		S_RX_SYNC: begin
			finish_reg <= 1'b0;
			// Sync the counter for the correct time to sample for UART data on the serial interface
			if (clock_count == (`RX_CLOCK_RATE/2 - 2) && UART_INPUT == 1'b0) begin
				// Finish sync process
				clock_count <= 10'h000;
				data_count <= 3'h0;
				data_buffer <= 8'h00;
				RX_state <= S_RX_ASSEMBLE_DATA;
			end else begin
				// If the Start bit does not stay on 1'b0 during synchronization
				// it will fail this sync process			
				// ******** INCREMENT UNTIL 434, THEN SAMPLE THAT
				if (UART_INPUT == 1'b0) clock_count <= clock_count + 10'h001;
				else RX_state <= S_RX_IDLE;
			end
		end
		S_RX_ASSEMBLE_DATA: begin
			finish_reg <= 1'b0;
			// Assembling the 8 bit serial data onto data buffer
			if (clock_count == (`RX_CLOCK_RATE-1)) begin
				// Only sample the data at the middle of transmission
				data_buffer <= {UART_INPUT, data_buffer[7:1]};
				clock_count <= 10'h000; 
				if (data_count == 3'h7) 
					// Finish assembling the 8 bit data
					RX_state <= S_RX_STOP_BIT;
				else data_count <= data_count + 3'h1;   
			end else 
				clock_count <= clock_count + 10'h001;
		end
		S_RX_STOP_BIT: begin
			// Sample for stop bit here
			if (clock_count == (`RX_CLOCK_RATE-1)) begin
				RX_state <= S_RX_IDLE;
				finish_reg <= 1'b1;
				// Put the new data to the output
				return_val  <= data_buffer;
			end else begin
				clock_count <= clock_count + 10'h001;
				finish_reg <= 1'b0;
			end
		end
		default: RX_state <= S_RX_IDLE;
		endcase
	end
end

endmodule 
