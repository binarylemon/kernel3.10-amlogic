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
 * TurstZone driver implementation
 *
 * Copyright (C) 2014 Amlogic, Inc.
 *
 * Author: Platform-BJ@amlogic.com
 *
 */


#include <linux/slab.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <asm/cacheflush.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include "otz_client.h"
#include "otz_common.h"
#include "otz_id.h"
#include "smc_id.h"
#include "sw_config.h"

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
#include <linux/sw_ipi.h>
#include <linux/smp.h>
#endif

//#define OTZONE_CPUFREQ_ENABLE
#ifdef OTZONE_CPUFREQ_ENABLE
#include <linux/cpufreq.h>
#endif


#define OTZONE_DRIVER_VERSION "3.0.0"


#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

static struct class *driver_class;
static dev_t otz_client_device_no;
static struct cdev otz_client_cdev;

static u32 cacheline_size;
static u32 device_file_cnt = 0;
static char *otz_svc_shrd_mem = NULL;

static struct otz_smc_cdata otz_smc_cd[NR_CPUS];

#define OTZ_DRIVER_INFO
#define OTZ_DRIVER_DEBUG
#undef TZINFO
#ifdef OTZ_DRIVER_INFO
#define TZINFO(fmt, args...) printk(KERN_INFO "%s(%i, %s): " fmt "\n", \
		__func__, current->pid, current->comm, ## args)
#else
#define TZINFO(fmt, args...)
#endif

#undef TZDEBUG
#ifdef OTZ_DRIVER_DEBUG
#define TZDEBUG(fmt, args...) printk("%s(%i, %s): " fmt "\n", \
		__func__, current->pid, current->comm, ## args)
#else
#define TZDEBUG(fmt, args...)
#endif

#undef TZERR
#define TZERR(fmt, args...) printk(KERN_ERR "%s(%i, %s): " fmt "\n", \
		__func__, current->pid, current->comm, ## args)

