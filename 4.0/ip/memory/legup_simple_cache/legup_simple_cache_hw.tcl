# TCL File Generated by Component Editor 13.0sp1
# Thu Dec 11 18:04:27 EST 2014
# DO NOT MODIFY


# 
# legup_simple_cache "legup_simple_cache" v1.0
#  2014.12.11.18:04:27
# 
# 

# 
# request TCL package from ACDS 13.1
# 
package require -exact qsys 13.0


# 
# module legup_simple_cache
# 
set_module_property DESCRIPTION ""
set_module_property NAME legup_simple_cache
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property GROUP "LegUp/Memory"
set_module_property AUTHOR "Legup"
set_module_property DISPLAY_NAME "Simple Cache"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property ANALYZE_HDL AUTO
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL legup_simple_cache
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file legup_simple_cache.v VERILOG PATH hdl/legup_simple_cache.v TOP_LEVEL_FILE
add_fileset_file true_dual_port_ram_single_clock.v VERILOG PATH hdl/true_dual_port_ram_single_clock.v

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL legup_simple_cache
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file legup_simple_cache.v VERILOG PATH hdl/legup_simple_cache.v
add_fileset_file true_dual_port_ram_single_clock.v VERILOG PATH hdl/true_dual_port_ram_single_clock.v

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL legup_simple_cache
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file legup_simple_cache.v VERILOG PATH hdl/legup_simple_cache.v
add_fileset_file true_dual_port_ram_single_clock.v VERILOG PATH hdl/true_dual_port_ram_single_clock.v


# 
# parameters
# 
#add_parameter DATA_WIDTH INTEGER 32
#set_parameter_property DATA_WIDTH DEFAULT_VALUE 32
#set_parameter_property DATA_WIDTH DISPLAY_NAME DATA_WIDTH
#set_parameter_property DATA_WIDTH TYPE INTEGER
#set_parameter_property DATA_WIDTH UNITS None
#set_parameter_property DATA_WIDTH HDL_PARAMETER true
#add_parameter CACHE_LINES INTEGER 512
#set_parameter_property CACHE_LINES DEFAULT_VALUE 512
#set_parameter_property CACHE_LINES DISPLAY_NAME CACHE_LINES
#set_parameter_property CACHE_LINES TYPE INTEGER
#set_parameter_property CACHE_LINES UNITS None
#set_parameter_property CACHE_LINES HDL_PARAMETER true


# 
# display items
# 


# 
# connection point reset
# 
add_interface reset reset end
set_interface_property reset associatedClock clk
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset reset reset Input 1


# 
# connection point clk
# 
add_interface clk clock end
set_interface_property clk clockRate 0
set_interface_property clk ENABLED true
set_interface_property clk EXPORT_OF ""
set_interface_property clk PORT_NAME_MAP ""
set_interface_property clk SVD_ADDRESS_GROUP ""

add_interface_port clk clk clk Input 1


# 
# connection point cache_slave
# 
add_interface cache_slave avalon end
set_interface_property cache_slave addressUnits SYMBOLS
set_interface_property cache_slave associatedClock clk
set_interface_property cache_slave associatedReset reset
set_interface_property cache_slave bitsPerSymbol 8
set_interface_property cache_slave burstOnBurstBoundariesOnly false
set_interface_property cache_slave burstcountUnits WORDS
set_interface_property cache_slave explicitAddressSpan 0
set_interface_property cache_slave holdTime 0
set_interface_property cache_slave isMemoryDevice true
set_interface_property cache_slave linewrapBursts false
set_interface_property cache_slave maximumPendingReadTransactions 4
set_interface_property cache_slave readLatency 0
set_interface_property cache_slave readWaitTime 1
set_interface_property cache_slave setupTime 0
set_interface_property cache_slave timingUnits Cycles
set_interface_property cache_slave writeWaitTime 0
set_interface_property cache_slave ENABLED true
set_interface_property cache_slave EXPORT_OF ""
set_interface_property cache_slave PORT_NAME_MAP ""
set_interface_property cache_slave SVD_ADDRESS_GROUP ""

add_interface_port cache_slave avs_cache_address address Input 31
add_interface_port cache_slave avs_cache_byteenable byteenable Input 4
add_interface_port cache_slave avs_cache_read read Input 1
add_interface_port cache_slave avs_cache_write write Input 1
add_interface_port cache_slave avs_cache_writedata writedata Input 32
add_interface_port cache_slave avs_cache_readdata readdata Output 32
add_interface_port cache_slave avs_cache_readdatavalid readdatavalid Output 1
add_interface_port cache_slave avs_cache_waitrequest waitrequest Output 1
set_interface_assignment cache_slave embeddedsw.configuration.isFlash 0
set_interface_assignment cache_slave embeddedsw.configuration.isMemoryDevice 1
set_interface_assignment cache_slave embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment cache_slave embeddedsw.configuration.isPrintableDevice 0


# 
# connection point cache_master
# 
add_interface cache_master avalon start
set_interface_property cache_master addressUnits SYMBOLS
set_interface_property cache_master associatedClock clk
set_interface_property cache_master associatedReset reset
set_interface_property cache_master bitsPerSymbol 8
set_interface_property cache_master burstOnBurstBoundariesOnly false
set_interface_property cache_master burstcountUnits WORDS
set_interface_property cache_master doStreamReads false
set_interface_property cache_master doStreamWrites false
set_interface_property cache_master holdTime 0
set_interface_property cache_master linewrapBursts false
set_interface_property cache_master maximumPendingReadTransactions 0
set_interface_property cache_master readLatency 0
set_interface_property cache_master readWaitTime 0
set_interface_property cache_master setupTime 0
set_interface_property cache_master timingUnits Cycles
set_interface_property cache_master writeWaitTime 0
set_interface_property cache_master ENABLED true
set_interface_property cache_master EXPORT_OF ""
set_interface_property cache_master PORT_NAME_MAP ""
set_interface_property cache_master SVD_ADDRESS_GROUP ""

add_interface_port cache_master avm_cache_readdata readdata Input 32
add_interface_port cache_master avm_cache_readdatavalid readdatavalid Input 1
add_interface_port cache_master avm_cache_waitrequest waitrequest Input 1
add_interface_port cache_master avm_cache_address address Output 32
add_interface_port cache_master avm_cache_byteenable byteenable Output 4
add_interface_port cache_master avm_cache_read read Output 1
add_interface_port cache_master avm_cache_write write Output 1
add_interface_port cache_master avm_cache_writedata writedata Output 32

