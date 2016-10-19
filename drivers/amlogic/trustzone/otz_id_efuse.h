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

#ifndef _OTZ_ID_EFUSE_H_
#define _OTZ_ID_EFUSE_H_


/**
 * @brief EFUSE service UUID
 */
#define OTZ_SVC_EFUSE 0x11


/**
 * @brief Enums used for efuse service task
 */
enum otz_efuse_cmd_id {
    OTZ_EFUSE_CMD_ID_INVALID = 0x0,
    OTZ_EFUSE_CMD_ID_GET_ENTRY,
    OTZ_EFUSE_CMD_ID_SET_ENTRY,
};

#endif
