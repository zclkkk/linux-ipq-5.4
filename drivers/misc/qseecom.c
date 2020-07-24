/* QTI Secure Execution Environment Communicator (QSEECOM) driver
 *
 * Copyright (c) 2012, 2015, 2017-2018 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* Refer to Documentation/qseecom.txt for detailed usage instructions.
 */

#include "qseecom.h"

static int unload_app_libs(void)
{
	struct qseecom_unload_ireq req;
	struct qseecom_command_scm_resp resp;
	int ret = 0;
	uint32_t cmd_id = 0;
	uint32_t smc_id = 0;

	cmd_id = QSEE_UNLOAD_SERV_IMAGE_COMMAND;

	smc_id = QTI_SYSCALL_CREATE_SMC_ID(QTI_OWNER_QSEE_OS, QTI_SVC_APP_MGR,
					   QTI_CMD_UNLOAD_LIB);

	/* SCM_CALL to unload the app */
	ret = qti_scm_qseecom_unload(smc_id, cmd_id, &req, sizeof(uint32_t),
				     &resp, sizeof(resp));

	if (ret) {
		pr_err("scm_call to unload app libs failed, ret val = %d\n",
		      ret);
		return ret;
	}

	pr_info("App libs unloaded successfully\n");

	return 0;
}

static int tzdbg_register_qsee_log_buf(struct device *dev)
{
	uint64_t len = 0;
	int ret = 0;
	void *buf = NULL;
	struct qsee_reg_log_buf_req req;
	struct qseecom_command_scm_resp resp;
	dma_addr_t dma_log_buf = 0;

	len = QSEE_LOG_BUF_SIZE;
	buf = dma_alloc_coherent(dev, len, &dma_log_buf, GFP_KERNEL);
	if (buf == NULL) {
		pr_err("Failed to alloc memory for size %llu\n", len);
		return -ENOMEM;
	}
	g_qsee_log = (struct tzdbg_log_t *)buf;

	req.phy_addr = dma_log_buf;
	req.len = len;

	ret = qti_scm_tz_register_log_buf(dev, &req, sizeof(req),
					  &resp, sizeof(resp));

	if (ret) {
		pr_err("SCM Call failed..SCM Call return value = %d\n", ret);
		dma_free_coherent(dev, len, (void *)g_qsee_log, dma_log_buf);
		return ret;
	}

	if (resp.result) {
		ret = resp.result;
		pr_err("Response status failure..return value = %d\n", ret);
		dma_free_coherent(dev, len, (void *)g_qsee_log, dma_log_buf);
		return ret;
	}

	return 0;
}

static ssize_t
show_qsee_app_log_buf(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	ssize_t count = 0;

	if (app_state) {
		if (g_qsee_log->log_pos.wrap != 0) {
			memcpy(buf, g_qsee_log->log_buf +
			      g_qsee_log->log_pos.offset, QSEE_LOG_BUF_SIZE -
			      g_qsee_log->log_pos.offset - 64);
			count = QSEE_LOG_BUF_SIZE -
				g_qsee_log->log_pos.offset - 64;
			memcpy(buf + count, g_qsee_log->log_buf,
			      g_qsee_log->log_pos.offset);
			count = count + g_qsee_log->log_pos.offset;
		} else {
			memcpy(buf, g_qsee_log->log_buf,
			      g_qsee_log->log_pos.offset);
			count = g_qsee_log->log_pos.offset;
		}
	} else {
		pr_err("load app and then view log..\n");
		return -EINVAL;
	}

	return count;
}

struct qseecom_props {
	const int function;
	bool logging_support_enabled;
};

const struct qseecom_props qseecom_props_ipq807x = {
	.function = (MUL | CRYPTO),
	.logging_support_enabled = true,
};

const struct qseecom_props qseecom_props_ipq6018 = {
	.function = (MUL | CRYPTO),
	.logging_support_enabled = true,
};

static const struct of_device_id qseecom_of_table[] = {
	{	.compatible = "ipq807x-qseecom",
		.data = (void *) &qseecom_props_ipq807x,
	},
	{	.compatible = "ipq6018-qseecom",
		.data = (void *) &qseecom_props_ipq6018,
	},
	{}
};
MODULE_DEVICE_TABLE(of, qseecom_of_table);

