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

#ifndef _OTZ_ID_ECHO_H_
#define _OTZ_ID_ECHO_H_


/**
 * @brief ECHO service UUID
 */
#define OTZ_SVC_ECHO 0x100

/**
 * @brief Enums used for echo service task
 */
enum otz_echo_cmd_id {
	OTZ_ECHO_CMD_ID_INVALID = 0x0,
	OTZ_ECHO_CMD_ID_SEND_CMD,
	OTZ_ECHO_CMD_ID_SEND_CMD_SHARED_BUF,
	OTZ_ECHO_CMD_ID_SEND_CMD_ARRAY_SPACE,
	OTZ_ECHO_CMD_ID_IPI_SEND_CMD,
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	OTZ_ECHO_CMD_ID_TEST_ASYNC_SEND_CMD,
#endif
	OTZ_ECHO_CMD_ID_TEST_SECURE_STORAGE,
	OTZ_ECHO_CMD_ID_UNKNOWN         = 0x7FFFFFFE,
	OTZ_ECHO_CMD_ID_MAX             = 0x7FFFFFFF
};

#endif
