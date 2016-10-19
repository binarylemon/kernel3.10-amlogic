/*
 * Copyright (c) 2010-2013 Sierraware, LLC.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions and derivatives of the Software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification is strictly prohibited without prior written consent from
 * Sierraware, LLC.
 *
 * Redistribution in binary form must reproduce the above copyright  notice, 
 * this list of conditions and  the following disclaimer in the documentation 
 * and/or other materials  provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */
/* 
 * Trustzone client driver defintions.
 */

#ifndef __OTZ_COMMON_H_
#define __OTZ_COMMON_H_


#define OTZ_MAX_REQ_PARAMS  12
#define OTZ_MAX_RES_PARAMS  4
#define OTZ_1K_SIZE 1024

#define TEEC_CONFIG_SHAREDMEM_MAX_SIZE 0x200000
#define TEEC_CONFIG_TAMEM_MAX_SIZE 0x400000


/**
 * @brief SMC return values
 */
/*enum otz_smc_ret {
  SMC_ENOMEM = -5,
  SMC_EOPNOTSUPP = -4,
  SMC_EINVAL_ADDR = -3,
  SMC_EINVAL_ARG = -2,
  SMC_ERROR = -1,
  SMC_INTERRUPTED = 1,
  SMC_PENDING = 2,
  SMC_SUCCESS = 0
  };
  */


/**
 * @brief Command status 
 */
enum otz_cmd_status {
	OTZ_STATUS_INCOMPLETE = 0,
	OTZ_STATUS_COMPLETE,
	OTZ_STATUS_MAX  = 0x7FFFFFFF
};

/**
 * @brief Command type
 */
enum otz_cmd_type {
	OTZ_CMD_TYPE_INVALID = 0,
	OTZ_CMD_TYPE_NS_TO_SECURE,
	OTZ_CMD_TYPE_SECURE_TO_NS,
	OTZ_CMD_TYPE_SECURE_TO_SECURE,
	OTZ_CMD_TYPE_MAX  = 0x7FFFFFFF
};

/**
 * @brief Parameters type
 */
enum otzc_param_type {
	OTZC_PARAM_IN = 0,
	OTZC_PARAM_OUT
};

/**
 * @brief Shared memory for Notification
 */
struct otzc_notify_data {
	int guest_no;
	int dev_file_id;
	int service_id;
	int client_pid;
	int session_id;
	int enc_id;
};

/**
 * @brief Metadata used for encoding/decoding
 */
struct otzc_encode_meta {
	int type;
	int len;
	unsigned int usr_addr;
	int ret_len;
};

/**
 * @brief SMC command structure
 */
struct otz_smc_cmd {
	unsigned int    id;
	unsigned int    context;
	unsigned int    enc_id;

	unsigned int    src_id;
	unsigned int    src_context;

	unsigned int    req_buf_len;
	unsigned int    resp_buf_len;
	unsigned int    ret_resp_buf_len;
	unsigned int    cmd_status;
	unsigned int    req_buf_phys;
	unsigned int    resp_buf_phys;
	unsigned int    meta_data_phys;
	unsigned int    dev_file_id;
};

#endif /* __OTZ_COMMON_H_ */    
