#ifndef _APP_H
#define _APP_H

#include "rtthread.h"

/* ---------------������ƿ�----------------- */
static struct rt_mailbox upload_mb;
static rt_uint32_t upload_mbpoll[3];

/* ---------------�߳̿��ƿ�----------------- */
static rt_thread_t Updata_thread=RT_NULL;
static rt_thread_t Lora_thread=RT_NULL;
/* ---------------�����ʱ�����ƿ�----------------- */
static rt_timer_t loraupload_broadcast_timer;		//�����ϴ����ݹ㲥��ʱ��
static rt_timer_t loraupload_overtime_timer;		//�ϴ����ڳ�ʱ��ʱ��

/* ---------------�߳̽��̺���----------------- */
static void Loraupload_broadcast_entry(void* parameter);
static void Updata_thread_entry(void* parameter);
static void Lora_thread_entry(void* parameter);

void Main_APP(void);

#endif