static ssize_t mdt_write(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr,
	char *buf, loff_t pos, size_t count)
{
	uint8_t *tmp;
	/*
	 * Position '0' means new file being written,
	 * Hence allocate new memory after freeing already allocated mem if any
	 */
	if (pos == 0) {
		kfree(mdt_file);
		mdt_file = kzalloc((count) * sizeof(uint8_t), GFP_KERNEL);
	} else {
		tmp = mdt_file;
		mdt_file = krealloc(tmp,
			(pos + count) * sizeof(uint8_t), GFP_KERNEL);
	}

	if (!mdt_file)
		return -ENOMEM;

	memcpy((mdt_file + pos), buf, count);
	mdt_size = pos + count;
	return count;
}

static ssize_t seg_write(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr,
	char *buf, loff_t pos, size_t count)
{
	uint8_t *tmp;
	if (pos == 0) {
		kfree(seg_file);
		seg_file = kzalloc((count) * sizeof(uint8_t), GFP_KERNEL);
	} else {
		tmp = seg_file;
		seg_file = krealloc(tmp, (pos + count) * sizeof(uint8_t),
					GFP_KERNEL);
	}

	if (!seg_file)
		return -ENOMEM;

	memcpy((seg_file + pos), buf, count);
	seg_size = pos + count;
	return count;
}

static int qseecom_unload_app(void)
{
	struct qseecom_unload_ireq req;
	struct qseecom_command_scm_resp resp;
	int ret = 0;
	uint32_t cmd_id = 0;
	uint32_t smc_id = 0;

	cmd_id = QSEOS_APP_SHUTDOWN_COMMAND;

	smc_id = QTI_SYSCALL_CREATE_SMC_ID(QTI_OWNER_QSEE_OS, QTI_SVC_APP_MGR,
					 QTI_CMD_UNLOAD_APP_ID);

	req.app_id = qsee_app_id;

	/* SCM_CALL to unload the app */
	ret = qti_scm_qseecom_unload(smc_id, cmd_id, &req,
				     sizeof(struct qseecom_unload_ireq),
				     &resp, sizeof(resp));
	if (ret) {
		pr_err("scm_call to unload app (id = %d) failed\n", req.app_id);
		pr_info("scm call ret value = %d\n", ret);
		return ret;
	}

	pr_info("App id %d now unloaded\n", req.app_id);

	return 0;
}

