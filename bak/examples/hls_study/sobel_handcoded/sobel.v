/******************************************************************************
 *                                                                            *
 * Copyright (c) 2010-2014 LegUp, Toronto, Ontario, Canada.                   *
 * All rights reserved.                                                       *
 *                                                                            *
 * THIS FILE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
 * FROM, OUT OF OR IN CONNECTION WITH THIS FILE OR THE USE OR OTHER DEALINGS  *
 * IN THIS FILE.                                                              *
 *                                                                            *
 ******************************************************************************/

module sobel 
/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

(
/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
input						clk,
input						reset,

output reg	[31: 0]	errors,
output reg				done,

output		[ 7: 0] output_mem_data
);

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[ 7: 0]	input_image_data;

wire			[ 7: 0]	sobel_image_data;
wire			[ 7: 0]	output_image_data;
wire			[ 7: 0]	golden_image_data;

// Internal Registers
reg			[31: 0]	input_image_address;
reg			[31: 0]	output_image_address;
reg			[31: 0]	golden_image_address;

reg						data_in_valid;
reg						data_out_valid;

reg			[10: 0]	x_location;
reg			[10: 0]	y_location;
reg						non_border;

// State Machine Registers

// Internal Variables

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// External Input Registers

// Output Registers
always @(posedge clk)
begin
	if (reset == 1'b1)
		errors <= 32'h00000000;
	else if (data_out_valid & (output_image_data != golden_image_data))
		errors <= errors + 32'h00000001;
end

always @(posedge clk)
begin
	if (reset == 1'b1)
		done <= 1'b0;
	else if ((x_location == 11'd511) & (y_location == 11'd511))
		done <= 1'b1;
end

// Internal Registers
always @(posedge clk)
begin
	if (reset == 1'b1)
		input_image_address <= 32'h00000000;
	else
		input_image_address <= input_image_address + 32'h00000001;
end

always @(posedge clk)
begin
	if (reset == 1'b1)
		output_image_address <= 32'h00000000;
	else if (data_out_valid)
		output_image_address <= output_image_address + 32'h00000001;
end

always @(posedge clk)
begin
	if (reset == 1'b1)
		golden_image_address <= 32'h00000002;
	else if (data_out_valid)
		golden_image_address <= golden_image_address + 32'h00000001;
end

always @(posedge clk)
begin
	if (reset == 1'b1)
		data_in_valid <= 1'b0;
	else
		data_in_valid <= 1'b1;
end

always @(posedge clk)
begin
	if (reset == 1'b1)
		data_out_valid <= 1'b0;
	else if (input_image_address == 32'h00000208)
		data_out_valid <= 1'b1;
	else if (input_image_address == 32'h00000208)
		data_out_valid <= 1'b1;
end

always @(posedge clk)
begin
	if (reset == 1'b1)
		x_location <= 11'h000;
	else if (data_out_valid & (x_location == 11'd511))
		x_location <= 11'h000;
	else if (data_out_valid)
		x_location <= x_location + 11'h001;
end

always @(posedge clk)
begin
	if (reset == 1'b1)
		y_location <= 11'h000;
	else if (data_out_valid & (x_location == 11'd511))
		y_location <= y_location + 11'h001;
end

always @(posedge clk)
begin
	if (reset == 1'b1)
		non_border <= 1'b0;
	else if ((x_location == 11'd0) & (y_location != 11'd0) & (y_location != 11'd511))
		non_border <= 1'b1;
	else if (x_location == 11'd510)
		non_border <= 1'b0;
end

always @(posedge clk)
begin
	if ((reset == 1'b0) && (done))
	begin
		if (errors == 32'h00000000)
			$write("PASS!\n");
		else
			$write("FAIL with %d differences\n", $signed(errors));

		$write("Took %d cycles\n", $signed(input_image_address));
		$finish;
	end
end 

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output Assignments

// Internal Assignments
assign output_image_data = (non_border) ? sobel_image_data : 8'h00;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

sobel_operator sobel (
	// Inputs
	.clk			(clk),
	.reset		(reset),

	.data_in		(input_image_data),
	.data_en		(data_in_valid),

	// Bidirectionals

	// Outputs
	.data_out	(sobel_image_data)
);
defparam sobel.WIDTH = 512;

ram_dual_port elaine_512_input (
	.clk( clk ),
	.address_a( input_image_address ),
	.address_b( 0 ),
	.wren_a( 0 ),
	.wren_b( 0 ),
	.data_a( 0 ),
	.data_b( 0 ),
	.byteena_a( 1'b1 ),
	.byteena_b( 1'b1 ),
	.q_a( input_image_data ),
	.q_b( )
);
defparam elaine_512_input.width_a = 8;
defparam elaine_512_input.width_b = 8;
defparam elaine_512_input.widthad_a = 18;
defparam elaine_512_input.widthad_b = 18;
defparam elaine_512_input.width_be_a = 1;
defparam elaine_512_input.width_be_b = 1;
defparam elaine_512_input.numwords_a = 262144;
defparam elaine_512_input.numwords_b = 262144;
defparam elaine_512_input.latency = 1;
defparam elaine_512_input.init_file = "../elaine_512_input.mif";

ram_dual_port elaine_512_golden_output (
	.clk( clk ),
	.address_a( golden_image_address ),
	.address_b( 0 ),
	.wren_a( 0 ),
	.wren_b( 0 ),
	.data_a( 0 ),
	.data_b( 0 ),
	.byteena_a( 1'b1 ),
	.byteena_b( 1'b1 ),
	.q_a( golden_image_data ),
	.q_b( )
);
defparam elaine_512_golden_output.width_a = 8;
defparam elaine_512_golden_output.width_b = 8;
defparam elaine_512_golden_output.widthad_a = 18;
defparam elaine_512_golden_output.widthad_b = 18;
defparam elaine_512_golden_output.width_be_a = 1;
defparam elaine_512_golden_output.width_be_b = 1;
defparam elaine_512_golden_output.numwords_a = 262144;
defparam elaine_512_golden_output.numwords_b = 262144;
defparam elaine_512_golden_output.latency = 2;
defparam elaine_512_golden_output.init_file = "../elaine_512_golden_output.mif";

ram_dual_port main_0_outdata (
	.clk( clk ),
	.address_a( output_image_address ),
	.address_b( 0 ),
	.wren_a( non_border ),
	.wren_b( 0 ),
	.data_a( output_image_data  ),
	.data_b( 0 ),
	.byteena_a( 1'b1 ),
	.byteena_b( 1'b1 ),
	.q_a( output_mem_data ),
	.q_b( )
);
defparam main_0_outdata.width_a = 8;
defparam main_0_outdata.width_b = 8;
defparam main_0_outdata.widthad_a = 18;
defparam main_0_outdata.widthad_b = 18;
defparam main_0_outdata.width_be_a = 1;
defparam main_0_outdata.width_be_b = 1;
defparam main_0_outdata.numwords_a = 262144;
defparam main_0_outdata.numwords_b = 262144;
defparam main_0_outdata.latency = 2;

endmodule


