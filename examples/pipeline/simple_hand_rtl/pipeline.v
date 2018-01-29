`timescale 1 ns / 1 ns

module top
	(
		clk,
		reset,
		start,
		finish,
		return_val
	);
input clk;
input reset;
input start;
output reg finish;
output wire [31:0] return_val;



reg [1:0] amem_address;
reg amem_write_enable;
reg [31:0] amem_in;
wire [31:0] amem_out;

ram_one_port amem (
.clk( clk ),
.address( amem_address ),
.write_enable( amem_write_enable ),
.data( amem_in ),
.byteena( 1'b1 ),
.q( amem_out )
);
defparam amem.width_a = 32;
defparam amem.widthad_a = 2;
defparam amem.width_be = 1;
defparam amem.numwords_a = 4;
defparam amem.init_file = "a.mif";

reg [1:0] bmem_address;
reg bmem_write_enable;
reg [31:0] bmem_in;
wire [31:0] bmem_out;

ram_one_port bmem (
.clk( clk ),
.address( bmem_address ),
.write_enable( bmem_write_enable ),
.data( bmem_in ),
.byteena( 1'b1 ),
.q( bmem_out )
);
defparam bmem.width_a = 32;
defparam bmem.widthad_a = 2;
defparam bmem.width_be = 1;
defparam bmem.numwords_a = 4;
defparam bmem.init_file = "b.mif";

reg [1:0] cmem_address;
reg cmem_write_enable;
reg [31:0] cmem_in;
wire [31:0] cmem_out;

ram_one_port cmem (
.clk( clk ),
.address( cmem_address ),
.write_enable( cmem_write_enable ),
.data( cmem_in ),
.byteena( 1'b1 ),
.q( cmem_out )
);
defparam cmem.width_a = 32;
defparam cmem.widthad_a = 2;
defparam cmem.width_be = 1;
defparam cmem.numwords_a = 4;
defparam cmem.init_file = "c.mif";


// simple fsm to keep track of the ii state:
// 0..1..2..0..1..2
// i goes from 0 to 3 (4 words in the rams)
reg [1:0] i_stage0, i_stage1, i_stage2;
reg [31:0] load_a_reg;
reg [31:0] add;
reg [1:0] ii_state;
reg valid_bit_0, valid_bit_1, valid_bit_2, valid_bit_3, valid_bit_4, valid_bit_5, valid_bit_6;
reg started;
reg epilogue;


always @(posedge clk)
begin
    if (reset) 
    begin
        //load_a_reg <= 0;
        $display("Reset triggered");

        started <= 0;
        epilogue <= 0;
    end
    else 
    begin

        // valid bit
        valid_bit_1 <= valid_bit_0;
        valid_bit_2 <= valid_bit_1;
        valid_bit_3 <= valid_bit_2;
        valid_bit_4 <= valid_bit_3;
        valid_bit_5 <= valid_bit_4;
        valid_bit_6 <= valid_bit_5;

        //if (i <= 4 & ~finish) 
        if ((start & ~started) | (started & ~epilogue & i_stage0 != 3)) 
            valid_bit_0 <= 1;
        else 
            valid_bit_0 <= 0;

        if (start & ~started)
            started <= 1;

        if (i_stage0 == 3)
            epilogue <= 1;

        if (start & ~started)
            ii_state <= 0;
        else if (ii_state == 0) 
            ii_state <= 1;
        else if (ii_state == 1) 
            ii_state <= 2;
        else if (ii_state == 2) 
            ii_state <= 0;
                

        load_a_reg <= amem_out;

        if (ii_state == 0) 
        begin
        end
        if (ii_state == 1 & valid_bit_4 == 1) 
        begin
            // add - stage 1
            add <= load_a_reg + bmem_out;
            $display("Adding %d + %d", load_a_reg, bmem_out);
        end

        if (start & ~started)
            i_stage0 <= 0;
        else if (ii_state == 2 & valid_bit_2 == 1)
            i_stage0 <= i_stage0 + 1;


        if (ii_state == 2) 
        begin
            i_stage1 <= i_stage0;
            i_stage2 <= i_stage1;
        end

    end

end


always @(ii_state)
    $display("(T=%d) ii_state = %d", $time/20+0.5, ii_state);


//always @(*)
always @(ii_state)
begin
    if (ii_state == 0 && valid_bit_6 == 1) 
    begin
        // store c[i] - stage 2
        cmem_write_enable <= 1; 
        cmem_address <= i_stage2; 
        cmem_in <= add; 
        $display("(T=%d) Storing %d into address c[%d]", $time/20+0.5, add, i_stage2);

    end
    if (ii_state == 1 & valid_bit_1 == 1) 
    begin
        // load a[i] - stage 0
        amem_write_enable <= 0;
        amem_address <= i_stage0;
        $display("(T=%d) first load from address a[%d]", $time/20+0.5, i_stage0);
    end
    if (ii_state == 2 & valid_bit_2 == 1) 
    begin
        // load b[i] - stage 0
        bmem_write_enable <= 0; 
        bmem_address <= i_stage0; 
        $display("(T=%d) second load from address b[%d]", $time/20+0.5, i_stage0);
    end
    

    // finish condition - only goes high for one cycle
    if (epilogue & ~valid_bit_6)
    begin
        finish <= 1;
        epilogue <= 0;
        started <= 0;
    end
    else //if (~finish)
        finish <= 0;
end





endmodule

`timescale 1 ns / 1 ns
module ram_one_port
(
    clk,
    address,
    write_enable,
    data,
    q,
    byteena
);