static int tzapp_test(struct device *dev, void *input,
		      void *output, int input_len, int option)
{
	int ret = 0;
	int ret1, ret2;
	struct qsee_64_send_cmd *msgreq;
	union qseecom_client_send_data_ireq send_data_req;
	struct qseecom_command_scm_resp resp;
	struct qsee_send_cmd_rsp *msgrsp; /* response data sent from QSEE */
	struct page *pg_tmp;
	unsigned long pg_addr;

	dev = qdev;

	/*
	 * Using alloc_pages to avoid colliding with input pointer's
	 * allocated page, since qsee_register_shared_buffer() in sampleapp
	 * checks if input ptr is in secure area. Page where msgreq/msgrsp
	 * is allocated is added to blacklisted area by sampleapp and added
	 * as secure memory region, hence input data (shared buffer)
	 * cannot be in that secure memory region
	 */
	pg_tmp = alloc_page(GFP_KERNEL);
	if (!pg_tmp) {
		pr_err("Failed to allocate page\n");
		return -ENOMEM;
	}
	/*
	 * Getting virtual page address. pg_tmp will be pointing to
	 * first page structure
	 */
	msgreq = (struct qsee_64_send_cmd *) page_address(pg_tmp);

	if (!msgreq) {
		pr_err("Unable to allocate memory\n");
		return -ENOMEM;
	}
	/* pg_addr for passing to free_page */
	pg_addr = (unsigned long) msgreq;

	msgrsp = (struct qsee_send_cmd_rsp *)((uint8_t *) msgreq +
				sizeof(struct qsee_64_send_cmd));
	if (!msgrsp) {
		kfree(msgreq);
		pr_err("Unable to allocate memory\n");
		return -ENOMEM;
	}

	/*
	 * option = 1 -> Basic Multiplication
	 * option = 4 -> Crypto Function
	 */

	switch (option) {
	case 1:
		msgreq->cmd_id = CLIENT_CMD1_BASIC_DATA;
		msgreq->data = *((dma_addr_t *)input);
		break;
	case 4:
		msgreq->cmd_id = CLIENT_CMD8_RUN_CRYPTO_TEST;
		break;
	default:
		pr_err("Invalid Option\n");
		goto fn_exit;
	}
	if (option == 2 || option == 3) {
		msgreq->data = dma_map_single(dev, input,
				input_len, DMA_TO_DEVICE);
		msgreq->data2 = dma_map_single(dev, output,
				input_len, DMA_FROM_DEVICE);

		ret1 = dma_mapping_error(dev, msgreq->data);
		ret2 = dma_mapping_error(dev, msgreq->data2);

		if (ret1 || ret2) {
			pr_err("DMA Mapping Error:input:%d output:%d\n",
			      ret1, ret2);
			if (!ret1) {
				dma_unmap_single(dev, msgreq->data,
					input_len, DMA_TO_DEVICE);
			}

			if (!ret2) {
				dma_unmap_single(dev, msgreq->data2,
					input_len, DMA_FROM_DEVICE);
			}
			return ret1 ? ret1 : ret2;
		}
		msgreq->test_buf_size = input_len;
		msgreq->len = input_len;
	}
	send_data_req.v1.app_id = qsee_app_id;

	send_data_req.v1.req_ptr = dma_map_single(dev, msgreq,
				sizeof(*msgreq), DMA_TO_DEVICE);
	send_data_req.v1.rsp_ptr = dma_map_single(dev, msgrsp,
				sizeof(*msgrsp), DMA_FROM_DEVICE);

	ret1 = dma_mapping_error(dev, send_data_req.v1.req_ptr);
	ret2 = dma_mapping_error(dev, send_data_req.v1.rsp_ptr);


	if (!ret1 && !ret2) {
		send_data_req.v1.req_len =
				sizeof(struct qsee_64_send_cmd);
		send_data_req.v1.rsp_len =
				sizeof(struct qsee_send_cmd_rsp);
		ret = qti_scm_qseecom_send_data(&send_data_req,
						sizeof(send_data_req.v2)
						, &resp, sizeof(resp));
	}

	if (option == 2 || option == 3) {
		dma_unmap_single(dev, msgreq->data,
					input_len, DMA_TO_DEVICE);
		dma_unmap_single(dev, msgreq->data2,
					input_len, DMA_FROM_DEVICE);

	}

	if (!ret1) {
		dma_unmap_single(dev, send_data_req.v1.req_ptr,
			sizeof(*msgreq), DMA_TO_DEVICE);
	}

	if (!ret2) {
		dma_unmap_single(dev, send_data_req.v1.rsp_ptr,
			sizeof(*msgrsp), DMA_FROM_DEVICE);
	}

	if (ret1 || ret2) {
		pr_err("DMA Mapping Error:req_ptr:%d rsp_ptr:%d\n",
		      ret1, ret2);
		return ret1 ? ret1 : ret2;
	}

	if (ret) {
		pr_err("qseecom_scm_call failed with err: %d\n", ret);
		goto fn_exit;
	}

	if (resp.result == QSEOS_RESULT_INCOMPLETE) {
		pr_err("Result incomplete\n");
		ret = -EINVAL;
		goto fn_exit;
	} else {
		if (resp.result != QSEOS_RESULT_SUCCESS) {
			pr_err("Response result %lu not supported\n",
							resp.result);
			ret = -EINVAL;
			goto fn_exit;
		} else {
			if (option == 4) {
				if (!msgrsp->status) {
					pr_info("Crypto operation success\n");
				} else {
					pr_info("Crypto operation failed\n");
					goto fn_exit;
				}
			}
		}
	}

	if (option == 1) {
		if (msgrsp->status) {
			pr_err("Input size exceeded supported range\n");
			ret = -EINVAL;
		}
		basic_output = msgrsp->data;
	}

fn_exit:
	free_page(pg_addr);
	return ret;
}

static int32_t copy_files(int *img_size)
{
	uint8_t *buf;

	if (mdt_file && seg_file) {
		*img_size = mdt_size + seg_size;

		qsee_sbuffer = kzalloc(*img_size, GFP_KERNEL);
		if (!qsee_sbuffer) {
			pr_err("Error: qsee_sbuffer alloc failed\n");
			return -ENOMEM;
		}
		buf = qsee_sbuffer;

		memcpy(buf, mdt_file, mdt_size);
		buf += mdt_size;
		memcpy(buf, seg_file, seg_size);
		buf += seg_size;
	} else {
		pr_err("Sampleapp file Inputs not provided\n");
		return -EINVAL;
	}
	return 0;
}

