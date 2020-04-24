/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2010-2015, 2018, The Linux Foundation. All rights reserved.
 * Copyright (C) 2015 Linaro Ltd.
 */
#ifndef __QCOM_SCM_H
#define __QCOM_SCM_H

#include <linux/err.h>
#include <linux/types.h>
#include <linux/cpumask.h>

#define QCOM_SCM_VERSION(major, minor)	(((major) << 16) | ((minor) & 0xFF))
#define QCOM_SCM_CPU_PWR_DOWN_L2_ON	0x0
#define QCOM_SCM_CPU_PWR_DOWN_L2_OFF	0x1
#define QCOM_SCM_HDCP_MAX_REQ_CNT	5

struct qcom_scm_hdcp_req {
	u32 addr;
	u32 val;
};

struct qcom_scm_vmperm {
	int vmid;
	int perm;
};

#define SCM_SVC_UTIL		0x03
#define SCM_CMD_SET_REGSAVE	0x02

#define QCOM_SCM_VMID_HLOS       0x3
#define QCOM_SCM_VMID_MSS_MSA    0xF
#define QCOM_SCM_VMID_WLAN       0x18
#define QCOM_SCM_VMID_WLAN_CE    0x19
#define QCOM_SCM_PERM_READ       0x4
#define QCOM_SCM_PERM_WRITE      0x2
#define QCOM_SCM_PERM_EXEC       0x1
#define QCOM_SCM_PERM_RW (QCOM_SCM_PERM_READ | QCOM_SCM_PERM_WRITE)
#define QCOM_SCM_PERM_RWX (QCOM_SCM_PERM_RW | QCOM_SCM_PERM_EXEC)

#define QTI_SCM_SVC_FUSE		0x8
#define QTI_KERNEL_AUTH_CMD		0x15
#define TZ_BLOW_FUSE_SECDAT             0x20
#define FUSEPROV_SUCCESS                0x0
#define FUSEPROV_INVALID_HASH           0x09
#define FUSEPROV_SECDAT_LOCK_BLOWN      0xB

#if IS_ENABLED(CONFIG_QCOM_SCM)
extern int qcom_scm_set_cold_boot_addr(void *entry, const cpumask_t *cpus);
extern int qcom_scm_set_warm_boot_addr(void *entry, const cpumask_t *cpus);
extern bool qcom_scm_is_available(void);
extern bool qcom_scm_hdcp_available(void);
extern int qcom_scm_hdcp_req(struct qcom_scm_hdcp_req *req, u32 req_cnt,
			     u32 *resp);
extern bool qcom_scm_pas_supported(u32 peripheral);
extern int qcom_scm_pas_init_image(u32 peripheral, const void *metadata,
				   size_t size);
extern int qcom_scm_pas_mem_setup(u32 peripheral, phys_addr_t addr,
				  phys_addr_t size);
extern int qcom_scm_pas_auth_and_reset(u32 peripheral, u32 debug,
					u32 reset_cmd_id);
extern int qcom_scm_pas_shutdown(u32 peripheral);
extern int qcom_scm_assign_mem(phys_addr_t mem_addr, size_t mem_sz,
			       unsigned int *src,
			       const struct qcom_scm_vmperm *newvm,
			       unsigned int dest_cnt);
extern void qcom_scm_cpu_power_down(u32 flags);
extern u32 qcom_scm_get_version(void);
extern int qcom_scm_set_remote_state(u32 state, u32 id);
extern int qcom_scm_restore_sec_cfg(u32 device_id, u32 spare);
extern int qcom_scm_iommu_secure_ptbl_size(u32 spare, size_t *size);
extern int qcom_scm_iommu_secure_ptbl_init(u64 addr, u32 size, u32 spare);
extern int qcom_scm_io_readl(phys_addr_t addr, unsigned int *val);
extern int qcom_scm_io_writel(phys_addr_t addr, unsigned int val);
extern int qti_qfprom_show_authenticate(void);
extern int qti_qfprom_write_version(void *wrip, int size);
extern int qti_qfprom_read_version(uint32_t sw_type,
					uint32_t value,
					uint32_t qfprom_ret_ptr);
extern int qti_sec_upgrade_auth(unsigned int scm_cmd_id, unsigned int sw_type,
					unsigned int img_size,
					unsigned int load_addr);
extern bool qti_scm_sec_auth_available(unsigned int scm_cmd_id);
extern int qti_fuseipq_scm_call(struct device *dev, u32 svc_id, u32 cmd_id,
					void *cmd_buf, size_t size);