#ifndef USE_SPIN_LOCK
#define DEFINE_LOCK(x) DEFINE_MUTEX(x)
#define LOCK(x) { \
	TZINFO("Try locking mutex_%s\n", #x); \
	mutex_lock((struct mutex*)&x); \
	TZINFO("Done locking mutex_%s\n", #x); \
	}
#define UNLOCK(x) { \
	TZINFO("Try unlocking mutex_%s\n", #x); \
	mutex_unlock((struct mutex*)&x); \
	TZINFO("Done unlocking mutex_%s\n", #x); \
	}
#define IS_LOCKED(x) mutex_is_locked(&x)
#else
#define DEFINE_LOCK(x) DEFINE_SPINLOCK(x); \
	unsigned long flag_##x;
#define LOCK(x) spin_lock_irqsave(&x, flag_##x)
#define UNLOCK(x) spin_unlock_irqrestore(&x, flag_##x)
#endif
/**
 * @brief 
 *
 * @param in_lock
 */
static DEFINE_LOCK(in_lock);

/*
 * @brief
 */
static DEFINE_LOCK(open_lock);

/*
 * @brief
 */
static DEFINE_LOCK(mmap_lock);

/**
 * @brief 
 *
 * @param send_cmd_lock
 */
static DEFINE_LOCK(send_cmd_lock);

/**
 * @brief 
 *
 * @param smc_lock
 */
static DEFINE_LOCK(smc_lock);

/**
 * @brief 
 *
 * @param encode_cmd_lock
 */
static DEFINE_LOCK(encode_cmd_lock);

/**
 * @brief 
 *
 * @param im_check_lock
 */
static DEFINE_LOCK(im_check_lock);

/**
 * @brief 
 *
 * @param decode_cmd_lock
 */
static DEFINE_LOCK(decode_cmd_lock);

/**
 * @brief 
 *
 * @param ses_open_lock
 */
static DEFINE_LOCK(ses_open_lock);

/**
 * @brief 
 *
 * @param ses_close_lock
 */
static DEFINE_LOCK(ses_close_lock);

/**
 * @brief 
 *
 * @param mem_free_lock
 */
static DEFINE_LOCK(mem_free_lock);

/**
 * @brief 
 *
 * @param mem_alloc_lock
 */
static DEFINE_LOCK(mem_alloc_lock);

/*
 * @Brief
 */
static DEFINE_LOCK(file_list_lock);

/*
 * @brief
 */
static DEFINE_LOCK(operation_release_lock);

/**
 * @brief 
 */
static struct otz_dev_file_head{
	u32 dev_file_cnt;
	struct list_head dev_file_list;
} otzc_dev_file_head;

/**
 *
 */
typedef struct otz_shared_session {
	struct list_head head;
	u32 service_id;
	int session_id;
	int open_count;
} otzc_shared_session;

/**
 *
 */
static struct list_head otzc_shared_sessions_head;

/**
 * @brief 
 */
typedef struct otzc_shrd_mem_head{

	int shared_mem_cnt;
	struct list_head shared_mem_list;
} otzc_shared_mem_head;

/**
 * @brief 
 */
typedef struct otz_dev_file{
	struct list_head head;
	u32 dev_file_id;
	u32 service_cnt;
	struct list_head services_list;
	otzc_shared_mem_head dev_shared_mem_head;
} otzc_dev_file;

/**
 * @brief 
 */
typedef struct otzc_service{
	struct list_head head;
	u32 service_id;
	struct list_head sessions_list;
} otzc_service;


/**
 * @brief 
 */
typedef struct otzc_session{
	struct list_head head;
	int session_id;
	int pid;
	char comm[32];

	struct list_head encode_list;
	struct list_head shared_mem_list;
} otzc_session;

/**
 * @brief 
 */
struct otz_wait_data {
	wait_queue_head_t send_cmd_wq;
	int               send_wait_flag;
};

/**
 * @brief 
 */
typedef struct otzc_encode{

	struct list_head head;

	int encode_id;

	void* ker_req_data_addr;
	void* ker_res_data_addr;

	u32  enc_req_offset;
	u32  enc_res_offset;
	u32  enc_req_pos;
	u32  enc_res_pos;
	u32  dec_res_pos;

	u32  dec_offset;

	struct otz_wait_data wait_data;

	struct otzc_encode_meta *meta;
} otzc_encode;



/**
 * @brief 
 */
typedef struct otzc_shared_mem{

	struct list_head head;
	struct list_head s_head;

	void* index;

	void* k_addr;
	void* u_addr;
	u32  len;
	u32  fd;
} otzc_shared_mem;

/**
 * @brief 
 */
typedef struct otz_register_node {
	u32 svc_id;
	u32 registered;
	char svc_name[32];
} otz_register_node_t;

/**
 * @brief 
 */
#define OTZ_REGISTER_MAX 16
struct otz_register_mgr {
	int count;
	otz_register_node_t nodes[OTZ_REGISTER_MAX];
} otz_register_mgr;

/**
 * @brief 
 */
typedef struct otz_trace_node{
	u32 fd;
	u32 pid;
	char comm[32];
	u32 svc_id;
	int ses_id;
	u32 cmd_id;

	u64 usecs;
	u64 time_start;
	u64 time_end;
} otz_trace_node_t;

/**
 * @brief 
 */
struct otz_status{

    int debug;
    int index;
    int count;
    u64 usecs;
    otz_trace_node_t nodes[11];
} otz_status;


static int otz_client_prepare_encode(void* private_data,
		struct otz_client_encode_cmd *enc,
		otzc_encode **penc_context,
		otzc_session **psession);

static int otz_alloc_svc_shared_memory(void)
{
	if (otz_svc_shrd_mem) {
		return 0;
	} else {
		otz_svc_shrd_mem = (char*) __get_free_pages(GFP_KERNEL,
				get_order(ROUND_UP(TEEC_CONFIG_TAMEM_MAX_SIZE, SZ_4K)));
		if (!otz_svc_shrd_mem) {
			TZERR("Unable to allocate svc space\n");
			return -1;
		}
	}

	return 0;
}

static void otz_free_svc_shared_memory(void)
{
	if (otz_svc_shrd_mem) {
		free_pages((u32)otz_svc_shrd_mem, get_order(ROUND_UP(TEEC_CONFIG_TAMEM_MAX_SIZE, SZ_4K)));
		otz_svc_shrd_mem = NULL;
	}

	return;
}

/**
 * @brief Function to open file.
 *
 * @param path
 * @param flags
 * @param rights
 *
 * @return 
 */
static struct file* my_file_open(const char* path, int flags, int rights)
{
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	TZINFO("File path is: %s\n",path);
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		TZERR("************File not able to open!! *************** %d\n",err);
		return NULL;
	}
	return filp;
}

/**
 * @brief Function to seek file (Similar to lseek)
 *
 * @param file
 * @param offset
 * @param origin
 *
 * @return 
 */
static int my_file_seek(struct file *file,unsigned long long offset, int origin)
{
	mm_segment_t oldfs;
	int ret;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	ret = vfs_llseek(file,offset,origin);
	vfs_llseek(file,offset,0);
	set_fs(oldfs); 
	return ret;
}
/**
 * @brief Function to read file
 *
 * @param file
 * @param offset
 * @param data
 * @param size
 *
 * @return 
 */
static int my_file_read(struct file* file, unsigned long long offset,
		unsigned char* data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	ret = vfs_read(file, data, size, &file->f_pos);
	set_fs(oldfs);
	return ret;
} 

/**
 * @brief Function to close file
 *
 * @param file
 */
static void my_file_close(struct file* file)
{
	filp_close(file, NULL);
}

/**
 * @brief Function to get file information.
 *
 * @param file
 *
 * @return 
 */
static char *get_file_data(char* file, int *size)
{
	char *data_addr = NULL;
	struct file *fp;
	int data_size = 0;
	int read_size = 0;

	fp = my_file_open(file, O_RDONLY, 0);
	if (!fp) {
		TZERR("error in file_open");
		return NULL;
	}

	data_size = my_file_seek(fp, 0, 2);
	if (data_size < 1) {
		TZERR("File is empty");
		goto return_func;
	}

	if (data_size > TEEC_CONFIG_TAMEM_MAX_SIZE) {
		TZERR("TA (size=%d) should be less than %d.\n", data_size, TEEC_CONFIG_TAMEM_MAX_SIZE);
		goto return_func;
	}
	if (otz_svc_shrd_mem) {
		data_addr = otz_svc_shrd_mem;
	} else {
		data_addr = (char*) __get_free_pages(GFP_KERNEL,
				get_order(ROUND_UP(data_size, SZ_4K)));
		if (!data_addr) {
			TZERR("Unable to allocate space\n");
			goto return_func;
		}
	}

	read_size = my_file_read(fp, 0, data_addr, data_size);
	if (read_size > 0) {
		//TZERR("read_size=%d, file_size=%d", read_size, data_size);
		*size = read_size;
	} else {
		TZERR("Read failed");
	}

return_func:
	my_file_close(fp);
	return data_addr;
}

/**
 * @brief Function to free file information.
 *
 * @param file
 *
 * @return 
 */
static void free_file_data(char *data_addr, int size)
{
	if (data_addr && (data_addr == otz_svc_shrd_mem)) {
		return;
	}

	if (data_addr && (size > 0)) {
		free_pages((u32)data_addr, get_order(ROUND_UP(size, SZ_4K)));
	}

	return;
}

static int otz_register_list_add(int svc_id, const char *svc_name)
{
	if (otz_register_mgr.count < OTZ_REGISTER_MAX) {
		otz_register_mgr.nodes[otz_register_mgr.count].svc_id = svc_id;
		strncpy(otz_register_mgr.nodes[otz_register_mgr.count].svc_name, svc_name, 32);
		otz_register_mgr.count++;
	} else {
		TZERR("regiser list full.\n");
		return -1;
	}

	return otz_register_mgr.count;
}

static int otz_register_find_by_id(int svc_id)
{
	int found = -1;
	int i = 0;

	for (i = 0; i < otz_register_mgr.count; i++) {
		if (otz_register_mgr.nodes[i].svc_id == svc_id) {
			found = i;
			break;
		}
	}

	return found;
}

static void otz_smc_trace_start(u32 fd, u32 svc_id, u32 ses_id, u32 cmd_id)
{
	int idx = -1;

	if (otz_status.debug < 2) {
		return;
	}

	idx = otz_status.index + 1;
	if (idx > 10) {
		idx = 1;
	}
	otz_status.nodes[idx].fd = fd;
	otz_status.nodes[idx].svc_id = svc_id;
	otz_status.nodes[idx].ses_id = ses_id;
	otz_status.nodes[idx].cmd_id = cmd_id;
	otz_status.nodes[idx].pid = current->pid;
	strncpy(otz_status.nodes[idx].comm, current->comm, 32);

	otz_status.nodes[idx].time_start = ktime_to_us(ktime_get());
	otz_status.index = idx;

	return;
}

static void otz_smc_trace_stop(void)
{
	int idx = -1;

	if (otz_status.debug < 2) {
		return;
	}

	idx = otz_status.index;
	otz_status.nodes[idx].time_end = ktime_to_us(ktime_get());
	otz_status.nodes[idx].usecs = otz_status.nodes[idx].time_end - otz_status.nodes[idx].time_start;
	otz_status.count++;
	otz_status.usecs += otz_status.nodes[idx].usecs;
	if (otz_status.nodes[0].usecs < otz_status.nodes[idx].usecs) {
		otz_status.nodes[0].fd = otz_status.nodes[idx].fd;
		otz_status.nodes[0].svc_id = otz_status.nodes[idx].svc_id;
		otz_status.nodes[0].ses_id = otz_status.nodes[idx].ses_id;
		otz_status.nodes[0].cmd_id = otz_status.nodes[idx].cmd_id;
		otz_status.nodes[0].pid = otz_status.nodes[idx].pid;
		strncpy(otz_status.nodes[0].comm, otz_status.nodes[idx].comm, 32);
		otz_status.nodes[0].usecs = otz_status.nodes[idx].usecs;
	}

	if (otz_status.debug > 2) {
#ifdef OTZONE_CPUFREQ_ENABLE
		printk("cpu0 @ %dKHZ, ", cpufreq_quick_get(0));
#endif
		printk("fd=%04d, svc_id=0x%03x, ses_id=0x%04x, cmd_id=0x%02x, time=%08llu us. (pid=%04d, %s)\n",
			otz_status.nodes[idx].fd,
			otz_status.nodes[idx].svc_id,
			otz_status.nodes[idx].ses_id,
			otz_status.nodes[idx].cmd_id,
			otz_status.nodes[idx].usecs,
			otz_status.nodes[idx].pid,
			otz_status.nodes[idx].comm);
	}

	return;
}

/**
 * @brief 
 *
 * @param cmd_addr
 *
 * @return 
 */
static u32 _otz_smc(u32 cmd_addr)
{
	register u32 r0 asm("r0") = CALL_TRUSTZONE_API;
	register u32 r1 asm("r1") = cmd_addr;
	register u32 r2 asm("r2") = OTZ_CMD_TYPE_NS_TO_SECURE;


	do {
		asm volatile(
#if USE_ARCH_EXTENSION_SEC
				".arch_extension sec\n\t"
#endif
				__asmeq("%0", "r0")
				__asmeq("%1", "r0")
				__asmeq("%2", "r1")
				__asmeq("%3", "r2")
				"smc    #0  @ switch to secure world\n"
				: "=r" (r0)
				: "r" (r0), "r" (r1), "r" (r2));
	} while (0);

	return r0;
}

/**
 * @brief 
 *
 * @param otz_smc handler for secondary cores
 *
 * @return 
 */
static void secondary_otz_smc_handler(void *info)
{
	struct otz_smc_cdata *cd = (struct otz_smc_cdata *)info;

	rmb();

	TZINFO("secondary otz smc handler...");

	cd->ret_val = _otz_smc(cd->cmd_addr);
	wmb();

	TZINFO("done smc on primary \n");
}

/**
 * @brief This function takes care of posting the smc to the 
 *        primary core
 *
 * @param cpu_id
 * @param cmd_addr
 *
 * @return 
 */
static u32 post_otz_smc(int cpu_id, u32 cmd_addr)
{
	struct otz_smc_cdata *cd = &otz_smc_cd[cpu_id];

	TZINFO("Post from secondary ...");

	cd->cmd_addr = cmd_addr;
	cd->ret_val  = 0;
	wmb();

	smp_call_function_single(0, secondary_otz_smc_handler, 
			(void *)cd, 1);
	rmb();

	TZINFO("completed smc on secondary \n");

	return cd->ret_val;
}

/**
 * @brief otz_smc wrapper to handle the multi core case
 *
 * @param cmd_addr
 *
 * @return 
 */
static u32 otz_smc(u32 cmd_addr)
{
	int cpu_id = raw_smp_processor_id();

	if (cpu_id != 0) {
		mb();
		return post_otz_smc(cpu_id, cmd_addr); /* post it to primary */
	} else {
		return _otz_smc(cmd_addr); /* called directly on primary core */
	}
}

/**
 * @brief 
 *
 * @param cmd_addr
 *
 * @return 
 */
static int otz_smc_mon(u32 cmd, u32 arg)
{
	register u32 r0 asm("r0") = CALL_TRUSTZONE_MON;
	register u32 r1 asm("r1") = cmd;
	register u32 r2 asm("r2") = arg;

	do {
		asm volatile(
#if USE_ARCH_EXTENSION_SEC
				".arch_extension sec\n\t"
#endif
				__asmeq("%0", "r0")
				__asmeq("%1", "r0")
				__asmeq("%2", "r1")
				__asmeq("%3", "r2")
				"smc    #0  @ switch to secure world\n"
				: "=r" (r0)
				: "r" (r0), "r" (r1), "r" (r2));
	} while (0);

	return r0;
}

/**
 * @brief
 *
 * @return
 */
static int otz_memory_debug(void)
{
	int ret = 0;

	ret = otz_smc_mon(TRUSTZONE_MON_MEM_DEBUG, 0);

	return ret;
}

/**
 * @brief Call SMC
 *
 * When the processor executes the Secure Monitor Call (SMC),
 * the core enters Secure Monitor mode to execute the Secure Monitor code
 *
 * @param svc_id  - service identifier
 * @param cmd_id  - command identifier
 * @param context - session context
 * @param enc_id - encoder identifier
 * @param cmd_buf - command buffer 
 * @param cmd_len - command buffer length
 * @param resp_buf - response buffer
 * @param resp_len - response buffer length
 * @param meta_data
 * @param ret_resp_len
 *
 * @return 
 */
static int otz_smc_call(u32 dev_file_id, u32 svc_id, u32 cmd_id,
		u32 context, u32 enc_id, const void *cmd_buf,
		size_t cmd_len, void *resp_buf, size_t resp_len, 
		const void *meta_data, int *ret_resp_len, 
		struct otz_wait_data* wq, void* arg_lock)
{
	int ret;
	u32 smc_cmd_phys;

	struct otz_smc_cmd *smc_cmd;

	smc_cmd = (struct otz_smc_cmd*)kmalloc(sizeof(struct otz_smc_cmd), 
			GFP_KERNEL);
	if(!smc_cmd){
		TZERR("kmalloc failed for smc command\n");
		ret = -ENOMEM;
		goto out;
	}

	if(ret_resp_len)
		*ret_resp_len = 0;

	smc_cmd->src_id = (svc_id << 10) | cmd_id;
	smc_cmd->src_context = task_tgid_vnr(current);

	smc_cmd->id = (svc_id << 10) | cmd_id;
	smc_cmd->context = context;
	smc_cmd->enc_id = enc_id;
	smc_cmd->dev_file_id = dev_file_id;
	smc_cmd->req_buf_len = cmd_len;
	smc_cmd->resp_buf_len = resp_len;
	smc_cmd->ret_resp_buf_len = 0;

	if(cmd_buf)
		smc_cmd->req_buf_phys = virt_to_phys((void*)cmd_buf);
	else
		smc_cmd->req_buf_phys = 0;

	if(resp_buf)
		smc_cmd->resp_buf_phys = virt_to_phys((void*)resp_buf);
	else
		smc_cmd->resp_buf_phys = 0;

	if(meta_data)
		smc_cmd->meta_data_phys = virt_to_phys(meta_data);
	else
		smc_cmd->meta_data_phys = 0;

	smc_cmd_phys = virt_to_phys((void*)smc_cmd);

	LOCK(smc_lock);
	otz_smc_trace_start(dev_file_id, svc_id, context, cmd_id);
#ifdef OTZONE_CPUFREQ_ENABLE
	cpufreq_busy_start();
#endif
	ret = otz_smc(smc_cmd_phys);
#ifdef OTZONE_CPUFREQ_ENABLE
	cpufreq_busy_end();
#endif
	otz_smc_trace_stop();
	UNLOCK(smc_lock);

#ifdef  OTZONE_ASYNC_NOTIFY_SUPPORT
	if(ret == SMC_PENDING){

		if(arg_lock)
			mutex_unlock(arg_lock);
		if(wq){
			if(wait_event_interruptible(wq->send_cmd_wq,
						wq->send_wait_flag)) {
				ret = -ERESTARTSYS;
				goto out;
			}
			wq->send_wait_flag = 0;
		}

		if(arg_lock)
			mutex_lock(arg_lock);


		svc_id = OTZ_SVC_GLOBAL;
		cmd_id = OTZ_GLOBAL_CMD_ID_RESUME_ASYNC_TASK;
		smc_cmd->src_id = (svc_id << 10) | cmd_id;
		smc_cmd->id = (svc_id << 10) | cmd_id;

		LOCK(smc_lock);

		ret = otz_smc(smc_cmd_phys);
		UNLOCK(smc_lock);

	}
#endif

	if (ret) {
		TZERR("smc_call returns error\n");

		goto out;
	}

	if(ret_resp_len) {
		*ret_resp_len = smc_cmd->ret_resp_buf_len;
	}

out:
	if(smc_cmd)
		kfree(smc_cmd);
	return ret;
}

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
static void non_secure_async_wakeup( struct work_struct *new_work)
{
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_encode *enc_temp;

	int enc_found = 0;
	struct sw_ipi_info* ipi_info;
	struct otzc_notify_data* sec_notify_data;

	ipi_info = container_of(new_work,struct sw_ipi_info, work);
	sec_notify_data = (struct otzc_notify_data *)ipi_info->pri_data;

	if(!sec_notify_data)
		return;

	TZINFO("guest id %d\n", sec_notify_data->guest_no);
	TZINFO("otz_client pid 0x%x\n", sec_notify_data->client_pid);
	TZINFO("otz_client_notify_handler service id 0x%x \
			session id 0x%x and encoder id 0x%x\n", 
			sec_notify_data->service_id, sec_notify_data->session_id, sec_notify_data->enc_id); 

	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == sec_notify_data->dev_file_id){
			TZINFO("dev file id %d \n",temp_dev_file->dev_file_id);

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head){
				if(temp_svc->service_id == sec_notify_data->service_id) {
					TZINFO("send cmd ser id %d \n",temp_svc->service_id);

					list_for_each_entry(temp_ses, &temp_svc->sessions_list, head) {
						if(temp_ses->session_id == sec_notify_data->session_id) {
							TZINFO("send cmd ses id %d \n",temp_ses->session_id);

							list_for_each_entry(enc_temp,&temp_ses->encode_list, head) {
								if(enc_temp->encode_id == sec_notify_data->enc_id) {
									TZINFO("send cmd enc id 0x%x\n",
											enc_temp->encode_id);
									enc_found = 1;
									break;
								}
							}
						}
						break;        
					}
					break;
				}
			}
			break;
		}
	}
	UNLOCK(file_list_lock);

	if(enc_found) {
		if(!enc_temp->wait_data.send_wait_flag) {
			enc_temp->wait_data.send_wait_flag = 1;
			wake_up_interruptible(&enc_temp->wait_data.send_cmd_wq);
		}
		else {
			printk("Duplicate  Notifcation data\n");
		}
	}
	else {
		printk("Notifcation data is missing\n");
	}
	return;
}
#endif
/**
 * @brief
 * 
 * Clears session id and closes the session 
 *
 * @param private_data
 * @param temp_svc
 * @param temp_ses
 *
 */