static int load_request(struct device *dev, uint32_t smc_id,
		       uint32_t cmd_id, size_t req_size)
{
	union qseecom_load_ireq load_req;
	struct qseecom_command_scm_resp resp;
	int ret, ret1;
	int img_size;

	kfree(qsee_sbuffer);
	ret = copy_files(&img_size);
	if (ret) {
		pr_err("Copying Files failed\n");
		return ret;
	}

	dev = qdev;

	load_req.load_lib_req.mdt_len = mdt_size;
	load_req.load_lib_req.img_len = img_size;
	load_req.load_lib_req.phy_addr = dma_map_single(dev, qsee_sbuffer,
						       img_size, DMA_TO_DEVICE);
	ret1 = dma_mapping_error(dev, load_req.load_lib_req.phy_addr);
	if (ret1 == 0) {
		ret = qti_scm_qseecom_load(smc_id, cmd_id, &load_req,
					   req_size, &resp, sizeof(resp));
		dma_unmap_single(dev, load_req.load_lib_req.phy_addr,
				img_size, DMA_TO_DEVICE);
	}
	if (ret1) {
		pr_err("DMA Mapping error (qsee_sbuffer)\n");
		return ret1;
	}
	if (ret) {
		pr_err("SCM_CALL to load app and services failed\n");
		return ret;
	}

	if (resp.result == QSEOS_RESULT_FAILURE) {
		pr_err("SCM_CALL rsp.result is QSEOS_RESULT_FAILURE\n");
		return -EFAULT;
	}

	if (resp.result == QSEOS_RESULT_INCOMPLETE)
		pr_err("Process_incomplete_cmd ocurred\n");

	if (resp.result != QSEOS_RESULT_SUCCESS) {
		pr_err("scm_call failed resp.result unknown, %lu\n",
				resp.result);
		return -EFAULT;
	}

	pr_info("Successfully loaded app and services!!!!!\n");

	qsee_app_id = resp.data;
	return 0;
}

/* To show basic multiplication output */
static ssize_t
show_basic_output(struct device *dev, struct device_attribute *attr,
					char *buf)
{
	return snprintf(buf, (basic_data_len + 1), "%lu\n", basic_output);
}

/* Basic multiplication App*/
static ssize_t
store_basic_input(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t count)
{
	dma_addr_t __aligned(sizeof(dma_addr_t) * 8) basic_input = 0;
	uint32_t ret = 0;
	basic_data_len = count;
	if ((count - 1) == 0) {
		pr_err("Input cannot be NULL!\n");
		return -EINVAL;
	}
	if (kstrtouint(buf, 10, (unsigned int *)&basic_input) || basic_input > (U32_MAX / 10))
		pr_err("Please enter a valid unsigned integer less than %u\n",
			(U32_MAX / 10));
	else
		ret = tzapp_test(dev, &basic_input, NULL, 0, 1);

	return ret ? ret : count;
}

static ssize_t
store_load_start(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int load_cmd;
	uint32_t smc_id = 0;
	uint32_t cmd_id = 0;
	size_t req_size = 0;

	dev = qdev;

	if (kstrtouint(buf, 10, &load_cmd)) {
		pr_err("Provide valid integer input!\n");
		pr_err("Echo 0 to load app libs\n");
		pr_err("Echo 1 to load app\n");
		pr_err("Echo 2 to unload app\n");
		return -EINVAL;
	}
	if (load_cmd == 0) {
		if (!app_libs_state) {
			smc_id = QTI_SYSCALL_CREATE_SMC_ID(QTI_OWNER_QSEE_OS,
				 QTI_SVC_APP_MGR, QTI_CMD_LOAD_LIB);
			cmd_id = QSEE_LOAD_SERV_IMAGE_COMMAND;
			req_size = sizeof(struct qseecom_load_lib_ireq);
			if (load_request(dev, smc_id, cmd_id, req_size))
				pr_info("Loading app libs failed\n");
			else
				app_libs_state = 1;
			if (props->logging_support_enabled) {
				if (tzdbg_register_qsee_log_buf(dev))
					pr_info("Registering log buf failed\n");
			}
		} else {
			pr_info("Libraries are either already loaded or are inbuilt in this platform\n");
		}
	} else if (load_cmd == 1) {
		if (app_libs_state) {
			if (!app_state) {
				smc_id = QTI_SYSCALL_CREATE_SMC_ID(
					 QTI_OWNER_QSEE_OS, QTI_SVC_APP_MGR,
					 QTI_CMD_LOAD_APP_ID);
				cmd_id = QSEOS_APP_START_COMMAND;
				req_size = sizeof(struct qseecom_load_app_ireq);
				if (load_request(dev, smc_id, cmd_id, req_size))
					pr_info("Loading app failed\n");
				else
					app_state = 1;
			} else {
				pr_info("App already loaded...\n");
			}
		} else {
			if (!app_libs_state)
				pr_info("App libs must be loaded first\n");
		}
	} else if (load_cmd == 2) {
		if (app_state) {
			if (qseecom_unload_app())
				pr_info("App unload failed\n");
			else
				app_state = 0;
		} else {
			pr_info("App already unloaded...\n");
		}
	} else {
		pr_info("Echo 0 to load app libs\n");
		pr_info("Echo 1 to load app\n");
		pr_info("Echo 2 to unload app\n");
	}

	return count;
}

