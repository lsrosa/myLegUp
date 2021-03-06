// megafunction wizard: %altmemphy v11.1%
// GENERATION: XML

// ============================================================
// Megafunction Name(s):
// 			ddr2_phy_alt_mem_phy
// ============================================================
// Generated by altmemphy 11.1 [Altera, IP Toolbench 1.3.0 Build 259]
// ************************************************************
// THIS IS A WIZARD-GENERATED FILE. DO NOT EDIT THIS FILE!
// ************************************************************
// Copyright (C) 1991-2013 Altera Corporation
// Any megafunction design, and related net list (encrypted or decrypted),
// support information, device programming or simulation file, and any other
// associated documentation or information provided by Altera or a partner
// under Altera's Megafunction Partnership Program may be used only to
// program PLD devices (but not masked PLD devices) from Altera.  Any other
// use of such megafunction design, net list, support information, device
// programming or simulation file, or any other related documentation or
// information is prohibited for any other purpose, including, but not
// limited to modification, reverse engineering, de-compiling, or use with
// any other silicon devices, unless such use is explicitly licensed under
// a separate agreement with Altera or a megafunction partner.  Title to
// the intellectual property, including patents, copyrights, trademarks,
// trade secrets, or maskworks, embodied in any such megafunction design,
// net list, support information, device programming or simulation file, or
// any other related documentation or information provided by Altera or a
// megafunction partner, remains with Altera, the megafunction partner, or
// their respective licensors.  No other licenses, including any licenses
// needed under any third party's intellectual property, are provided herein.


