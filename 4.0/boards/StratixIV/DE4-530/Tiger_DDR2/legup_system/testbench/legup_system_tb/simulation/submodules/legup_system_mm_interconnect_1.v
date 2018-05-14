// legup_system_mm_interconnect_1.v

// This file was auto-generated from altera_merlin_interconnect_wrapper_hw.tcl.  If you edit it your changes
// will probably be lost.
// 
// Generated using ACDS version 13.1 162 at 2015.05.09.10:25:13

`timescale 1 ps / 1 ps
module legup_system_mm_interconnect_1 (
		input  wire        DDR2_SDRAM_afi_clk_clk,                             //                           DDR2_SDRAM_afi_clk.clk
		input  wire        Leap_Sim_Control_reset_reset_bridge_in_reset_reset, // Leap_Sim_Control_reset_reset_bridge_in_reset.reset
		input  wire [31:0] Leap_Sim_Control_bridge_master_address,             //               Leap_Sim_Control_bridge_master.address
		output wire        Leap_Sim_Control_bridge_master_waitrequest,         //                                             .waitrequest
		input  wire [3:0]  Leap_Sim_Control_bridge_master_byteenable,          //                                             .byteenable
		input  wire        Leap_Sim_Control_bridge_master_read,                //                                             .read
		output wire [31:0] Leap_Sim_Control_bridge_master_readdata,            //                                             .readdata
		input  wire        Leap_Sim_Control_bridge_master_write,               //                                             .write
		input  wire [31:0] Leap_Sim_Control_bridge_master_writedata,           //                                             .writedata
		output wire [29:0] Leap_Profiler_leapslave_address,                    //                      Leap_Profiler_leapslave.address
		output wire        Leap_Profiler_leapslave_write,                      //                                             .write
		output wire        Leap_Profiler_leapslave_read,                       //                                             .read
		input  wire [31:0] Leap_Profiler_leapslave_readdata,                   //                                             .readdata
		output wire [31:0] Leap_Profiler_leapslave_writedata                   //                                             .writedata
	);

	wire         leap_sim_control_bridge_master_translator_avalon_universal_master_0_waitrequest;   // Leap_Profiler_leapslave_translator:uav_waitrequest -> Leap_Sim_Control_bridge_master_translator:uav_waitrequest
	wire   [2:0] leap_sim_control_bridge_master_translator_avalon_universal_master_0_burstcount;    // Leap_Sim_Control_bridge_master_translator:uav_burstcount -> Leap_Profiler_leapslave_translator:uav_burstcount
	wire  [31:0] leap_sim_control_bridge_master_translator_avalon_universal_master_0_writedata;     // Leap_Sim_Control_bridge_master_translator:uav_writedata -> Leap_Profiler_leapslave_translator:uav_writedata
	wire  [31:0] leap_sim_control_bridge_master_translator_avalon_universal_master_0_address;       // Leap_Sim_Control_bridge_master_translator:uav_address -> Leap_Profiler_leapslave_translator:uav_address
	wire         leap_sim_control_bridge_master_translator_avalon_universal_master_0_lock;          // Leap_Sim_Control_bridge_master_translator:uav_lock -> Leap_Profiler_leapslave_translator:uav_lock
	wire         leap_sim_control_bridge_master_translator_avalon_universal_master_0_write;         // Leap_Sim_Control_bridge_master_translator:uav_write -> Leap_Profiler_leapslave_translator:uav_write
	wire         leap_sim_control_bridge_master_translator_avalon_universal_master_0_read;          // Leap_Sim_Control_bridge_master_translator:uav_read -> Leap_Profiler_leapslave_translator:uav_read
	wire  [31:0] leap_sim_control_bridge_master_translator_avalon_universal_master_0_readdata;      // Leap_Profiler_leapslave_translator:uav_readdata -> Leap_Sim_Control_bridge_master_translator:uav_readdata
	wire         leap_sim_control_bridge_master_translator_avalon_universal_master_0_debugaccess;   // Leap_Sim_Control_bridge_master_translator:uav_debugaccess -> Leap_Profiler_leapslave_translator:uav_debugaccess
	wire   [3:0] leap_sim_control_bridge_master_translator_avalon_universal_master_0_byteenable;    // Leap_Sim_Control_bridge_master_translator:uav_byteenable -> Leap_Profiler_leapslave_translator:uav_byteenable
	wire         leap_sim_control_bridge_master_translator_avalon_universal_master_0_readdatavalid; // Leap_Profiler_leapslave_translator:uav_readdatavalid -> Leap_Sim_Control_bridge_master_translator:uav_readdatavalid

	altera_merlin_master_translator #(
		.AV_ADDRESS_W                (32),
		.AV_DATA_W                   (32),
		.AV_BURSTCOUNT_W             (1),
		.AV_BYTEENABLE_W             (4),
		.UAV_ADDRESS_W               (32),
		.UAV_BURSTCOUNT_W            (3),
		.USE_READ                    (1),
		.USE_WRITE                   (1),
		.USE_BEGINBURSTTRANSFER      (0),
		.USE_BEGINTRANSFER           (0),
		.USE_CHIPSELECT              (0),
		.USE_BURSTCOUNT              (0),
		.USE_READDATAVALID           (0),
		.USE_WAITREQUEST             (1),
		.USE_READRESPONSE            (0),
		.USE_WRITERESPONSE           (0),
		.AV_SYMBOLS_PER_WORD         (4),
		.AV_ADDRESS_SYMBOLS          (1),
		.AV_BURSTCOUNT_SYMBOLS       (0),
		.AV_CONSTANT_BURST_BEHAVIOR  (0),
		.UAV_CONSTANT_BURST_BEHAVIOR (0),
		.AV_LINEWRAPBURSTS           (0),
		.AV_REGISTERINCOMINGSIGNALS  (0)
	) leap_sim_control_bridge_master_translator (
		.clk                      (DDR2_SDRAM_afi_clk_clk),                                                            //                       clk.clk
		.reset                    (Leap_Sim_Control_reset_reset_bridge_in_reset_reset),                                //                     reset.reset
		.uav_address              (leap_sim_control_bridge_master_translator_avalon_universal_master_0_address),       // avalon_universal_master_0.address
		.uav_burstcount           (leap_sim_control_bridge_master_translator_avalon_universal_master_0_burstcount),    //                          .burstcount
		.uav_read                 (leap_sim_control_bridge_master_translator_avalon_universal_master_0_read),          //                          .read
		.uav_write                (leap_sim_control_bridge_master_translator_avalon_universal_master_0_write),         //                          .write
		.uav_waitrequest          (leap_sim_control_bridge_master_translator_avalon_universal_master_0_waitrequest),   //                          .waitrequest
		.uav_readdatavalid        (leap_sim_control_bridge_master_translator_avalon_universal_master_0_readdatavalid), //                          .readdatavalid
		.uav_byteenable           (leap_sim_control_bridge_master_translator_avalon_universal_master_0_byteenable),    //                          .byteenable
		.uav_readdata             (leap_sim_control_bridge_master_translator_avalon_universal_master_0_readdata),      //                          .readdata
		.uav_writedata            (leap_sim_control_bridge_master_translator_avalon_universal_master_0_writedata),     //                          .writedata
		.uav_lock                 (leap_sim_control_bridge_master_translator_avalon_universal_master_0_lock),          //                          .lock
		.uav_debugaccess          (leap_sim_control_bridge_master_translator_avalon_universal_master_0_debugaccess),   //                          .debugaccess
		.av_address               (Leap_Sim_Control_bridge_master_address),                                            //      avalon_anti_master_0.address
		.av_waitrequest           (Leap_Sim_Control_bridge_master_waitrequest),                                        //                          .waitrequest
		.av_byteenable            (Leap_Sim_Control_bridge_master_byteenable),                                         //                          .byteenable
		.av_read                  (Leap_Sim_Control_bridge_master_read),                                               //                          .read
		.av_readdata              (Leap_Sim_Control_bridge_master_readdata),                                           //                          .readdata
		.av_write                 (Leap_Sim_Control_bridge_master_write),                                              //                          .write
		.av_writedata             (Leap_Sim_Control_bridge_master_writedata),                                          //                          .writedata
		.av_burstcount            (1'b1),                                                                              //               (terminated)
		.av_beginbursttransfer    (1'b0),                                                                              //               (terminated)
		.av_begintransfer         (1'b0),                                                                              //               (terminated)
		.av_chipselect            (1'b0),                                                                              //               (terminated)
		.av_readdatavalid         (),                                                                                  //               (terminated)
		.av_lock                  (1'b0),                                                                              //               (terminated)
		.av_debugaccess           (1'b0),                                                                              //               (terminated)
		.uav_clken                (),                                                                                  //               (terminated)
		.av_clken                 (1'b1),                                                                              //               (terminated)
		.uav_response             (2'b00),                                                                             //               (terminated)
		.av_response              (),                                                                                  //               (terminated)
		.uav_writeresponserequest (),                                                                                  //               (terminated)
		.uav_writeresponsevalid   (1'b0),                                                                              //               (terminated)
		.av_writeresponserequest  (1'b0),                                                                              //               (terminated)
		.av_writeresponsevalid    ()                                                                                   //               (terminated)
	);

	altera_merlin_slave_translator #(
		.AV_ADDRESS_W                   (30),
		.AV_DATA_W                      (32),
		.UAV_DATA_W                     (32),
		.AV_BURSTCOUNT_W                (1),
		.AV_BYTEENABLE_W                (4),
		.UAV_BYTEENABLE_W               (4),
		.UAV_ADDRESS_W                  (32),
		.UAV_BURSTCOUNT_W               (3),
		.AV_READLATENCY                 (0),
		.USE_READDATAVALID              (0),
		.USE_WAITREQUEST                (0),
		.USE_UAV_CLKEN                  (0),
		.USE_READRESPONSE               (0),
		.USE_WRITERESPONSE              (0),
		.AV_SYMBOLS_PER_WORD            (4),
		.AV_ADDRESS_SYMBOLS             (0),
		.AV_BURSTCOUNT_SYMBOLS          (0),
		.AV_CONSTANT_BURST_BEHAVIOR     (0),
		.UAV_CONSTANT_BURST_BEHAVIOR    (0),
		.AV_REQUIRE_UNALIGNED_ADDRESSES (0),
		.CHIPSELECT_THROUGH_READLATENCY (0),
		.AV_READ_WAIT_CYCLES            (1),
		.AV_WRITE_WAIT_CYCLES           (0),
		.AV_SETUP_WAIT_CYCLES           (0),
		.AV_DATA_HOLD_CYCLES            (0)
	) leap_profiler_leapslave_translator (
		.clk                      (DDR2_SDRAM_afi_clk_clk),                                                            //                      clk.clk
		.reset                    (Leap_Sim_Control_reset_reset_bridge_in_reset_reset),                                //                    reset.reset
		.uav_address              (leap_sim_control_bridge_master_translator_avalon_universal_master_0_address),       // avalon_universal_slave_0.address
		.uav_burstcount           (leap_sim_control_bridge_master_translator_avalon_universal_master_0_burstcount),    //                         .burstcount
		.uav_read                 (leap_sim_control_bridge_master_translator_avalon_universal_master_0_read),          //                         .read
		.uav_write                (leap_sim_control_bridge_master_translator_avalon_universal_master_0_write),         //                         .write
		.uav_waitrequest          (leap_sim_control_bridge_master_translator_avalon_universal_master_0_waitrequest),   //                         .waitrequest
		.uav_readdatavalid        (leap_sim_control_bridge_master_translator_avalon_universal_master_0_readdatavalid), //                         .readdatavalid
		.uav_byteenable           (leap_sim_control_bridge_master_translator_avalon_universal_master_0_byteenable),    //                         .byteenable
		.uav_readdata             (leap_sim_control_bridge_master_translator_avalon_universal_master_0_readdata),      //                         .readdata
		.uav_writedata            (leap_sim_control_bridge_master_translator_avalon_universal_master_0_writedata),     //                         .writedata
		.uav_lock                 (leap_sim_control_bridge_master_translator_avalon_universal_master_0_lock),          //                         .lock
		.uav_debugaccess          (leap_sim_control_bridge_master_translator_avalon_universal_master_0_debugaccess),   //                         .debugaccess
		.av_address               (Leap_Profiler_leapslave_address),                                                   //      avalon_anti_slave_0.address
		.av_write                 (Leap_Profiler_leapslave_write),                                                     //                         .write
		.av_read                  (Leap_Profiler_leapslave_read),                                                      //                         .read
		.av_readdata              (Leap_Profiler_leapslave_readdata),                                                  //                         .readdata
		.av_writedata             (Leap_Profiler_leapslave_writedata),                                                 //                         .writedata
		.av_begintransfer         (),                                                                                  //              (terminated)
		.av_beginbursttransfer    (),                                                                                  //              (terminated)
		.av_burstcount            (),                                                                                  //              (terminated)
		.av_byteenable            (),                                                                                  //              (terminated)
		.av_readdatavalid         (1'b0),                                                                              //              (terminated)
		.av_waitrequest           (1'b0),                                                                              //              (terminated)
		.av_writebyteenable       (),                                                                                  //              (terminated)
		.av_lock                  (),                                                                                  //              (terminated)
		.av_chipselect            (),                                                                                  //              (terminated)
		.av_clken                 (),                                                                                  //              (terminated)
		.uav_clken                (1'b0),                                                                              //              (terminated)
		.av_debugaccess           (),                                                                                  //              (terminated)
		.av_outputenable          (),                                                                                  //              (terminated)
		.uav_response             (),                                                                                  //              (terminated)
		.av_response              (2'b00),                                                                             //              (terminated)
		.uav_writeresponserequest (1'b0),                                                                              //              (terminated)
		.uav_writeresponsevalid   (),                                                                                  //              (terminated)
		.av_writeresponserequest  (),                                                                                  //              (terminated)
		.av_writeresponsevalid    (1'b0)                                                                               //              (terminated)
	);

endmodule