static void otz_client_close_session_for_service(
		void* private_data,
		otzc_service* temp_svc, 
		otzc_session *temp_ses)
{

	otzc_encode *temp_encode, *enc_context;
	otzc_shared_mem *shared_mem, *temp_shared;
	u32 dev_file_id = (u32)private_data;
	bool close_session = true;
	otzc_shared_session* shs = NULL;

	if(!temp_svc || !temp_ses)
		return;

	if(temp_ses->session_id <= 0) {
		TZERR("invalid ses_id 0x%x\n",temp_ses->session_id); 
		return;
	}

	list_for_each_entry(shs, &otzc_shared_sessions_head, head) {
		if (shs->service_id == temp_svc->service_id) {
			shs->open_count--;
			if (shs->session_id != temp_ses->session_id)
				TZERR("unexpected\n");
			if (shs->open_count > 0) {
				close_session = false;
			} else {
				list_del(&shs->head);
			}
			break;
		}
	}

	//TZDEBUG("free ses_id 0x%x \n",temp_ses->session_id); 
	if (close_session)
		otz_smc_call(dev_file_id, OTZ_SVC_GLOBAL,
				OTZ_GLOBAL_CMD_ID_CLOSE_SESSION, 0, 0,
				&temp_svc->service_id,
				sizeof(temp_svc->service_id),&temp_ses->session_id,
				sizeof(temp_ses->session_id), NULL, NULL, NULL, NULL);

	list_del(&temp_ses->head);

	if (!list_empty(&temp_ses->encode_list)) {
		list_for_each_entry_safe(enc_context, temp_encode, 
				&temp_ses->encode_list, head) {
			list_del(&enc_context->head);   
			kfree(enc_context);
		}
	}

	if (!list_empty(&temp_ses->shared_mem_list)) {
		list_for_each_entry_safe(shared_mem, temp_shared, 
				&temp_ses->shared_mem_list, s_head) {
			list_del(&shared_mem->s_head);   

			if(shared_mem->k_addr)
				free_pages((u32)shared_mem->k_addr,
						get_order(ROUND_UP(shared_mem->len, SZ_4K)));

			kfree(shared_mem);
		}
	}

	kfree(temp_ses);
}

/**
 * @brief 
 *
 * @param service_id
 *
 * @return 
 */
static int otz_client_service_init(otzc_dev_file* dev_file, int service_id)
{
	int ret_code = 0;
	otzc_service* svc_new;
	otzc_service* temp_pos;
	svc_new = (otzc_service*)kmalloc(sizeof(otzc_service), GFP_KERNEL);
	if(!svc_new){
		TZERR("kmalloc failed \n");
		ret_code = -ENOMEM;
		goto clean_prev_malloc;
	}

	svc_new->service_id = service_id;
	dev_file->service_cnt++;
	INIT_LIST_HEAD(&svc_new->sessions_list);
	list_add(&svc_new->head, &dev_file->services_list);
	if (otz_status.debug > 0) {
		TZDEBUG("dev_file_id=%d, service_id=0x%x, service_cnt=%d", dev_file->dev_file_id, service_id, dev_file->service_cnt);
	}
	goto return_func;

clean_prev_malloc:
	if (!list_empty(&dev_file->services_list)) {
		list_for_each_entry_safe(svc_new, temp_pos, 
				&dev_file->services_list, head) {
			list_del(&svc_new->head);   
			kfree(svc_new);
		}
	}

return_func:
	return ret_code;
}


/**
 * @brief 
 *
 * @return 
 */
static int otz_client_service_exit(void* private_data)
{
	otzc_shared_mem* temp_shared_mem;
	otzc_shared_mem  *temp_pos;
	otzc_dev_file *tem_dev_file, *tem_dev_file_pos;
	otzc_session *temp_ses, *temp_ses_pos;
	otzc_service* tmp_svc = NULL, *tmp_pos;
	u32 dev_file_id;


	dev_file_id = (u32)(private_data);
	LOCK(file_list_lock);
	list_for_each_entry_safe(tem_dev_file, tem_dev_file_pos,
			&otzc_dev_file_head.dev_file_list, head) {
		if(tem_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry_safe(temp_shared_mem, temp_pos, 
					&tem_dev_file->dev_shared_mem_head.shared_mem_list, head){
				list_del(&temp_shared_mem->head);

				if(temp_shared_mem->k_addr) {
					free_pages((u32)temp_shared_mem->k_addr,
					get_order(ROUND_UP(temp_shared_mem->len, SZ_4K)));
					if (otz_status.debug > 0) {
						TZDEBUG("FREE k_addr 0x%p, fd=%d", temp_shared_mem->k_addr, dev_file_id);
					}
				}

				if(temp_shared_mem)
					kfree(temp_shared_mem);
			}
			if (!list_empty(&tem_dev_file->services_list)) {

				list_for_each_entry_safe(tmp_svc, tmp_pos,
						&tem_dev_file->services_list, head) {

					list_for_each_entry_safe(temp_ses, temp_ses_pos, 
							&tmp_svc->sessions_list, head) {
						otz_client_close_session_for_service(private_data, 
								tmp_svc, temp_ses);
					}
					list_del(&tmp_svc->head);   
					kfree(tmp_svc);
				}
			}

			list_del(&tem_dev_file->head);   
			kfree(tem_dev_file);
			otzc_dev_file_head.dev_file_cnt--;
			break;
		}
	}
	UNLOCK(file_list_lock);

	return 0;
}

/**
 * @brief 
 *
 * Register service
 *
 * @param private_data - Holds the device file ID
 * @param argp - Contains the Service info
 *
 * @return 
 */
static int otz_client_service_register(void* private_data, void* argp)
{
	int svc_ret = 0;
	int ret_val = 0;
	int ret_resp_len = 0;
	char *elf_addr= NULL;
	int elf_size = 0;
	struct otz_service_info svc_info;
	u32 dev_file_id = (u32)private_data;

	if(copy_from_user(&svc_info, argp, sizeof(svc_info))){
		TZERR("copy from user failed\n");
		ret_val =  -EFAULT;
		goto return_func;
	}

	if (otz_status.debug > 0) {
		TZDEBUG("service_id: 0x%x", svc_info.service_id);
		TZDEBUG("service_name: %s", svc_info.service_name);
		TZDEBUG("process_name: %s", svc_info.process_name);
		TZDEBUG("entry_name: %s", svc_info.entry_name);
		TZDEBUG("file_path: %s", svc_info.file_path);
	}

	if (otz_register_find_by_id(svc_info.service_id) >= 0) {
		if (otz_status.debug > 0) {
			TZDEBUG("service_id: 0x%x already registered!", svc_info.service_id);
		}
		ret_val =  0;
		goto return_func;
	}

	elf_addr = get_file_data(svc_info.file_path, &elf_size);
	if (elf_addr && elf_size > 0) {
		svc_info.elf_addr = (void *)virt_to_phys((void *)elf_addr);
		svc_info.elf_size = elf_size;
	} else {
		TZERR("open elf file failed\n");
		ret_val =  -EFAULT;
		goto return_func;
	}

	if (otz_status.debug > 0) {
		TZDEBUG("elf_addr: 0x%p", svc_info.elf_addr);
		TZDEBUG("elf_size: %d", svc_info.elf_size);
	}
	ret_val = otz_smc_call(dev_file_id, OTZ_SVC_GLOBAL,
			OTZ_GLOBAL_CMD_ID_REGISTER_SERVICE, 0, 0,
			&svc_info, sizeof(svc_info),
			&svc_ret, sizeof(svc_ret), NULL,
			&ret_resp_len, NULL, NULL);

	if(ret_val != SMC_SUCCESS) {
		goto return_func;
	}

	if(svc_ret != 0) {
		TZERR("register service id: 0x%x ERROR\n", svc_info.service_id);
		ret_val =  -EINVAL;
		goto return_func;
	} else {
		otz_register_list_add(svc_info.service_id, svc_info.service_name);
	}

	if (otz_status.debug > 0) {
		TZDEBUG("register service_id=0x%x OK", svc_info.service_id);
	}

return_func:
	if (elf_addr && elf_size > 0) {
		free_file_data(elf_addr, elf_size);
	}

	return ret_val;
}

/**
 * @brief 
 *
 * Opens session for the requested service by getting the service ID
 * form the user. After opening the session, the session ID is copied back
 * to the user space.
 *
 * @param private_data - Holds the device file ID
 * @param argp - Contains the Service ID
 *
 * @return 
 */
