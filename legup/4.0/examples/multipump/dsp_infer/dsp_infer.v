// Simple verilog file to check whether a constant will infer a DSP
// Author: Andrew Canis
// Date: June 21, 2012
//
// To compile with Quartus run:
//      make p
//      make f
module de4(clk, a, b, c);

input wire clk;
input wire [31:0] a, b;
//output wire [31:0] c;
output wire [63:0] c;

reg [31:0] a_r, b_r;
reg [63:0] c_r;


always @(posedge clk)
begin
    a_r <= a;
    b_r <= b;
    //c_r <= a_r * b_r;
    //c_r <= a_r * 32;
    c_r <= a_r * 26;
end

assign c = c_r;
//assign c = a * b;

endmodule

