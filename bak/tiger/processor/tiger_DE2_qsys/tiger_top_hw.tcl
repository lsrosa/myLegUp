# TCL File Generated by Component Editor 11.1sp2
# Thu Nov 01 19:13:32 EDT 2012
# DO NOT MODIFY


# +-----------------------------------
# | 
# | tiger_top "tiger_top" v1.0
# | null 2012.11.01.19:13:32
# | 
# | 
# | /home/choijon5/legup/tiger/processor/tiger_DE2/tiger_top.v
# | 
# |    ./tiger_top.v syn, sim
# |    ./LEAP/AddressHash.v syn, sim
# |    ./LEAP/AddressStack.v syn, sim
# |    ./LEAP/CounterStack.v syn, sim
# |    ./LEAP/CounterStorage.v syn, sim
# |    ./LEAP/CountingBlock.v syn, sim
# |    ./LEAP/IncCounter.v syn, sim
# |    ./LEAP/LeapProfiler.v syn, sim
# |    ./LEAP/OpDecode.v syn, sim
# |    ./LEAP/tiger_leap_slave_handler.v syn, sim
# | 
# +-----------------------------------

# +-----------------------------------
# | request TCL package from ACDS 11.0
# | 
package require -exact sopc 11.0
# | 
# +-----------------------------------

# +-----------------------------------
# | module tiger_top
# | 
set_module_property NAME tiger_top
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property DISPLAY_NAME tiger_top
set_module_property TOP_LEVEL_HDL_FILE tiger_top.v
set_module_property TOP_LEVEL_HDL_MODULE tiger_top
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property ANALYZE_HDL TRUE
set_module_property STATIC_TOP_LEVEL_MODULE_NAME tiger_top
set_module_property FIX_110_VIP_PATH false
# | 
# +-----------------------------------

# +-----------------------------------
# | files
# | 
add_file tiger_top.v {SYNTHESIS SIMULATION}
add_file LEAP/AddressHash.v {SYNTHESIS SIMULATION}
add_file LEAP/AddressStack.v {SYNTHESIS SIMULATION}
add_file LEAP/CounterStack.v {SYNTHESIS SIMULATION}
add_file LEAP/CounterStorage.v {SYNTHESIS SIMULATION}
add_file LEAP/CountingBlock.v {SYNTHESIS SIMULATION}
add_file LEAP/IncCounter.v {SYNTHESIS SIMULATION}
add_file LEAP/LeapProfiler.v {SYNTHESIS SIMULATION}
add_file LEAP/OpDecode.v {SYNTHESIS SIMULATION}
add_file LEAP/tiger_leap_slave_handler.v {SYNTHESIS SIMULATION}
# | 
# +-----------------------------------

# +-----------------------------------
# | parameters
# | 
add_parameter prof_param_N2 INTEGER 8
set_parameter_property prof_param_N2 DEFAULT_VALUE 8
set_parameter_property prof_param_N2 DISPLAY_NAME prof_param_N2
set_parameter_property prof_param_N2 TYPE INTEGER
set_parameter_property prof_param_N2 UNITS None
set_parameter_property prof_param_N2 AFFECTS_GENERATION false
set_parameter_property prof_param_N2 HDL_PARAMETER true
add_parameter prof_param_S2 INTEGER 5
set_parameter_property prof_param_S2 DEFAULT_VALUE 5
set_parameter_property prof_param_S2 DISPLAY_NAME prof_param_S2
set_parameter_property prof_param_S2 TYPE INTEGER
set_parameter_property prof_param_S2 UNITS None
set_parameter_property prof_param_S2 AFFECTS_GENERATION false
set_parameter_property prof_param_S2 HDL_PARAMETER true
add_parameter prof_param_CW INTEGER 32
set_parameter_property prof_param_CW DEFAULT_VALUE 32
set_parameter_property prof_param_CW DISPLAY_NAME prof_param_CW
set_parameter_property prof_param_CW TYPE INTEGER
set_parameter_property prof_param_CW UNITS None
set_parameter_property prof_param_CW AFFECTS_GENERATION false
set_parameter_property prof_param_CW HDL_PARAMETER true
# | 
# +-----------------------------------

# +-----------------------------------
# | display items
# | 
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point clock
# | 
add_interface clock clock end
set_interface_property clock clockRate 0

set_interface_property clock ENABLED true

add_interface_port clock clk clk Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point reset
# | 
add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT

set_interface_property reset ENABLED true

add_interface_port reset reset reset Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point CACHE
# | 
add_interface CACHE avalon start
set_interface_property CACHE addressUnits SYMBOLS
set_interface_property CACHE associatedClock clock
set_interface_property CACHE associatedReset reset
set_interface_property CACHE burstOnBurstBoundariesOnly false
set_interface_property CACHE doStreamReads false
set_interface_property CACHE doStreamWrites false
set_interface_property CACHE linewrapBursts false
set_interface_property CACHE readLatency 0

set_interface_property CACHE ENABLED true

