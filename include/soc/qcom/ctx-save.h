/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */
#ifndef __CTX_SAVE_H
#define __CTX_SAVE_H

enum tlvs {
	CTX_SAVE_TLV_INVALID,
	CTX_SAVE_TLV_UNAME,
	CTX_SAVE_TLV_DMESG,
	CTX_SAVE_TLV_LEVEL1_PT,
	CTX_SAVE_TLV_WLAN_MOD,
	CTX_SAVE_TLV_WLAN_MOD_DEBUGFS,
	CTX_SAVE_TLV_WLAN_MOD_INFO,
	CTX_SAVE_TLV_WLAN_MMU_INFO,
	CTX_SAVE_TLV_EMPTY,
};

#if IS_ENABLED(CONFIG_QTI_CTXT_SAVE)
int ctx_save_replace_tlv(unsigned char type, unsigned int size,
		const void *data);
bool ctx_save_is_type_available(unsigned char type, unsigned int *size);
int ctx_save_add_tlv(unsigned char type, unsigned int size,
						const void *data);
#else
static inline int ctx_save_replace_tlv(unsigned char type, unsigned int size,
		const void *data)
{
	return 0;
}

static inline int ctx_save_add_tlv(unsigned char type, unsigned int size,
							const void *data)
{
	return 0;
}

static inline bool ctx_save_is_type_available(unsigne char type,
						unsigned int *size)
{
	return false;
}
#endif
#endif
