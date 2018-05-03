module top (
inclk,
inA,
inB,
outAxB,
inC,
inD,
outCxD,
reset);

input inclk, reset;
wire clk, clk2x, clk1x_follower;

pll pll_inst (
    .inclk0 (inclk),
    .c0 (clk),
    .c1 (clk2x)
);

clock_follower clock_follower_inst (
    .reset ( reset ),
    .clk1x ( clk ),
    .clk2x ( clk2x ),
    .clk1x_follower ( clk1x_follower )
);


parameter size = 32;
parameter sign = "UNSIGNED";
parameter pipeline = 0;

reg [size-1:0] multipump_0_main_180_206_inA;
reg [size-1:0] multipump_0_main_180_206_inB;
wire [2*size-1:0] multipump_0_main_180_206_outAxB_actual;
reg [2*size-1:0] multipump_0_main_180_206_AxB;
wire [2*size-1:0] multipump_0_main_180_206_outCxD_actual;
reg [size-1:0] multipump_0_main_180_206_inC;
reg [size-1:0] multipump_0_main_180_206_inD;

input	   [size-1:0] inA, inB, inC, inD;
output reg [2*size-1:0] outAxB, outCxD;


always @(posedge clk)
begin
	multipump_0_main_180_206_inA <= inA;
	multipump_0_main_180_206_inB <= inB;
	multipump_0_main_180_206_inC <= inC;
	multipump_0_main_180_206_inD <= inD;
	outAxB <= multipump_0_main_180_206_outAxB_actual;
	outCxD <= multipump_0_main_180_206_outCxD_actual;
end

multipump multipump_main_180_203 (
	.clk (clk),
	.clk2x (clk2x),
	.clk1x_follower (clk1x_follower),
	.inA (multipump_0_main_180_206_inA),
	.inB (multipump_0_main_180_206_inB),
	.outAxB (multipump_0_main_180_206_outAxB_actual),
	.outCxD (multipump_0_main_180_206_outCxD_actual),
	.inC (multipump_0_main_180_206_inC),
	.inD (multipump_0_main_180_206_inD)
);
defparam
	multipump_main_180_203.size = size,
	multipump_main_180_203.sign = sign,
	multipump_main_180_203.pipeline = pipeline;

endmodule
module multipump (
clk,
clk2x,
clk1x_follower,
inA,
inB,
outAxB,
inC,
inD,
outCxD);

parameter size = 32;
parameter sign = "UNSIGNED";
parameter pipeline = 0;

input	   clk, clk2x, clk1x_follower;
input	   [size-1:0] inA, inB, inC, inD;
output reg [size*2-1:0] outAxB, outCxD;

wire [size*2-1:0] dsp_out;
reg [size*2-1:0]  resultAB_reg, resultCD_reg;


always @(*) begin
outCxD = resultCD_reg;
outAxB = resultAB_reg;
end


reg [size*2-1:0] dsp_out_fast;



always @(posedge clk2x) begin
dsp_out_fast <= dsp_out;
end


// the C x D result is ready
always @(*) begin
resultCD_reg <= dsp_out;
end

// the A x B result is ready
always @(*) begin
resultAB_reg <= dsp_out_fast;
end



wire      mux_sel;


assign mux_sel = ~clk1x_follower;

reg       [size-1:0]  dataa;
reg       [size-1:0]  datab;
wire      [size-1:0]  dataa_wire;
wire      [size-1:0]  datab_wire;
assign dataa_wire = dataa;
assign datab_wire = datab;

always @(*)
begin
if (mux_sel == 0) 
begin
dataa = inA;
datab = inB;
end
else 
begin
dataa = inC;
datab = inD;
end
end



