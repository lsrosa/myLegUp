onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -label clk -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/clk
add wave -noupdate -label pc -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/core/pc
add wave -noupdate -label ins -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/ins
add wave -noupdate -label reset_n -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/reset
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/*
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/*
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/state
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/*
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/state
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/core/*
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/core/de/*
add wave -noupdate -radix hexadecimal /test_bench/DUT/the_tiger_top_0/tiger_top_0/core/de/b/*

add wave -noupdate -radix hexadecimal /test_bench/DUT/the_data_cache_0/data_cache_0/loop_instantiateRAM[0]/dcacheMemIns/*

add wave -noupdate -radix hexadecimal /test_bench/DUT/the_mt_0/mt_0/*

add wave -noupdate -radix hexadecimal /test_bench/DUT/the_mt2_0/mt2_0/*

TreeUpdate [SetDefaultTree]
configure wave -namecolwidth 556
configure wave -valuecolwidth 145
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
WaveRestoreZoom {109112943 ps} {109266688 ps}
update
