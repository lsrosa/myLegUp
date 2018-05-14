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
# | module altera_up_avalon_audio_pll
# | 
set_module_property DESCRIPTION "Creates the required PLL for the audio clock on the DE-series boards"
set_module_property NAME altera_up_avalon_audio_pll
set_module_property VERSION 13.1
set_module_property GROUP "University Program/Clock"
set_module_property AUTHOR "Altera University Program"
set_module_property DISPLAY_NAME "Audio Clock for DE-series Boards"
set_module_property DATASHEET_URL "../doc/Altera_UP_PLLs.pdf"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property HIDE_FROM_SOPC true
set_module_property EDITABLE false
set_module_property ANALYZE_HDL false
set_module_property VALIDATION_CALLBACK validate
set_module_property COMPOSITION_CALLBACK compose
# | 
# +-----------------------------------


# +-----------------------------------
# | files
# | 
# | 
# +-----------------------------------


# +-----------------------------------
# | parameters
# | 
add_parameter gui_refclk float 50.0
set_parameter_property gui_refclk DISPLAY_NAME "Reference clock"
set_parameter_property gui_refclk GROUP "Settings"
set_parameter_property gui_refclk UNITS Megahertz
set_parameter_property gui_refclk AFFECTS_ELABORATION true
set_parameter_property gui_refclk VISIBLE true
set_parameter_property gui_refclk ENABLED true

add_parameter refclk float 50.0
set_parameter_property refclk DISPLAY_NAME "Reference clock"
set_parameter_property refclk GROUP "Settings"
set_parameter_property refclk UNITS Megahertz
set_parameter_property refclk AFFECTS_ELABORATION true
set_parameter_property refclk DERIVED true
set_parameter_property refclk VISIBLE false
set_parameter_property refclk ENABLED false

add_parameter audio_clk_freq float 12.288
set_parameter_property audio_clk_freq DISPLAY_NAME "Audio Clock Frequency"
set_parameter_property audio_clk_freq GROUP "Settings"
set_parameter_property audio_clk_freq UNITS Megahertz
set_parameter_property audio_clk_freq AFFECTS_ELABORATION true
set_parameter_property audio_clk_freq AFFECTS_GENERATION true
set_parameter_property audio_clk_freq ALLOWED_RANGES {18.432 16.9344 12.288 12.0 11.2896}
set_parameter_property audio_clk_freq VISIBLE true
set_parameter_property audio_clk_freq ENABLED true

add_parameter device_family string ""
set_parameter_property device_family system_info_type device_family
set_parameter_property device_family VISIBLE false
# | 
# +-----------------------------------


# +-----------------------------------
# | Elaboration function
# | 
proc validate {} {
	set gui_refclk				[ get_parameter_value "gui_refclk" ]
	set device_family			[ get_parameter_value "device_family" ]
	
	set_parameter_property gui_refclk VISIBLE false
	set_parameter_property refclk VISIBLE false

	if {  [ string match -nocase "Cyclone IV*" $device_family ]  || 
			[ string match -nocase "Cyclone III*" $device_family ] ||
			[ string match -nocase "Arria II*" $device_family ] ||
			[ string match -nocase "Stratix III*" $device_family ] ||
			[ string match -nocase "Stratix IV*" $device_family ]} {
		set_parameter_property refclk VISIBLE true
		set_parameter_value refclk 50.0
		
	} else {
		set_parameter_property gui_refclk VISIBLE true
		set_parameter_value refclk $gui_refclk
	}
}
# | 
# +-----------------------------------


# +-----------------------------------
# | Composition function
# | 
proc compose {} {
	set refclk				[ get_parameter_value "refclk" ]
	set audio_clk_freq	[ get_parameter_value "audio_clk_freq" ]
	set device_family		[ get_parameter_value "device_family" ]


	# +-----------------------------------
	# | add instances
	# | 
	if {  [ string match -nocase "Cyclone IV*" $device_family ]  ||
			[ string match -nocase "Cyclone III*" $device_family ] ||
			[ string match -nocase "Arria II*" $device_family ] ||
			[ string match -nocase "Stratix III*" $device_family ] ||
			[ string match -nocase "Stratix IV*" $device_family ]} {
		add_instance audio_pll altera_up_altpll
		set_instance_property audio_pll SUPPRESS_ALL_INFO_MESSAGES true
		set_instance_property audio_pll SUPPRESS_ALL_WARNINGS true

		set_instance_parameter_value audio_pll type "Audio"
		set_instance_parameter_value audio_pll audio_clk_freq $audio_clk_freq
		
	} else {
		add_instance audio_pll altera_pll
		set_instance_property audio_pll SUPPRESS_ALL_INFO_MESSAGES true
		set_instance_property audio_pll SUPPRESS_ALL_WARNINGS true

		set_instance_parameter_value audio_pll gui_reference_clock_frequency $refclk
		set_instance_parameter_value audio_pll gui_output_clock_frequency0 $audio_clk_freq
	}

	add_instance reset_from_locked altera_up_avalon_reset_from_locked_signal
	# | 
	# +-----------------------------------


	# +-----------------------------------
	# | add connection points
	# | 
	add_interface ref_clk clock end
	add_interface ref_reset reset end
	add_interface audio_clk clock start
	add_interface reset_source reset start

	set_interface_property ref_clk EXPORT_OF "audio_pll.refclk"
	set_interface_property ref_reset EXPORT_OF "audio_pll.reset"
	set_interface_property audio_clk EXPORT_OF "audio_pll.outclk0"
	set_interface_property reset_source EXPORT_OF "reset_from_locked.reset_source"

	add_connection "audio_pll.locked" "reset_from_locked.locked" conduit
	# | 
	# +-----------------------------------
}
# | 
# +-----------------------------------