// DSP multiplier - has two pipeline stages, so both inputs and outputs
// are registered
lpm_mult	lpm_mult_component (
.clock (),
.dataa (dataa_wire),
.datab (datab_wire),
.result (dsp_out),
.aclr (1'b0),
.clken (1'b1),
.sum (1'b0));

defparam
lpm_mult_component.lpm_hint = "DEDICATED_MULTIPLIER_CIRCUITRY=YES,MAXIMIZE_SPEED=5",
lpm_mult_component.lpm_representation = sign,
lpm_mult_component.lpm_type = "LPM_MULT",
lpm_mult_component.lpm_pipeline = pipeline,
lpm_mult_component.lpm_widtha = size,
lpm_mult_component.lpm_widthb = size,
lpm_mult_component.lpm_widthp = size*2;


endmodule
// synopsys translate_off
`timescale 1 ns / 1 ns
// synopsys translate_on
module pll (
inclk0,
c0,
c1);

input	  inclk0;
output	  c0;
output	  c1;

wire [5:0] sub_wire0;
wire [0:0] sub_wire5 = 1'h0;
wire [1:1] sub_wire2 = sub_wire0[1:1];
wire [0:0] sub_wire1 = sub_wire0[0:0];
wire  c0 = sub_wire1;
wire  c1 = sub_wire2;
wire  sub_wire3 = inclk0;
wire [1:0] sub_wire4 = {sub_wire5, sub_wire3};

altpll	altpll_component (
.inclk (sub_wire4),
.clk (sub_wire0),
.activeclock (),
.areset (1'b0),
.clkbad (),
.clkena ({6{1'b1}}),
.clkloss (),
.clkswitch (1'b0),
.configupdate (1'b0),
.enable0 (),
.enable1 (),
.extclk (),
.extclkena ({4{1'b1}}),
.fbin (1'b1),
.fbmimicbidir (),
.fbout (),
.fref (),
.icdrclk (),
.locked (),
.pfdena (1'b1),
.phasecounterselect ({4{1'b1}}),
.phasedone (),
.phasestep (1'b1),
.phaseupdown (1'b1),
.pllena (1'b1),
.scanaclr (1'b0),
.scanclk (1'b0),
.scanclkena (1'b1),
.scandata (1'b0),
.scandataout (),
.scandone (),
.scanread (1'b0),
.scanwrite (1'b0),
.sclkout0 (),
.sclkout1 (),
.vcooverrange (),
.vcounderrange ());
defparam
altpll_component.clk0_divide_by = 1,
altpll_component.clk0_duty_cycle = 50,
altpll_component.clk0_multiply_by = 1,
altpll_component.clk0_phase_shift = "0",
altpll_component.clk1_divide_by = 1,
altpll_component.clk1_duty_cycle = 50,
altpll_component.clk1_multiply_by = 2,
altpll_component.clk1_phase_shift = "0",
altpll_component.compensate_clock = "CLK0",
altpll_component.inclk0_input_frequency = 20000,
altpll_component.intended_device_family = "Cyclone II",
altpll_component.lpm_hint = "CBX_MODULE_PREFIX=pll",
altpll_component.lpm_type = "altpll",
altpll_component.operation_mode = "NORMAL",
altpll_component.port_activeclock = "PORT_UNUSED",
altpll_component.port_areset = "PORT_UNUSED",
altpll_component.port_clkbad0 = "PORT_UNUSED",
altpll_component.port_clkbad1 = "PORT_UNUSED",
altpll_component.port_clkloss = "PORT_UNUSED",
altpll_component.port_clkswitch = "PORT_UNUSED",
altpll_component.port_configupdate = "PORT_UNUSED",
altpll_component.port_fbin = "PORT_UNUSED",
altpll_component.port_inclk0 = "PORT_USED",
altpll_component.port_inclk1 = "PORT_UNUSED",
altpll_component.port_locked = "PORT_UNUSED",
altpll_component.port_pfdena = "PORT_UNUSED",
altpll_component.port_phasecounterselect = "PORT_UNUSED",
altpll_component.port_phasedone = "PORT_UNUSED",
altpll_component.port_phasestep = "PORT_UNUSED",
altpll_component.port_phaseupdown = "PORT_UNUSED",
altpll_component.port_pllena = "PORT_UNUSED",
altpll_component.port_scanaclr = "PORT_UNUSED",
altpll_component.port_scanclk = "PORT_UNUSED",
altpll_component.port_scanclkena = "PORT_UNUSED",
altpll_component.port_scandata = "PORT_UNUSED",
altpll_component.port_scandataout = "PORT_UNUSED",
altpll_component.port_scandone = "PORT_UNUSED",
altpll_component.port_scanread = "PORT_UNUSED",
altpll_component.port_scanwrite = "PORT_UNUSED",
altpll_component.port_clk0 = "PORT_USED",
altpll_component.port_clk1 = "PORT_USED",
altpll_component.port_clk2 = "PORT_UNUSED",
altpll_component.port_clk3 = "PORT_UNUSED",
altpll_component.port_clk4 = "PORT_UNUSED",
altpll_component.port_clk5 = "PORT_UNUSED",
altpll_component.port_clkena0 = "PORT_UNUSED",
altpll_component.port_clkena1 = "PORT_UNUSED",
altpll_component.port_clkena2 = "PORT_UNUSED",
altpll_component.port_clkena3 = "PORT_UNUSED",
altpll_component.port_clkena4 = "PORT_UNUSED",
altpll_component.port_clkena5 = "PORT_UNUSED",
altpll_component.port_extclk0 = "PORT_UNUSED",
altpll_component.port_extclk1 = "PORT_UNUSED",
altpll_component.port_extclk2 = "PORT_UNUSED",
altpll_component.port_extclk3 = "PORT_UNUSED";


endmodule

module clock_follower (reset, clk1x, clk2x, clk1x_follower);
input clk1x, clk2x, reset;
output reg clk1x_follower;
reg toggle, tog_1;

always @(posedge clk1x or posedge reset)
if (reset)
toggle <= 0;
else
toggle <= ~toggle;

always @(posedge clk2x)
tog_1 <= toggle;

always @(posedge clk2x)
clk1x_follower <= ~(toggle ^ tog_1);

endmodule

