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
 * Trusted application defintions. 
 */

#ifndef __OTZ_APP_EG_H
#define __OTZ_APP_EG_H

#define DATA_BUF_LEN 1024
#define CRYPT_BUF_LEN 256
#define DRM_BUFFER_SIZE 1024

/**
 * @brief 
 */
struct our_data_buffer {
	int len;
	char data[DATA_BUF_LEN];
	char response[DATA_BUF_LEN];
};

/**
 * @brief 
 */
struct drm_data_buffer {
	int len;
	char data[DRM_BUFFER_SIZE];
	char response[DRM_BUFFER_SIZE];
};

/**
 * @brief 
 */
struct crypt_data_buffer {
	int len;
	char data[CRYPT_BUF_LEN];
	char response[CRYPT_BUF_LEN];
};

/**
 * @brief 
 */
typedef struct drm_data_buffer drm_data_t;
/**
 * @brief 
 */
typedef struct crypt_data_buffer crypto_data_t;
/**
 * @brief 
 */
typedef struct our_data_buffer otz_mutex_test_data_t;


#endif