add_interface_port CACHE avm_CACHE_address address Output 32
add_interface_port CACHE avm_CACHE_read read Output 1
add_interface_port CACHE avm_CACHE_write write Output 1
add_interface_port CACHE avm_CACHE_writedata writedata Output 128
add_interface_port CACHE avm_CACHE_readdata readdata Input 128
add_interface_port CACHE avm_CACHE_waitrequest waitrequest Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point PROC
# | 
add_interface PROC avalon_streaming end
set_interface_property PROC associatedClock clock
set_interface_property PROC dataBitsPerSymbol 8
set_interface_property PROC errorDescriptor ""
set_interface_property PROC firstSymbolInHighOrderBits true
set_interface_property PROC maxChannel 0
set_interface_property PROC readyLatency 0

set_interface_property PROC ENABLED true

add_interface_port PROC asi_PROC_data data Input 8
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point procMaster
# | 
add_interface procMaster avalon start
set_interface_property procMaster addressUnits SYMBOLS
set_interface_property procMaster associatedClock clock
set_interface_property procMaster associatedReset reset
set_interface_property procMaster burstOnBurstBoundariesOnly false
set_interface_property procMaster doStreamReads false
set_interface_property procMaster doStreamWrites false
set_interface_property procMaster linewrapBursts false
set_interface_property procMaster readLatency 0

set_interface_property procMaster ENABLED true

add_interface_port procMaster avm_procMaster_address address Output 32
add_interface_port procMaster avm_procMaster_read read Output 1
add_interface_port procMaster avm_procMaster_write write Output 1
add_interface_port procMaster avm_procMaster_writedata writedata Output 32
add_interface_port procMaster avm_procMaster_byteenable byteenable Output 4
add_interface_port procMaster avm_procMaster_readdata readdata Input 32
add_interface_port procMaster avm_procMaster_waitrequest waitrequest Input 1
add_interface_port procMaster avm_procMaster_readdatavalid readdatavalid Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point instructionMaster
# | 
add_interface instructionMaster avalon start
set_interface_property instructionMaster addressUnits SYMBOLS
set_interface_property instructionMaster associatedClock clock
set_interface_property instructionMaster associatedReset reset
set_interface_property instructionMaster burstOnBurstBoundariesOnly false
set_interface_property instructionMaster doStreamReads false
set_interface_property instructionMaster doStreamWrites false
set_interface_property instructionMaster linewrapBursts false
set_interface_property instructionMaster readLatency 0

set_interface_property instructionMaster ENABLED true

add_interface_port instructionMaster avm_ins_read read Output 1
add_interface_port instructionMaster avm_ins_address address Output 32
add_interface_port instructionMaster avm_ins_readdata readdata Input 32
add_interface_port instructionMaster avm_ins_lock lock Output 1
add_interface_port instructionMaster avm_ins_waitrequest waitrequest Input 1
add_interface_port instructionMaster avm_ins_readdatavalid readdatavalid Input 1
# add_interface_port instructionMaster avm_instructionMaster_read read Output 1
# add_interface_port instructionMaster avm_instructionMaster_address address Output 32
# add_interface_port instructionMaster avm_instructionMaster_readdata readdata Input 32
# add_interface_port instructionMaster avm_instructionMaster_beginbursttransfer beginbursttransfer Output 1
# add_interface_port instructionMaster avm_instructionMaster_burstcount burstcount Output 6
# add_interface_port instructionMaster avm_instructionMaster_waitrequest waitrequest Input 1
# add_interface_port instructionMaster avm_instructionMaster_readdatavalid readdatavalid Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point conduit_end_0
# | 
add_interface conduit_end_0 conduit end

set_interface_property conduit_end_0 ENABLED true

add_interface_port conduit_end_0 coe_exe_start export Output 1
add_interface_port conduit_end_0 coe_exe_end export Output 1
add_interface_port conduit_end_0 coe_debug_select export Input 3
add_interface_port conduit_end_0 coe_debug_lights export Output 18
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point leapSlave
# | 
add_interface leapSlave avalon end
set_interface_property leapSlave addressAlignment DYNAMIC
set_interface_property leapSlave addressUnits WORDS
set_interface_property leapSlave associatedClock clock
set_interface_property leapSlave burstOnBurstBoundariesOnly false
set_interface_property leapSlave explicitAddressSpan 0
set_interface_property leapSlave holdTime 0
set_interface_property leapSlave isMemoryDevice false
set_interface_property leapSlave isNonVolatileStorage false
set_interface_property leapSlave linewrapBursts false
set_interface_property leapSlave maximumPendingReadTransactions 0
set_interface_property leapSlave printableDevice false
set_interface_property leapSlave readLatency 0
set_interface_property leapSlave readWaitTime 1
set_interface_property leapSlave setupTime 0
set_interface_property leapSlave timingUnits Cycles
set_interface_property leapSlave writeWaitTime 0

set_interface_property leapSlave ENABLED true

add_interface_port leapSlave avs_leapSlave_chipselect chipselect Input 1
add_interface_port leapSlave avs_leapSlave_address address Input prof_param_N2
add_interface_port leapSlave avs_leapSlave_read read Input 1
add_interface_port leapSlave avs_leapSlave_write write Input 1
add_interface_port leapSlave avs_leapSlave_writedata writedata Input 32
add_interface_port leapSlave avs_leapSlave_readdata readdata Output 32
# | 
# +-----------------------------------
