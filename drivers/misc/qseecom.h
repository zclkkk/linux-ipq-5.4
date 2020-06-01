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

#ifndef _qseecom_h
#define _qseecom_h

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/highuid.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kobject.h>
#include <linux/qcom_scm.h>
#include <linux/sysfs.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>

#define CLIENT_CMD1_BASIC_DATA		1
#define CLIENT_CMD8_RUN_CRYPTO_TEST	3
#define MAX_INPUT_SIZE			4096

#define QSEE_LOG_BUF_SIZE		0x1000

static int app_state;
static int app_libs_state;
struct qseecom_props *props;

struct qsee_64_send_cmd {
	uint32_t cmd_id;
	uint64_t data;
	uint64_t data2;
	uint32_t len;
	uint32_t start_pkt;
	uint32_t end_pkt;
	uint32_t test_buf_size;
};

struct qsee_send_cmd_rsp {
	uint32_t data;
	int32_t status;
};

enum qseecom_qceos_cmd_status {
	QSEOS_RESULT_SUCCESS	= 0,
	QSEOS_RESULT_INCOMPLETE,
	QSEOS_RESULT_FAILURE	= 0xFFFFFFFF
};

static uint32_t qsee_app_id;
static void *qsee_sbuffer;
static unsigned long basic_output;
static int basic_data_len;
static int mdt_size;
static int seg_size;
static uint8_t *mdt_file;
static uint8_t *seg_file;

struct kobject *tzapp_kobj;
struct attribute_group tzapp_attr_grp;

static struct tzdbg_log_t *g_qsee_log;

static struct device *qdev;

#define MUL		0x1
#define CRYPTO		0x8

static ssize_t show_qsee_app_log_buf(struct device *dev,
				    struct device_attribute *attr, char *buf);

static ssize_t mdt_write(struct file *filp, struct kobject *kobj,
			struct bin_attribute *bin_attr,
			char *buf, loff_t pos, size_t count);

static ssize_t seg_write(struct file *filp, struct kobject *kobj,
			struct bin_attribute *bin_attr,
			char *buf, loff_t pos, size_t count);

static ssize_t store_load_start(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count);

static ssize_t show_basic_output(struct device *dev,
				struct device_attribute *attr, char *buf);

static ssize_t store_basic_input(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);

static ssize_t store_crypto_input(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count);

static DEVICE_ATTR(log_buf, 0644, show_qsee_app_log_buf, NULL);
static DEVICE_ATTR(load_start, S_IWUSR, NULL, store_load_start);
static DEVICE_ATTR(basic_data, 0644, show_basic_output, store_basic_input);
static DEVICE_ATTR(crypto, 0644, NULL, store_crypto_input);

struct bin_attribute mdt_attr = {
	.attr = {.name = "mdt_file", .mode = 0666},
	.write = mdt_write,
};

struct bin_attribute seg_attr = {
	.attr = {.name = "seg_file", .mode = 0666},
	.write = seg_write,
};

#endif