static int otz_client_session_open(void* private_data, void* argp)
{
	otzc_service* svc = NULL;
	otzc_dev_file *temp_dev_file;
	otzc_session* ses_new;
	struct ser_ses_id ses_open;
	int svc_found = 0;
	int ret_val = 0, ret_resp_len;
	u32 dev_file_id = (u32)private_data;
	bool shared_session_found = false;

	TZINFO("inside session open\n");

	if(copy_from_user(&ses_open, argp, sizeof(ses_open))){
		TZERR("copy from user failed\n");
		ret_val =  -EFAULT;
		goto return_func;
	}

	TZINFO("service_id = %d\n",ses_open.service_id);

	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){
			list_for_each_entry(svc, &temp_dev_file->services_list, head){
				if( svc->service_id == ses_open.service_id){
					svc_found = 1;
					break;
				}
			}
			if(!svc_found) {
				ret_val = otz_client_service_init(temp_dev_file, ses_open.service_id);
				if(ret_val == 0) {
					list_for_each_entry(svc, &temp_dev_file->services_list, head){
						if( svc->service_id == ses_open.service_id){
							svc_found = 1;
							break;
						}
					}
				}
			}
			break;
		}
	}

	if (ses_open.flags & SES_REQ_FLAG_SHARED) {
		otzc_shared_session* shs = NULL;
		list_for_each_entry(shs, &otzc_shared_sessions_head, head) {
			if (shs->service_id == ses_open.service_id) {
				ses_open.session_id = shs->session_id;
				shs->open_count++;
				shared_session_found = true;
				break;
			}
		}
	}

	if(!svc_found) {
		ret_val =  -EINVAL;
		TZERR("svc not found. service_id = %d, dev_file_id=%d", ses_open.service_id, dev_file_id);
		UNLOCK(file_list_lock);
		goto return_func;
	}

	ses_new = (otzc_session*)kmalloc(sizeof(otzc_session), GFP_KERNEL);
	if(!ses_new) {
		TZERR("kmalloc failed\n");
		ret_val =  -ENOMEM;
		UNLOCK(file_list_lock);
		goto return_func;
	}

	TZINFO("service id 0x%x\n", ses_open.service_id);

	if (shared_session_found) {
		ses_new->session_id = ses_open.session_id;
		ret_val = SMC_SUCCESS;
	} else {
		ret_val = otz_smc_call(dev_file_id, OTZ_SVC_GLOBAL,
				OTZ_GLOBAL_CMD_ID_OPEN_SESSION, 0, 0,
				&ses_open.service_id, sizeof(ses_open.service_id),
				&ses_new->session_id,sizeof(ses_new->session_id), NULL,
				&ret_resp_len, NULL, NULL);
		if (ses_open.flags & SES_REQ_FLAG_SHARED) {
			otzc_shared_session* shs = NULL;
			shs = (otzc_shared_session*)kmalloc(sizeof(otzc_shared_session), GFP_KERNEL);
			if (!shs) {
				ret_val = -ENOMEM;
				goto return_func;
			}
			shs->service_id = ses_open.service_id;
			shs->session_id = ses_new->session_id;
			shs->open_count = 1;
			INIT_LIST_HEAD(&shs->head);
			list_add_tail(&shs->head, &otzc_shared_sessions_head);
		}
	}
	UNLOCK(file_list_lock);


	if(ret_val != SMC_SUCCESS) {
		goto clean_session;
	}

	if(ses_new->session_id == -1) {
		TZERR("invalid session id: 0x%x\n", ses_new->session_id);
		ret_val =  -EINVAL;
		goto clean_session;
	}

	if (otz_status.debug > 0) {
		TZDEBUG("session_id=0x%x, service_id=0x%x", ses_new->session_id,
				ses_open.service_id);
	}

	ses_open.session_id = ses_new->session_id;
	ses_new->pid = current->pid;
	if (current->pid != 0)
		strncpy(ses_new->comm, current->comm, 32);
	else
		ses_new->comm[0] = '\0';

	INIT_LIST_HEAD(&ses_new->encode_list);
	INIT_LIST_HEAD(&ses_new->shared_mem_list);
	list_add_tail(&ses_new->head, &svc->sessions_list);

	if(copy_to_user(argp, &ses_open, sizeof(ses_open))) {
		TZERR("copy from user failed\n");
		ret_val =  -EFAULT;
		goto clean_hdr_buf;
	}

	/*   TZINFO("session created from service \n"); */
	goto return_func;

clean_hdr_buf:
	list_del(&ses_new->head);

clean_session:
	kfree(ses_new);

return_func:

	return ret_val;
}

/**
 * @brief 
 *
 * Closes the client session by getting the service ID and
 * session ID from user space.
 *
 * @param private_data - Contains the device file ID
 * @param argp - Contains the service ID and Session ID
 *
 * @return 
 */
static int otz_client_session_close(void* private_data, void* argp)
{

	struct ser_ses_id ses_close;
	int ret_val = 0;
	u32 dev_file_id = (u32)private_data;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;

	TZINFO("inside session close\n");

	if(copy_from_user(&ses_close, argp, sizeof(ses_close))) {
		TZERR("copy from user failed \n");
		ret_val = -EFAULT;
		goto return_func;
	}

	if (otz_status.debug > 0) {
		TZDEBUG("session_id=0x%x, service_id=0x%x", ses_close.session_id,
				ses_close.service_id);
	}

	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == ses_close.service_id) {

					list_for_each_entry(temp_ses,
							&temp_svc->sessions_list, head) {
						if(temp_ses->session_id == ses_close.session_id) {
							otz_client_close_session_for_service(private_data, 
									temp_svc, temp_ses);
							break;
						}                      
					}
					break;
				}
			}
			break;
		}
	}
	UNLOCK(file_list_lock);

	TZINFO("return from close\n");

return_func:
	return ret_val;
}

/**
 * @brief 
 * 
 * Creates shared memory between non secure world applicaion
 * and non secure world kernel.
 * 
 * @param filp
 * @param vma
 *
 * @return 
 */
static int otz_client_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	otzc_shared_mem *mem_new;
	u32*  alloc_addr;
	long length = vma->vm_end - vma->vm_start;
	otzc_dev_file *temp_dev_file;
	u32 dev_file_id = (u32)(filp->private_data);

	TZINFO("Inside otz_client mmap\n");

	LOCK(mmap_lock);
	if (length > TEEC_CONFIG_SHAREDMEM_MAX_SIZE) {
		TZERR("shared memory (size=%ld) should be less than %d\n", length, TEEC_CONFIG_SHAREDMEM_MAX_SIZE);
		ret = -ENOMEM;
		goto return_func;
	}
	alloc_addr =  (void*) __get_free_pages(GFP_KERNEL,
			get_order(ROUND_UP(length, SZ_4K)));
	if(!alloc_addr) {
		TZERR("get free pages failed \n");
		ret = -ENOMEM;
		goto return_func;
	}

	if (otz_status.debug > 0) {
		TZDEBUG("MMAP k_addr 0x%p, fd=%d", alloc_addr, dev_file_id);
	}

	if (remap_pfn_range(vma,
				vma->vm_start,
				((virt_to_phys(alloc_addr)) >> PAGE_SHIFT),
				length,
				vma->vm_page_prot)) {
		ret = -EAGAIN;
		goto return_func;
	}

	mem_new = kmalloc(sizeof(otzc_shared_mem), GFP_KERNEL); 
	if(!mem_new) {
		TZERR("kmalloc failed\n");
		ret = -ENOMEM;
		goto return_func;
	}

	mem_new->k_addr = alloc_addr;
	mem_new->len = length;
	mem_new->u_addr = (void*)vma->vm_start;
	mem_new->index = mem_new->u_addr;
	mem_new->fd = dev_file_id;
	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == (u32)filp->private_data){
			break;
		}
	}
	temp_dev_file->dev_shared_mem_head.shared_mem_cnt++;
	list_add_tail( &mem_new->head ,
			&temp_dev_file->dev_shared_mem_head.shared_mem_list);
	UNLOCK(file_list_lock);

return_func:
	UNLOCK(mmap_lock);
	return ret;
}

/**
 * @brief 
 *
 * Sends a command from the non secure world Kernel to the 
 * secure world.
 *
 * @param private_data - Contains device file ID
 * @param argp
 *
 * @return 
 */
static int otz_client_kernel_send_cmd(void *private_data, void *argp)
{

	int ret = 0;
	int ret_resp_len = 0;
	int dev_file_id;
	struct otz_client_encode_cmd *enc = (struct otz_client_encode_cmd *)argp;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_encode *enc_temp;

	int enc_found = 0;
	dev_file_id = (u32)private_data;

	TZINFO("inside send cmd \n");

	TZINFO("enc id %d\n",enc->encode_id);
	TZINFO("dev file id %d\n",dev_file_id);
	TZINFO("ser id %d\n",enc->service_id);
	TZINFO("ses id %d\n",enc->session_id);

	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head){
				if(temp_svc->service_id == enc->service_id) {
					TZINFO("send cmd ser id %d \n",temp_svc->service_id);

					list_for_each_entry(temp_ses, &temp_svc->sessions_list, 
							head) {
						if(temp_ses->session_id
								== enc->session_id) {
							TZINFO("send cmd ses id %d \n",
									temp_ses->session_id);

							if(enc->encode_id != -1) {
								list_for_each_entry(enc_temp, 
										&temp_ses->encode_list, head) {
									if(enc_temp->encode_id == enc->encode_id){
										TZINFO("send cmd enc id 0x%x\n",
												enc_temp->encode_id);
										enc_found = 1;
										break;
									}
								}
							}
							else {
								UNLOCK(file_list_lock);
								ret = otz_client_prepare_encode(
										private_data,
										enc, &enc_temp, &temp_ses);
								LOCK(file_list_lock);
								if(!ret) {
									enc_found = 1;
								}
								break;
							}
						}
						break;        
					}
					break;
				}
			}
			break;
		}
	}
	UNLOCK(file_list_lock);

	if(!enc_found){
		ret = -EINVAL;
		goto return_func;
	}


	ret = otz_smc_call(dev_file_id, enc->service_id, enc->cmd_id,
			enc->session_id, 
			enc->encode_id,
			enc_temp->ker_req_data_addr, enc_temp->enc_req_offset, 
			enc_temp->ker_res_data_addr, enc_temp->enc_res_offset, 
			enc_temp->meta, &ret_resp_len, &enc_temp->wait_data ,
			&send_cmd_lock);

	if(ret != SMC_SUCCESS) {
		TZERR("send cmd secure call failed \n");
		goto return_func;
	}

	TZINFO("smc_success\n");


return_func:
	return ret; 
}

/**
 * @brief 
 *
 * Sends a command from the non secure application
 * to the secure world.
 *
 * @param private_data
 * @param argp
 *
 * @return 
 */

static int otz_client_send_cmd(void* private_data, void* argp)
{
	struct otz_client_encode_cmd enc;
	int ret = 0;
	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_kernel_send_cmd(private_data,&enc);
	if (ret == 0){
		if(copy_to_user(argp, &enc, sizeof(enc))) {
			TZERR("copy to user failed \n");
			ret = -EFAULT;
			goto return_func;
		}
	}
return_func:
	return ret;

}

/**
 * @brief 
 *
 * Frees the encode context associated with a particular device and session
 *
 * @param private_data - device file ID
 * @param argp - Encode structure
 *
 * @return 
 */
static int otz_client_kernel_operation_release(void *private_data, void *argp)
{

	otzc_encode *enc_context = NULL;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	u32 dev_file_id = (u32)private_data;
	int  session_found = 0, enc_found = 0;
	int ret =0;
	struct otz_client_encode_cmd *enc = (struct otz_client_encode_cmd *)argp;

	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == enc->service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id == enc->session_id) {
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}
	UNLOCK(file_list_lock);

	if(!session_found) {
		TZERR("session=0x%x not found\n", enc->session_id);
		ret = -EINVAL;
		goto return_func;
	}

	if(enc->encode_id != -1) {
		list_for_each_entry(enc_context,&temp_ses->encode_list, head) {
			if(enc_context->encode_id == enc->encode_id) {
				enc_found = 1;
				break;        
			}
		}
	}
	if(enc_found && enc_context) {
		if(enc_context->ker_req_data_addr) 
			kfree(enc_context->ker_req_data_addr);

		if(enc_context->ker_res_data_addr) 
			kfree(enc_context->ker_res_data_addr);

		list_del(&enc_context->head);

		kfree(enc_context->meta);
		kfree(enc_context);
	}
