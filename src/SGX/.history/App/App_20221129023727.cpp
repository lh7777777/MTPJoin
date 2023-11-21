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


#include <stdio.h>
#include <string.h>
#include <assert.h>
# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX

#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"
//#include "mbuffer.h"
#include "ds_mgr.h"

#include <iostream>
//#include <openssl/aes.h>
//#include <openssl/md5.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <vector>
#include <fstream>
#include <time.h> 
#include <random>
#include <set>
#include <queue>

using namespace std;
using std::make_pair;
using std::pair;
using std::set;
using std::stoi;
using std::vector;


extern const int NUM_PAGE_TOTAL;
extern const int PAGE_SIZE;
extern const int MBUFFER_SIZE;
//extern bool LOG_DEBUG_ON;


/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    
    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
/*void ocall_print_string(const char *str)
{

    printf("%s", str);
}*/

DataStorageMgr* ds_mgr;

void ocall_writepage(int page_id, char* buffer_field)
{
    MBuf_pool mbuffer_pool;
    memcpy(mbuffer_pool.field, buffer_field, 4096);
    ds_mgr->WritePage(page_id,mbuffer_pool);


}

void ocall_readpage(int page_id,char* buffer_field)
{
    MBuf_pool mbuffer_pool;
    ds_mgr->ReadPage(page_id,mbuffer_pool);
    memcpy(buffer_field, mbuffer_pool.field, 4096);

}


auto ha = [](const char *str) -> void {
    printf("%s\n", str);
};


// 初始化所有记录到磁盘
void ds_init() {

    //LOG_DEBUG_ON = true;
    printf("PAGE_SIZE: %d, NUM_DATA = %d\n", PAGE_SIZE, NUM_PAGE_TOTAL);
    //printf("stderr >> log.out\n");
    //printf("stdout >> data.out\n");
    //freopen("File/log.out", "w", stderr);
    //freopen("File/data.out", "w", stdout);
    //fflush(stdout);

    /*for(int i=1;i<=EBUFFER_SIZE;i++)
    {
        MBuf_id mbuf_id;
        mbuf_id.page_id = (i-1)/4 + 1;
        mbuf_id.offset = (i-1)%4;
        node2page[i] = mbuf_id;
    }*/
    
    ha("Start Writing Data Into Pages");
    DataStorageMgr init_dsmgr = DataStorageMgr(true);
    //MBuffer init_mbuffer(&init_dsmgr);
    MBuf_pool tmpbuffer;

    for (int i = 0; i < NUM_PAGE_TOTAL; i++) {
        memset(tmpbuffer.field, 0, sizeof(tmpbuffer.field));
        for (int j = 0; j < MBUFFER_SIZE; j++)
            tmpbuffer.field[j] = 'a' + (j % 26);//使用一个页大小的数据测试
        //init_mbuffer.FixNewPage(tmpbuffer);
        init_dsmgr.WriteNewPage(tmpbuffer);
    }
    ha("Data Written, Start Job");
}
/*
auto itf = [&](deque <k_r*>&e) {
	int _s = e.size();
	for (int i = 0; i < _s; ++i) {
		printf("%d ", e[i]->k);
	}
};
*/


// 执行任务
void m2estart() {

    ds_mgr = new DataStorageMgr(false);
    //MBuffer* mbuffer;
    //mbuffer = new MBuffer;//MBuffer
    //printf("app mbuffer size: %d\n",mbuffer.size);
    
    ecall_init(global_eid,5);

    ha("Start Reading Command Data");
    // 读取数据到内存
    ha("Command Data Read, Start Processing");
    // 处理读写命令，每次都输出命中率
    //MB mb;

    MBuf_des* mbuf_des;
    MBuf_pool* mbuf_pool;
    //memset(mbuf_des, -1, sizeof(mbuf_des));
    //memset(mbuf_pool, -1, sizeof(mbuf_pool));
    mbuf_des = new MBuf_des[MBUFFER_NUM_MAX_SIZE];
    mbuf_pool = new MBuf_pool[MBUFFER_NUM_MAX_SIZE];

    ifstream de("File/test.in");
    while(!de.eof())
    {
        int cmd=0, key=0, rid=0;
        de >> cmd >> key;
        if(cmd == 0)
        {
            //cout<<"mb.buffer2page[0]: "<<mb.buffer2page[0]<<endl;
            int rid = 0;
            ecall_search(global_eid,&rid,key,mbuf_des,mbuf_des,1024);
            cout<<"找到了！rid: "<<rid<<endl;
        }
        if (cmd == 1) 
        {
            de >> rid;
            k_r key_rid(key,rid);//key-value对
            ecall_insert(global_eid,&key_rid,mbuf_des,mbuf_pool,1024);
            cout<<"插入完毕！"<<endl;
            cout<<"mb.mbuf_des[0].page_id: "<<mbuf_des[0].page_id<<endl;
        }
    }
    de.close();

    //tree.ebuffer->WriteDirtys();
    ecall_traversal(global_eid);

    ha("Processing Over, Job Done!");
}


/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);


    /* Initialize the enclave */
    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }

    // 写入数据
    ds_init();
    // 执行任务
    m2estart();
    
    //freopen("/dev/tty", "w", stdout);
    //printf("OK!\n");

	
    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    

    return 0;
}
