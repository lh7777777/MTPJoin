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
#include "mbuffer.h"

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


int order=5;
//const bool create_file = false;
//DataStorageMgr dsmgr = DataStorageMgr(create_file);//Disk
//MBuffer mbuffer;//MBuffer

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
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}


void ocall_strcpy(char *DeStr,char *SoStr,size_t DeLen,size_t SoLen)
{
    if (DeLen > SoLen)
        DeLen = SoLen;
    if (DeLen)
        memcpy(DeStr,SoStr,DeLen);
}

void ocall_sprintf(char *pt, int ipt)
{
	sprintf(pt, "%d", ipt);
}

 void ocall_input(int input)
 {
	 cin>>input;
 }

/*
void ocall_opendn1sgx()
{
    dn1sgx = fopen("text/DN1_to_sgx.dat", "rb");
}
int ocall_readdn1sgx(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));
        int j=0;
		for (j = 0; j < 17; j++) {
			if (fscanf(dn1sgx, "%02x", &res[j]) == EOF) {
                
				return 1;//判断是否读到文件末尾
				break;
			}
		}

			for (int k = 0; k < 17; k++) {
				fres[k] = res[k];
			}
        return 0;
}
void ocall_closedn1sgx()
{
    cout << "DN1密文已接收完毕！" << endl;
	fclose(dn1sgx);
}


ofstream outfile1;//输出M1
void ocall_openoutfile1()
{
	outfile1.open("text/sgx_to_DN1M1.txt");
}
void ocall_writeoutfile1(int m1)
{
	outfile1 << m1 << " ";
}
void ocall_closeoutfile1()
{
	outfile1.close();
}

*/
clock_t start,stop;
void ocall_startclock1()
{
    start = clock();  
}
void ocall_endclock1()
{
    stop = clock();  
}
int ocall_time1()
{
    return  (int)(stop - start);
}


const bool create_file = false;
DataStorageMgr dsmgr = DataStorageMgr(create_file);//Disk
MBuffer mbuffer;//MBuffer

void ocall_writepage(int page_id, void* buffer_pool)
{
    MBuf_pool mbuffer_pool = (MBuf_pool &) buffer_pool;
    dsmgr.WritePage(page_id,mbuffer_pool);
}

void ocall_readpage(int page_id, void* buffer_pool)
{
    MBuf_pool mbuffer_pool = (MBuf_pool &) buffer_pool;
    dsmgr.ReadPage(page_id,mbuffer_pool);
}

void ocall_writenewpage(void* buffer_pool)
{
    MBuf_pool mbuffer_pool = (MBuf_pool &) buffer_pool;
    dsmgr.WriteNewPage(mbuffer_pool);
}


auto ha = [](const char *str) -> void {
    printf("%s\n", str);
};


// 初始化所有记录到磁盘
void ds_init() {

    //LOG_DEBUG_ON = true;
    printf("BUFFER_SIZE = PAGE_SIZE: %d, NUM_DATA = %d\n", PAGE_SIZE, NUM_PAGE_TOTAL);
    //printf("stderr >> log.out\n");
    //printf("stdout >> data.out\n");
    //freopen("../File/log.out", "w", stderr);
    //freopen("File/data.out", "w", stdout);
    
    //fflush(stdout);
    for(int i=1;i<=EBUFFER_SIZE;i++)
    {
        MBuf_id mbuf_id;
        mbuf_id.page_id = (i-1)/4 + 1;
        mbuf_id.offset = (i-1)%4;
        node2page[i] = mbuf_id;
        //cout<<"pageid:"<<node2page[i].page_id<<"offset:"<<node2page[i].offset<<endl;
    }
    
    ha("Start Writing Data Into Pages");
    const bool init_file = true;
    DataStorageMgr init_dsmgr = DataStorageMgr(init_file);
    //MBuffer init_mbuffer(&init_dsmgr);
    MBuffer init_mbuffer;
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

auto itf = [&](deque <k_r*>&e) {
	int _s = e.size();
	for (int i = 0; i < _s; ++i) {
		printf("%d ", e[i]->k);
	}
};

// 执行任务
void m2estart() {
    //const bool create_file = false;
    //DataStorageMgr dsmgr = DataStorageMgr(create_file);//Disk
    //MBuffer mbuffer;
    //MBuffer mbuffer(&dsmgr);//MBuffer
    //BAddTree<int,k_r> tree(order,&mbuffer);//EBuffer

    ecall_init(global_eid,order,&mbuffer);

    ha("Start Reading Command Data");
    // 读取数据到内存
    ha("Command Data Read, Start Processing");
    // 处理读写命令，每次都输出命中率
    ifstream de("File/test.in");
    while(!de.eof())
    {
        int cmd, key, rid;
        de >> cmd >> key;
        if(cmd == 0)
        {
            //k_r* kr = tree.find(key);
            //k_r* kr = ecall_search(key);
            k_r* kr;
            ecall_search(global_eid,key,kr);
            cout<<"找到了！rid: "<<kr->rid<<endl;
        }
        if (cmd == 1) 
        {
            de >> rid;
            k_r key_rid(key,rid);//key-value对
            //tree.insert(key_rid);
            ecall_insert(global_eid,&key_rid);//可以替换为int int
        }
    }
    de.close();

    //tree.ebuffer->WriteDirtys();
    //cout << "\nebuffer size: " << tree.ebuffer->size << endl;
    //cout << "\ntree size: " << tree.size() << endl;
    //cout << "start  traversal:\n";
    //tree.list_traversal(itf);
	//cout << endl;

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

