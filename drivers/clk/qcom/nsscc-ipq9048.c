// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/regmap.h>

#include <dt-bindings/clock/qcom,nsscc-ipq9048.h>
#include <dt-bindings/reset/qcom,nsscc-ipq9048.h>

#include "common.h"
#include "clk-regmap.h"
#include "reset.h"

static int clk_dummy_is_enabled(struct clk_hw *hw)
{
	return 1;
};

static int clk_dummy_enable(struct clk_hw *hw)
{
	return 0;
};

static void clk_dummy_disable(struct clk_hw *hw)
{
	return;
};

static u8 clk_dummy_get_parent(struct clk_hw *hw)
{
	return 0;
};

static int clk_dummy_set_parent(struct clk_hw *hw, u8 index)
{
	return 0;
};

static int clk_dummy_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	return 0;
};

static int clk_dummy_determine_rate(struct clk_hw *hw,
				struct clk_rate_request *req)
{
	return 0;
};

static unsigned long clk_dummy_recalc_rate(struct clk_hw *hw,
					   unsigned long parent_rate)
{
	return parent_rate;
};

static const struct clk_ops clk_dummy_ops = {
	.is_enabled = clk_dummy_is_enabled,
	.enable = clk_dummy_enable,
	.disable = clk_dummy_disable,
	.get_parent = clk_dummy_get_parent,
	.set_parent = clk_dummy_set_parent,
	.set_rate = clk_dummy_set_rate,
	.recalc_rate = clk_dummy_recalc_rate,
	.determine_rate = clk_dummy_determine_rate,
};

#define DEFINE_DUMMY_CLK(clk_name)				\
(&(struct clk_regmap) {						\
	.hw.init = &(struct clk_init_data){			\
		.name = #clk_name,				\
		.parent_names = (const char *[]){ "xo"},	\
		.num_parents = 1,				\
		.ops = &clk_dummy_ops,				\
	},							\
})