return_func: 
	return ret;
}
/**
 * @brief 
 *
 * Same as otz_client_kernel_operation_release() but this is for a 
 * non-secure user application. So copy_from_user() is used.
 *
 * @param private_data - device file ID
 * @param argp - Encode structure
 *
 * @return 
 */
static int otz_client_operation_release(void* private_data, void *argp)
{
	struct otz_client_encode_cmd enc;
	int ret =0;

	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}
	ret = otz_client_kernel_operation_release(private_data,&enc);
	if (ret != 0){
		TZERR("Error in release\n");
	}
return_func: 
	return ret;
}

/**
 * @brief 
 *
 * Prepares and initializes the encode context.
 *
 * @param private_data
 * @param enc
 * @param penc_context
 * @param psession
 *
 * @return 
 */
static int otz_client_prepare_encode( void* private_data,
		struct otz_client_encode_cmd *enc,
		otzc_encode **penc_context,
		otzc_session **psession) 
{
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc = NULL;
	otzc_session *temp_ses = NULL;
	otzc_encode *enc_context = NULL;
	int  session_found = 0, enc_found = 0;
	int ret = 0;
	u32 dev_file_id = (u32)private_data;

	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){
			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == enc->service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id == enc->session_id) {
							TZINFO("enc cmd ses id %d \n",temp_ses->session_id);
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}

	UNLOCK(file_list_lock);

	if(!session_found) {
		TZERR("session not found, dev_id=0x%x, svc_id=0x%x, ses_id=0x%x", 
				dev_file_id, enc->service_id, enc->session_id);
		ret = -EINVAL;
		goto return_func;
	}

	if(enc->encode_id != -1) {
		list_for_each_entry(enc_context,&temp_ses->encode_list, head) {
			if(enc_context->encode_id == enc->encode_id) {
				enc_found = 1;
				break;        
			}
		}
	}

	if(!enc_found) {
		enc_context = kmalloc(sizeof(otzc_encode), GFP_KERNEL);
		if(!enc_context) {
			TZERR("kmalloc failed \n");
			ret = -ENOMEM;
			goto return_func;
		}
		enc_context->meta = kmalloc(sizeof(struct otzc_encode_meta ) * 
				(OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS),
				GFP_KERNEL);
		if(!enc_context->meta) {
			TZERR("kmalloc failed \n");
			kfree(enc_context);
			ret = -ENOMEM;
			goto return_func;
		}
		enc_context->encode_id = (int)enc_context;
		enc->encode_id = enc_context->encode_id;
		enc_context->ker_req_data_addr = NULL;
		enc_context->ker_res_data_addr = NULL;
		enc_context->enc_req_offset = 0;
		enc_context->enc_res_offset = 0;
		enc_context->enc_req_pos = 0;
		enc_context->enc_res_pos = OTZ_MAX_REQ_PARAMS;
		enc_context->dec_res_pos = OTZ_MAX_REQ_PARAMS;
		enc_context->dec_offset = 0;
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
		enc_context->wait_data.send_wait_flag = 0; 
		init_waitqueue_head(&enc_context->wait_data.send_cmd_wq);
#endif
		list_add_tail(&enc_context->head, &temp_ses->encode_list);    
	}

	*penc_context = enc_context;
	*psession = temp_ses;

return_func:
	return ret;
}

/**
 * @brief 
 *
 * Funcion to encode uint32
 * 
 * @param private_data
 * @param argp
 * 
 * @return 
 */
static int otz_client_encode_uint32(void* private_data, void* argp)
{
	struct otz_client_encode_cmd enc;
	int ret = 0;
	otzc_session *session;
	otzc_encode *enc_context;


	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_encode(private_data, &enc, &enc_context, &session);

	if(ret){
		goto return_func;
	}

	if(enc.param_type == OTZC_PARAM_IN) {
		if(!enc_context->ker_req_data_addr) {
			enc_context->ker_req_data_addr = 
				kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_req_data_addr) {
				TZERR("kmalloc failed \n");
				ret =  -ENOMEM;
				goto ret_encode_u32;
			}
		}
		if( (enc_context->enc_req_offset + sizeof(u32) <= OTZ_1K_SIZE) &&
				(enc_context->enc_req_pos < OTZ_MAX_REQ_PARAMS)) {
			*((u32*)(enc_context->ker_req_data_addr + 
					enc_context->enc_req_offset)) = *((u32*)enc.data);
			enc_context->enc_req_offset += sizeof(u32);
			enc_context->meta[enc_context->enc_req_pos].type 
				= OTZ_ENC_UINT32;
			enc_context->meta[enc_context->enc_req_pos].len = sizeof(u32);
			enc_context->enc_req_pos++;
			if (otz_status.debug > 2) {
				TZERR("smc enc_uint32: data=0x%x\n", *((u32*)enc.data));
			}
		}
		else {
			ret =  -ENOMEM;/* Check this */
			goto ret_encode_u32;
		}
	}
	else if(enc.param_type == OTZC_PARAM_OUT) {
		if(!enc_context->ker_res_data_addr) {
			enc_context->ker_res_data_addr = 
				kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_res_data_addr) {
				TZERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_u32;
			}
		}
		if( (enc_context->enc_res_offset + sizeof(u32) <= OTZ_1K_SIZE) &&
				(enc_context->enc_res_pos < 
				 (OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS ))) {

			if(enc.data != NULL) {
				enc_context->meta[enc_context->enc_res_pos].usr_addr 
					= (u32)enc.data;
			}
			else {
				enc_context->meta[enc_context->enc_res_pos].usr_addr = 0;
			}
			enc_context->enc_res_offset += sizeof(u32);
			enc_context->meta[enc_context->enc_res_pos].type = OTZ_ENC_UINT32;
			enc_context->meta[enc_context->enc_res_pos].len = sizeof(u32);
			enc_context->enc_res_pos++;
		}
		else {
			ret =  -ENOMEM; /* check this */
			goto ret_encode_u32;
		}
	}


ret_encode_u32:
	if(copy_to_user(argp, &enc, sizeof(enc))){
		TZERR("copy from user failed \n");
		return -EFAULT;
	}

return_func:
	return ret;
}

/**
 * @brief 
 *
 * Function to encode an array
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_encode_array(void* private_data, void* argp)
{
	struct otz_client_encode_cmd enc;
	int ret = 0;
	otzc_encode *enc_context;
	otzc_session *session;

	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_encode(private_data, &enc, &enc_context, &session);

	if(ret){
		goto return_func;
	}
	TZINFO("enc_id 0x%x\n",enc_context->encode_id);

	if(enc.param_type == OTZC_PARAM_IN) {
		if(!enc_context->ker_req_data_addr) {
			TZINFO("allocate req data\n");
			enc_context->ker_req_data_addr = kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_req_data_addr) {
				TZERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_array;
			}
		}
		TZINFO("append encode data\n");

		if((enc_context->enc_req_offset + enc.len <= OTZ_1K_SIZE) &&
				(enc_context->enc_req_pos < OTZ_MAX_REQ_PARAMS)) {
			if(copy_from_user(
						enc_context->ker_req_data_addr + enc_context->enc_req_offset, 
						enc.data , 
						enc.len)) {
				TZERR("copy from user failed \n");
				ret = -EFAULT;
				goto ret_encode_array;
			}
			enc_context->enc_req_offset += enc.len;

			enc_context->meta[enc_context->enc_req_pos].type = OTZ_ENC_ARRAY;
			enc_context->meta[enc_context->enc_req_pos].len = enc.len;
			enc_context->enc_req_pos++;
		}
		else {
			ret = -ENOMEM; /* Check this */
			goto ret_encode_array;
		}
	}
	else if(enc.param_type == OTZC_PARAM_OUT) {
		if(!enc_context->ker_res_data_addr) {
			enc_context->ker_res_data_addr = kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_res_data_addr) {
				TZERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_array;
			}
		}
		if((enc_context->enc_res_offset + enc.len <= OTZ_1K_SIZE) &&
				(enc_context->enc_res_pos < 
				 (OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS ))) {
			if(enc.data != NULL) {
				enc_context->meta[enc_context->enc_res_pos].usr_addr 
					= (u32)enc.data;
			}
			else {
				enc_context->meta[enc_context->enc_res_pos].usr_addr = 0;
			}
			enc_context->enc_res_offset += enc.len;
			enc_context->meta[enc_context->enc_res_pos].type = OTZ_ENC_ARRAY;
			enc_context->meta[enc_context->enc_res_pos].len = enc.len;

			enc_context->enc_res_pos++;
		}
		else {
			ret = -ENOMEM;/* Check this */
			goto ret_encode_array;
		}
	}

ret_encode_array:
	if(copy_to_user(argp, &enc, sizeof(enc))){
		TZERR("copy from user failed \n");
		return -EFAULT;
	}

return_func:
	return ret;
}


/**
 * @brief 
 *
 * Function to encode a memory reference (from kernel)
 *
 * @param private_data
 * @param argp
 *
 * @return 
 */
static int otz_client_kernel_encode_mem_ref(void *private_data, void *argp)
{


	int ret = 0, shared_mem_found = 0;
	otzc_encode *enc_context;
	otzc_session *session;
	otzc_shared_mem* temp_shared_mem;
	struct otz_client_encode_cmd *enc = (struct otz_client_encode_cmd *)argp;

	ret = otz_client_prepare_encode(private_data, enc, &enc_context, &session);

	if(ret){
		goto return_func;
	}
	list_for_each_entry(temp_shared_mem, &session->shared_mem_list,s_head){
		if(temp_shared_mem->index == (u32*)enc->data){
			shared_mem_found = 1;
			break;
		}
	}
	LOCK(file_list_lock);
	if(!shared_mem_found) {
		otzc_dev_file *temp_dev_file;
		list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
				head) {
			if(temp_dev_file->dev_file_id == (u32)private_data){
				break;
			}
		}
		list_for_each_entry(temp_shared_mem, 
				&temp_dev_file->dev_shared_mem_head.shared_mem_list ,head) {
			TZINFO("dev id : %d shrd_mem_index : 0x%x\n",
					temp_dev_file->dev_file_id, temp_shared_mem->index);
			if(temp_shared_mem->index == (u32*)enc->data){
				shared_mem_found = 1;
				break;
			}
		}
	}
	UNLOCK(file_list_lock);

	if(!shared_mem_found) {

		TZERR("shared memory not registered for \
				this session 0x%x\n", session->session_id);
		ret = -EINVAL;
		goto return_func;
	}
	if(enc->param_type == OTZC_PARAM_IN) {
		if(!enc_context->ker_req_data_addr) {
			enc_context->ker_req_data_addr = kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_req_data_addr) {
				TZERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_array;
			}
		}

		if((enc_context->enc_req_offset + sizeof(u32) <= 
					OTZ_1K_SIZE) &&
				(enc_context->enc_req_pos < OTZ_MAX_REQ_PARAMS)) {
			*((u32*)(enc_context->ker_req_data_addr + 
					enc_context->enc_req_offset)) 
				= virt_to_phys(temp_shared_mem->k_addr+enc->offset);
			enc_context->enc_req_offset += sizeof(u32);
			enc_context->meta[enc_context->enc_req_pos].usr_addr
				= (u32)(temp_shared_mem->u_addr + enc->offset);
			enc_context->meta[enc_context->enc_req_pos].type = OTZ_MEM_REF;
			enc_context->meta[enc_context->enc_req_pos].len = enc->len;

			enc_context->enc_req_pos++;
		}
		else {
			ret = -ENOMEM; /* Check this */
			goto ret_encode_array;
		}
	}
	else if(enc->param_type == OTZC_PARAM_OUT) {
		if(!enc_context->ker_res_data_addr) {
			enc_context->ker_res_data_addr = kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_res_data_addr) {
				TZERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_array;
			}
		}
		if((enc_context->enc_res_offset + sizeof(u32)
					<= OTZ_1K_SIZE) &&
				(enc_context->enc_res_pos < 
				 (OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS ))) {
			*((u32*)(enc_context->ker_res_data_addr + 
					enc_context->enc_res_offset)) 
				= virt_to_phys(temp_shared_mem->k_addr + enc->offset);
			enc_context->enc_res_offset += sizeof(u32);
			enc_context->meta[enc_context->enc_res_pos].usr_addr
				= (u32)(temp_shared_mem->u_addr + enc->offset);
			enc_context->meta[enc_context->enc_res_pos].type 
				=  OTZ_MEM_REF;
			enc_context->meta[enc_context->enc_res_pos].len = enc->len;
			enc_context->enc_res_pos++;
		}
		else {
			ret = -ENOMEM; /*Check this */
			goto ret_encode_array;
		}
	}