module ddr2_phy (
	pll_ref_clk,
	global_reset_n,
	soft_reset_n,
	ctl_dqs_burst,
	ctl_wdata_valid,
	ctl_wdata,
	ctl_dm,
	ctl_addr,
	ctl_ba,
	ctl_cas_n,
	ctl_cke,
	ctl_cs_n,
	ctl_odt,
	ctl_ras_n,
	ctl_we_n,
	ctl_rst_n,
	ctl_doing_rd,
	ctl_cal_req,
	ctl_mem_clk_disable,
	mem_err_out_n,
	ctl_cal_byte_lane_sel_n,
	oct_ctl_rs_value,
	oct_ctl_rt_value,
	dqs_offset_delay_ctrl,
	dqs_delay_ctrl_import,
	dbg_clk,
	dbg_reset_n,
	dbg_addr,
	dbg_wr,
	dbg_rd,
	dbg_cs,
	dbg_wr_data,
	pll_reconfig_enable,
	pll_phasecounterselect,
	pll_phaseupdown,
	pll_phasestep,
	hc_scan_enable_access,
	hc_scan_enable_dq,
	hc_scan_enable_dm,
	hc_scan_enable_dqs,
	hc_scan_enable_dqs_config,
	hc_scan_din,
	hc_scan_update,
	hc_scan_ck,
	reset_request_n,
	ctl_clk,
	ctl_reset_n,
	ctl_wlat,
	ctl_rdata,
	ctl_rdata_valid,
	ctl_rlat,
	ctl_cal_success,
	ctl_cal_fail,
	ctl_cal_warning,
	mem_ac_parity,
	parity_error_n,
	mem_addr,
	mem_ba,
	mem_cas_n,
	mem_cke,
	mem_cs_n,
	mem_dm,
	mem_odt,
	mem_ras_n,
	mem_we_n,
	mem_reset_n,
	dqs_delay_ctrl_export,
	dll_reference_clk,
	aux_half_rate_clk,
	aux_full_rate_clk,
	aux_scan_clk,
	aux_scan_clk_reset_n,
	dbg_rd_data,
	dbg_waitrequest,
	pll_phase_done,
	hc_scan_dout,
	mem_clk,
	mem_clk_n,
	mem_dq,
	mem_dqs,
	mem_dqs_n);


	input		pll_ref_clk;
	input		global_reset_n;
	input		soft_reset_n;
	input	[15:0]	ctl_dqs_burst;
	input	[15:0]	ctl_wdata_valid;
	input	[255:0]	ctl_wdata;
	input	[31:0]	ctl_dm;
	input	[27:0]	ctl_addr;
	input	[5:0]	ctl_ba;
	input	[1:0]	ctl_cas_n;
	input	[1:0]	ctl_cke;
	input	[1:0]	ctl_cs_n;
	input	[1:0]	ctl_odt;
	input	[1:0]	ctl_ras_n;
	input	[1:0]	ctl_we_n;
	input	[1:0]	ctl_rst_n;
	input	[15:0]	ctl_doing_rd;
	input		ctl_cal_req;
	input	[1:0]	ctl_mem_clk_disable;
	input		mem_err_out_n;
	input	[7:0]	ctl_cal_byte_lane_sel_n;
	input	[13:0]	oct_ctl_rs_value;
	input	[13:0]	oct_ctl_rt_value;
	input	[5:0]	dqs_offset_delay_ctrl;
	input	[5:0]	dqs_delay_ctrl_import;
	input		dbg_clk;
	input		dbg_reset_n;
	input	[12:0]	dbg_addr;
	input		dbg_wr;
	input		dbg_rd;
	input		dbg_cs;
	input	[31:0]	dbg_wr_data;
	input		pll_reconfig_enable;
	input	[3:0]	pll_phasecounterselect;
	input		pll_phaseupdown;
	input		pll_phasestep;
	input		hc_scan_enable_access;
	input	[63:0]	hc_scan_enable_dq;
	input	[7:0]	hc_scan_enable_dm;
	input	[7:0]	hc_scan_enable_dqs;
	input	[7:0]	hc_scan_enable_dqs_config;
	input	[7:0]	hc_scan_din;
	input	[7:0]	hc_scan_update;
	input		hc_scan_ck;
	output		reset_request_n;
	output		ctl_clk;
	output		ctl_reset_n;
	output	[4:0]	ctl_wlat;
	output	[255:0]	ctl_rdata;
	output	[1:0]	ctl_rdata_valid;
	output	[4:0]	ctl_rlat;
	output		ctl_cal_success;
	output		ctl_cal_fail;
	output		ctl_cal_warning;
	output		mem_ac_parity;
	output		parity_error_n;
	output	[13:0]	mem_addr;
	output	[2:0]	mem_ba;
	output		mem_cas_n;
	output	[0:0]	mem_cke;
	output	[0:0]	mem_cs_n;
	output	[7:0]	mem_dm;
	output	[0:0]	mem_odt;
	output		mem_ras_n;
	output		mem_we_n;
	output		mem_reset_n;
	output	[5:0]	dqs_delay_ctrl_export;
	output		dll_reference_clk;
	output		aux_half_rate_clk;
	output		aux_full_rate_clk;
	output		aux_scan_clk;
	output		aux_scan_clk_reset_n;
	output	[31:0]	dbg_rd_data;
	output		dbg_waitrequest;
	output		pll_phase_done;
	output	[63:0]	hc_scan_dout;
	inout	[1:0]	mem_clk;
	inout	[1:0]	mem_clk_n;
	inout	[63:0]	mem_dq;
	inout	[7:0]	mem_dqs;
	inout	[7:0]	mem_dqs_n;


	ddr2_phy_alt_mem_phy	ddr2_phy_alt_mem_phy_inst(
		.pll_ref_clk(pll_ref_clk),
		.global_reset_n(global_reset_n),
		.soft_reset_n(soft_reset_n),
		.ctl_dqs_burst(ctl_dqs_burst),
		.ctl_wdata_valid(ctl_wdata_valid),
		.ctl_wdata(ctl_wdata),
		.ctl_dm(ctl_dm),
		.ctl_addr(ctl_addr),
		.ctl_ba(ctl_ba),
		.ctl_cas_n(ctl_cas_n),
		.ctl_cke(ctl_cke),
		.ctl_cs_n(ctl_cs_n),
		.ctl_odt(ctl_odt),
		.ctl_ras_n(ctl_ras_n),
		.ctl_we_n(ctl_we_n),
		.ctl_rst_n(ctl_rst_n),
		.ctl_doing_rd(ctl_doing_rd),
		.ctl_cal_req(ctl_cal_req),
		.ctl_mem_clk_disable(ctl_mem_clk_disable),
		.mem_err_out_n(mem_err_out_n),
		.ctl_cal_byte_lane_sel_n(ctl_cal_byte_lane_sel_n),
		.oct_ctl_rs_value(oct_ctl_rs_value),
		.oct_ctl_rt_value(oct_ctl_rt_value),
		.dqs_offset_delay_ctrl(dqs_offset_delay_ctrl),
		.dqs_delay_ctrl_import(dqs_delay_ctrl_import),
		.dbg_clk(dbg_clk),
		.dbg_reset_n(dbg_reset_n),
		.dbg_addr(dbg_addr),
		.dbg_wr(dbg_wr),
		.dbg_rd(dbg_rd),
		.dbg_cs(dbg_cs),
		.dbg_wr_data(dbg_wr_data),
		.pll_reconfig_enable(pll_reconfig_enable),
		.pll_phasecounterselect(pll_phasecounterselect),
		.pll_phaseupdown(pll_phaseupdown),
		.pll_phasestep(pll_phasestep),
		.hc_scan_enable_access(hc_scan_enable_access),
		.hc_scan_enable_dq(hc_scan_enable_dq),
		.hc_scan_enable_dm(hc_scan_enable_dm),
		.hc_scan_enable_dqs(hc_scan_enable_dqs),
		.hc_scan_enable_dqs_config(hc_scan_enable_dqs_config),
		.hc_scan_din(hc_scan_din),
		.hc_scan_update(hc_scan_update),
		.hc_scan_ck(hc_scan_ck),
		.reset_request_n(reset_request_n),
		.ctl_clk(ctl_clk),
		.ctl_reset_n(ctl_reset_n),
		.ctl_wlat(ctl_wlat),
		.ctl_rdata(ctl_rdata),
		.ctl_rdata_valid(ctl_rdata_valid),
		.ctl_rlat(ctl_rlat),
		.ctl_cal_success(ctl_cal_success),
		.ctl_cal_fail(ctl_cal_fail),
		.ctl_cal_warning(ctl_cal_warning),
		.mem_ac_parity(mem_ac_parity),
		.parity_error_n(parity_error_n),
		.mem_addr(mem_addr),
		.mem_ba(mem_ba),
		.mem_cas_n(mem_cas_n),
		.mem_cke(mem_cke),
		.mem_cs_n(mem_cs_n),
		.mem_dm(mem_dm),
		.mem_odt(mem_odt),
		.mem_ras_n(mem_ras_n),
		.mem_we_n(mem_we_n),
		.mem_reset_n(mem_reset_n),
		.dqs_delay_ctrl_export(dqs_delay_ctrl_export),
		.dll_reference_clk(dll_reference_clk),
		.aux_half_rate_clk(aux_half_rate_clk),
		.aux_full_rate_clk(aux_full_rate_clk),
		.aux_scan_clk(aux_scan_clk),
		.aux_scan_clk_reset_n(aux_scan_clk_reset_n),
		.dbg_rd_data(dbg_rd_data),
		.dbg_waitrequest(dbg_waitrequest),
		.pll_phase_done(pll_phase_done),
		.hc_scan_dout(hc_scan_dout),
		.mem_clk(mem_clk),
		.mem_clk_n(mem_clk_n),
		.mem_dq(mem_dq),
		.mem_dqs(mem_dqs),
		.mem_dqs_n(mem_dqs_n));

	defparam
		ddr2_phy_alt_mem_phy_inst.FAMILY = "Stratix IV",
		ddr2_phy_alt_mem_phy_inst.MEM_IF_MEMTYPE = "DDR2",
		ddr2_phy_alt_mem_phy_inst.DLL_DELAY_BUFFER_MODE = "HIGH",
		ddr2_phy_alt_mem_phy_inst.DLL_DELAY_CHAIN_LENGTH = 10,
		ddr2_phy_alt_mem_phy_inst.DQS_DELAY_CTL_WIDTH = 6,
		ddr2_phy_alt_mem_phy_inst.DQS_OUT_MODE = "DELAY_CHAIN2",
		ddr2_phy_alt_mem_phy_inst.DQS_PHASE = 7200,
		ddr2_phy_alt_mem_phy_inst.DQS_PHASE_SETTING = 2,
		ddr2_phy_alt_mem_phy_inst.DWIDTH_RATIO = 4,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_DWIDTH = 64,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_ADDR_WIDTH = 14,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_BANKADDR_WIDTH = 3,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_CS_WIDTH = 1,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_CS_PER_RANK = 1,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_DM_WIDTH = 8,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_DM_PINS_EN = 1,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_DQ_PER_DQS = 8,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_DQS_WIDTH = 8,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_OCT_EN = 1,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_CLK_PAIR_COUNT = 2,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_CLK_PS = 2500,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_CLK_PS_STR = "2500 ps",
		ddr2_phy_alt_mem_phy_inst.MEM_IF_MR_0 = 2659,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_MR_1 = 68,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_MR_2 = 0,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_MR_3 = 0,
		ddr2_phy_alt_mem_phy_inst.PLL_STEPS_PER_CYCLE = 32,
		ddr2_phy_alt_mem_phy_inst.SCAN_CLK_DIVIDE_BY = 4,
		ddr2_phy_alt_mem_phy_inst.MEM_IF_DQSN_EN = 1,
		ddr2_phy_alt_mem_phy_inst.DLL_EXPORT_IMPORT = "EXPORT",
		ddr2_phy_alt_mem_phy_inst.MEM_IF_ADDR_CMD_PHASE = 240,
		ddr2_phy_alt_mem_phy_inst.RANK_HAS_ADDR_SWAP = 0,
		ddr2_phy_alt_mem_phy_inst.LEVELLING = 0,
		ddr2_phy_alt_mem_phy_inst.READ_DESKEW_MODE = "NONE",
		ddr2_phy_alt_mem_phy_inst.WRITE_DESKEW_MODE = "NONE",
		ddr2_phy_alt_mem_phy_inst.PLL_RECONFIG_PORTS_EN = 0,
		ddr2_phy_alt_mem_phy_inst.INVERT_POSTAMBLE_CLK = "true",
		ddr2_phy_alt_mem_phy_inst.INVERT_ADDR_CMD_TXFR_CLK = "true",
		ddr2_phy_alt_mem_phy_inst.CHIP_OR_DIMM = "Unbuffered DIMM";
endmodule
