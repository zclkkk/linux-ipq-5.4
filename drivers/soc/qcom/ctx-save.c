// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/qcom_scm.h>
#include <linux/module.h>
#include <soc/qcom/ctx-save.h>
#include <linux/utsname.h>

struct ctx_save_tlv_msg {
	unsigned char *msg_buffer;
	unsigned char *cur_pos;
	unsigned int len;
	spinlock_t spinlock;
	bool is_panic;
};

struct ctx_save_props {
	unsigned int tlv_msg_offset;
	unsigned int crashdump_page_size;
};

#define QTI_WDT_SCM_TLV_TYPE_LEN_SIZE 3

static struct ctx_save_tlv_msg ctx_tlv_msg;

static bool __ctx_save_find_type(unsigned char type, unsigned int *size,
					unsigned char **ptr)
{
	unsigned long flags;
	struct ctx_save_tlv_msg *tlv_msg = &ctx_tlv_msg;
	unsigned char *itr = tlv_msg->msg_buffer;
	unsigned char *max = tlv_msg->cur_pos;
	unsigned int curr_size = 0;

	spin_lock_irqsave(&tlv_msg->spinlock, flags);
	while (itr < max) {
		curr_size = itr[1] + (itr[2] << 8);
		if (*itr == type) {
			*size = curr_size;
			if (ptr)
				*ptr = itr;
			spin_unlock_irqrestore(&tlv_msg->spinlock, flags);
			return true;
		}
		itr += curr_size;
	}
	spin_unlock_irqrestore(&tlv_msg->spinlock, flags);
	return false;
}

/*
 * Function - ctx_save_is_type_available
 * Description: Check if a specified type is available in global
 *		TLV buffer
 * @param:	[in] type -type value of TLV
 *		[out] size - size of the current TLV in global buffer
 * Return true if found otherwise false
 */
bool ctx_save_is_type_available(unsigned char type, unsigned int *size)
{
	if (IS_ERR_OR_NULL(size))
		return false;
	return __ctx_save_find_type(type, size, NULL);
}
EXPORT_SYMBOL(ctx_save_is_type_available);

/*
 * Function: ctx_save_replace_tlv
 * Description: Replace the specified TLV in global TLV buffer
 *
 * @param:       [in] type - Type associated with Dump segment
 *		[in] size - Size associted with Dump segment
 *		[in] data - Physical address of the Dump segment
 *
 * Return: 0 on success, -ENOBUFS on failure
 */
int ctx_save_replace_tlv(unsigned char type, unsigned int size,
		const void *data)
{
	unsigned char *x;
	unsigned char *y;
	unsigned long flags;
	unsigned int cur_size;
	struct ctx_save_tlv_msg *tlv_msg = &ctx_tlv_msg;
	unsigned char *offset = NULL;

	if (!__ctx_save_find_type(type, &cur_size, &offset))
		return -ENOENT;

	spin_lock_irqsave(&tlv_msg->spinlock, flags);
	x = offset;

	/* Make sure we don't overwrite next buffers */
	if (size > cur_size) {
		spin_unlock_irqrestore(&tlv_msg->spinlock, flags);
		return -EOVERFLOW;
	}

	y = tlv_msg->msg_buffer + tlv_msg->len;
	if ((x + QTI_WDT_SCM_TLV_TYPE_LEN_SIZE + size) >= y) {
		spin_unlock_irqrestore(&tlv_msg->spinlock, flags);
		return -ENOBUFS;
	}

	x[0] = type;
	x[1] = size;
	x[2] = size >> 8;

	memcpy(x + 3, data, size);
	spin_unlock_irqrestore(&tlv_msg->spinlock, flags);

	return 0;
}
EXPORT_SYMBOL(ctx_save_replace_tlv);

/*
 * Function: ctx_save_add_tlv
 * Description: Appends dump segment as a TLV entry to the end of the
 * global crashdump buffer.
 *
 * @param:       [in] type - Type associated with Dump segment
 *		[in] size - Size associated with Dump segment
 *		[in] data - Physical address of the Dump segment
 *
 * Return: 0 on success, -ENOBUFS on failure
 */
int ctx_save_add_tlv(unsigned char type, unsigned int size,
						const void *data)
{
	unsigned char *x;
	unsigned char *y;
	unsigned long flags;
	struct ctx_save_tlv_msg *tlv_msg = &ctx_tlv_msg;

	spin_lock_irqsave(&tlv_msg->spinlock, flags);
	x = tlv_msg->cur_pos;
	y = tlv_msg->msg_buffer + tlv_msg->len;

	if ((x + QTI_WDT_SCM_TLV_TYPE_LEN_SIZE + size) >= y) {
		spin_unlock_irqrestore(&tlv_msg->spinlock, flags);
		return -ENOBUFS;
	}

	x[0] = type;
	x[1] = size;
	x[2] = size >> 8;

	memcpy(x + 3, data, size);

	tlv_msg->cur_pos += (size + QTI_WDT_SCM_TLV_TYPE_LEN_SIZE);

	spin_unlock_irqrestore(&tlv_msg->spinlock, flags);
	return 0;
}
EXPORT_SYMBOL(ctx_save_add_tlv);

