create_clock -period 2 -name clk [get_ports clk]
create_clock -period 1 -name clk2x [get_ports clk2x]
create_clock -period 2 -name OSC_50_BANK2 [get_ports OSC_50_BANK2]
create_clock -period 20 -name CLOCK_50 [get_ports CLOCK_50]
create_clock -period 10MHz {altera_reserved_tck}
set_clock_groups -asynchronous -group {altera_reserved_tck}
set_input_delay -clock {altera_reserved_tck} 20 [get_ports altera_reserved_tdi]
set_input_delay -clock {altera_reserved_tck} 20 [get_ports altera_reserved_tms]
set_output_delay -clock {altera_reserved_tck} 20 [get_ports altera_reserved_tdo]
derive_pll_clocks
derive_clock_uncertainty