static struct clk_regmap *nss_cc_ipq9048_dummy_clocks[] = {
	[NSS_CC_CE_APB_CLK] = DEFINE_DUMMY_CLK(nss_cc_ce_apb_clk),
	[NSS_CC_CE_AXI_CLK] = DEFINE_DUMMY_CLK(nss_cc_ce_axi_clk),
	[NSS_CC_CE_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ce_clk_src),
	[NSS_CC_CFG_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_cfg_clk_src),
	[NSS_CC_CLC_AXI_CLK] = DEFINE_DUMMY_CLK(nss_cc_clc_axi_clk),
	[NSS_CC_CLC_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_clc_clk_src),
	[NSS_CC_CRYPTO_CLK] = DEFINE_DUMMY_CLK(nss_cc_crypto_clk),
	[NSS_CC_CRYPTO_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_crypto_clk_src),
	[NSS_CC_CRYPTO_PPE_CLK] = DEFINE_DUMMY_CLK(nss_cc_crypto_ppe_clk),
	[NSS_CC_HAQ_AHB_CLK] = DEFINE_DUMMY_CLK(nss_cc_haq_ahb_clk),
	[NSS_CC_HAQ_AXI_CLK] = DEFINE_DUMMY_CLK(nss_cc_haq_axi_clk),
	[NSS_CC_HAQ_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_haq_clk_src),
	[NSS_CC_IMEM_AHB_CLK] = DEFINE_DUMMY_CLK(nss_cc_imem_ahb_clk),
	[NSS_CC_IMEM_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_imem_clk_src),
	[NSS_CC_IMEM_QSB_CLK] = DEFINE_DUMMY_CLK(nss_cc_imem_qsb_clk),
	[NSS_CC_INT_CFG_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_int_cfg_clk_src),
	[NSS_CC_NSS_CSR_CLK] = DEFINE_DUMMY_CLK(nss_cc_nss_csr_clk),
	[NSS_CC_NSSNOC_CE_APB_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_ce_apb_clk),
	[NSS_CC_NSSNOC_CE_AXI_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_ce_axi_clk),
	[NSS_CC_NSSNOC_CLC_AXI_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_clc_axi_clk),
	[NSS_CC_NSSNOC_CRYPTO_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_crypto_clk),
	[NSS_CC_NSSNOC_HAQ_AHB_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_haq_ahb_clk),
	[NSS_CC_NSSNOC_HAQ_AXI_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_haq_axi_clk),
	[NSS_CC_NSSNOC_IMEM_AHB_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_imem_ahb_clk),
	[NSS_CC_NSSNOC_IMEM_QSB_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_imem_qsb_clk),
	[NSS_CC_NSSNOC_NSS_CSR_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_nss_csr_clk),
	[NSS_CC_NSSNOC_PPE_CFG_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_ppe_cfg_clk),
	[NSS_CC_NSSNOC_PPE_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_ppe_clk),
	[NSS_CC_NSSNOC_UBI32_AHB0_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_ubi32_ahb0_clk),
	[NSS_CC_NSSNOC_UBI32_AXI0_CLK] = DEFINE_DUMMY_CLK(nss_cc_nssnoc_ubi32_axi0_clk),
	[NSS_CC_NSSNOC_UBI32_INT0_AHB_CLK] =
		DEFINE_DUMMY_CLK(nss_cc_nssnoc_ubi32_int0_ahb_clk),
	[NSS_CC_NSSNOC_UBI32_NC_AXI0_1_CLK] =
		DEFINE_DUMMY_CLK(nss_cc_nssnoc_ubi32_nc_axi0_1_clk),
	[NSS_CC_NSSNOC_UBI32_NC_AXI0_CLK] =
		DEFINE_DUMMY_CLK(nss_cc_nssnoc_ubi32_nc_axi0_clk),
	[NSS_CC_PORT1_MAC_CLK] = DEFINE_DUMMY_CLK(nss_cc_port1_mac_clk),
	[NSS_CC_PORT1_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port1_rx_clk),
	[NSS_CC_PORT1_RX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port1_rx_clk_src),
	[NSS_CC_PORT1_RX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port1_rx_div_clk_src),
	[NSS_CC_PORT1_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port1_tx_clk),
	[NSS_CC_PORT1_TX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port1_tx_clk_src),
	[NSS_CC_PORT1_TX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port1_tx_div_clk_src),
	[NSS_CC_PORT2_MAC_CLK] = DEFINE_DUMMY_CLK(nss_cc_port2_mac_clk),
	[NSS_CC_PORT2_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port2_rx_clk),
	[NSS_CC_PORT2_RX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port2_rx_clk_src),
	[NSS_CC_PORT2_RX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port2_rx_div_clk_src),
	[NSS_CC_PORT2_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port2_tx_clk),
	[NSS_CC_PORT2_TX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port2_tx_clk_src),
	[NSS_CC_PORT2_TX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port2_tx_div_clk_src),
	[NSS_CC_PORT3_MAC_CLK] = DEFINE_DUMMY_CLK(nss_cc_port3_mac_clk),
	[NSS_CC_PORT3_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port3_rx_clk),
	[NSS_CC_PORT3_RX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port3_rx_clk_src),
	[NSS_CC_PORT3_RX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port3_rx_div_clk_src),
	[NSS_CC_PORT3_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port3_tx_clk),
	[NSS_CC_PORT3_TX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port3_tx_clk_src),
	[NSS_CC_PORT3_TX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port3_tx_div_clk_src),
	[NSS_CC_PORT4_MAC_CLK] = DEFINE_DUMMY_CLK(nss_cc_port4_mac_clk),
	[NSS_CC_PORT4_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port4_rx_clk),
	[NSS_CC_PORT4_RX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port4_rx_clk_src),
	[NSS_CC_PORT4_RX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port4_rx_div_clk_src),
	[NSS_CC_PORT4_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port4_tx_clk),
	[NSS_CC_PORT4_TX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port4_tx_clk_src),
	[NSS_CC_PORT4_TX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port4_tx_div_clk_src),
	[NSS_CC_PORT5_MAC_CLK] = DEFINE_DUMMY_CLK(nss_cc_port5_mac_clk),
	[NSS_CC_PORT5_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port5_rx_clk),
	[NSS_CC_PORT5_RX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port5_rx_clk_src),
	[NSS_CC_PORT5_RX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port5_rx_div_clk_src),
	[NSS_CC_PORT5_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port5_tx_clk),
	[NSS_CC_PORT5_TX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port5_tx_clk_src),
	[NSS_CC_PORT5_TX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port5_tx_div_clk_src),
	[NSS_CC_PORT6_MAC_CLK] = DEFINE_DUMMY_CLK(nss_cc_port6_mac_clk),
	[NSS_CC_PORT6_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port6_rx_clk),
	[NSS_CC_PORT6_RX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port6_rx_clk_src),
	[NSS_CC_PORT6_RX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port6_rx_div_clk_src),
	[NSS_CC_PORT6_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_port6_tx_clk),
	[NSS_CC_PORT6_TX_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port6_tx_clk_src),
	[NSS_CC_PORT6_TX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_port6_tx_div_clk_src),
	[NSS_CC_PPE_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ppe_clk_src),
	[NSS_CC_PPE_EDMA_CFG_CLK] = DEFINE_DUMMY_CLK(nss_cc_ppe_edma_cfg_clk),
	[NSS_CC_PPE_EDMA_CLK] = DEFINE_DUMMY_CLK(nss_cc_ppe_edma_clk),
	[NSS_CC_PPE_SWITCH_BTQ_CLK] = DEFINE_DUMMY_CLK(nss_cc_ppe_switch_btq_clk),
	[NSS_CC_PPE_SWITCH_CFG_CLK] = DEFINE_DUMMY_CLK(nss_cc_ppe_switch_cfg_clk),
	[NSS_CC_PPE_SWITCH_CLK] = DEFINE_DUMMY_CLK(nss_cc_ppe_switch_clk),
	[NSS_CC_PPE_SWITCH_IPE_CLK] = DEFINE_DUMMY_CLK(nss_cc_ppe_switch_ipe_clk),
	[NSS_CC_UBI0_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi0_clk_src),
	[NSS_CC_UBI0_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi0_div_clk_src),
	[NSS_CC_UBI1_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi1_clk_src),
	[NSS_CC_UBI1_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi1_div_clk_src),
	[NSS_CC_UBI2_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi2_clk_src),
	[NSS_CC_UBI2_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi2_div_clk_src),
	[NSS_CC_UBI32_AHB0_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_ahb0_clk),
	[NSS_CC_UBI32_AHB1_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_ahb1_clk),
	[NSS_CC_UBI32_AHB2_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_ahb2_clk),
	[NSS_CC_UBI32_AHB3_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_ahb3_clk),
	[NSS_CC_UBI32_AXI0_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_axi0_clk),
	[NSS_CC_UBI32_AXI1_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_axi1_clk),
	[NSS_CC_UBI32_AXI2_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_axi2_clk),
	[NSS_CC_UBI32_AXI3_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_axi3_clk),
	[NSS_CC_UBI32_CORE0_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_core0_clk),
	[NSS_CC_UBI32_CORE1_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_core1_clk),
	[NSS_CC_UBI32_CORE2_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_core2_clk),
	[NSS_CC_UBI32_CORE3_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_core3_clk),
	[NSS_CC_UBI32_INTR0_AHB_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_intr0_ahb_clk),
	[NSS_CC_UBI32_INTR1_AHB_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_intr1_ahb_clk),
	[NSS_CC_UBI32_INTR2_AHB_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_intr2_ahb_clk),
	[NSS_CC_UBI32_INTR3_AHB_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_intr3_ahb_clk),
	[NSS_CC_UBI32_NC_AXI0_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_nc_axi0_clk),
	[NSS_CC_UBI32_NC_AXI1_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_nc_axi1_clk),
	[NSS_CC_UBI32_NC_AXI2_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_nc_axi2_clk),
	[NSS_CC_UBI32_NC_AXI3_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_nc_axi3_clk),
	[NSS_CC_UBI32_UTCM0_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_utcm0_clk),
	[NSS_CC_UBI32_UTCM1_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_utcm1_clk),
	[NSS_CC_UBI32_UTCM2_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_utcm2_clk),
	[NSS_CC_UBI32_UTCM3_CLK] = DEFINE_DUMMY_CLK(nss_cc_ubi32_utcm3_clk),
	[NSS_CC_UBI3_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi3_clk_src),
	[NSS_CC_UBI3_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi3_div_clk_src),
	[NSS_CC_UBI_AXI_CLK_SRC] = DEFINE_DUMMY_CLK(nss_cc_ubi_axi_clk_src),
	[NSS_CC_UBI_NC_AXI_BFDCD_CLK_SRC] =
		DEFINE_DUMMY_CLK(nss_cc_ubi_nc_axi_bfdcd_clk_src),
	[NSS_CC_UNIPHY_PORT1_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port1_rx_clk),
	[NSS_CC_UNIPHY_PORT1_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port1_tx_clk),
	[NSS_CC_UNIPHY_PORT2_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port2_rx_clk),
	[NSS_CC_UNIPHY_PORT2_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port2_tx_clk),
	[NSS_CC_UNIPHY_PORT3_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port3_rx_clk),
	[NSS_CC_UNIPHY_PORT3_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port3_tx_clk),
	[NSS_CC_UNIPHY_PORT4_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port4_rx_clk),
	[NSS_CC_UNIPHY_PORT4_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port4_tx_clk),
	[NSS_CC_UNIPHY_PORT5_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port5_rx_clk),
	[NSS_CC_UNIPHY_PORT5_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port5_tx_clk),
	[NSS_CC_UNIPHY_PORT6_RX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port6_rx_clk),
	[NSS_CC_UNIPHY_PORT6_TX_CLK] = DEFINE_DUMMY_CLK(nss_cc_uniphy_port6_tx_clk),
	[NSS_CC_XGMAC0_PTP_REF_CLK] = DEFINE_DUMMY_CLK(nss_cc_xgmac0_ptp_ref_clk),
	[NSS_CC_XGMAC0_PTP_REF_DIV_CLK_SRC] =
		DEFINE_DUMMY_CLK(nss_cc_xgmac0_ptp_ref_div_clk_src),
	[NSS_CC_XGMAC1_PTP_REF_CLK] = DEFINE_DUMMY_CLK(nss_cc_xgmac1_ptp_ref_clk),
	[NSS_CC_XGMAC1_PTP_REF_DIV_CLK_SRC] =
		DEFINE_DUMMY_CLK(nss_cc_xgmac1_ptp_ref_div_clk_src),
	[NSS_CC_XGMAC2_PTP_REF_CLK] = DEFINE_DUMMY_CLK(nss_cc_xgmac2_ptp_ref_clk),
	[NSS_CC_XGMAC2_PTP_REF_DIV_CLK_SRC] =
		DEFINE_DUMMY_CLK(nss_cc_xgmac2_ptp_ref_div_clk_src),
	[NSS_CC_XGMAC3_PTP_REF_CLK] = DEFINE_DUMMY_CLK(nss_cc_xgmac3_ptp_ref_clk),
	[NSS_CC_XGMAC3_PTP_REF_DIV_CLK_SRC] =
		DEFINE_DUMMY_CLK(nss_cc_xgmac3_ptp_ref_div_clk_src),
	[NSS_CC_XGMAC4_PTP_REF_CLK] = DEFINE_DUMMY_CLK(nss_cc_xgmac4_ptp_ref_clk),
	[NSS_CC_XGMAC4_PTP_REF_DIV_CLK_SRC] =
		DEFINE_DUMMY_CLK(nss_cc_xgmac4_ptp_ref_div_clk_src),
	[NSS_CC_XGMAC5_PTP_REF_CLK] = DEFINE_DUMMY_CLK(nss_cc_xgmac5_ptp_ref_clk),
	[NSS_CC_XGMAC5_PTP_REF_DIV_CLK_SRC] =
		DEFINE_DUMMY_CLK(nss_cc_xgmac5_ptp_ref_div_clk_src),
	[UBI32_PLL] = DEFINE_DUMMY_CLK(ubi32_pll),
};

