#ifndef _APP_H
#define _APP_H

#include "rtthread.h"

/* ---------------邮箱控制块----------------- */
static struct rt_mailbox upload_mb;
static rt_uint32_t upload_mbpoll[3];

/* ---------------线程控制块----------------- */
static rt_thread_t Updata_thread=RT_NULL;
static rt_thread_t Lora_thread=RT_NULL;
/* ---------------软件定时器控制块----------------- */
static rt_timer_t loraupload_broadcast_timer;		//周期上传数据广播定时器
static rt_timer_t loraupload_overtime_timer;		//上传窗口超时定时器

/* ---------------线程进程函数----------------- */
static void Loraupload_broadcast_entry(void* parameter);
static void Updata_thread_entry(void* parameter);
static void Lora_thread_entry(void* parameter);

void Main_APP(void);

#endif
