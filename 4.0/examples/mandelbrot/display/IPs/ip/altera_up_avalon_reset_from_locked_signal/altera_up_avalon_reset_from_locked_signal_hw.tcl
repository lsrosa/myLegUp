# +----------------------------------------------------------------------------+
# | License Agreement                                                          |
# |                                                                            |
# | Copyright (c) 1991-2013 Altera Corporation, San Jose, California, USA.     |
# | All rights reserved.                                                       |
# |                                                                            |
# | Any megafunction design, and related net list (encrypted or decrypted),    |
# |  support information, device programming or simulation file, and any other |
# |  associated documentation or information provided by Altera or a partner   |
# |  under Altera's Megafunction Partnership Program may be used only to       |
# |  program PLD devices (but not masked PLD devices) from Altera.  Any other  |
# |  use of such megafunction design, net list, support information, device    |
# |  programming or simulation file, or any other related documentation or     |
# |  information is prohibited for any other purpose, including, but not       |
# |  limited to modification, reverse engineering, de-compiling, or use with   |
# |  any other silicon devices, unless such use is explicitly licensed under   |
# |  a separate agreement with Altera or a megafunction partner.  Title to     |
# |  the intellectual property, including patents, copyrights, trademarks,     |
# |  trade secrets, or maskworks, embodied in any such megafunction design,    |
# |  net list, support information, device programming or simulation file, or  |
# |  any other related documentation or information provided by Altera or a    |
# |  megafunction partner, remains with Altera, the megafunction partner, or   |
# |  their respective licensors.  No other licenses, including any licenses    |
# |  needed under any third party's intellectual property, are provided herein.|
# |  Copying or modifying any file, or portion thereof, to which this notice   |
# |  is attached violates this copyright.                                      |
# |                                                                            |
# | THIS FILE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    |
# | IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   |
# | FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    |
# | THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER |
# | LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    |
# | FROM, OUT OF OR IN CONNECTION WITH THIS FILE OR THE USE OR OTHER DEALINGS  |
# | IN THIS FILE.                                                              |
# |                                                                            |
# | This agreement shall be governed in all respects by the laws of the State  |
# |  of California and by the laws of the United States of America.            |
# |                                                                            |
# +----------------------------------------------------------------------------+

# TCL File Generated by Altera University Program
# DO NOT MODIFY

package require -exact qsys 13.1

# +-----------------------------------
# | module altera_up_avalon_reset_from_locked_signal
# | 
set_module_property DESCRIPTION "Creates a reset from the clock locked signal"
set_module_property NAME altera_up_avalon_reset_from_locked_signal
set_module_property VERSION 13.1
set_module_property GROUP "University Program/Subcomponents"
set_module_property AUTHOR "Altera University Program"
set_module_property DISPLAY_NAME "Reset from clock locked signal"
# set_module_property DATASHEET_URL "doc/Altera_UP_Clocks.pdf"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property HIDE_FROM_SOPC true
set_module_property EDITABLE false
set_module_property ANALYZE_HDL false
set_module_property INTERNAL true
# | 
# +-----------------------------------

# +-----------------------------------
# | files
# | 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL altera_up_avalon_reset_from_locked_signal
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file altera_up_avalon_reset_from_locked_signal.v VERILOG PATH hdl/altera_up_avalon_reset_from_locked_signal.v TOP_LEVEL_FILE

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL altera_up_avalon_reset_from_locked_signal
set_fileset_property SIM_VERILOG ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file altera_up_avalon_reset_from_locked_signal.v VERILOG PATH hdl/altera_up_avalon_reset_from_locked_signal.v

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL altera_up_avalon_reset_from_locked_signal
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file altera_up_avalon_reset_from_locked_signal.v VERILOG PATH hdl/altera_up_avalon_reset_from_locked_signal.v
# | 
# +-----------------------------------

# +-----------------------------------
# | parameters
# | 
add_display_item "" subcomponent_text text "<html><b>Note</b>: Do not use this component directly. It should only be used by other components in the QSys System Integration Tool or the MegaWizard Plug-In manager.</html>"
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point reset_source
# | 
add_interface reset_source reset start
set_interface_property reset_source associatedClock ""
set_interface_property reset_source associatedDirectReset ""
set_interface_property reset_source associatedResetSinks ""
set_interface_property reset_source synchronousEdges NONE
set_interface_property reset_source ENABLED true
set_interface_property reset_source EXPORT_OF ""
set_interface_property reset_source PORT_NAME_MAP ""
set_interface_property reset_source CMSIS_SVD_VARIABLES ""
set_interface_property reset_source SVD_ADDRESS_GROUP ""

add_interface_port reset_source reset reset Output 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point locked
# | 
add_interface locked conduit end
set_interface_property locked associatedClock ""
set_interface_property locked associatedReset ""
set_interface_property locked ENABLED true
set_interface_property locked EXPORT_OF ""
set_interface_property locked PORT_NAME_MAP ""
set_interface_property locked CMSIS_SVD_VARIABLES ""
set_interface_property locked SVD_ADDRESS_GROUP ""

add_interface_port locked locked export Input 1
# | 
# +-----------------------------------