static const struct qcom_reset_map nss_cc_ipq9048_resets[] = {
	[NSS_CC_CE_BCR] = { 0x28400, 0 },
	[NSS_CC_CLC_BCR] = { 0x28600, 0 },
	[NSS_CC_EIP197_BCR] = { 0x16004, 0 },
	[NSS_CC_HAQ_BCR] = { 0x28300, 0 },
	[NSS_CC_IMEM_BCR] = { 0xe004, 0 },
	[NSS_CC_MAC_BCR] = { 0x28100, 0 },
	[NSS_CC_PPE_BCR] = { 0x28200, 0 },
	[NSS_CC_UBI_BCR] = { 0x28700, 0 },
	[NSS_CC_UNIPHY_BCR] = { 0x28900, 0 },
	[UBI3_CLKRST_CLAMP_ENABLE] = { 0x28A04, 9 },
	[UBI3_CORE_CLAMP_ENABLE] = { 0x28A04, 8 },
	[UBI2_CLKRST_CLAMP_ENABLE] = { 0x28A04, 7 },
	[UBI2_CORE_CLAMP_ENABLE] = { 0x28A04, 6 },
	[UBI1_CLKRST_CLAMP_ENABLE] = { 0x28A04, 5 },
	[UBI1_CORE_CLAMP_ENABLE] = { 0x28A04, 4 },
	[UBI0_CLKRST_CLAMP_ENABLE] = { 0x28A04, 3 },
	[UBI0_CORE_CLAMP_ENABLE] = { 0x28A04, 2 },
	[NSSNOC_NSS_CSR_ARES] = { 0x28A04, 1 },
	[NSS_CSR_ARES]  { 0x28A04, 0 },
	[PPE_BTQ_ARES] = { 0x28A08, 20 },
	[PPE_IPE_ARES] = { 0x28A08, 19 },
	[PPE_ARES] = { 0x28A08, 18 },
	[PPE_CFG_ARES] = { 0x28A08, 17 },
	[PPE_EDMA_ARES] = { 0x28A08, 16 },
	[PPE_EDMA_CFG_ARES] = { 0x28A08, 15 },
	[CRY_PPE_ARES] = { 0x28A08, 14 },
	[NSSNOC_PPE_ARES] = { 0x28A08, 13 },
	[NSSNOC_PPE_CFG_ARES] = { 0x28A08, 12 },
	[PORT1_MAC_ARES] = { 0x28A08, 11 },
	[PORT2_MAC_ARES] = { 0x28A08, 10 },
	[PORT3_MAC_ARES] = { 0x28A08, 9 },
	[PORT4_MAC_ARES] = { 0x28A08, 8 },
	[PORT5_MAC_ARES] = { 0x28A08, 7 },
	[PORT6_MAC_ARES] = { 0x28A08, 6 },
	[XGMAC0_PTP_REF_ARES] = { 0x28A08, 5 },
	[XGMAC1_PTP_REF_ARES] = { 0x28A08, 4 },
	[XGMAC2_PTP_REF_ARES] = { 0x28A08, 3 },
	[XGMAC3_PTP_REF_ARES] = { 0x28A08, 2 },
	[XGMAC4_PTP_REF_ARES] = { 0x28A08, 1 },
	[XGMAC5_PTP_REF_ARES] = { 0x28A08, 0 },
	[HAQ_AHB_ARES] = { 0x28A0C, 3 },
	[HAQ_AXI_ARES] = { 0x28A0C, 2 },
	[NSSNOC_HAQ_AHB_ARES] = { 0x28A0C, 1 },
	[NSSNOC_HAQ_AXI_ARES] = { 0x28A0C, 0 },
	[CE_APB_ARES] = { 0x28A10, 3 },
	[CE_AXI_ARES] = { 0x28A10, 2 },
	[NSSNOC_CE_APB_ARES] = { 0x28A10, 1 },
	[NSSNOC_CE_AXI_ARES] = { 0x28A10, 0 },
	[CRYPTO_ARES] = { 0x28A14, 1 },
	[NSSNOC_CRYPTO_ARES] = { 0x28A14, 0 },
	[NSSNOC_NC_AXI0_1_ARES] = { 0x28A1C, 28 },
	[UBI0_CORE_ARES] = { 0x28A1C, 27 },
	[UBI1_CORE_ARES] = { 0x28A1C, 26 },
	[UBI2_CORE_ARES] = { 0x28A1C, 25 },
	[UBI3_CORE_ARES] = { 0x28A1C, 24 },
	[NC_AXI0_ARES] = { 0x28A1C, 23 },
	[UTCM0_ARES] = { 0x28A1C, 22 },
	[NC_AXI1_ARES] = { 0x28A1C, 21 },
	[UTCM1_ARES] = { 0x28A1C, 20 },
	[NC_AXI2_ARES] = { 0x28A1C, 19 },
	[UTCM2_ARES] = { 0x28A1C, 18 },
	[NC_AXI3_ARES] = { 0x28A1C, 17 },
	[UTCM3_ARES] = { 0x28A1C, 16 },
	[NSSNOC_NC_AXI0_ARES] = { 0x28A1C, 15 },
	[AHB0_ARES] = { 0x28A1C, 14 },
	[INTR0_AHB_ARES] = { 0x28A1C, 13 },
	[AHB1_ARES] = { 0x28A1C, 12 },
	[INTR1_AHB_ARES] = { 0x28A1C, 11 },
	[AHB2_ARES] = { 0x28A1C, 10 },
	[INTR2_AHB_ARES] = { 0x28A1C, 9 },
	[AHB3_ARES] = { 0x28A1C, 8 },
	[INTR3_AHB_ARES] = { 0x28A1C, 7 },
	[NSSNOC_AHB0_ARES] = { 0x28A1C, 6 },
	[NSSNOC_INT0_AHB_ARES] = { 0x28A1C, 5 },
	[AXI0_ARES] = { 0x28A1C, 4 },
	[AXI1_ARES] = { 0x28A1C, 3 },
	[AXI2_ARES] = { 0x28A1C, 2 },
	[AXI3_ARES] = { 0x28A1C, 1 },
	[NSSNOC_AXI0_ARES] = { 0x28A1C, 0 },
	[IMEM_QSB_ARES] = { 0x28A20, 3 },
	[NSSNOC_IMEM_QSB_ARES] = { 0x28A20, 2 },
	[IMEM_AHB_ARES] = { 0x28A20, 1 },
	[NSSNOC_IMEM_AHB_ARES] = { 0x28A20, 0 },
	[UNIPHY_PORT1_RX_ARES] = { 0x28A24, 23 },
	[UNIPHY_PORT1_TX_ARES] = { 0x28A24, 22 },
	[UNIPHY_PORT2_RX_ARES] = { 0x28A24, 21 },
	[UNIPHY_PORT2_TX_ARES] = { 0x28A24, 20 },
	[UNIPHY_PORT3_RX_ARES] = { 0x28A24, 19 },
	[UNIPHY_PORT3_TX_ARES] = { 0x28A24, 18 },
	[UNIPHY_PORT4_RX_ARES] = { 0x28A24, 17 },
	[UNIPHY_PORT4_TX_ARES] = { 0x28A24, 16 },
	[UNIPHY_PORT5_RX_ARES] = { 0x28A24, 15 },
	[UNIPHY_PORT5_TX_ARES] = { 0x28A24, 14 },
	[UNIPHY_PORT6_RX_ARES] = { 0x28A24, 13 },
	[UNIPHY_PORT6_TX_ARES] = { 0x28A24, 12 },
	[PORT1_RX_ARES] = { 0x28A24, 11 },
	[PORT1_TX_ARES] = { 0x28A24, 10 },
	[PORT2_RX_ARES] = { 0x28A24, 9 },
	[PORT2_TX_ARES] = { 0x28A24, 8 },
	[PORT3_RX_ARES] = { 0x28A24, 7 },
	[PORT3_TX_ARES] = { 0x28A24, 6 },
	[PORT4_RX_ARES] = { 0x28A24, 5 },
	[PORT4_TX_ARES] = { 0x28A24, 4 },
	[PORT5_RX_ARES] = { 0x28A24, 3 },
	[PORT5_TX_ARES] = { 0x28A24, 2 },
	[PORT6_RX_ARES] = { 0x28A24, 1 },
	[PORT6_TX_ARES] = { 0x28A24, 0 },
};

