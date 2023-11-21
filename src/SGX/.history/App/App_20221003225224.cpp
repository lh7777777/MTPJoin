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

#include <iostream>
//#include <stdio.h>
//#include <openssl/aes.h>
//#include <openssl/md5.h>
//#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <vector>
#include <fstream>
#include  <time.h> 
using namespace std;

int mm = 0;//Label数
char ssk[17] = ""; //密钥
int ssklen = 0;//密钥长度字符数

FILE *dn1sgx;
FILE *dn2sgx;
FILE *dn3sgx;
FILE *sgxdn1;
FILE *sgxdn2;
FILE *sgxdn3;
FILE *sgxM1;
FILE *sgxM2;
FILE *sgxM3;
FILE *dn1label;
FILE *dn2label;
FILE *dn3label;


int o = 1;

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

 void ocall_opendn2sgx()
{
    dn2sgx = fopen("text/DN2_to_sgx.dat", "rb");
}
int ocall_readdn2sgx(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));
        int j=0;
		for (j = 0; j < 17; j++) {
			if (fscanf(dn2sgx, "%02x", &res[j]) == EOF) {
                
				return 1;//判断是否读到文件末尾
				break;
			}
		}

			for (int k = 0; k < 17; k++) {
				fres[k] = res[k];
			}
        return 0;
}
void ocall_closedn2sgx()
{
    cout << "DN2密文已接收完毕！" << endl;
	fclose(dn2sgx);
}

 void ocall_opendn3sgx()
{
    dn3sgx = fopen("text/DN3_to_sgx.dat", "rb");
}
int ocall_readdn3sgx(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));
        int j=0;
		for (j = 0; j < 17; j++) {
			if (fscanf(dn3sgx, "%02x", &res[j]) == EOF) {
                
				return 1;//判断是否读到文件末尾
				break;
			}
		}

			for (int k = 0; k < 17; k++) {
				fres[k] = res[k];
			}
        return 0;
}
void ocall_closedn3sgx()
{
    cout << "DN3密文已接收完毕！" << endl;
	fclose(dn3sgx);
}
//
 void ocall_opensgxdn1()
{
    sgxdn1 = fopen("text/sgx_to_DN1.dat", "wb");
}
void ocall_writesgxdn1(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));
		for (int i = 0; i < 17; i++)
		{
			fprintf(sgxdn1, "%02x", fres[i]);
			fprintf(sgxdn1, "\r\n");
		}
}
void ocall_closesgxdn1()
{
	fclose(sgxdn1);
}

 void ocall_opensgxdn2()
{
    sgxdn2 = fopen("text/sgx_to_DN2.dat", "wb");
}
void ocall_writesgxdn2(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));

		for (int i = 0; i < 17; i++)
		{
			fprintf(sgxdn2, "%02x", fres[i]);
			fprintf(sgxdn2, "\r\n");
		}
}
void ocall_closesgxdn2()
{
	fclose(sgxdn2);
}

 void ocall_opensgxdn3()
{
    sgxdn3 = fopen("text/sgx_to_DN3.dat", "wb");
}
void ocall_writesgxdn3(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));
		for (int i = 0; i < 17; i++)
		{
			fprintf(sgxdn3, "%02x", fres[i]);
			fprintf(sgxdn3, "\r\n");
		}
}
void ocall_closesgxdn3()
{
	fclose(sgxdn3);
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

ofstream outfile2;//输出M2
void ocall_openoutfile2()
{
	outfile2.open("text/sgx_to_DN2M2.txt");
}
void ocall_writeoutfile2(int m2)
{
	outfile2 << m2 << " ";
}
void ocall_closeoutfile2()
{
	outfile2.close();
}

ofstream outfile3;//输出M3
void ocall_openoutfile3()
{
	outfile3.open("text/sgx_to_DN3M3.txt");
}
void ocall_writeoutfile3(int m3)
{
	outfile3 << m3 << " ";
}
void ocall_closeoutfile3()
{
	outfile3.close();
}
//
void ocall_opendn1label()
{
    dn1label = fopen("text/DN1_to_sgxLabel.dat", "rb");
}
int ocall_readdn1label(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));
        int j=0;
		for (j = 0; j < 17; j++) {
			if (fscanf(dn1label, "%02x", &res[j]) == EOF) {
                
				return 1;//判断是否读到文件末尾
				break;
			}
		}

			for (int k = 0; k < 17; k++) {
				fres[k] = res[k];
			}
        return 0;
}
void ocall_closedn1label()
{
	fclose(dn1label);
}

void ocall_opendn2label()
{
    dn2label = fopen("text/DN2_to_sgxLabel.dat", "rb");
}
int ocall_readdn2label(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));
        int j=0;
		for (j = 0; j < 17; j++) {
			if (fscanf(dn2label, "%02x", &res[j]) == EOF) {
                
				return 1;//判断是否读到文件末尾
				break;
			}
		}

			for (int k = 0; k < 17; k++) {
				fres[k] = res[k];
			}
        return 0;
}
void ocall_closedn2label()
{
	fclose(dn2label);
}

void ocall_opendn3label()
{
    dn3label = fopen("text/DN3_to_sgxLabel.dat", "rb");
}
int ocall_readdn3label(unsigned char *fres)
{
    	unsigned int* res = (unsigned int*)malloc(17 * (sizeof(int)));
        int j=0;
		for (j = 0; j < 17; j++) {
			if (fscanf(dn3label, "%02x", &res[j]) == EOF) {
                
				return 1;//判断是否读到文件末尾
				break;
			}
		}


			for (int k = 0; k < 17; k++) {
				fres[k] = res[k];
			}
        return 0;
}
void ocall_closedn3label()
{
	fclose(dn3label);
}

ofstream outputfile;//输出结果
void ocall_openoutputfile()
{
	outputfile.open("text/output.txt");
}
void ocall_writeoutputfile(int m)
{
	outputfile << m << " ";
}
void ocall_writeoutputfile2(char *m)
{
	outputfile << m << endl;
}

void ocall_closeoutputfile()
{
	outputfile.close();
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

clock_t start2,stop2;
void ocall_startclock()
{
    start2 = clock();  
}
void ocall_endclock()
{
    stop2 = clock();  
}
int ocall_time()
{
    return  (int)(stop2 - start2);
}

void getmsk()
{
	ifstream de("text/c_to_sgx.txt");
	de >> ssk >> mm;
	de.close();
	cout << "密钥sk：" << ssk << " Label数量m:" << mm << endl;
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
    getmsk();
    ecall_init(global_eid,mm,ssk);

	//对三个DN分别进行部分排序
	ecall_update(global_eid,1);
	ecall_update(global_eid,2);
	ecall_update(global_eid,3);

	//最终排序
	ecall_orderby(global_eid);
	//输出排序结果
	ecall_output(global_eid);
	
    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    

    return 0;
}

