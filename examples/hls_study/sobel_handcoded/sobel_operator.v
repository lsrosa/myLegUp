
module sobel_operator (
	// Inputs
	clk,
	reset,

	data_in,
	data_en,

	// Bidirectionals

	// Outputs
	data_out
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter WIDTH	= 512; // Image width in pixels

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input			[ 7: 0]	data_in;
input						data_en;

// Bidirectionals

// Outputs
output		[ 7: 0]	data_out;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/


/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/
// Internal Wires
wire			[ 7: 0]	shift_reg_out[ 1: 0];

// Internal Registers
reg			[ 7: 0]	original_line_1[ 2: 0];
reg			[ 7: 0]	original_line_2[ 2: 0];
reg			[ 7: 0]	original_line_3[ 2: 0];

reg			[10: 0]	gx_level_1[ 3: 0];
reg			[10: 0]	gx_level_2[ 1: 0];
reg			[10: 0]	gx_level_3;

reg			[10: 0]	gy_level_1[ 3: 0];
reg			[10: 0]	gy_level_2[ 1: 0];
reg			[10: 0]	gy_level_3;

reg			[ 7: 0]	gx_bounded;
reg			[ 7: 0]	gy_bounded;

reg			[ 7: 0]	g_bounded;

reg			[ 7: 0]	result;

// State Machine Registers

// Integers
integer					i;

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

// Sobel Operator
// 
//      [ -1  0  1 ]           [  1  2  1 ]
// Gx   [ -2  0  2 ]      Gy   [  0  0  0 ]
//      [ -1  0  1 ]           [ -1 -2 -1 ]
//
// |G| = |Gx| + |Gy|

always @(posedge clk)
begin
	if (reset == 1'b1)
	begin
		for (i = 2; i >= 0; i = i-1)
		begin
			original_line_1[i] <= 8'h00;
			original_line_2[i] <= 8'h00;
			original_line_3[i] <= 8'h00;
		end

		gx_level_1[0] <= 11'h000;
		gx_level_1[1] <= 11'h000;
		gx_level_1[2] <= 11'h000;
		gx_level_1[3] <= 11'h000;
		gx_level_2[0] <= 11'h000;
		gx_level_2[1] <= 11'h000;
		gx_level_3	  <= 11'h000;

		gy_level_1[0] <= 11'h000;
		gy_level_1[1] <= 11'h000;
		gy_level_1[2] <= 11'h000;
		gy_level_1[3] <= 11'h000;
		gy_level_2[0] <= 11'h000;
		gy_level_2[1] <= 11'h000;
		gy_level_3	  <= 11'h000;

		gx_bounded    <= 11'h000;
		gx_bounded    <= 11'h000;

		g_bounded	  <= 11'h000;

		result		  <= 8'h00;
	end
	else if (data_en == 1'b1)
	begin	
		for (i = 2; i > 0; i = i-1)
		begin
			original_line_1[i] <= original_line_1[i-1];
			original_line_2[i] <= original_line_2[i-1];
			original_line_3[i] <= original_line_3[i-1];
		end
		original_line_1[0] <= data_in;
		original_line_2[0] <= shift_reg_out[0];
		original_line_3[0] <= shift_reg_out[1];

		// Calculate Gx
		gx_level_1[0] <= {3'h0,original_line_1[0]} + {3'h0,original_line_1[2]};
		gx_level_1[1] <= {2'h0,original_line_1[1], 1'b0};
		gx_level_1[2] <= {3'h0,original_line_3[0]} + {3'h0,original_line_3[2]};
		gx_level_1[3] <= {2'h0,original_line_3[1], 1'b0};

		gx_level_2[0] <= gx_level_1[0] + gx_level_1[1];
		gx_level_2[1] <= gx_level_1[2] + gx_level_1[3];

		gx_level_3    <= gx_level_2[0] - gx_level_2[1];

		// Calculate Gy
		gy_level_1[0] <= {3'h0,original_line_1[2]} + {3'h0,original_line_3[2]};
		gy_level_1[1] <= {2'h0,original_line_2[2], 1'b0};
		gy_level_1[2] <= {3'h0,original_line_1[0]} + {3'h0,original_line_3[0]};
		gy_level_1[3] <= {2'h0,original_line_2[0], 1'b0};

		gy_level_2[0] <= gy_level_1[0] + gy_level_1[1];
		gy_level_2[1] <= gy_level_1[2] + gy_level_1[3];

		gy_level_3    <= gy_level_2[0] - gy_level_2[1];
		
		// Calculate the magnitude and sign of Gx and Gy
		gx_bounded    <= (gx_level_3[10]) ? 8'h00 : (gx_level_3[9:8] != 2'h0) ? 8'hFF : gx_level_3[7:0]; 
		gy_bounded    <= (gy_level_3[10]) ? 8'h00 : (gy_level_3[9:8] != 2'h0) ? 8'hFF : gy_level_3[7:0]; 

		// Calculate the magnitude G
		g_bounded	  <= gx_bounded + gy_bounded;

		// Calculate the final result
		result[7:0]	  <= 8'hFF - g_bounded;
	end
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

assign data_out = result; 

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

data_shift_register shift_register_1 (
	// Inputs
	.clock		(clk),
	.clken		(data_en),
	.shiftin		(data_in),

	// Bidirectionals

	// Outputs
	.shiftout	(shift_reg_out[0]),
	.taps			()
);
defparam 
	shift_register_1.DW		= 8,
	shift_register_1.SIZE	= WIDTH;

data_shift_register shift_register_2 (
	// Inputs
	.clock		(clk),
	.clken		(data_en),
	.shiftin		(shift_reg_out[0]),

	// Bidirectionals

	// Outputs
	.shiftout	(shift_reg_out[1]),
	.taps			()
);
defparam
	shift_register_2.DW		= 8,
	shift_register_2.SIZE	= WIDTH;

endmodule