parameter  width_a = 1'd0;
parameter  widthad_a = 1'd0;
parameter  numwords_a = 1'd0;
parameter  init_file = "UNUSED";
parameter  width_be = 1'd0;

input  clk;
input [(widthad_a-1):0] address;
input  write_enable;
input [(width_a-1):0] data;
output [(width_a-1):0] q;
input [width_be-1:0] byteena;
reg  clk_wire;


altsyncram altsyncram_component (
    .wren_a (write_enable),
    .clock0 (clk_wire),
    .address_a (address),
    .data_a (data),
    .q_a (q),
    .aclr0 (1'd0),
    .aclr1 (1'd0),
    .address_b (1'd1),
    .addressstall_a (1'd0),
    .addressstall_b (1'd0),
    .byteena_a (byteena),
    .byteena_b (1'd1),
    .clock1 (1'd1),
    .clocken0 (1'd1),
    .clocken1 (1'd1),
    .clocken2 (1'd1),
    .clocken3 (1'd1),
    .data_b (1'd1),
    .eccstatus (),
    .q_b (),
    .rden_a (1'd1),
    .rden_b (1'd1),
    .wren_b (1'd0)
);

defparam
    altsyncram_component.clock_enable_input_a = "BYPASS",
    altsyncram_component.clock_enable_output_a = "BYPASS",
    altsyncram_component.init_file = init_file,
    altsyncram_component.intended_device_family = "Cyclone II",
    altsyncram_component.lpm_hint = "ENABLE_RUNTIME_MOD=NO",
    altsyncram_component.lpm_type = "altsyncram",
    altsyncram_component.numwords_a = numwords_a,
    altsyncram_component.operation_mode = "SINGLE_PORT",
    altsyncram_component.outdata_aclr_a = "NONE",
    // add another cycle of latency to the loads
    //altsyncram_component.outdata_reg_a = "UNREGISTERED",
    altsyncram_component.outdata_reg_a = "CLOCK0",
    altsyncram_component.power_up_uninitialized = "FALSE",
    altsyncram_component.widthad_a = widthad_a,
    altsyncram_component.width_a = width_a,
    altsyncram_component.width_byteena_a = width_be;


always @(*) begin
clk_wire = clk;
end


endmodule 

`timescale 1 ns / 1 ns
module main_tb
(
);

reg  clk;
wire  clk2x;
wire  clk1x_follower;
reg  reset;
reg  start;
wire [31:0] return_val;
wire  finish;
reg finish2;


top top_inst (
	.clk (clk),
	.reset (reset),
	.start (start),
	.finish (finish),
	.return_val (return_val)
);

initial 
    clk = 0;
always @(clk)
    clk <= #10 ~clk;

initial begin
//$monitor("At t=%t clk=%b %b %b %b %d", $time, clk, reset, start, finish, return_val);
finish2 <= 0;
@(negedge clk);
reset <= 1;
@(negedge clk);
reset <= 0;
start <= 1;
@(negedge clk);
start <= 0;
@(posedge finish);
@(negedge finish);
@(negedge clk);
start <= 1;
@(negedge clk);
start <= 0;
@(posedge finish);
finish2 <= 1;
$finish;

end

always@(finish) begin
    if (finish == 1) begin
        $display("At t=%t clk=%b finish=%b return_val=%d", $time, clk, finish, return_val);
        $display("Cycles: %d", $time/20+0.5);
    end
end


endmodule 
