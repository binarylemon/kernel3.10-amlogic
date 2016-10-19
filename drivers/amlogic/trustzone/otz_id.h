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

#ifndef __OTZ_ID_H_
#define __OTZ_ID_H_

#define SMC_ENOMEM          7
#define SMC_EOPNOTSUPP      6
#define SMC_EINVAL_ADDR     5
#define SMC_EINVAL_ARG      4
#define SMC_ERROR           3
#define SMC_INTERRUPTED     2
#define SMC_PENDING         1
#define SMC_SUCCESS         0

/**
 * @brief Encoding data type
 */
enum otz_enc_data_type {
	OTZ_ENC_INVALID_TYPE = 0,
	OTZ_ENC_UINT32,
	OTZ_ENC_ARRAY,
	OTZ_MEM_REF,
	OTZ_SECURE_MEM_REF
};

/**
 * @brief Service identifiers
 */
enum otz_svc_id {
	OTZ_SVC_INVALID = 0x0,
	OTZ_SVC_GLOBAL,
	OTZ_SVC_DRM,
	OTZ_SVC_CRYPT,
	OTZ_SVC_MUTEX_TEST,
	OTZ_SVC_VIRTUAL_KEYBOARD,
	OTZ_SVC_KERNEL_INTEGRITY_CHECK,
	OTZ_SVC_NS,
	OTZ_SVC_SHELL,
	OTZ_SVC_TEST_SUITE_KERNEL,
	OTZ_SVC_FFMPEG_TEST,
	OTZ_SVC_GP_INTERNAL,
	OTZ_SVC_TEST_SUITE_USER,
	OTZ_SVC_TEST_HEAP,
	OTZ_SVC_TEE_WAIT,
	OTZ_SVC_INT_CONTXT_SWITCH,
	OTZ_SVC_TEST_SHM = 0x10,
};

/**
 * @brief Command ID's for global service
 */
enum otz_global_cmd_id {
	OTZ_GLOBAL_CMD_ID_INVALID = 0x0,
	OTZ_GLOBAL_CMD_ID_BOOT_ACK,
	OTZ_GLOBAL_CMD_ID_OPEN_SESSION,
	OTZ_GLOBAL_CMD_ID_CLOSE_SESSION,
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	OTZ_GLOBAL_CMD_ID_REGISTER_NOTIFY_MEMORY,
	OTZ_GLOBAL_CMD_ID_UNREGISTER_NOTIFY_MEMORY,
#endif
	OTZ_GLOBAL_CMD_ID_RESUME_ASYNC_TASK,
	OTZ_GLOBAL_CMD_ID_REGISTER_SERVICE,
	OTZ_GLOBAL_CMD_ID_UNKNOWN         = 0x7FFFFFFE,
	OTZ_GLOBAL_CMD_ID_MAX             = 0x7FFFFFFF
};

/**
 * @brief Enums used for crypto service task
 */
enum otz_crypt_cmd_id {
	OTZ_CRYPT_CMD_ID_INVALID = 0x0,
	OTZ_CRYPT_CMD_ID_LOAD_LIBS,
	OTZ_CRYPT_CMD_ID_UNLOAD_LIBS,
	OTZ_CRYPT_CMD_ID_ENCRYPT,
	OTZ_CRYPT_CMD_ID_DECRYPT,
	OTZ_CRYPT_CMD_ID_MD5,
	OTZ_CRYPT_CMD_ID_SHA1,
	OTZ_CRYPT_CMD_ID_SHA224,
	OTZ_CRYPT_CMD_ID_SHA256,
	OTZ_CRYPT_CMD_ID_SHA384,
	OTZ_CRYPT_CMD_ID_SHA512,
	OTZ_CRYPT_CMD_ID_HMAC_MD5,
	OTZ_CRYPT_CMD_ID_HMAC_SHA1,
	OTZ_CRYPT_CMD_ID_HMAC_SHA224,
	OTZ_CRYPT_CMD_ID_HMAC_SHA256,
	OTZ_CRYPT_CMD_ID_HMAC_SHA384,
	OTZ_CRYPT_CMD_ID_HMAC_SHA512,
	OTZ_CRYPT_CMD_ID_CIPHER_AES_128_CBC,
	OTZ_CRYPT_CMD_ID_CIPHER_AES_192_CBC,
	OTZ_CRYPT_CMD_ID_CIPHER_AES_256_CBC,
	OTZ_CRYPT_CMD_ID_CIPHER_AES_128_ECB,
	OTZ_CRYPT_CMD_ID_CIPHER_AES_128_CTR,
	OTZ_CRYPT_CMD_ID_CIPHER_AES_192_CTR,
	OTZ_CRYPT_CMD_ID_CIPHER_AES_256_CTR,
	OTZ_CRYPT_CMD_ID_CIPHER_AES_128_XTS,
	OTZ_CRYPT_CMD_ID_CIPHER_DES_ECB,
	OTZ_CRYPT_CMD_ID_CIPHER_DES_CBC,
	OTZ_CRYPT_CMD_ID_CIPHER_DES3_ECB,
	OTZ_CRYPT_CMD_ID_CIPHER_DES3_CBC,
	OTZ_CRYPT_CMD_ID_UNKNOWN = 0x7FFFFFFE,
	OTZ_CRYPT_CMD_ID_MAX = 0x7FFFFFFF
};