ret_encode_array:

return_func:
	return ret;
}


/**
 * @brief 
 *
 * Function to encode a memory reference (from user application)
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_encode_mem_ref(void* private_data, void* argp)
{
	struct otz_client_encode_cmd enc;
	int ret = 0;

	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}
	ret = otz_client_kernel_encode_mem_ref(private_data,&enc);
	if(enc.encode_id != -1){
		if(copy_to_user(argp, &enc, sizeof(enc))){
			TZERR("copy from user failed \n");
			return -EFAULT;
		}
	}
return_func:
	return ret;
}

/**
 * @brief 
 *
 * This function prepares and initializes the decode context
 *
 * @param dec
 * @param pdec_context
 *
 * @return 
 */
static int otz_client_prepare_decode(void* private_data, 
		struct otz_client_encode_cmd *dec,
		otzc_encode **pdec_context) 
{
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc = NULL;
	otzc_session *temp_ses = NULL;
	otzc_encode *dec_context = NULL;
	int  session_found = 0, enc_found = 0;
	int ret = 0;
	u32 dev_file_id = (u32)private_data;

	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == dec->service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list, 
							head) {
						if(temp_ses->session_id == dec->session_id) {
							TZINFO("enc cmd ses id %d \n",temp_ses->session_id);
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}
	UNLOCK(file_list_lock);

	if(!session_found) {
		TZERR("session not found session_id=0x%x\n", dec->session_id);
		ret = -EINVAL;
		goto return_func;
	}

	if(dec->encode_id != -1) {
		list_for_each_entry(dec_context,&temp_ses->encode_list, head) {
			if(dec_context->encode_id == dec->encode_id){
				enc_found = 1;
				break;        
			}
		}
	}

	if(!enc_found) {
		ret =  -EINVAL;
		goto return_func;
	}

	*pdec_context = dec_context;
return_func:
	return ret;
}

/**
 * @brief 
 *
 * Function to decode uint32
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_decode_uint32(void* private_data, void* argp)
{
	struct otz_client_encode_cmd dec;
	int ret = 0;
	otzc_encode *dec_context;


	if(copy_from_user(&dec, argp, sizeof(dec))) {
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_decode(private_data, &dec, &dec_context);

	if(ret) {
		goto return_func;
	}

	if((dec_context->dec_res_pos <= dec_context->enc_res_pos) && 
			(dec_context->meta[dec_context->dec_res_pos].type
			 == OTZ_ENC_UINT32)){ 

		if(dec_context->meta[dec_context->dec_res_pos].usr_addr) {
			dec.data = 
				(void*)dec_context->meta[dec_context->dec_res_pos].usr_addr;
		}

		*(u32*)dec.data =  *((u32*)(dec_context->ker_res_data_addr
					+ dec_context->dec_offset));
		dec_context->dec_offset += sizeof(u32);
		dec_context->dec_res_pos++;
	}
	if(copy_to_user(argp, &dec, sizeof(dec))){
		TZERR("copy to user failed \n");
		return -EFAULT;
	}

return_func:
	return ret;
}

static int otz_client_kernel_decode_array_space(void *private_data, void *argp) __attribute__((used));
/**
 * @brief 
 *
 * Function to decode an array
 *
 * @param private_data
 * @param argp
 *
 * @return 
 */
static int otz_client_kernel_decode_array_space(void *private_data, void *argp)
{
	struct otz_client_encode_cmd *dec = NULL;
	int ret = 0;
	otzc_encode *dec_context;
	dec = (struct otz_client_encode_cmd *)argp;

	ret = otz_client_prepare_decode(private_data, dec, &dec_context);
	if(ret){
		goto return_func;
	}
	if((dec_context->dec_res_pos <= dec_context->enc_res_pos) && 
			(dec_context->meta[dec_context->dec_res_pos].type 
			 == OTZ_MEM_REF)) {
		if (dec_context->meta[dec_context->dec_res_pos].len >=
				dec_context->meta[dec_context->dec_res_pos].ret_len) {
			dec->data = 
				(void*)dec_context->meta[dec_context->dec_res_pos].usr_addr;
		}
		else {
			TZERR("buffer length is small. Length required %d \
					and supplied length %d\n", 
					dec_context->meta[dec_context->dec_res_pos].ret_len,
					dec_context->meta[dec_context->dec_res_pos].len);
			ret = -EFAULT;/* Check this */
			goto return_func;
		}

		dec->len = dec_context->meta[dec_context->dec_res_pos].ret_len;
		dec_context->dec_offset += sizeof(u32);
		dec_context->dec_res_pos++;
	}

	else {
		TZERR("invalid data type or decoder at wrong position\n");
		ret = -EINVAL;
		goto return_func;
	}

return_func:
	return ret;


}

/**
 * @brief 
 *
 * Function to decode an array (from user application)
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_decode_array_space(void* private_data, void* argp)
{
	struct otz_client_encode_cmd dec;
	int ret = 0;
	otzc_encode *dec_context;


	if(copy_from_user(&dec, argp, sizeof(dec))) {
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_decode(private_data, &dec, &dec_context);

	if(ret){
		goto return_func;
	}

	if((dec_context->dec_res_pos <= dec_context->enc_res_pos) && 
			(dec_context->meta[dec_context->dec_res_pos].type 
			 == OTZ_ENC_ARRAY)) {
		if (dec_context->meta[dec_context->dec_res_pos].len >=
				dec_context->meta[dec_context->dec_res_pos].ret_len) {
			if(dec_context->meta[dec_context->dec_res_pos].usr_addr) {
				dec.data = 
					(void*)dec_context->meta[dec_context->dec_res_pos].usr_addr;
			}
			if(copy_to_user(dec.data, 
						dec_context->ker_res_data_addr + dec_context->dec_offset,
						dec_context->meta[dec_context->dec_res_pos].ret_len)){
				TZERR("copy from user failed while copying array\n");
				ret = -EFAULT;
				goto return_func;
			} 
		}
		else {
			TZERR("buffer length is small. Length required %d "
					"and supplied length %d\n",
					dec_context->meta[dec_context->dec_res_pos].ret_len,
					dec_context->meta[dec_context->dec_res_pos].len);
			ret = -EFAULT; /* check this */
			goto return_func;
		}

		dec.len = dec_context->meta[dec_context->dec_res_pos].ret_len;
		dec_context->dec_offset +=  
			dec_context->meta[dec_context->dec_res_pos].len;
		dec_context->dec_res_pos++;
	}
	else if((dec_context->dec_res_pos <= dec_context->enc_res_pos) && 
			(dec_context->meta[dec_context->dec_res_pos].type 
			 == OTZ_MEM_REF)) {
		if (dec_context->meta[dec_context->dec_res_pos].len >=
				dec_context->meta[dec_context->dec_res_pos].ret_len) {
			dec.data = 
				(void*)dec_context->meta[dec_context->dec_res_pos].usr_addr;
		}
		else {
			TZERR("buffer length is small. Length required %d "
					"and supplied length %d\n",
					dec_context->meta[dec_context->dec_res_pos].ret_len,
					dec_context->meta[dec_context->dec_res_pos].len);
			ret = -EFAULT;/* Check this */
			goto return_func;
		}

		dec.len = dec_context->meta[dec_context->dec_res_pos].ret_len;
		dec_context->dec_offset += sizeof(u32);
		dec_context->dec_res_pos++;
	}

	else {
		TZERR("invalid data type or decoder at wrong position\n");
		ret = -EINVAL;
		goto return_func;
	}

	if(copy_to_user(argp, &dec, sizeof(dec))){
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

return_func:
	return ret;
}

/**
 * @brief 
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_get_decode_type(void* private_data, void* argp)
{
	struct otz_client_encode_cmd dec;
	int ret = 0;
	otzc_encode *dec_context;


	if(copy_from_user(&dec, argp, sizeof(dec))){
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_decode(private_data, &dec, &dec_context);

	if(ret){
		goto return_func;
	}

	TZINFO("decoder pos 0x%x and encoder pos 0x%x\n",
			dec_context->dec_res_pos, dec_context->enc_res_pos);

	if(dec_context->dec_res_pos <= dec_context->enc_res_pos) 
		dec.data = (void*)dec_context->meta[dec_context->dec_res_pos].type;
	else {
		ret = -EINVAL; /* check this */
		goto return_func;
	}

	if(copy_to_user(argp, &dec, sizeof(dec))){
		TZERR("copy to user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

return_func:
	return ret;
}

static int otz_client_kernel_shared_mem_alloc(void *private_data, void *argp,
		struct otzc_shared_mem *sh_mem) __attribute__((used));
/**
 * @brief 
 *
 * @param private_data
 * @param argp
 * @param sh_mem
 *
 * @return 
 */
static int otz_client_kernel_shared_mem_alloc(void *private_data, void *argp,
		struct otzc_shared_mem *sh_mem)
{

	struct otz_session_shared_mem_info *mem_info;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc = NULL;
	otzc_session *temp_ses = NULL;
	otzc_shared_mem* temp_shared_mem = sh_mem;
	int  session_found = 0;
	int ret = 0;
	u32 dev_file_id = (u32)private_data;
	mem_info = (struct otz_session_shared_mem_info *)argp;
	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == mem_info->service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id ==
								mem_info->session_id) {
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}
	UNLOCK(file_list_lock);

	if(!session_found){
		TZERR("Session not found!!\n");
		ret = -1;
		return ret;
	}
	/* If sessions is found..add the shared mem region to the session
	 * specific list */


	list_add_tail( &temp_shared_mem->s_head ,
			&temp_ses->shared_mem_list);
	return ret;

}

