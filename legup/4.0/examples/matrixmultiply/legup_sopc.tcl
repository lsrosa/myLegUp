load_system tiger/tiger.sopc
remove_module data_cache_0
add_module data_cache data_cache_0
set_avalon_base_address data_cache_0.CACHE1 "0x00000000"
add_connection clk.clk data_cache_0.clockreset
add_connection data_cache_0.PROC tiger_top_0.PROC
add_connection data_cache_0.dataMaster0  pipeline_bridge_MEMORY.s1
add_connection data_cache_0.dataMaster1  pipeline_bridge_MEMORY.s1
add_connection tiger_top_0.CACHE data_cache_0.CACHE0

add_module mt mt_0
add_connection clk.clk mt_0.clockreset
add_connection pipeline_bridge_PERIPHERALS.m1 mt_0.s1
set_avalon_base_address mt_0.s1 "0x0"
add_connection mt_0.ACCEL data_cache_0.CACHE1

add_module mt2 mt2_0
add_connection clk.clk mt2_0.clockreset
add_connection pipeline_bridge_PERIPHERALS.m1 mt2_0.s1
set_avalon_base_address mt2_0.s1 "0x20"
add_connection mt2_0.ACCEL data_cache_0.CACHE1

save_system

generate_system