static ssize_t
store_crypto_input(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	tzapp_test(dev, NULL, NULL, 0, 4);
	return count;
}

static int __init tzapp_init(void)
{
	int err;
	int i = 0;
	struct attribute **tzapp_attrs = kzalloc((hweight_long(props->function)
				+ 1) * sizeof(*tzapp_attrs), GFP_KERNEL);

	if (!tzapp_attrs) {
		pr_err("Cannot allocate memory..tzapp\n");
		return -ENOMEM;
	}

	tzapp_attrs[i++] = &dev_attr_load_start.attr;

	if (props->function & MUL)
		tzapp_attrs[i++] = &dev_attr_basic_data.attr;

	if (props->function & CRYPTO)
		tzapp_attrs[i++] = &dev_attr_crypto.attr;

	if (props->logging_support_enabled)
		tzapp_attrs[i++] = &dev_attr_log_buf.attr;

	tzapp_attrs[i] = NULL;

	tzapp_attr_grp.attrs = tzapp_attrs;

	tzapp_kobj = kobject_create_and_add("tzapp", firmware_kobj);

	err = sysfs_create_group(tzapp_kobj, &tzapp_attr_grp);

	if (err) {
		kobject_put(tzapp_kobj);
		return err;
	}
	return 0;
}

static int __init qseecom_probe(struct platform_device *pdev)
{
	struct device_node *of_node = pdev->dev.of_node;
	const struct of_device_id *id;
	unsigned int start = 0, size = 0;
	struct qsee_notify_app notify_app;
	struct qseecom_command_scm_resp resp;
	int ret = 0, ret1 = 0;

	if (!of_node)
		return -ENODEV;

	qdev = &pdev->dev;
	id = of_match_device(qseecom_of_table, &pdev->dev);

	if (!id)
		return -ENODEV;

	ret = of_property_read_u32(of_node, "mem-start", &start);
	ret1 = of_property_read_u32(of_node, "mem-size", &size);
	if (ret || ret1) {
		pr_err("No mem-region specified, using default\n");
		goto load;
	}
	/* 4KB adjustment is to ensure TZApp does not overlap
	 * the region alloted for SMMU WAR
	 */
	if (of_property_read_bool(of_node, "notify-align")) {
		start += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	notify_app.applications_region_addr = start;
	notify_app.applications_region_size = size;

	ret = qti_scm_qseecom_notify(&notify_app,
				     sizeof(struct qsee_notify_app),
				     &resp, sizeof(resp));
	if (ret) {
		pr_err("Notify App failed\n");
		return -1;
	}

load:
	props = ((struct qseecom_props *)id->data);

	sysfs_create_bin_file(firmware_kobj, &mdt_attr);
	sysfs_create_bin_file(firmware_kobj, &seg_attr);

	if (!tzapp_init())
		pr_info("Loaded tz app Module Successfully!\n");
	else
		pr_info("Failed to load tz app module\n");

	return 0;
}

static int __exit qseecom_remove(struct platform_device *pdev)
{
	if (app_state) {
		if (qseecom_unload_app())
			pr_err("App unload failed\n");
		else
			app_state = 0;
	}

	sysfs_remove_bin_file(firmware_kobj, &mdt_attr);
	sysfs_remove_bin_file(firmware_kobj, &seg_attr);

	sysfs_remove_group(tzapp_kobj, &tzapp_attr_grp);
	kobject_put(tzapp_kobj);

	kfree(mdt_file);
	kfree(seg_file);

	kfree(qsee_sbuffer);

	if (app_libs_state) {
		if (unload_app_libs())
			pr_err("App libs unload failed\n");
		else
			app_libs_state = 0;
	}

	return 0;
}

static struct platform_driver qseecom_driver = {
	.remove = qseecom_remove,
	.driver = {
		.name = KBUILD_MODNAME,
		.of_match_table = qseecom_of_table,
	},
};
module_platform_driver_probe(qseecom_driver, qseecom_probe);

MODULE_DESCRIPTION("QSEECOM Driver");
MODULE_LICENSE("GPL v2");
