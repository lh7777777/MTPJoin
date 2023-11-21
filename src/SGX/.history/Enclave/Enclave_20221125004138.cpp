/*
 * Copyright (C) 2011-2020 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include "baddtree.h"


#include "Ocall_wrappers.h"
#include <openssl/aes.h>

#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <vector>

using namespace BAT;

#define AES_BITS 128
#define MSG_LEN 128

BAddTree<int,k_r>* tree;
//MBuffer* mbuffer;

int aes_encrypt(char* in, char* key, char* out,int len)//, int olen)可能会设置buf长度
{
    if(!in || !key || !out) return 0;
    unsigned char iv[AES_BLOCK_SIZE];//加密的初始化向量
    for(int i=0; i<AES_BLOCK_SIZE; ++i)//iv一般设置为全0,可以设置其他，但是加密解密要一样就行
        iv[i]=0;
    AES_KEY aes;
    if(AES_set_encrypt_key((unsigned char*)key, 128, &aes) < 0)
    {
        return 0;
    }
    //int len=strlen(in);//这里的长度是char*in的长度，但是如果in中间包含'\0'字符的话

    //那么就只会加密前面'\0'前面的一段，所以，这个len可以作为参数传进来，记录in的长度

    //至于解密也是一个道理，光以'\0'来判断字符串长度，确有不妥，后面都是一个道理。
    AES_cbc_encrypt((unsigned char*)in, (unsigned char*)out, len, &aes, iv, AES_ENCRYPT);
    return 1;
}
int aes_decrypt(char* in, char* key, char* out,int len)
{
    if(!in || !key || !out) return 0;
    unsigned char iv[AES_BLOCK_SIZE];//加密的初始化向量
    for(int i=0; i<AES_BLOCK_SIZE; ++i)//iv一般设置为全0,可以设置其他，但是加密解密要一样就行
        iv[i]=0;
    AES_KEY aes;
    if(AES_set_decrypt_key((unsigned char*)key, 128, &aes) < 0)
    {
        return 0;
    }
 //   int len=strlen(in);
    AES_cbc_encrypt((unsigned char*)in, (unsigned char*)out, len, &aes, iv, AES_DECRYPT);
    return 1;
}

int test()
{
    char sourceStringTemp[MSG_LEN];
    char dstStringTemp[MSG_LEN];
    memset((char*)sourceStringTemp, 0 ,MSG_LEN);
    memset((char*)dstStringTemp, 0 ,MSG_LEN);
    memcpy((char*)sourceStringTemp, "123456789 123456789 123456789 12a",MSG_LEN);
    //strcpy((char*)sourceStringTemp, argv[1]);

    char key[AES_BLOCK_SIZE];
    int i;
    for(i = 0; i < 16; i++)//可自由设置密钥
    {
        key[i] = 32 + i;
    }
    int sour_len = strlen((char*)sourceStringTemp);
    if(!aes_encrypt(sourceStringTemp,key,dstStringTemp,MSG_LEN))
    {
        printf("encrypt error\n");
        return -1;
    }
    printf("enc %d:",strlen((char*)dstStringTemp));
    for(i= 0;dstStringTemp[i];i+=1){
        printf("%x",(unsigned char)dstStringTemp[i]);
    }
    memset((char*)sourceStringTemp, 0 ,MSG_LEN);
    if(!aes_decrypt(dstStringTemp,key,sourceStringTemp,MSG_LEN))
    {
        printf("decrypt error\n");
        return -1;
    }
    printf("\n");
    printf("dec %d:",strlen((char*)sourceStringTemp));
    printf("%s\n",sourceStringTemp);
    for(i= 0;i < sour_len;i+=1){
        printf("%x",(unsigned char)sourceStringTemp[i]);
    }
    printf("\n");
    return 0;
}

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char* fmt, ...)
{
    char buf[BUFSIZ] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

void ecall_init(void* mb)
{

    test();

    for(int i=1;i<=EBUFFER_SIZE;i++)
    {
        MBuf_id mbuf_id;
        mbuf_id.page_id = (i-1)/4 + 1;
        mbuf_id.offset = (i-1)%4;
        node2page[i] = mbuf_id;
        //cout<<"pageid:"<<node2page[i].page_id<<"offset:"<<node2page[i].offset<<endl;
    }
    //mbuffer = (MBuffer*) mb;
    //printf("mbuffer size: %d\n",(MBuffer*)mb->size);
    //MBuffer* mbuffer = (MBuffer*)mb;
    tree = new BAddTree<int,k_r>(5,(MBuffer*)mb);

    //mb = mbuffer;

    printf("初始化完毕！\n");
}

int ecall_search(int key)
{
    printf("ecall_search\n");
    printf("rid: %d\n",tree->find(key)->rid);
    return tree->find(key)->rid;
    
}

void ecall_insert(void* key_rid)
{
    printf("ecall_insert\n");
    k_r* kr = (k_r*)key_rid;
    printf("插入key,rid:%d,%d\n",kr->k,kr->rid);
    //printf("111\n");
    tree->insert(*kr);
    
}

auto itf = [&](deque <k_r*>&e) {
	int _s = e.size();
	for (int i = 0; i < _s; ++i) {
		printf("%d ", e[i]->k);
	}
};

void ecall_traversal()
{
    printf("ebuffer size: %d\n",tree->ebuffer->size);
    printf("tree size: %d\n",tree->size());
    printf("start traversal:\n");
    tree->list_traversal(itf);
}