#define MD5_OUTPUT_LEN 16
#define SHA1_OUTPUT_LEN 20
#define SHA224_OUTPUT_LEN 28
#define SHA256_OUTPUT_LEN 32
#define SHA384_OUTPUT_LEN 48
#define SHA512_OUTPUT_LEN 64
#define HMAC_KEY_LEN 16
#define HMAC_DATA_LEN 50
#define HMAC_OUTPUT_LEN 16
#define AES_128_CBC_LEN 16
#define AES_192_CBC_LEN 24
#define AES_256_CBC_LEN 32
#define AES_128_ECB_LEN 16
#define AES_128_CTR_LEN 16
#define AES_192_CTR_LEN 24
#define AES_256_CTR_LEN 32
#define AES_128_XTS_LEN 16
#define DES_ECB_LEN 8
#define DES_CBC_LEN 8
#define DES3_CBC_LEN 8
#define DES3_ECB_LEN 8
#define CIPHER_ENCRYPT 2
#define CIPHER_DECRYPT 1
#define IGNORE_PARAM  0xff


/**
 * @brief Enums used for mutex test task
 *
 **/
enum open_tz_mutex_test_cmd_id {
	OTZ_MUTEX_TEST_CMD_ID_INVALID = 0x0,
	OTZ_MUTEX_TEST_CMD_ID_TEST,
	OTZ_MUTEX_TEST_CMD_ID_UNKNOWN = 0x7FFFFFFE,
	OTZ_MUTEX_TEST_CMD_ID_MAX = 0x7FFFFFFF
};

/**
 * @brief 
 */
enum otz_virtual_keyboard_cmd_id {
	OTZ_VIRTUAL_KEYBOARD_CMD_ID_INVALID = 0x0,
	OTZ_VIRTUAL_KEYBOARD_CMD_ID_SHOW
};

/**
 * @brief Enums used for ffmpeg test task
 *
 **/

enum otz_ffmpeg_test_cmd_id {
	OTZ_FFMPEG_TEST_CMD_ID_INVALID = 0x0,
	OTZ_FFMPEG_TEST_CMD
};

/**
 * @brief 
 */
enum otz_kernel_check_cmd_id {
	OTZ_KERNEL_IM_INVALID = 0x0,
	OTZ_KERNEL_IM

};

/**
 * @brief 
 */
enum otz_drm_cmd_id {
	OTZ_DRM_CMD_ID_INVALID = 0x0,
	OTZ_DRM_CMD_ID_SEND_CMD,
	OTZ_DRM_CMD_ID_SEND_CMD_SHARED_BUF
};

/**
 * @brief Enums used for gp internal api test task
 */
enum otz_gp_internal_cmd_id {
	OTZ_GP_INTERNAL_CMD_ID_INVALID = 0x0,
	OTZ_GP_INTERNAL_CMD_ID_ARITHMATIC_API,
	OTZ_GP_INTERNAL_CMD_ID_STORAGE_API
};

/**
 * @brief Enums used for test suite
 */
enum otz_test_suite_kernel_cmd_id {
	OTZ_TEST_SUITE_CMD_ID_INVALID = 0x0,
	OTZ_TEST_SUITE_CMD_ID_ASYNC,
};

/**
 * @brief 
 */
enum otz_test_suite_user_cmd_id {
	OTZ_TEST_SUITE_USER_CMD_ID_INVALID = 0x0,
	OTZ_TEST_SUITE_CMD_ID_SHM,
	OTZ_TEST_SUITE_CMD_ID_MUTEX
};

#endif /* __OPEN_OTZ_ID_H_ */   
