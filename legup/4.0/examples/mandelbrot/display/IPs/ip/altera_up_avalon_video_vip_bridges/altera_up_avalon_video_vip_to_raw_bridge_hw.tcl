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

# +-----------------------------------
# | module altera_up_avalon_video_vip_to_raw_bridge
# | 
set_module_property DESCRIPTION "VIP Bridge: VIP to RAW"
set_module_property NAME altera_up_avalon_video_bridge_vip_to_raw_bridge
set_module_property VERSION 13.0
set_module_property GROUP "University Program/Audio & Video/Video"
set_module_property AUTHOR "Altera University Program"
set_module_property DISPLAY_NAME "VIP Bridge: VIP to RAW"
set_module_property DATASHEET_URL "../doc/Video.pdf"
#set_module_property TOP_LEVEL_HDL_FILE altera_up_avalon_video_vip_to_raw.v
#set_module_property TOP_LEVEL_HDL_MODULE altera_up_avalon_video_vip_to_raw
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE false
set_module_property ANALYZE_HDL false
set_module_property ELABORATION_CALLBACK elaborate
set_module_property GENERATION_CALLBACK generate
# | 
# +-----------------------------------

# +-----------------------------------
# | files
# | 
#add_file altera_up_avalon_video_vip_to_raw.v {SYNTHESIS SIMULATION}
# | 
# +-----------------------------------

# +-----------------------------------
# | parameters
# | 
add_parameter color_bits positive "8"
set_parameter_property color_bits DISPLAY_NAME "Color Bits"
set_parameter_property color_bits GROUP "Pixel Format"
set_parameter_property color_bits UNITS None
set_parameter_property color_bits AFFECTS_ELABORATION true
set_parameter_property color_bits AFFECTS_GENERATION true
#set_parameter_property color_bits ALLOWED_RANGES 8
set_parameter_property color_bits VISIBLE true
set_parameter_property color_bits ENABLED true

add_parameter color_planes positive "3"
set_parameter_property color_planes DISPLAY_NAME "Color Planes"
set_parameter_property color_planes GROUP "Pixel Format"
set_parameter_property color_planes UNITS None
set_parameter_property color_planes AFFECTS_ELABORATION true
set_parameter_property color_planes AFFECTS_GENERATION true
set_parameter_property color_planes ALLOWED_RANGES {1 2 3 4}
set_parameter_property color_planes VISIBLE true
set_parameter_property color_planes ENABLED true
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point clock_reset
# | 
add_interface clock_reset clock end
set_interface_property clock_reset ptfSchematicName ""

add_interface_port clock_reset clk clk Input 1
add_interface_port clock_reset reset reset Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point avalon_vip_to_raw_sink
# | 
add_interface avalon_vip_to_raw_sink avalon_streaming end clock_reset
set_interface_property avalon_vip_to_raw_sink errorDescriptor ""
set_interface_property avalon_vip_to_raw_sink maxChannel 0
set_interface_property avalon_vip_to_raw_sink readyLatency 0

add_interface_port avalon_vip_to_raw_sink vip_startofpacket startofpacket Input 1
add_interface_port avalon_vip_to_raw_sink vip_endofpacket endofpacket Input 1
add_interface_port avalon_vip_to_raw_sink vip_valid valid Input 1
add_interface_port avalon_vip_to_raw_sink vip_ready ready Output 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point avalon_vip_to_raw_source
# | 
add_interface avalon_vip_to_raw_source avalon_streaming start clock_reset
set_interface_property avalon_vip_to_raw_source errorDescriptor ""
set_interface_property avalon_vip_to_raw_source maxChannel 0
set_interface_property avalon_vip_to_raw_source readyLatency 0

add_interface_port avalon_vip_to_raw_source raw_ready ready Input 1
add_interface_port avalon_vip_to_raw_source raw_startofpacket startofpacket Output 1
add_interface_port avalon_vip_to_raw_source raw_endofpacket endofpacket Output 1
add_interface_port avalon_vip_to_raw_source raw_valid valid Output 1
# | 
# +-----------------------------------

# +-----------------------------------
# | Elaboration function
# | 
proc elaborate {} {
	set color_bits		[ get_parameter_value "color_bits" ]
	set color_planes	[ get_parameter_value "color_planes" ]

	set dw [ expr $color_bits * $color_planes ]

	if { ($color_planes == 4) || ($color_planes == 3) } {
		set ew 2
	} else {
		set ew 1
	}

	# +-----------------------------------
	# | connection point avalon_vip_to_raw_sink
	# | 
	set_interface_property avalon_vip_to_raw_sink dataBitsPerSymbol $color_bits
	set_interface_property avalon_vip_to_raw_sink symbolsPerBeat $color_planes
	
	add_interface_port avalon_vip_to_raw_sink vip_data data Input $dw
#	add_interface_port avalon_vip_to_raw_sink vip_empty empty Input $ew
	# | 
	# +-----------------------------------

	# +-----------------------------------
	# | connection point avalon_vip_to_raw_source
	# | 
	set_interface_property avalon_vip_to_raw_source dataBitsPerSymbol $color_bits
	set_interface_property avalon_vip_to_raw_source symbolsPerBeat $color_planes

	add_interface_port avalon_vip_to_raw_source raw_data data Output $dw
#	add_interface_port avalon_vip_to_raw_source raw_empty empty Output $ew
	# | 
	# +-----------------------------------
}
# | 
# +-----------------------------------

# +-----------------------------------
# | Generation function
# | 
proc generate {} {
	send_message info "Starting Generation of VIP to RAW Bridge"

	# get parameter values
	set color_bits		[ get_parameter_value "color_bits" ]
	set color_planes	[ get_parameter_value "color_planes" ]

	# get parameter values
	set dw	[ format "DW:%d" [ expr (($color_bits * $color_planes) - 1) ] ]
	if { ($color_planes == 4) || ($color_planes == 3) } {
		set ew "EW:1"
	} else {
		set ew "EW:0"
	}

	# set section values

	# set arguments
	set params "$dw;$ew"
	set sections ""

	# get generation settings
	set dest_language	[ get_generation_property HDL_LANGUAGE ]
	set dest_dir 		[ get_generation_property OUTPUT_DIRECTORY ]
	set dest_name		[ get_generation_property OUTPUT_NAME ]
	
	set file_ending "v"
	if { $dest_language == "VHDL" || $dest_language == "vhdl" } {
		set file_ending "vhd"
	}

	add_file "$dest_dir$dest_name.$file_ending" {SYNTHESIS SIMULATION}

	# Generate HDL
	source "up_ip_generator.tcl"
	alt_up_generate "$dest_dir$dest_name.$file_ending" "hdl/altera_up_avalon_video_vip_to_raw_bridge.$file_ending" "altera_up_avalon_video_vip_to_raw_bridge" $dest_name $params $sections
}
# | 
# +-----------------------------------