/**
 * @brief 
 *
 * Registers the shared memory from the device list to the session list. This is
 * because when we mmap, we cannot specify the session to which the memory has
 * to be mapped because the parameters do not allow us. So during mmap,
 * the memory is mapped to the device. Here, the memory which was mapped to
 * the device is mapped to the session shared memory list.
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_shared_mem_alloc(void* private_data, void* argp)
{
	struct otz_session_shared_mem_info mem_info;
	int ret = 0;

	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_shared_mem* temp_shared_mem;
	int  session_found = 0;
	u32 dev_file_id = (u32)private_data;
	if(copy_from_user(&mem_info, argp, sizeof(mem_info))){
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}    


	TZINFO("service id 0x%x session id 0x%x user mem addr 0x%x \n", 
			mem_info.service_id,
			mem_info.session_id,
			mem_info.user_mem_addr);



	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == mem_info.service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id ==
								mem_info.session_id) {
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}
	UNLOCK(file_list_lock);

	if(!session_found) {
		TZERR("session not found\n");
		ret = -EINVAL;
		goto return_func;
	}

	list_for_each_entry(temp_shared_mem,
			&temp_dev_file->dev_shared_mem_head.shared_mem_list , head){
		if(temp_shared_mem->index == (u32*)mem_info.user_mem_addr){
			list_del(&temp_shared_mem->head);
			temp_dev_file->dev_shared_mem_head.shared_mem_cnt--;
			list_add_tail( &temp_shared_mem->s_head ,
					&temp_ses->shared_mem_list);
			break;
		}
	}
return_func:
	return ret;
}

/**
 * @brief 
 *
 * Frees the shared memory for a particular session for the device
 *
 * @param private_data - Contains the device file ID
 * @param argp - Contains shared memory information for the session
 *
 * @return 
 */
static int otz_client_shared_mem_free(void* private_data, void* argp)
{
	otzc_shared_mem* temp_shared_mem;
	struct otz_session_shared_mem_info mem_info;

	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;

	int  session_found = 0;
	int ret = 0;
	u32 dev_file_id = (u32)private_data;

	if(copy_from_user(&mem_info, argp, sizeof(mem_info))){
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}     

	TZINFO("service id 0x%x session id 0x%x user mem addr 0x%x \n", 
			mem_info.service_id,
			mem_info.session_id,
			mem_info.user_mem_addr);
	LOCK(file_list_lock);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == mem_info.service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id == mem_info.session_id) {
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}
	UNLOCK(file_list_lock);

	if(!session_found) {
		TZERR("session not found\n");
		ret = -EINVAL;
		goto return_func;
	}

	list_for_each_entry(temp_shared_mem, &temp_ses->shared_mem_list,s_head){
		if(temp_shared_mem->index == (u32*)mem_info.user_mem_addr){
			list_del(&temp_shared_mem->s_head);

			if(temp_shared_mem->k_addr)
				free_pages((u32)temp_shared_mem->k_addr,
						get_order(ROUND_UP(temp_shared_mem->len, SZ_4K)));

			if(temp_shared_mem)
				kfree(temp_shared_mem);            
			break;
		}
	}
return_func:
	return ret;
}

/**
 * @brief Free shared mem. For TEE API. Not to be confused with otz_client_shared_mem_free.
 *
 * @param argp
 *
 * @return
 */
