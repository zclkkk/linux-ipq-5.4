/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "license_manager.h"

struct qmi_handle *lm_clnt_hdl;

static struct lm_svc_ctx *lm_svc;

static void qmi_handle_license_termination_mode_req(struct qmi_handle *handle,
			struct sockaddr_qrtr *sq,
			struct qmi_txn *txn,
			const void *decoded_msg)
{
	struct lm_get_termination_mode_req_msg_v01 *req;
	struct lm_get_termination_mode_resp_msg_v01 *resp;
	struct client_info *client;
	int ret;

	req = (struct lm_get_termination_mode_req_msg_v01 *)decoded_msg;

	resp = kzalloc(sizeof(*resp), GFP_KERNEL);
	if (!resp) {
		pr_err("%s: Memory allocation failed for resp buffer\n",
							__func__);
		return;
	}

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (!client) {
		pr_err("%s: Memory allocation failed for client ctx\n",
							__func__);
		goto free_resp_mem;
	}

	pr_debug("License termination: Request rcvd: node_id: 0x%x, timestamp: "
			"0x%llx,chip_id: 0x%x,chip_name :%s,serialno:0x%x\n",
			sq->sq_node, req->timestamp, req->id,
			req->name, req->serialnum);

	client->sq_node = sq->sq_node;
	client->sq_port = sq->sq_port;
	list_add_tail(&client->node, &lm_svc->clients_connected);

	resp->termination_mode =  lm_svc->termination_mode;
	resp->reserved = 77;

	ret = qmi_send_response(handle, sq, txn,
			QMI_LM_GET_TERMINATION_MODE_RESP_V01,
			LM_GET_TERMINATION_MODE_RESP_MSG_V01_MAX_MSG_LEN,
			lm_get_termination_mode_resp_msg_v01_ei, resp);
	if (ret < 0)
		pr_err("%s: Sending license termination response failed"
					"with error_code:%d\n", __func__, ret);
	pr_debug("License termination: Response sent, license termination mode "
			"0x%x\n", resp->termination_mode);

free_resp_mem:
	kfree(resp);
}

static void lm_qmi_svc_disconnect_cb(struct qmi_handle *qmi,
	unsigned int node, unsigned int port)
{
	struct client_info *itr, *tmp;

	if (!list_empty(&lm_svc->clients_connected)) {
		list_for_each_entry_safe(itr, tmp, &lm_svc->clients_connected,
								node) {
			if (itr->sq_node == node && itr->sq_port == port) {
				pr_debug("Received LM QMI client disconnect "
					"from node:0x%x port:%d\n",
					node, port);
				list_del(&itr->node);
				kfree(itr);
			}
		}
	}
}

static struct qmi_ops lm_server_ops = {
	.del_client = lm_qmi_svc_disconnect_cb,
};
static struct qmi_msg_handler lm_req_handlers[] = {
	{
		.type = QMI_REQUEST,
		.msg_id = QMI_LM_GET_TERMINATION_MODE_REQ_V01,
		.ei = lm_get_termination_mode_req_msg_v01_ei,
		.decoded_size = LM_GET_TERMINATION_MODE_REQ_MSG_V01_MAX_MSG_LEN,
		.fn = qmi_handle_license_termination_mode_req,
	},
	{}
};

static int license_manager_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	bool host_license_termination = false;
	bool device_license_termination = false;
	int ret = 0;
	struct lm_svc_ctx *svc;

	svc = kzalloc(sizeof(struct lm_svc_ctx), GFP_KERNEL);
	if (!svc)
		return -ENOMEM;

	device_license_termination = of_property_read_bool(node,
						"device-license-termination");
	pr_debug("device-license-termination : %s \n",
				device_license_termination ? "true" : "false");

	if (device_license_termination)
		goto skip_device_mode;

	host_license_termination = of_property_read_bool(node,
						"host-license-termination");
	pr_debug("host-license-termination : %s \n",
				host_license_termination ? "true" : "false");

	if (!host_license_termination) {
		pr_debug("License Termination mode not given in DT,"
			"Assuming Device license termination by default \n");
		device_license_termination = true;
	}

skip_device_mode:
	svc->termination_mode =  host_license_termination ?
			QMI_LICENSE_TERMINATION_AT_HOST_V01 :
				QMI_LICENSE_TERMINATION_AT_DEVICE_V01;


	svc->lm_svc_hdl = kzalloc(sizeof(struct qmi_handle), GFP_KERNEL);
	if (!svc->lm_svc_hdl) {
		ret = -ENOMEM;
		goto free_lm_svc;
	}
	ret = qmi_handle_init(svc->lm_svc_hdl,
				QMI_LICENSE_MANAGER_SERVICE_MAX_MSG_LEN,
				&lm_server_ops,
				lm_req_handlers);
	if (ret < 0) {
		pr_err("%s:Error registering license manager svc %d\n",
							__func__, ret);
		goto free_lm_svc_handle;
	}
	ret = qmi_add_server(svc->lm_svc_hdl, LM_SERVICE_ID_V01,
					LM_SERVICE_VERS_V01,
					0);
	if (ret < 0) {
		pr_err("%s: failed to add license manager svc server :%d\n",
							__func__, ret);
		goto release_lm_svc_handle;
	}

	INIT_LIST_HEAD(&svc->clients_connected);

	lm_svc = svc;

	pr_info("License Manager driver registered\n");

	return 0;

release_lm_svc_handle:
	qmi_handle_release(svc->lm_svc_hdl);
free_lm_svc_handle:
	kfree(svc->lm_svc_hdl);
free_lm_svc:
	kfree(svc);

	return ret;
}

static int license_manager_remove(struct platform_device *pdev)
{
	struct lm_svc_ctx *svc = lm_svc;
	struct client_info *itr, *tmp;

	qmi_handle_release(svc->lm_svc_hdl);

	if (!list_empty(&svc->clients_connected)) {
		list_for_each_entry_safe(itr, tmp, &svc->clients_connected,
								node) {
			list_del(&itr->node);
			kfree(itr);
		}
	}
	kfree(svc->lm_svc_hdl);
	kfree(svc);

	lm_svc = NULL;

	return 0;
}

static const struct of_device_id of_license_manager_match[] = {
	{.compatible = "qti,license-manager-service"},
	{  /* sentinel value */ },
};

static struct platform_driver license_manager_driver = {
	.probe		= license_manager_probe,
	.remove		= license_manager_remove,
	.driver		= {
		.name	= "license_manager",
		.of_match_table	= of_license_manager_match,
	},
};

static int __init license_manager_init(void)
{
	return platform_driver_register(&license_manager_driver);
}
module_init(license_manager_init);

static void __exit license_manager_exit(void)
{
	platform_driver_unregister(&license_manager_driver);
}
module_exit(license_manager_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("License manager driver");