extern int qti_scm_dload(u32 svc_id, u32 cmd_id, void *cmd_buf);
extern int qti_scm_sdi(u32 svc_id, u32 cmd_id);
extern int qti_scm_tz_log(void *ker_buf, u32 buf_len);
extern int qti_scm_hvc_log(void *ker_buf, u32 buf_len);
extern int qti_scm_get_smmustate(void);
extern int qti_scm_regsave(u32 svc_id, u32 cmd_id,
				void *scm_regsave, u32 buf_size);
extern bool is_scm_armv8(void);
#else

#include <linux/errno.h>

static inline
int qcom_scm_set_cold_boot_addr(void *entry, const cpumask_t *cpus)
{
	return -ENODEV;
}
static inline
int qcom_scm_set_warm_boot_addr(void *entry, const cpumask_t *cpus)
{
	return -ENODEV;
}
static inline bool qcom_scm_is_available(void) { return false; }
static inline bool qcom_scm_hdcp_available(void) { return false; }
static inline int qcom_scm_hdcp_req(struct qcom_scm_hdcp_req *req, u32 req_cnt,
				    u32 *resp) { return -ENODEV; }
static inline bool qcom_scm_pas_supported(u32 peripheral) { return false; }
static inline int qcom_scm_pas_init_image(u32 peripheral, const void *metadata,
					  size_t size) { return -ENODEV; }
static inline int qcom_scm_pas_mem_setup(u32 peripheral, phys_addr_t addr,
					 phys_addr_t size) { return -ENODEV; }
static inline int
qcom_scm_pas_auth_and_reset(u32 peripheral) { return -ENODEV; }
static inline int qcom_scm_pas_shutdown(u32 peripheral) { return -ENODEV; }
static inline int qcom_scm_assign_mem(phys_addr_t mem_addr, size_t mem_sz,
				      unsigned int *src,
				      const struct qcom_scm_vmperm *newvm,
				      unsigned int dest_cnt) { return -ENODEV; }
static inline void qcom_scm_cpu_power_down(u32 flags) {}
static inline u32 qcom_scm_get_version(void) { return 0; }
static inline u32
qcom_scm_set_remote_state(u32 state,u32 id) { return -ENODEV; }
static inline int qcom_scm_restore_sec_cfg(u32 device_id, u32 spare) { return -ENODEV; }
static inline int qcom_scm_iommu_secure_ptbl_size(u32 spare, size_t *size) { return -ENODEV; }
static inline int qcom_scm_iommu_secure_ptbl_init(u64 addr, u32 size, u32 spare) { return -ENODEV; }
static inline int qcom_scm_io_readl(phys_addr_t addr, unsigned int *val) { return -ENODEV; }
static inline int qcom_scm_io_writel(phys_addr_t addr, unsigned int val) { return -ENODEV; }
static inline int qti_qfprom_show_authenticate(void) { return -ENODEV; }
static inline int qti_qfprom_write_version(void *wrip, int size) { return -ENODEV; }
static inline int qti_qfprom_read_version(uint32_t sw_type,
						uint32_t value,
						uint32_t qfprom_ret_ptr)
{
	return -ENODEV;
}
static inline int qti_sec_upgrade_auth(unsigned int scm_cmd_id, unsigned int sw_type,
					unsigned int img_size,
					unsigned int load_addr)
{
	return -ENODEV;
}
static inline bool qti_scm_sec_auth_available(unsigned int scm_cmd_id)
{
	return -ENODEV;
}
static inline int qti_fuseipq_scm_call(struct device *dev, u32 svc_id, u32 cmd_id,
						void *cmd_buf, size_t size)
{
	return -ENODEV;
}
static inline int qti_scm_dload(u32 svc_id, u32 cmd_id, void *cmd_buf) { return -ENODEV; }
static inline int qti_scm_sdi(u32 svc_id, u32 cmd_id) { return -ENODEV; }
static inline int qti_scm_tz_log(void *ker_buf, u32 buf_len) { return -ENODEV; }
static inline int qti_scm_hvc_log(void *ker_buf, u32 buf_len) { return -ENODEV; }
static inline int qti_scm_get_smmustate(void) { return -ENODEV; }
static inline int qti_scm_regsave(u32 svc_id, u32 cmd_id, void *scm_regsave,
								u32 buf_size)
{
	return -ENODEV;
}
static inline bool is_scm_armv8(void) { return false; }
#endif
#endif
