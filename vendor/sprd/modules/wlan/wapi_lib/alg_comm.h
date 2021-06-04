/**************************************************************
* Copyright (c) 2001,���������ͨ��������ͨ�����޹�˾
* All rights reserved.
* 
* �ļ����ƣ�alg_comm.h
* ժ    Ҫ������WAPID���õĵ��㷨ʵ�ֺ�����ԭ��
* 
* ��ǰ�汾��1.1
* ��    �ߣ����»�yhwang@iwncomm.com
* ������ڣ�2005��6��10��
*
* ȡ���汾��1.0 
* ԭ����  �����»�yhwang@iwncomm.com
* ������ڣ�2004��1��10��
*************************************************************/

#ifndef _ALG_COMM_H
#define _ALG_COMM_H

void update_gnonce(unsigned long *gnonce, int type);
int overflow(unsigned long * gnonce);
void add(unsigned long *a, unsigned long b, unsigned short len);
//void smash_random(unsigned char *buffer, int len );
void get_random(unsigned char *buffer, int len);
int mhash_sha256(unsigned char *data, unsigned length, unsigned char *digest);
void KD_hmac_sha256(unsigned char *text,unsigned text_len,unsigned char *key,
					unsigned key_len, unsigned char  *output,unsigned length);
int wapi_hmac_sha256(unsigned char *text, int text_len, 
				unsigned char *key, unsigned key_len,
				unsigned char *digest, unsigned digest_length);

#endif
