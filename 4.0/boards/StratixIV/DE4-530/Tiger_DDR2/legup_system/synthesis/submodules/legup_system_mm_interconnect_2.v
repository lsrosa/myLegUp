// legup_system_mm_interconnect_2.v

// This file was auto-generated from altera_merlin_interconnect_wrapper_hw.tcl.  If you edit it your changes
// will probably be lost.
// 
// Generated using ACDS version 13.1 162 at 2015.05.09.10:25:47

`timescale 1 ps / 1 ps
module legup_system_mm_interconnect_2 (
		input  wire        DDR2_SDRAM_afi_clk_clk,                          //                        DDR2_SDRAM_afi_clk.clk
		input  wire        Leap_Profiler_reset_reset_bridge_in_reset_reset, // Leap_Profiler_reset_reset_bridge_in_reset.reset
		input  wire [31:0] Leap_Profiler_to_memory_address,                 //                   Leap_Profiler_to_memory.address
		output wire        Leap_Profiler_to_memory_waitrequest,             //                                          .waitrequest
		input  wire [3:0]  Leap_Profiler_to_memory_byteenable,              //                                          .byteenable
		input  wire        Leap_Profiler_to_memory_read,                    //                                          .read
		output wire [31:0] Leap_Profiler_to_memory_readdata,                //                                          .readdata
		output wire        Leap_Profiler_to_memory_readdatavalid,           //                                          .readdatavalid
		input  wire        Leap_Profiler_to_memory_write,                   //                                          .write
		input  wire [31:0] Leap_Profiler_to_memory_writedata,               //                                          .writedata
		output wire [29:0] Tiger_ICache_icache_slave_address,               //                 Tiger_ICache_icache_slave.address
		output wire        Tiger_ICache_icache_slave_read,                  //                                          .read
		input  wire [31:0] Tiger_ICache_icache_slave_readdata,              //                                          .readdata
		input  wire        Tiger_ICache_icache_slave_readdatavalid,         //                                          .readdatavalid
		input  wire        Tiger_ICache_icache_slave_waitrequest            //                                          .waitrequest
	);

	wire         leap_profiler_to_memory_translator_avalon_universal_master_0_waitrequest;   // Tiger_ICache_icache_slave_translator:uav_waitrequest -> Leap_Profiler_to_memory_translator:uav_waitrequest
	wire   [2:0] leap_profiler_to_memory_translator_avalon_universal_master_0_burstcount;    // Leap_Profiler_to_memory_translator:uav_burstcount -> Tiger_ICache_icache_slave_translator:uav_burstcount
	wire  [31:0] leap_profiler_to_memory_translator_avalon_universal_master_0_writedata;     // Leap_Profiler_to_memory_translator:uav_writedata -> Tiger_ICache_icache_slave_translator:uav_writedata
	wire  [31:0] leap_profiler_to_memory_translator_avalon_universal_master_0_address;       // Leap_Profiler_to_memory_translator:uav_address -> Tiger_ICache_icache_slave_translator:uav_address
	wire         leap_profiler_to_memory_translator_avalon_universal_master_0_lock;          // Leap_Profiler_to_memory_translator:uav_lock -> Tiger_ICache_icache_slave_translator:uav_lock
	wire         leap_profiler_to_memory_translator_avalon_universal_master_0_write;         // Leap_Profiler_to_memory_translator:uav_write -> Tiger_ICache_icache_slave_translator:uav_write
	wire         leap_profiler_to_memory_translator_avalon_universal_master_0_read;          // Leap_Profiler_to_memory_translator:uav_read -> Tiger_ICache_icache_slave_translator:uav_read
	wire  [31:0] leap_profiler_to_memory_translator_avalon_universal_master_0_readdata;      // Tiger_ICache_icache_slave_translator:uav_readdata -> Leap_Profiler_to_memory_translator:uav_readdata
	wire         leap_profiler_to_memory_translator_avalon_universal_master_0_debugaccess;   // Leap_Profiler_to_memory_translator:uav_debugaccess -> Tiger_ICache_icache_slave_translator:uav_debugaccess
	wire   [3:0] leap_profiler_to_memory_translator_avalon_universal_master_0_byteenable;    // Leap_Profiler_to_memory_translator:uav_byteenable -> Tiger_ICache_icache_slave_translator:uav_byteenable
	wire         leap_profiler_to_memory_translator_avalon_universal_master_0_readdatavalid; // Tiger_ICache_icache_slave_translator:uav_readdatavalid -> Leap_Profiler_to_memory_translator:uav_readdatavalid

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
		.USE_READDATAVALID           (1),
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
	) leap_profiler_to_memory_translator (
		.clk                      (DDR2_SDRAM_afi_clk_clk),                                                     //                       clk.clk
		.reset                    (Leap_Profiler_reset_reset_bridge_in_reset_reset),                            //                     reset.reset
		.uav_address              (leap_profiler_to_memory_translator_avalon_universal_master_0_address),       // avalon_universal_master_0.address
		.uav_burstcount           (leap_profiler_to_memory_translator_avalon_universal_master_0_burstcount),    //                          .burstcount
		.uav_read                 (leap_profiler_to_memory_translator_avalon_universal_master_0_read),          //                          .read
		.uav_write                (leap_profiler_to_memory_translator_avalon_universal_master_0_write),         //                          .write
		.uav_waitrequest          (leap_profiler_to_memory_translator_avalon_universal_master_0_waitrequest),   //                          .waitrequest
		.uav_readdatavalid        (leap_profiler_to_memory_translator_avalon_universal_master_0_readdatavalid), //                          .readdatavalid
		.uav_byteenable           (leap_profiler_to_memory_translator_avalon_universal_master_0_byteenable),    //                          .byteenable
		.uav_readdata             (leap_profiler_to_memory_translator_avalon_universal_master_0_readdata),      //                          .readdata
		.uav_writedata            (leap_profiler_to_memory_translator_avalon_universal_master_0_writedata),     //                          .writedata
		.uav_lock                 (leap_profiler_to_memory_translator_avalon_universal_master_0_lock),          //                          .lock
		.uav_debugaccess          (leap_profiler_to_memory_translator_avalon_universal_master_0_debugaccess),   //                          .debugaccess
		.av_address               (Leap_Profiler_to_memory_address),                                            //      avalon_anti_master_0.address
		.av_waitrequest           (Leap_Profiler_to_memory_waitrequest),                                        //                          .waitrequest
		.av_byteenable            (Leap_Profiler_to_memory_byteenable),                                         //                          .byteenable
		.av_read                  (Leap_Profiler_to_memory_read),                                               //                          .read
		.av_readdata              (Leap_Profiler_to_memory_readdata),                                           //                          .readdata
		.av_readdatavalid         (Leap_Profiler_to_memory_readdatavalid),                                      //                          .readdatavalid
		.av_write                 (Leap_Profiler_to_memory_write),                                              //                          .write
		.av_writedata             (Leap_Profiler_to_memory_writedata),                                          //                          .writedata
		.av_burstcount            (1'b1),                                                                       //               (terminated)
		.av_beginbursttransfer    (1'b0),                                                                       //               (terminated)
		.av_begintransfer         (1'b0),                                                                       //               (terminated)
		.av_chipselect            (1'b0),                                                                       //               (terminated)
		.av_lock                  (1'b0),                                                                       //               (terminated)
		.av_debugaccess           (1'b0),                                                                       //               (terminated)
		.uav_clken                (),                                                                           //               (terminated)
		.av_clken                 (1'b1),                                                                       //               (terminated)
		.uav_response             (2'b00),                                                                      //               (terminated)
		.av_response              (),                                                                           //               (terminated)
		.uav_writeresponserequest (),                                                                           //               (terminated)
		.uav_writeresponsevalid   (1'b0),                                                                       //               (terminated)
		.av_writeresponserequest  (1'b0),                                                                       //               (terminated)
		.av_writeresponsevalid    ()                                                                            //               (terminated)
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
		.USE_READDATAVALID              (1),
		.USE_WAITREQUEST                (1),
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
	) tiger_icache_icache_slave_translator (
		.clk                      (DDR2_SDRAM_afi_clk_clk),                                                     //                      clk.clk
		.reset                    (Leap_Profiler_reset_reset_bridge_in_reset_reset),                            //                    reset.reset
		.uav_address              (leap_profiler_to_memory_translator_avalon_universal_master_0_address),       // avalon_universal_slave_0.address
		.uav_burstcount           (leap_profiler_to_memory_translator_avalon_universal_master_0_burstcount),    //                         .burstcount
		.uav_read                 (leap_profiler_to_memory_translator_avalon_universal_master_0_read),          //                         .read
		.uav_write                (leap_profiler_to_memory_translator_avalon_universal_master_0_write),         //                         .write
		.uav_waitrequest          (leap_profiler_to_memory_translator_avalon_universal_master_0_waitrequest),   //                         .waitrequest
		.uav_readdatavalid        (leap_profiler_to_memory_translator_avalon_universal_master_0_readdatavalid), //                         .readdatavalid
		.uav_byteenable           (leap_profiler_to_memory_translator_avalon_universal_master_0_byteenable),    //                         .byteenable
		.uav_readdata             (leap_profiler_to_memory_translator_avalon_universal_master_0_readdata),      //                         .readdata
		.uav_writedata            (leap_profiler_to_memory_translator_avalon_universal_master_0_writedata),     //                         .writedata
		.uav_lock                 (leap_profiler_to_memory_translator_avalon_universal_master_0_lock),          //                         .lock
		.uav_debugaccess          (leap_profiler_to_memory_translator_avalon_universal_master_0_debugaccess),   //                         .debugaccess
		.av_address               (Tiger_ICache_icache_slave_address),                                          //      avalon_anti_slave_0.address
		.av_read                  (Tiger_ICache_icache_slave_read),                                             //                         .read
		.av_readdata              (Tiger_ICache_icache_slave_readdata),                                         //                         .readdata
		.av_readdatavalid         (Tiger_ICache_icache_slave_readdatavalid),                                    //                         .readdatavalid
		.av_waitrequest           (Tiger_ICache_icache_slave_waitrequest),                                      //                         .waitrequest
		.av_write                 (),                                                                           //              (terminated)
		.av_writedata             (),                                                                           //              (terminated)
		.av_begintransfer         (),                                                                           //              (terminated)
		.av_beginbursttransfer    (),                                                                           //              (terminated)
		.av_burstcount            (),                                                                           //              (terminated)
		.av_byteenable            (),                                                                           //              (terminated)
		.av_writebyteenable       (),                                                                           //              (terminated)
		.av_lock                  (),                                                                           //              (terminated)
		.av_chipselect            (),                                                                           //              (terminated)
		.av_clken                 (),                                                                           //              (terminated)
		.uav_clken                (1'b0),                                                                       //              (terminated)
		.av_debugaccess           (),                                                                           //              (terminated)
		.av_outputenable          (),                                                                           //              (terminated)
		.uav_response             (),                                                                           //              (terminated)
		.av_response              (2'b00),                                                                      //              (terminated)
		.uav_writeresponserequest (1'b0),                                                                       //              (terminated)
		.uav_writeresponsevalid   (),                                                                           //              (terminated)
		.av_writeresponserequest  (),                                                                           //              (terminated)
		.av_writeresponsevalid    (1'b0)                                                                        //              (terminated)
	);

endmodule
