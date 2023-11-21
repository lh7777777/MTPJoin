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

//#include "baddtree.h"
#include <random>
#include <set>
#include <iostream>
#include <queue>

using namespace BAT;
using namespace std;
using std::make_pair;
using std::pair;
using std::set;
using std::stoi;
using std::vector;

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

void getmsk()
{
	ifstream de("text/c_to_sgx.txt");
	de >> ssk >> mm;
	de.close();
	cout << "密钥sk：" << ssk << " Label数量m:" << mm << endl;
}

int order=5;
extern const int NUM_PAGE_TOTAL;
extern const int PAGE_SIZE;
extern const int MBUFFER_SIZE;
const int EBUFFER_SIZE = 1024;
//extern bool LOG_DEBUG_ON;

auto ha = [](const char *str) -> void {
    printf("%s\n", str);
};

// 打印初始化配置信息，重定向IO
void init() {
    //LOG_DEBUG_ON = true;
    printf("BUFFER_SIZE = PAGE_SIZE: %d, NUM_DATA = %d\n", PAGE_SIZE, NUM_PAGE_TOTAL);
    //printf("stderr >> log.out\n");
    printf("stdout >> data.out\n");
    //freopen("./log.out", "w", stderr);
    freopen("../file/data.out", "w", stdout);
    fflush(stdout);
}

// 初始化所有记录到磁盘
void ds_init() {
    const bool create_file = true;
    DataStorageMgr dsmgr = DataStorageMgr(create_file);
    MBuffer mbuffer(&dsmgr);
    MBuf_pool tmpbuffer;
    
    for(int i=1;i<EBUFFER_SIZE;i++)
    {
        MBuf_id mbuf_id;
        mbuf_id.page_id = (i-1)/4 + 1;
        mbuf_id.offset = (i-1)%4;
        node2page[i] = mbuf_id;
        //cout<<"pageid:"<<node2page[i].page_id<<"offset:"<<node2page[i].offset<<endl;
    }

    ha("Start Writing Data Into Pages");
    for (int i = 0; i < NUM_PAGE_TOTAL; i++) {
        memset(tmpbuffer.field, 0, sizeof(tmpbuffer.field));
        for (int j = 0; j < MBUFFER_SIZE; j++)
            tmpbuffer.field[j] = 'a' + (j % 26);//使用一个页大小的数据测试
        mbuffer.FixNewPage(tmpbuffer);
    }
    ha("Data Written, Start Job");
}

auto itf = [&](deque <k_r*>&e) {
	int _s = e.size();
	for (int i = 0; i < _s; ++i) {
		printf("%d ", e[i]->key());
	}
};

// 执行任务
void m2estart() {
    const bool create_file = false;
    DataStorageMgr dsmgr = DataStorageMgr(create_file);//Disk
    MBuffer mbuffer(&dsmgr);//MBuffer
    //BAddTree<int,k_r> tree(order,&mbuffer);//EBuffer
    ha("Start Reading Command Data");
    // 读取数据到内存
    //FILE *in_fp = fopen("./data.in", "r+");
    FILE *in_fp = fopen("../file/test.in", "r+");
    if (in_fp == nullptr)
        FAIL;
    vector<pair<int, int>> cmds;
    while (true) {
        char line[12] = {};
        fgets(line, sizeof(line), in_fp);
        if (feof(in_fp))
            break;
        line[1] = '\0';
        auto tmp_data = make_pair(stoi(line), stoi(line + 2));
        cmds.push_back(tmp_data);
    }
    fclose(in_fp);
    ha("Command Data Read, Start Processing");
    // 处理读写命令，每次都输出命中率
    //EBuf_pool tmpbuffer;
    for (size_t i = 0; i < cmds.size(); i++) {
        memset(tmpbuffer.field, 0, sizeof(tmpbuffer.field));
        int tmpcmd = cmds[i].first;
        int tmpkey = cmds[i].second;
        
        // 读
        
        if (tmpcmd == 0){
            //k_r* kr = tree.find(tmpkey);
            cout<<"找到了！rid: "<<kr->rid<<endl;
            //cout<<"找到了！key: "<<kr->k<<endl;
        }
        // 写
        if (tmpcmd == 1) {
            //for (int j = 0; j < 5; j++)
                //tmpbuffer.field[j] = '|';
            //buffermgr.UpdatePage(tmppageid, tmpbuffer);

            k_r key_rid(tmpkey,i);//key-value对
            //cout<<"插入的key-rid: "<<key_rid.k<<" - "<<key_rid.rid<<endl;
            //tree.insert(key_rid);
        }
    }
    /*tree.ebuffer->node2buffer.erase(1);
    for (auto iter = tree.ebuffer->node2buffer.begin(); iter != tree.ebuffer->node2buffer.end(); ++iter) {
        cout << iter->first << " " << iter->second << endl;
    }*/

    //tree.ebuffer->WriteDirtys();
    cout << "\nebuffer size: " << tree.ebuffer->size << endl;
    cout << "\ntree size: " << tree.size() << endl;
    cout << "start  traversal:\n";
    tree.list_traversal(itf);
	cout << endl;

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
	//enclave获取m，sk
    //getmsk();
    //ecall_init(global_eid,mm,ssk);

    init();
    // 写入数据
    ds_init();
    // 执行任务
    m2estart();
    
    freopen("/dev/tty", "w", stdout);
    printf("OK!\n");

	
    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    

    return 0;
}

