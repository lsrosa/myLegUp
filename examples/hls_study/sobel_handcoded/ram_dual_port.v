
module ram_dual_port
(
	clk,
	address_a,
	address_b,
	wren_a,
	wren_b,
	data_a,
	data_b,
	byteena_a,
	byteena_b,
	q_a,
	q_b
);

parameter  width_a = 1'd0;
parameter  width_b = 1'd0;
parameter  widthad_a = 1'd0;
parameter  widthad_b = 1'd0;
parameter  numwords_a = 1'd0;
parameter  numwords_b = 1'd0;
parameter  init_file = "UNUSED.mif";
parameter  width_be_a = 1'd0;
parameter  width_be_b = 1'd0;
parameter  latency = 1;

input  clk;
input [(widthad_a-1):0] address_a;
input [(widthad_b-1):0] address_b;
output wire [(width_a-1):0] q_a;
output wire [(width_b-1):0] q_b;
reg [(width_a-1):0] q_a_wire;
reg [(width_b-1):0] q_b_wire;
input  wren_a;
input  wren_b;
input [(width_a-1):0] data_a;
input [(width_b-1):0] data_b;
// byte enable is unsupported by inferred RAMs
input [width_be_a-1:0] byteena_a;
input [width_be_b-1:0] byteena_b;

(* ramstyle = "no_rw_check", ram_init_file = init_file *) reg [width_a-1:0] ram[numwords_a-1:0];

/* synthesis translate_off */
integer i;
ALTERA_MF_MEMORY_INITIALIZATION mem ();
reg [8*256:1] ram_ver_file;
initial begin
	if (init_file == "UNUSED.mif")
    begin
		for (i = 0; i < numwords_a; i = i + 1)
			ram[i] = 0;
    end
	else
    begin
        // modelsim can't read .mif files directly. So use Altera function to
        // convert them to .ver files
        mem.convert_to_ver_file(init_file, width_a, ram_ver_file);
        $readmemh(ram_ver_file, ram);
    end
end
/* synthesis translate_on */

always @ (posedge clk)
begin // Port A
if (wren_a)
begin
    ram[address_a] <= data_a;
    q_a_wire <= {width_a{1'bX}};
end
else
    q_a_wire <= ram[address_a];
end
always @ (posedge clk)
begin // Port b
if (wren_b)
begin
    ram[address_b] <= data_b;
    q_b_wire <= {width_b{1'bX}};
end
else
    q_b_wire <= ram[address_b];
end



integer j;
reg [(width_a-1):0] q_a_reg[latency:1], q_b_reg[latency:1];

always @(*)
begin
   q_a_reg[1] <= q_a_wire;
   q_b_reg[1] <= q_b_wire;
end

always @(posedge clk)
begin
   for (j = 1; j < latency; j=j+1)
   begin
       q_a_reg[j+1] <= q_a_reg[j];
       q_b_reg[j+1] <= q_b_reg[j];
   end
end

assign q_a = q_a_reg[latency];
assign q_b = q_b_reg[latency];


endmodule

