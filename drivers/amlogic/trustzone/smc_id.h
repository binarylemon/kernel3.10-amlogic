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
 * Header file for SMC identifiers 
 */

#ifndef __OTZ_SMC_ID_H__
#define __OTZ_SMC_ID_H__


/* SMC Identifiers for non-secure world functions */
#define CALL_TRUSTZONE_API                      0x1
#define CALL_TRUSTZONE_REG_RD                   0x2
#define CALL_TRUSTZONE_REG_WR                   0x3
#define CALL_TRUSTZONE_MON                      0x4
#define CALL_TRUSTZONE_HAL_API                  0x5

/* Secure Monitor mode APIs */
#define TRUSTZONE_MON_TYPE_MASK                 0xF00
#define TRUSTZONE_MON_FUNC_MASK                 0x0FF
#define TRUSTZONE_MON_L2X0                      0x100
#define TRUSTZONE_MON_L2X0_CTRL_INDEX           0x101
#define TRUSTZONE_MON_L2X0_AUXCTRL_INDEX        0x102
#define TRUSTZONE_MON_L2X0_PREFETCH_INDEX       0x103
#define TRUSTZONE_MON_L2X0_TAGLATENCY_INDEX     0x104
#define TRUSTZONE_MON_L2X0_DATALATENCY_INDEX    0x105
#define TRUSTZONE_MON_L2X0_FILTERSTART_INDEX    0x106
#define TRUSTZONE_MON_L2X0_FILTEREND_INDEX      0x107
#define TRUSTZONE_MON_L2X0_DEBUG_INDEX          0x108
#define TRUSTZONE_MON_L2X0_POWER_INDEX          0x109

#define TRUSTZONE_MON_CORE                      0x200
#define TRUSTZONE_MON_CORE_RD_CTRL_INDEX        0x201
#define TRUSTZONE_MON_CORE_WR_CTRL_INDEX        0x202
#define TRUSTZONE_MON_CORE_RD_STATUS0_INDEX     0x203
#define TRUSTZONE_MON_CORE_WR_STATUS0_INDEX     0x204
#define TRUSTZONE_MON_CORE_RD_STATUS1_INDEX     0x205
#define TRUSTZONE_MON_CORE_WR_STATUS1_INDEX     0x206
#define TRUSTZONE_MON_CORE_BOOTADDR_INDEX       0x207
#define TRUSTZONE_MON_CORE_DDR_INDEX            0x208
#define TRUSTZONE_MON_CORE_RD_SOC_REV1          0x209
#define TRUSTZONE_MON_CORE_RD_SOC_REV2          0x20A
#define TRUSTZONE_MON_CORE_OFF                  0x20B

#define TRUSTZONE_MON_SUSPNED_FIRMWARE          0x300
#define TRUSTZONE_MON_SUSPNED_FIRMWARE_INIT     0x301
#define TRUSTZONE_MON_SUSPNED_FIRMWARE_UBOOT    0x302

#define TRUSTZONE_MON_SAVE_CPU_GIC              0x400

#define TRUSTZONE_MON_RTC                       0x500
#define TRUSTZONE_MON_RTC_RD_REG_INDEX          0x501
#define TRUSTZONE_MON_RTC_WR_REG_INDEX          0x502

#define TRUSTZONE_MON_REG                       0x600
#define TRUSTZONE_MON_REG_RD_INDEX              0x601
#define TRUSTZONE_MON_REG_WR_INDEX              0x602

#define TRUSTZONE_MON_MEM                       0x700
#define TRUSTZONE_MON_MEM_BASE                  0x701
#define TRUSTZONE_MON_MEM_TOTAL_SIZE            0x702
#define TRUSTZONE_MON_MEM_FLASH                 0x703
#define TRUSTZONE_MON_MEM_FLASH_SIZE            0x704
#define TRUSTZONE_MON_MEM_DEBUG                 0x705


/* Secure HAL APIs */
#define TRUSTZONE_HAL_TYPE_MASK                 0xF00
#define TRUSTZONE_HAL_FUNC_MASK                 0x0FF
#define TRUSTZONE_HAL_API_EFUSE                 0x100
#define TRUSTZONE_HAL_API_STORAGE               0x200
#define TRUSTZONE_HAL_API_MEMCONFIG             0x300
#define TRUSTZONE_HAL_API_MEMCONFIG_GE2D        0x301
#define TRUSTZONE_HAL_API_SRAM                  0x400
#define TRUSTZONE_HAL_API_SRAM_ACS              0x401
#define TRUSTZONE_HAL_API_SRAM_BOOTCHECK        0x402
#define TRUSTZONE_HAL_API_SRAM_EFUSECHECK       0x403
#define TRUSTZONE_HAL_API_SRAM_KEYCHECK         0x404
#define TRUSTZONE_HAL_API_SRAM_ISSECURESET      0x405
#define TRUSTZONE_HAL_API_SRAM_WR_ADDR		0x406
#define TRUSTZONE_HAL_API_HDCP 		0x500

#define VIRTIO_QUEUE_INIT                       (0xffffffce)
#define VIRTIO_QUEUE_NOTIFY                     (0xffffffc4)

#endif /* __OTZ_SMC_ID__ */
