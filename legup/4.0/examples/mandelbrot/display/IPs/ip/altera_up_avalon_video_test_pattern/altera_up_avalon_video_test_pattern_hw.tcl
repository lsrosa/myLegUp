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
# | module altera_up_avalon_video_test_pattern
# | 
set_module_property DESCRIPTION "Creates Sample Streaming Video for DE-series Boards"
set_module_property NAME altera_up_avalon_video_test_pattern
set_module_property VERSION 13.0
set_module_property GROUP "University Program/Audio & Video/Video"
set_module_property AUTHOR "Altera University Program"
set_module_property DISPLAY_NAME "Test-Pattern Generator"
set_module_property DATASHEET_URL "../doc/Video.pdf"
#set_module_property TOP_LEVEL_HDL_FILE altera_up_avalon_video_test_pattern.v
#set_module_property TOP_LEVEL_HDL_MODULE altera_up_avalon_video_test_pattern
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE false
set_module_property ANALYZE_HDL false
set_module_property GENERATION_CALLBACK generate
# | 
# +-----------------------------------

# +-----------------------------------
# | files
# | 
#add_file altera_up_avalon_video_test_pattern.v {SYNTHESIS SIMULATION}
# | 
# +-----------------------------------

# +-----------------------------------
# | parameters
# | 
add_parameter width integer "320"
set_parameter_property width DISPLAY_NAME "Width (# in width)"
set_parameter_property width GROUP "Outgoing Frame Resolution"
set_parameter_property width UNITS None
set_parameter_property width AFFECTS_ELABORATION true
set_parameter_property width AFFECTS_GENERATION true
set_parameter_property width VISIBLE true
set_parameter_property width ENABLED true

add_parameter height integer "240"
set_parameter_property height DISPLAY_NAME "Height (# in height)"
set_parameter_property height GROUP "Outgoing Frame Resolution"
set_parameter_property height UNITS None
set_parameter_property height AFFECTS_ELABORATION true
set_parameter_property height AFFECTS_GENERATION true
set_parameter_property height VISIBLE true
set_parameter_property height ENABLED true
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
# | connection point avalon_generator_source
# | 
add_interface avalon_generator_source avalon_streaming start clock_reset
set_interface_property avalon_generator_source dataBitsPerSymbol 8
set_interface_property avalon_generator_source errorDescriptor ""
set_interface_property avalon_generator_source maxChannel 0
set_interface_property avalon_generator_source readyLatency 0
set_interface_property avalon_generator_source symbolsPerBeat 3

add_interface_port avalon_generator_source ready ready Input 1
add_interface_port avalon_generator_source data data Output 24
add_interface_port avalon_generator_source startofpacket startofpacket Output 1
add_interface_port avalon_generator_source endofpacket endofpacket Output 1
#add_interface_port avalon_generator_source empty empty Output 2
add_interface_port avalon_generator_source valid valid Output 1
# | 
# +-----------------------------------

# +-----------------------------------
# | Generation function
# | 
proc generate {} {
	send_message info "Starting Generation of Test Pattern Generator"

	# get parameter values
	set width [ get_parameter_value "width" ]
	set height [ get_parameter_value "height" ]

	set width_p		"WIDTH:$width"
	set height_p	"HEIGHT:$height"
	set width_aw	[ format "WW:%.0f" [ expr ceil (log ($width) / (log (2))) ] ]
	set height_aw	[ format "HW:%.0f" [ expr ceil (log ($height) / (log (2))) ] ]

	set value					"VALUE:8'd160"
	set p_rate					[ format "P_RATE:24'd%.0f" [ expr ((160 * 256 * 256) / ($height)) ] ]
	set tq_start_rate			[ format "TQ_START_RATE:25'd%.0f" [ expr ((160 * 256 * 256 * 6) / ($width)) ] ]
	set tq_rate_deceleration	[ format "TQ_RATE_DECELERATION:25'd%.0f" [ expr ((160 * 256 * 256 * 6) / ($width * $height)) ] ]

	# set section values
	set use_HSV_value			"USE_HSV_VALUE:0"

	# set arguments
	set params "$width_p;$height_p;$width_aw;$height_aw;$value;$p_rate;$tq_start_rate;$tq_rate_deceleration"
	set sections "$use_HSV_value"

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
	source "UP_IP_Generator.tcl"
	alt_up_generate "$dest_dir$dest_name.$file_ending" "hdl/altera_up_avalon_video_test_pattern.$file_ending" "altera_up_avalon_video_test_pattern" $dest_name $params $sections
#		file copy -force "hdl/p_rate.mif" $dest_dir
#		file copy -force "hdl/tq_accelerate.mif" $dest_dir
#		file copy -force "hdl/tq_rate.mif" $dest_dir
}
# | 
# +-----------------------------------

