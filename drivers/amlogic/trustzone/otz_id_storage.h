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

#ifndef _OTZ_ID_STORAGE_H_
#define _OTZ_ID_STORAGE_H_


/**
 * @brief STORAGE service UUID
 */
#define OTZ_SVC_STORAGE 0x16

/**
 * @brief Enums used for efuse service task
 */
enum otz_storage_cmd_id {
    OTZ_STORAGE_CMD_ID_INVALID = 0x0,
    OTZ_STORAGE_CMD_ID_SECUREBOOT_ENA,
    OTZ_STORAGE_CMD_ID_INIT,
    OTZ_STORAGE_CMD_ID_EFUSE_SET_DATA,
    OTZ_STORAGE_CMD_ID_EFUSE_GET_DATA,
    OTZ_STORAGE_CMD_ID_GET_DATA,
    OTZ_STORAGE_CMD_ID_SET_DATA,
};

#endif