static int otz_client_tee_shared_mem_free(void* private_data, void* argp)
{
	otzc_shared_mem* temp_shared_mem;
	struct otz_session_shared_mem_info mem_info;
	otzc_dev_file *tem_dev_file;

	int ret = 0;
	int mem_found = 0;
	u32 dev_file_id = 0;

	if (copy_from_user(&mem_info, argp, sizeof(mem_info))) {
		TZERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	dev_file_id = (u32)(private_data);
	LOCK(file_list_lock);
	list_for_each_entry(tem_dev_file, &otzc_dev_file_head.dev_file_list, head) {
		if(tem_dev_file->dev_file_id == dev_file_id) {
			list_for_each_entry(temp_shared_mem, 
					&tem_dev_file->dev_shared_mem_head.shared_mem_list, head) {
				if (temp_shared_mem->fd == dev_file_id &&
						temp_shared_mem->index == (u32*)mem_info.user_mem_addr) {
					list_del(&temp_shared_mem->head);
					tem_dev_file->dev_shared_mem_head.shared_mem_cnt--;

					if(temp_shared_mem->k_addr) {
						free_pages((u32)temp_shared_mem->k_addr,
						get_order(ROUND_UP(temp_shared_mem->len, SZ_4K)));
						if (otz_status.debug > 0) {
							TZERR("FREE k_addr 0x%p, fd=%d", temp_shared_mem->k_addr, dev_file_id);
						}
					}
					if(temp_shared_mem)
						kfree(temp_shared_mem);
					mem_found = 1;
					break;
				}
			}
		}
	}
	UNLOCK(file_list_lock);

	if (!mem_found) {
		TZERR("mem not found\n");
		ret = -EINVAL;
	}
return_func:
	return ret;
}

/**
 * @brief Function which resolves the ioctl ID's and carries out
 * 			the corressponding task.
 *
 * @param file
 * @param cmd
 * @param arg
 *
 * @return 
 */
static long otz_client_ioctl(struct file *file, unsigned cmd,
		unsigned long arg)
{
	int ret = -EINVAL;
	void *argp = (void __user *) arg;

	switch (cmd) {
		case OTZ_CLIENT_IOCTL_SEND_CMD_REQ: 
			/* Only one client allowed here at a time */
			LOCK(send_cmd_lock);
			ret = otz_client_send_cmd(file->private_data, argp);
			UNLOCK(send_cmd_lock);

			if (ret)
				TZERR("failed otz_client_send_cmd: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_ENC_UINT32:
			/* Only one client allowed here
			 * at a time */
			LOCK(encode_cmd_lock);
			ret = otz_client_encode_uint32(file->private_data, argp);
			UNLOCK(encode_cmd_lock);
			if (ret)
				TZERR("failed otz_client_encode_cmd: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_DEC_UINT32:
			/* Only one client allowed here
			 * at a time */
			LOCK(decode_cmd_lock);
			ret = otz_client_decode_uint32(file->private_data, argp);
			UNLOCK(decode_cmd_lock);
			if (ret)
				TZERR("failed otz_client_decode_cmd: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_ENC_ARRAY:
			/* Only one client allowed here
			 * at a time */
			LOCK(encode_cmd_lock);
			ret = otz_client_encode_array(file->private_data, argp);
			UNLOCK(encode_cmd_lock);
			if (ret)
				TZERR("failed otz_client_encode_cmd: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_DEC_ARRAY_SPACE:
			/* Only one client allowed
			 * here at a time */
			LOCK(decode_cmd_lock);
			ret = 
				otz_client_decode_array_space(file->private_data, argp);
			UNLOCK(decode_cmd_lock);
			if (ret)
				TZERR("failed otz_client_decode_cmd: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_ENC_MEM_REF:
			/* Only one client allowed here at a time */
			LOCK(encode_cmd_lock);
			ret = otz_client_encode_mem_ref(file->private_data, argp);
			UNLOCK(encode_cmd_lock);
			if (ret)
				TZERR("failed otz_client_encode_cmd: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_ENC_ARRAY_SPACE:
			/* Only one client allowed here at a time */
			LOCK(encode_cmd_lock);
			ret = otz_client_encode_mem_ref(file->private_data, argp);
			UNLOCK(encode_cmd_lock);
			if (ret)
				TZERR("failed otz_client_encode_cmd: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_GET_DECODE_TYPE:
			/* Only one client allowed here at a time */
			LOCK(encode_cmd_lock);
			ret = otz_client_get_decode_type(file->private_data, argp);
			UNLOCK(encode_cmd_lock);
			if (ret)
				TZERR("failed otz_client_decode_cmd: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_SES_OPEN_REQ:
			/* Only one client allowed here at a time */
			LOCK(ses_open_lock);
			ret = otz_client_session_open(file->private_data, argp);
			UNLOCK(ses_open_lock);
			if (ret)
				TZERR("failed otz_client_session_open: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_SES_CLOSE_REQ:
			/* Only one client allowed here at a time */
			LOCK(ses_close_lock);
			ret = otz_client_session_close(file->private_data, argp);
			UNLOCK(ses_close_lock);
			if (ret)
				TZERR("failed otz_client_session_close: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_SHR_MEM_ALLOCATE_REQ:
			/* Only one client allowed here at a time */
			LOCK(mem_alloc_lock);
			ret = otz_client_shared_mem_alloc(file->private_data, argp);
			UNLOCK(mem_alloc_lock);
			if (ret)
				TZERR("failed otz_client_shared_mem_alloc: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_SHR_MEM_FREE_REQ:
			/* Only one client allowed here at a time */
			LOCK(mem_free_lock);
			ret = otz_client_shared_mem_free(file->private_data, argp);
			UNLOCK(mem_free_lock);
			if (ret)
				TZERR("failed otz_client_shared_mem_free: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_OPERATION_RELEASE:
			LOCK(operation_release_lock);
			ret = otz_client_operation_release(file->private_data, argp);
			UNLOCK(operation_release_lock);
			if (ret)
				TZERR("failed operation release: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_TEE_SHR_MEM_FREE_REQ:
			/* Only one client allowed here at a time */
			LOCK(mem_free_lock);
			ret = otz_client_tee_shared_mem_free(file->private_data, argp);
			UNLOCK(mem_free_lock);
			if (ret)
				TZERR("failed otz_client_tee_shared_mem_free: %d", ret);
			break;
		case OTZ_CLIENT_IOCTL_SVC_REGISTER_REQ:
			/* Only one client allowed here at a time */
			LOCK(ses_open_lock);
			ret = otz_client_service_register(file->private_data, argp);
			UNLOCK(ses_open_lock);
			if (ret)
				TZERR("failed otz_client_service_register: %d", ret);
			break;
 
		default:
			TZERR("invalid cmd: %d", cmd);
			return -EINVAL;
	}
	return ret;
}

/**
 * @brief 
 *
 * @param inode
 * @param file
 *
 * @return 
 */
static int otz_client_open(struct inode *inode, struct file *file)
{
	int ret;
	otzc_dev_file *new_dev;

	LOCK(open_lock);
	device_file_cnt++;
	file->private_data = (void*)device_file_cnt;

	if (otz_status.debug > 0) {
		TZDEBUG("OPEN device_file_id:%d", device_file_cnt);
	}
	new_dev = (otzc_dev_file*)kmalloc(sizeof(otzc_dev_file), GFP_KERNEL);
	if(!new_dev){
		TZERR("kmalloc failed for new dev file allocation\n");
		ret = -ENOMEM;
		goto ret_func;
	}
	new_dev->dev_file_id = device_file_cnt;
	new_dev->service_cnt = 0;
	INIT_LIST_HEAD(&new_dev->services_list);

	memset(&new_dev->dev_shared_mem_head, 0, sizeof(otzc_shared_mem_head));
	new_dev->dev_shared_mem_head.shared_mem_cnt = 0;
	INIT_LIST_HEAD(&new_dev->dev_shared_mem_head.shared_mem_list);


	LOCK(file_list_lock);
	list_add(&new_dev->head, &otzc_dev_file_head.dev_file_list);
	otzc_dev_file_head.dev_file_cnt++;
	UNLOCK(file_list_lock);

	if((ret = otz_client_service_init(new_dev, OTZ_SVC_GLOBAL)) != 0) {
		goto ret_func;
	}

ret_func:
	UNLOCK(open_lock);
	return ret;
}

/**
 * @brief 
 *
 * @param inode
 * @param file
 *
 * @return 
 */
static int otz_client_release(struct inode *inode, struct file *file)
{
	u32 dev_file_id = (u32)file->private_data;
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	int ret;
	ret = otz_smc_call(dev_file_id, OTZ_SVC_GLOBAL, 
			OTZ_GLOBAL_CMD_ID_UNREGISTER_NOTIFY_MEMORY, 
			0, 0,
			NULL, 0, NULL, 
			0, NULL, NULL, NULL, NULL);      

	if(ret != SMC_SUCCESS) {
		TZERR("Shared memory un-registration for \
				secure world notification failed\n");
	}

#endif

	otz_client_service_exit(file->private_data);
	if (otz_status.debug > 0) {
		TZDEBUG("RELEASE device_file_id: %d", dev_file_id);
	}

	LOCK(file_list_lock);
	if(list_empty(&otzc_dev_file_head.dev_file_list)){
	}
	UNLOCK(file_list_lock);
	return 0;
}

/**
 * @brief 
 *
 * @return 
 */
static int otz_client_smc_init(void)
{
	u32 ctr;

	asm volatile("mrc p15, 0, %0, c0, c0, 1" : "=r" (ctr));
	cacheline_size =  4 << ((ctr >> 16) & 0xf);

	return 0;
}


/**
 * @brief 
 */
static const struct file_operations otz_client_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = otz_client_ioctl,
	.open = otz_client_open,
	.mmap = otz_client_mmap,
	.release = otz_client_release
};

static ssize_t debug_show(struct class *class, struct class_attribute *attr, char *buf)
{
	int ret = 0;
	ret = sprintf(buf, "%s\ncurrent debug=%d\n",
			"trustzone debug:\n"
			"    0. disable trustzone debug.\n"
			"    1. basic trustzone debug level.\n"
			"    2. more trustzone smc info.\n"
			"    3. trace trustzone smc call.\n",
			otz_status.debug);

	return ret;
}

static ssize_t debug_store(struct class *class, struct class_attribute *attr, const char *buf, size_t count)
{
	unsigned int cmd = 0;

	cmd = simple_strtoul(buf, NULL, 0);
	switch (cmd) {
	case 0:
	case 1:
	case 2:
	case 3:
		break;

	default:
		printk("unknow command.\n");
		return count;
	}

	printk("set trustzone debug %d->%d\n", otz_status.debug, cmd);
	otz_status.debug = cmd;
	return count;
}

static ssize_t status_show(struct class *class, struct class_attribute *attr, char *buf)
{
	int ret = 0;
	int idx = 0;
	int cnt = 0;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_shared_mem *temp_shared_mem, *temp_pos;

	/* dump device list info */
	printk("trustzone device status:\ndev_file_count: %d\n", otzc_dev_file_head.dev_file_cnt);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list, head) {
		printk("device_file_id=%d, service_cnt=%d, services_list=0x%p\n",
				temp_dev_file->dev_file_id,
				temp_dev_file->service_cnt,
				&(temp_dev_file->services_list));
		list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
			list_for_each_entry(temp_ses, &temp_svc->sessions_list, head) {
				printk("\tservice_id=0x%x, sessions_list=0x%p\n",
						temp_svc->service_id,
						&(temp_svc->sessions_list));
				printk("\t\tsession_id=0x%x (%d, %s)\n",
						temp_ses->session_id,
						temp_ses->pid,
						temp_ses->comm);
			}
		}
	}

	/* dump shared mem list info */
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list, head) {
		list_for_each_entry_safe(temp_shared_mem, temp_pos,
					&temp_dev_file->dev_shared_mem_head.shared_mem_list, head) {
			printk("\tshared_mem index=0x%p, k_addr=0x%p, u_addr=0x%p, len=0x%x, fd=%u\n",
					temp_shared_mem->index,
					temp_shared_mem->k_addr,
					temp_shared_mem->u_addr,
					temp_shared_mem->len,
					temp_shared_mem->fd);
		}
	}

	/* dump trace list info */
	if (otz_status.debug > 1) {
		printk("secure_total_count: %d\n", otz_status.count);
		printk("secure_total_time: %12llu us\n", otz_status.usecs);
		printk("normal_total_time: %12llu us\n", ktime_to_us(ktime_get()) - otz_status.usecs);
		if (otz_status.index > 0) {
			idx = 0;
			printk("\tfd=%04d, svc_id=0x%03x, ses_id=0x%04x, cmd_id=0x%02x, time=%08llu us. (pid=%04d, %s)\n\n",
					otz_status.nodes[idx].fd,
					otz_status.nodes[idx].svc_id,
					otz_status.nodes[idx].ses_id,
					otz_status.nodes[idx].cmd_id,
					otz_status.nodes[idx].usecs,
					otz_status.nodes[idx].pid,
					otz_status.nodes[idx].comm);

			idx = otz_status.index;
			while (cnt < 10 && (otz_status.nodes[idx].fd > 0)) {
				idx++;
				cnt++;
				if (idx > 10)
					idx = 1;
				printk("\tfd=%04d, svc_id=0x%03x, ses_id=0x%04x, cmd_id=0x%02x, time=%08llu us. (pid=%04d, %s)\n",
						otz_status.nodes[idx].fd,
						otz_status.nodes[idx].svc_id,
						otz_status.nodes[idx].ses_id,
						otz_status.nodes[idx].cmd_id,
						otz_status.nodes[idx].usecs,
						otz_status.nodes[idx].pid,
						otz_status.nodes[idx].comm);
			}
		}
	}

	return ret;
}

static ssize_t status_store(struct class *class, struct class_attribute *attr, const char *buf, size_t count)
{
	unsigned int cmd = 0;

	cmd = simple_strtoul(buf, NULL, 0);
	switch (cmd) {
	case 0:
		printk("reset trustzone status.\n");
		memset(&otz_status, 0x0, sizeof(otz_status));
		break;

	default:
		printk("unknow command.\n");
		return count;
	}

	return count;
}

static ssize_t version_show(struct class *class, struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", OTZONE_DRIVER_VERSION);
}

static void otz_version_show(void)
{
	printk("OTZ driver version: %s\n", OTZONE_DRIVER_VERSION);
}

static ssize_t service_show(struct class *class, struct class_attribute *attr, char *buf)
{
	int ret = 0;
	int i = 0;

	printk("registered service count: %d\n", otz_register_mgr.count);
	for (i = 0; i < otz_register_mgr.count; i++) {
		printk("\tservice_id: 0x%03x, service_name: %s\n", otz_register_mgr.nodes[i].svc_id, otz_register_mgr.nodes[i].svc_name);
	}
	return ret;
}

static ssize_t mem_show(struct class *class, struct class_attribute *attr, char *buf)
{
	int ret = 0;

	ret = otz_memory_debug();

	return ret;
}

static struct class_attribute otz_attrs[] = {
	__ATTR(debug, S_IWUSR | S_IRUGO, debug_show, debug_store),
	__ATTR(status, S_IWUSR | S_IRUGO, status_show, status_store),
	__ATTR(version, S_IWUSR | S_IRUGO, version_show, NULL),
	__ATTR(service, S_IWUSR | S_IRUGO, service_show, NULL),
	__ATTR(mem, S_IWUSR | S_IRUGO, mem_show, NULL),
};

static int otz_attrs_create(struct class* class)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < sizeof(otz_attrs) / sizeof(otz_attrs[0]); i++) {
		if (class_create_file(class, &otz_attrs[i]) < 0) {
			TZERR("class_create_file failed\n");
			break;
		}
	}

	return ret;
}

static int otz_attrs_remove(struct class* class)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < sizeof(otz_attrs) / sizeof(otz_attrs[0]); i++) {
		class_remove_file(class, &otz_attrs[i]);
	}

	return ret;
}

/**
 * @brief 
 *
 * @return 
 */
static int otz_client_init(void)
{
	int ret_code = 0;
	struct device *class_dev;

	TZINFO("open otzclient init");
	otz_version_show();
	otz_client_smc_init();

	ret_code = alloc_chrdev_region(&otz_client_device_no, 0, 1,
			OTZ_CLIENT_DEV);
	if (ret_code < 0) {
		TZERR("alloc_chrdev_region failed %d", ret_code);
		return ret_code;
	}

	driver_class = class_create(THIS_MODULE, OTZ_CLIENT_DEV);
	if (IS_ERR(driver_class)) {
		ret_code = -ENOMEM;
		TZERR("class_create failed %d", ret_code);
		goto unregister_chrdev_region;
	}

	ret_code = otz_attrs_create(driver_class);

	class_dev = device_create(driver_class, NULL, otz_client_device_no, NULL,
			OTZ_CLIENT_DEV);
	if (!class_dev) {
		TZERR("class_device_create failed %d", ret_code);
		ret_code = -ENOMEM;
		goto class_destroy;
	}

	cdev_init(&otz_client_cdev, &otz_client_fops);
	otz_client_cdev.owner = THIS_MODULE;

	ret_code = cdev_add(&otz_client_cdev,
			MKDEV(MAJOR(otz_client_device_no), 0), 1);
	if (ret_code < 0) {
		TZERR("cdev_add failed %d", ret_code);
		goto class_device_destroy;
	}

	/* Initialize structure for services and sessions*/
	TZINFO("Initializing list for services\n");
	LOCK(file_list_lock);
	memset(&otzc_dev_file_head, 0, sizeof(otzc_dev_file_head));
	otzc_dev_file_head.dev_file_cnt = 0;
	INIT_LIST_HEAD(&otzc_dev_file_head.dev_file_list);

	INIT_LIST_HEAD(&otzc_shared_sessions_head);
	UNLOCK(file_list_lock);

	/* Allocate service shared memory */
	otz_alloc_svc_shared_memory();

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	sw_register_ipi_handler(0, 0, &non_secure_async_wakeup, NULL);
#endif
	goto return_fn;

class_device_destroy:
	device_destroy(driver_class, otz_client_device_no);
class_destroy:
	class_destroy(driver_class);
unregister_chrdev_region:
	unregister_chrdev_region(otz_client_device_no, 1);
return_fn:
	return ret_code;
}

/**
 * @brief 
 */
static void otz_client_exit(void)
{
	TZINFO("otz_client exit");

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	sw_unregister_ipi_handler(0);
#endif
	device_destroy(driver_class, otz_client_device_no);
	otz_attrs_remove(driver_class);
	class_destroy(driver_class);
	unregister_chrdev_region(otz_client_device_no, 1);

	/* free service shared memory */
	otz_free_svc_shared_memory();
}


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Platform-BJ@amlogic.com");
MODULE_DESCRIPTION("Amlogic TrustZone Driver");
MODULE_VERSION(OTZONE_DRIVER_VERSION);

module_init(otz_client_init);

module_exit(otz_client_exit);