static int ctx_save_panic_handler(struct notifier_block *nb,
				  unsigned long event, void *ptr)
{
	ctx_tlv_msg.is_panic = true;
	return NOTIFY_DONE;
}

static struct notifier_block panic_nb = {
	.notifier_call = ctx_save_panic_handler,
};

static int ctx_save_probe(struct platform_device *pdev)
{
	void *scm_regsave;
	const struct ctx_save_props *prop = device_get_match_data(&pdev->dev);
	int ret;
	struct new_utsname *uname;

	if (!prop)
		return -ENODEV;

	scm_regsave = (void *) __get_free_pages(GFP_KERNEL,
				get_order(prop->crashdump_page_size));

	if (!scm_regsave)
		return -ENOMEM;

	ctx_tlv_msg.msg_buffer = scm_regsave + prop->tlv_msg_offset;
	ctx_tlv_msg.cur_pos = scm_regsave + prop->tlv_msg_offset;
	ctx_tlv_msg.len = prop->crashdump_page_size - prop->tlv_msg_offset;
	spin_lock_init(&ctx_tlv_msg.spinlock);
	uname = utsname();
	ctx_save_add_tlv(CTX_SAVE_TLV_UNAME, sizeof(*uname), uname);

	atomic_notifier_chain_register(&panic_notifier_list, &panic_nb);
	ret = qti_scm_regsave(SCM_SVC_UTIL, SCM_CMD_SET_REGSAVE,
			scm_regsave, prop->crashdump_page_size);
	return ret;
}

const struct ctx_save_props ctx_save_props_ipq807x = {
	.tlv_msg_offset = (500 * SZ_1K),
	/* As SBL overwrites the NSS IMEM, TZ has to copy it to some memory
	 * on crash before it restarts the system. Hence, reserving of 384K
	 * is required to copy the NSS IMEM before restart is done.
	 * So that TZ can dump NSS dump data after the first 8K.
	 * Additionally 8K memory is allocated which can be used by TZ
	 * to dump PMIC memory.
	 * get_order function returns the next higher order as output,
	 * so when we pass 400K as argument 512K will be allocated.
	 * 3K is required for DCC regsave memory.
	 * 15K is required for CPR.
	 * 82K is unused currently and can be used based on future needs.
	 * 12K is used for crashdump TLV buffer for Minidump feature.
	 *
	 * The memory is allocated using alloc_pages, hence it will be in
	 * power of 2. The unused memory is the result of using alloc_pages.
	 * As we need contigous memory for > 256K we have to use alloc_pages.
	 *
	 *		*---------------*
	 *		|      8K	|
	 *		|    regsave	|
	 *		*---------------*
	 *		|		|
	 *		|     384K	|
	 *		|    NSS IMEM	|
	 *		|		|
	 *		|		|
	 *		*---------------*
	 *		|      8K	|
	 *		|    PMIC mem	|
	 *		*---------------*
	 *		|    3K - DCC	|
	 *		|		|
	 *		*---------------*
	 *		|      15K      |
	 *		|    CPR Reg    |
	 *		* --------------*
	 *		|		|
	 *		|     82K	|
	 *		|    Unused	|
	 *		|		|
	 *		* --------------*
	 *		|     12 K      |
	 *		|   TLV Buffer  |
	 *		*---------------*
	 *
	 */
	.crashdump_page_size = (SZ_8K + (384 * SZ_1K) + (SZ_8K) + (3 * SZ_1K) +
				(15 * SZ_1K) + (82 * SZ_1K) + (12 * SZ_1K)),
};

static const struct of_device_id ctx_save_of_table[] = {
	{
		.compatible = "qti,ctxt-save-ipq8074",
		.data = (void *)&ctx_save_props_ipq807x,
	},
	{}
};

static struct platform_driver ctx_save_driver = {
	.probe = ctx_save_probe,
	.driver = {
		.name = "qti_ctx_save_driver",
		.of_match_table = ctx_save_of_table,
	},
};
module_platform_driver(ctx_save_driver);

MODULE_DESCRIPTION("QTI context save driver for storing cpu regs, etc");
MODULE_LICENSE("GPL v2");