static const struct regmap_config nss_cc_ipq9048_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = 0x28a34,
	.fast_io = true,
};

static const struct qcom_cc_desc nss_cc_ipq9048_dummy_desc = {
	.config = &nss_cc_ipq9048_regmap_config,
	.clks = nss_cc_ipq9048_dummy_clocks,
	.num_clks = ARRAY_SIZE(nss_cc_ipq9048_dummy_clocks),
	.resets = nss_cc_ipq9048_resets,
	.num_resets = ARRAY_SIZE(nss_cc_ipq9048_resets),
};

static const struct of_device_id nss_cc_ipq9048_match_table[] = {
	{ .compatible = "qcom,nsscc-ipq9048" },
	{ }
};
MODULE_DEVICE_TABLE(of, nss_cc_ipq9048_match_table);

static int nss_cc_ipq9048_probe(struct platform_device *pdev)
{
	int ret;
	struct regmap *regmap;
	struct qcom_cc_desc nsscc_ipq9048_desc = nss_cc_ipq9048_dummy_desc;

	regmap = qcom_cc_map(pdev, &nsscc_ipq9048_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	ret = qcom_cc_really_probe(pdev, &nsscc_ipq9048_desc, regmap);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register NSS CC clocks\n");
		return ret;
	}

	dev_info(&pdev->dev, "Registered NSS CC dummy clocks\n");

	return ret;
}

static int nss_cc_ipq9048_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver nss_cc_ipq9048_driver = {
	.probe = nss_cc_ipq9048_probe,
	.remove = nss_cc_ipq9048_remove,
	.driver = {
		.name = "qcom,nsscc-ipq9048",
		.owner  = THIS_MODULE,
		.of_match_table = nss_cc_ipq9048_match_table,
	},
};

static int __init nss_cc_ipq9048_init(void)
{
	return platform_driver_register(&nss_cc_ipq9048_driver);
}
subsys_initcall(nss_cc_ipq9048_init);

static void __exit nss_cc_ipq9048_exit(void)
{
	platform_driver_unregister(&nss_cc_ipq9048_driver);
}
module_exit(nss_cc_ipq9048_exit);

MODULE_DESCRIPTION("QTI NSS_CC IPQ9048 Driver");
MODULE_LICENSE("GPL v2");
