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

#ifndef __OTZ_CLIENT_H_
#define __OTZ_CLIENT_H_

#define OTZ_CLIENT_FULL_PATH_DEV_NAME "/dev/otz_client"
#define OTZ_CLIENT_DEV "otz_client"

#define OTZ_CLIENT_IOC_MAGIC 0x775B777F /* "OTZ Client" */

/** IOCTL request */

/** @defgroup tzuserapi  TrustZone Linux User API
 *  User API. Used for communicating with the driver from user applications
 *  @{
 */

/**
 * @brief Encode command structure
 */
struct otz_client_encode_cmd {
	unsigned int len;
	void* data;
	int   offset;
	int   flags;
	int   param_type;

	int encode_id;
	int service_id;
	int session_id;
	unsigned int cmd_id;    
};

/**
 * @brief 
 */
struct otz_client_im_check {
	int encode_id;
	unsigned int cmd_id;
	unsigned int service_id,session_id;
};

#define SES_REQ_FLAG_SHARED (1 << 0)
/**
 * @brief Session details structure
 */
struct ser_ses_id{
	int service_id;
	int session_id;
	int flags;
};

/**
 * @brief Shared memory information for the session
 */
struct otz_session_shared_mem_info{
	int service_id;
	int session_id;
	unsigned int user_mem_addr;
};

/**
 * @brief Shared memory used for smc processing
 */
struct otz_smc_cdata {
	int cmd_addr;
	int ret_val;    
};

/**
 * @brief Service details structure
 */
struct otz_service_info {
	int magic;
	int service_id;
	void *elf_addr;
	int elf_size;
	char service_name[32];
	char process_name[32];
	char entry_name[32];
	char file_path[256];
};

/* For general service */
#define OTZ_CLIENT_IOCTL_SEND_CMD_REQ \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 3, struct otz_client_encode_cmd)
#define OTZ_CLIENT_IOCTL_SES_OPEN_REQ \
	_IOW(OTZ_CLIENT_IOC_MAGIC, 4, struct ser_ses_id)
#define OTZ_CLIENT_IOCTL_SES_CLOSE_REQ \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 5, struct ser_ses_id)
#define OTZ_CLIENT_IOCTL_SHR_MEM_FREE_REQ \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 6, struct otz_session_shared_mem_info )

#define OTZ_CLIENT_IOCTL_ENC_UINT32 \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 7, struct otz_client_encode_cmd)
#define OTZ_CLIENT_IOCTL_ENC_ARRAY \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 8, struct otz_client_encode_cmd)
#define OTZ_CLIENT_IOCTL_ENC_ARRAY_SPACE \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 9, struct otz_client_encode_cmd)
#define OTZ_CLIENT_IOCTL_ENC_MEM_REF \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 10, struct otz_client_encode_cmd)

#define OTZ_CLIENT_IOCTL_DEC_UINT32 \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 11, struct otz_client_encode_cmd)
#define OTZ_CLIENT_IOCTL_DEC_ARRAY_SPACE \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 12, struct otz_client_encode_cmd)
#define OTZ_CLIENT_IOCTL_OPERATION_RELEASE \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 13, struct otz_client_encode_cmd)
#define OTZ_CLIENT_IOCTL_SHR_MEM_ALLOCATE_REQ \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 14, struct otz_session_shared_mem_info)
#define OTZ_CLIENT_IOCTL_GET_DECODE_TYPE \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 15, struct otz_client_encode_cmd)
#define OTZ_CLIENT_IOCTL_IM_CHECK \
	_IOWR(OTZ_CLIENT_IOC_MAGIC,16,struct otz_client_im_check)

#define OTZ_CLIENT_IOCTL_TEE_SHR_MEM_FREE_REQ \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 17, struct otz_session_shared_mem_info )
#define OTZ_CLIENT_IOCTL_SVC_REGISTER_REQ \
	_IOWR(OTZ_CLIENT_IOC_MAGIC, 18, struct otz_service_info)

/** @} */ // end of tzuserapi

#endif /* __OTZ_CLIENT_H_ */    
