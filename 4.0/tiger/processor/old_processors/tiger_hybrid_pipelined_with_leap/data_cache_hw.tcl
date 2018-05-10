# TCL File Generated by Component Editor 11.1sp2
# Tue Oct 23 14:24:08 EDT 2012
# DO NOT MODIFY


# +-----------------------------------
# | 
# | data_cache "data_cache" v1.0
# | null 2012.10.23.14:24:08
# | 
# | 
# | /home/choijon5/legup/tiger/processor/tiger_hybrid_pipelined_with_leap/data_cache.v
# | 
# |    ./data_cache.v syn, sim
# | 
# +-----------------------------------

# +-----------------------------------
# | request TCL package from ACDS 11.0
# | 
package require -exact sopc 11.0
# | 
# +-----------------------------------

# +-----------------------------------
# | module data_cache
# | 
set_module_property NAME data_cache
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property DISPLAY_NAME data_cache
set_module_property TOP_LEVEL_HDL_FILE data_cache.v
set_module_property TOP_LEVEL_HDL_MODULE data_cache
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property ANALYZE_HDL TRUE
set_module_property STATIC_TOP_LEVEL_MODULE_NAME data_cache
set_module_property FIX_110_VIP_PATH false
# | 
# +-----------------------------------

# +-----------------------------------
# | files
# | 
add_file data_cache.v {SYNTHESIS SIMULATION}
# | 
# +-----------------------------------

# +-----------------------------------
# | parameters
# | 
# | 
# +-----------------------------------

# +-----------------------------------
# | display items
# | 
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point clockreset
# | 
add_interface clockreset clock end
set_interface_property clockreset clockRate 0

set_interface_property clockreset ENABLED true

add_interface_port clockreset csi_clockreset_clk clk Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point clockreset_reset
# | 
add_interface clockreset_reset reset end
set_interface_property clockreset_reset associatedClock clockreset
set_interface_property clockreset_reset synchronousEdges DEASSERT

set_interface_property clockreset_reset ENABLED true

add_interface_port clockreset_reset csi_clockreset_reset_n reset_n Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point PROC
# | 
add_interface PROC avalon_streaming start
set_interface_property PROC associatedClock clockreset
set_interface_property PROC dataBitsPerSymbol 8
set_interface_property PROC errorDescriptor ""
set_interface_property PROC firstSymbolInHighOrderBits true
set_interface_property PROC maxChannel 0
set_interface_property PROC readyLatency 0

set_interface_property PROC ENABLED true

add_interface_port PROC aso_PROC_data data Output 8
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point CACHE0
# | 
add_interface CACHE0 avalon end
set_interface_property CACHE0 addressAlignment DYNAMIC
set_interface_property CACHE0 addressUnits WORDS
set_interface_property CACHE0 associatedClock clockreset
set_interface_property CACHE0 burstOnBurstBoundariesOnly false
set_interface_property CACHE0 explicitAddressSpan 0
set_interface_property CACHE0 holdTime 0
set_interface_property CACHE0 isMemoryDevice false
set_interface_property CACHE0 isNonVolatileStorage false
set_interface_property CACHE0 linewrapBursts false
set_interface_property CACHE0 maximumPendingReadTransactions 0
set_interface_property CACHE0 printableDevice false
set_interface_property CACHE0 readLatency 0
set_interface_property CACHE0 readWaitTime 1
set_interface_property CACHE0 setupTime 0
set_interface_property CACHE0 timingUnits Cycles
set_interface_property CACHE0 writeWaitTime 0

set_interface_property CACHE0 ENABLED true

add_interface_port CACHE0 avs_CACHE0_begintransfer begintransfer Input 1
add_interface_port CACHE0 avs_CACHE0_read read Input 1
add_interface_port CACHE0 avs_CACHE0_write write Input 1
add_interface_port CACHE0 avs_CACHE0_writedata writedata Input 128
add_interface_port CACHE0 avs_CACHE0_readdata readdata Output 128
add_interface_port CACHE0 avs_CACHE0_waitrequest waitrequest Output 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point dataMaster0
# | 
add_interface dataMaster0 avalon start
set_interface_property dataMaster0 addressUnits SYMBOLS
set_interface_property dataMaster0 associatedClock clockreset
set_interface_property dataMaster0 associatedReset ""
set_interface_property dataMaster0 burstOnBurstBoundariesOnly false
set_interface_property dataMaster0 doStreamReads false
set_interface_property dataMaster0 doStreamWrites false
set_interface_property dataMaster0 linewrapBursts false
set_interface_property dataMaster0 readLatency 0

set_interface_property dataMaster0 ENABLED true

add_interface_port dataMaster0 avm_dataMaster0_read read Output 1
add_interface_port dataMaster0 avm_dataMaster0_write write Output 1
add_interface_port dataMaster0 avm_dataMaster0_address address Output 32
add_interface_port dataMaster0 avm_dataMaster0_writedata writedata Output 32
add_interface_port dataMaster0 avm_dataMaster0_byteenable byteenable Output 4
add_interface_port dataMaster0 avm_dataMaster0_readdata readdata Input 32
add_interface_port dataMaster0 avm_dataMaster0_beginbursttransfer beginbursttransfer Output 1
add_interface_port dataMaster0 avm_dataMaster0_burstcount burstcount Output 10
add_interface_port dataMaster0 avm_dataMaster0_waitrequest waitrequest Input 1
add_interface_port dataMaster0 avm_dataMaster0_readdatavalid readdatavalid Input 1
# | 
# +-----------------------------------
