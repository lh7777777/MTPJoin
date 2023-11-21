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
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string>

#include <stdlib.h>
#include <math.h>
#include <map>
#include <vector>
#include "aes.h"

int NB = 2000;//sgx内存大小
int m = 0;//Label数
char sk[17] = ""; //密钥
int sklen = 0;//密钥长度字符数


std::map<int,std:: string> DN1;//获取的密文
std::map<int, std::string> DN2;
std::map<int, std::string> DN3;
std::map<int, int> M1;//每个DN属于不同Label的数量
std::map<int, int> M2;
std::map<int, int> M3;
std::map<int,std:: vector<int>> Key_M1;
std::map<int, std::vector<int>> Key_M2;
std::map<int, std::vector<int>> Key_M3;
std::map<int, std::string> Sort_DN1;//部分排序后密文
std::map<int, std::string> Sort_DN2;
std::map<int, std::string> Sort_DN3;
std::map<int, std::string> Label_DN1;//按照Label获取的密文
std::map<int, std::string> Label_DN2;
std::map<int, std::string> Label_DN3;

std::map<int, std::vector<long long>> Lpt;

int o = 1;

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

//获取sk和m
void ecall_init(int mm,char* ssk)
{
	m=mm;
	ocall_strcpy(sk,ssk,17,17);
	sklen=strlen(sk);
}

//对每个DN的NB个密文进行部分排序
void part_sort(std::map<int,std::string> DN, int dnc)
{
	if (dnc == 1)
	{
		int t = 1;
		for (int k = 1; k <= m; k++)
		{
			
			if (M1[k] != 0)
			{
				for (int h = 0; h < M1[k]; h++)
				{
					
					Sort_DN1[t].append(DN[Key_M1[k][h]], 0, 17);
					t++;
				}
			}
		}

		Aes Decryption;
        Decryption.getKey(sk);
        Decryption.initAes();

		for (int i = 1; i <= Sort_DN1.size(); i++)
		{

			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文

			//AES解密
			Decryption.decode((unsigned char*)Sort_DN1[i].c_str(),(unsigned char*)pt2);

			mint = atoi(pt2);
		}

	}
	else if (dnc == 2)
	{
		int t = 1;
		for (int k = 1; k <= m; k++)
		{
			if (M2[k] != 0)
			{
				for (int h = 0; h < M2[k]; h++)
				{
					Sort_DN2[t].append(DN[Key_M2[k][h]], 0, 17);
					t++;
				}
			}
		}
		Aes Decryption;
        Decryption.getKey(sk);
        Decryption.initAes();

		for (int i = 1; i <= Sort_DN2.size(); i++)
		{
			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文

			//AES解密
			Decryption.decode((unsigned char*)Sort_DN2[i].c_str(),(unsigned char*)pt2);

			mint = atoi(pt2);
		}

	}
	else if (dnc == 3)
	{
		int t = 1;
		for (int k = 1; k <= m; k++)
		{
			if (M3[k] != 0)
			{
				for (int h = 0; h < M3[k]; h++)
				{
					Sort_DN3[t].append(DN[Key_M3[k][h]], 0, 17);
					t++;
				}
			}
		}

		Aes Decryption;
        Decryption.getKey(sk);
        Decryption.initAes();

		for (int i = 1; i <= Sort_DN3.size(); i++)
		{
			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文
			Decryption.decode((unsigned char*)Sort_DN3[i].c_str(),(unsigned char*)pt2);

			mint = atoi(pt2);
		}

	}

}
//获取DN发过来的NB个密文
void getDN(int dnc)
{
	if (dnc == 1)
	{
		int cn1 = 0;
		ocall_opendn1sgx();
		
		unsigned char* fres = (unsigned char*)malloc(17 * sizeof(char));

		int a1 = 0;
		while (1)
		{
			ocall_readdn1sgx(&a1,fres);

			if (a1 == 1)
				break;

			cn1++;
			DN1[cn1].assign((char*)fres,17);

		}
		ocall_closedn1sgx();

	}

	else if (dnc == 2)
	{
		int cn2 = 0;
		ocall_opendn2sgx();
		unsigned char* fres = (unsigned char*)malloc(17 * sizeof(char));

		int a2 = 0;
		while (1)
		{
			ocall_readdn2sgx(&a2,fres);

			if (a2 == 1)
				break;

			cn2++;
			DN2[cn2].assign((char*)fres,17);

		}
		ocall_closedn2sgx();

	}

	else if (dnc == 3)
	{
		int cn3 = 0;
		ocall_opendn3sgx();
		unsigned char* fres = (unsigned char*)malloc(17 * sizeof(char));

		int a3 = 0;
		while (1)
		{
			ocall_readdn3sgx(&a3,fres);

			if (a3 == 1)
				break;

			cn3++;
			DN3[cn3].assign((char*)fres,17);

		}
		ocall_closedn3sgx();
	}

}
//部分排序
void getSortDN(int dnc)
{
	if (dnc == 1)
	{
		//解密
		Aes Decryption;
        Decryption.getKey(sk);
        Decryption.initAes();

		for (int i = 1; i <= DN1.size(); i++)
		{
			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文

			Decryption.decode((unsigned char*)DN1[i].c_str(),(unsigned char*)pt2);

			mint = atoi(pt2);
			//按照label存入M1
			for (int j = 1; j <= m; j++)
			{
				if (mint <= j * NB && mint >= ((j - 1)*NB + 1))
				{
					M1[j]++;
					Key_M1[j].push_back(i);//把属于此label的key添加到此map中，方便后续排序
				}
				
			}

		}
		part_sort(DN1, dnc);

	}

	else if (dnc == 2)
	{
		//解密
		Aes Decryption;
        Decryption.getKey(sk);
        Decryption.initAes();

		for (int i = 1; i <= DN2.size(); i++)
		{
			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文

			Decryption.decode((unsigned char*)DN2[i].c_str(),(unsigned char*)pt2);

			mint = atoi(pt2);
			//按照label存入M2
			for (int j = 1; j <= m; j++)
			{
				if (mint <= j * NB && mint >= ((j - 1)*NB + 1))
				{
					M2[j]++;
					Key_M2[j].push_back(i);//把属于此label的key添加到此map中，方便后续排序
				}
			}
		}
		part_sort(DN2, dnc);

	}

	else if (dnc == 3)
	{

		Aes Decryption;
        Decryption.getKey(sk);
        Decryption.initAes();

		for (int i = 1; i <= DN3.size(); i++)
		{
			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文

			Decryption.decode((unsigned char*)DN3[i].c_str(),(unsigned char*)pt2);


			mint = atoi(pt2);
			//按照label存入M3
			for (int j = 1; j <= m; j++)
			{
				if (mint <= j * NB && mint >= ((j - 1)*NB + 1))
				{
					M3[j]++;
					Key_M3[j].push_back(i);//把属于此label的key添加到此map中，方便后续排序
				}
			}
		}
		part_sort(DN3, dnc);
		//把Mi和部分排序好的密文Sort_DNi发给DNi

	}

}
void ecall_update(int dnc)
{
	int input = 0;
	unsigned char* fres = (unsigned char*)malloc(17 * sizeof(char));
	while (1)
	{
		printf("接收DN %d，发送Mi、Sort_DNi给DN %d，请输入1：\n",dnc,dnc);
		ocall_input(input);

		getDN(dnc);//获得DNi

		ocall_startclock1();
		getSortDN(dnc);//获得Mi、Sort_DNi
		ocall_endclock1();
		int kk;
		ocall_time1(&kk);
		printf("部分排序DN %d密文用时 %d us\n" , dnc, kk);

		if (dnc == 1)
		{
			//清空输出的文件sgx_to_DN1.dat
			ocall_opensgxdn1();

			for (int t = 1; t <= Sort_DN1.size() ; t++)
			{
				fres=(unsigned char*)Sort_DN1[t].c_str();
				//传一个密文到文件中
				ocall_writesgxdn1(fres);

			}
			ocall_closesgxdn1();

			ocall_openoutfile1();
			for(int p=1;p<=M1.size();p++){
				ocall_writeoutfile1(M1[p]);
			}

			ocall_closeoutfile1();
			printf("********sgx已发送给DN1 M1、Sort_DN1！\n");

			if (DN1.size() != NB)
			{
				printf("Sort_DN1密文已全部发送完毕！\n\n");
				break;
			}
		}
		else if (dnc == 2)
		{
			//清空输出的文件sgx_to_DN2.dat
			ocall_opensgxdn2();

			for (int t = 1; t <= Sort_DN2.size()  ; t++)
			{
				fres=(unsigned char*)Sort_DN2[t].c_str();
				//传一个密文到文件中
				ocall_writesgxdn2(fres);

			}
			ocall_closesgxdn2();

			ocall_openoutfile2();
			for(int p=1;p<=M2.size();p++){
				ocall_writeoutfile2(M2[p]);
			}

			ocall_closeoutfile2();

			printf("********sgx已发送给DN2 M2、Sort_DN2！\n");
			

			if (DN2.size() != NB)
			{
				printf("Sort_DN2密文已全部发送完毕！\n\n");
				break;
			}
		}

		else if (dnc == 3)
		{
			ocall_opensgxdn3();

			for (int t = 1; t <= Sort_DN3.size() ; t++)
			{
				fres=(unsigned char*)Sort_DN3[t].c_str();
				//传一个密文到文件中
				ocall_writesgxdn3(fres);

			}
			ocall_closesgxdn3();

			ocall_openoutfile3();
			for(int p=1;p<=M3.size();p++){
				ocall_writeoutfile3(M3[p]);
			}
			ocall_closeoutfile3();

			printf("********sgx已发送给DN3 M3、Sort_DN3！\n");


			if (DN3.size() != NB)
			{
				printf("Sort_DN3密文已全部发送完毕！\n\n");
				break;
			}
		}

		DN1.clear();
		DN2.clear();
		DN3.clear();
		Sort_DN1.clear();
		Sort_DN2.clear();
		Sort_DN3.clear();
		M1.clear();
		M2.clear();
		M3.clear();
		Key_M1.clear();
		Key_M2.clear();
		Key_M3.clear();

	}
}

//获取DN发过来的每个Label的密文
void getlabel(int dnc)
{
	if (dnc == 1)
	{
		int cn1 = 0;
		ocall_opendn1label();
		unsigned char* fres = (unsigned char*)malloc(17 * sizeof(char));

		int a1 = 0;
		while (1)
		{
			ocall_readdn1label(&a1,fres);

			if (a1 == 1)
				break;

			cn1++;			
			Label_DN1[cn1].assign((char*)fres,17);

		}

	}
	else if (dnc == 2)
	{
		int cn2 = 0;
		ocall_opendn2label();
		unsigned char* fres = (unsigned char*)malloc(17 * sizeof(char));
		
		int a2 = 0;
		while (1)
		{
			ocall_readdn2label(&a2,fres);

			if (a2 == 1)
				break;

			cn2++;
			Label_DN2[cn2].assign((char*)fres,17);

		}

	}	
	else if (dnc == 3)
	{
		int cn3 = 0;
		ocall_opendn3label();
		unsigned char* fres = (unsigned char*)malloc(17 * sizeof(char));

		int a3 = 0;
		while (1)
		{
			ocall_readdn3label(&a3,fres);

			if (a3 == 1)
				break;

			cn3++;			
			Label_DN3[cn3].assign((char*)fres,17);

		}

	}
}
//快速排序
void quickSort(int left, int right, std::vector<long long>& arr)
{
	if (left >= right)
		return;
	int i, j, base, temp;
	i = left, j = right;
	base = arr[left];  //取最左边的数为基准数
	while (i < j)
	{
		while (arr[j] >= base && i < j)
			j--;
		while (arr[i] <= base && i < j)
			i++;
		if (i < j)
		{
			temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
		}
	}
	//基准数归位
	arr[left] = arr[i];
	arr[i] = base;
	quickSort(left, i - 1, arr);//递归左边
	quickSort(i + 1, right, arr);//递归右边
}

//使用Label_DNi实现最终排序
void ALL_orderby()
{
		getlabel(1);
		getlabel(2);
		getlabel(3);

		ocall_startclock();
		for (int j = 1; j <= Label_DN1.size(); j++)
		{

			Aes Decryption;
        	Decryption.getKey(sk);
        	Decryption.initAes();

			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文
			//AES解密
			Decryption.decode((unsigned char*)Label_DN1[j].c_str(), (unsigned char*)pt2);


			mint = atoi(pt2);
			Lpt[o].push_back(mint);
		}

		for (int j = 1; j <= Label_DN2.size(); j++)
		{

			Aes Decryption;
        	Decryption.getKey(sk);
        	Decryption.initAes();

			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文
			//AES解密
			Decryption.decode((unsigned char*)Label_DN2[j].c_str(), (unsigned char*)pt2);

			mint = atoi(pt2);
			Lpt[o].push_back(mint);
		}

		for (int j = 1; j <= Label_DN3.size(); j++)
		{

			Aes Decryption;
        	Decryption.getKey(sk);
        	Decryption.initAes();

			char pt2[17] = { 0 }; // 每一个AES明文
			int mint = 0;//每一个int明文

			//AES解密
			Decryption.decode((unsigned char*)Label_DN3[j].c_str(), (unsigned char*)pt2);

			mint = atoi(pt2);
			Lpt[o].push_back(mint);
		}


		quickSort(0, Lpt[o].size() - 1, Lpt[o]);//明文排序后Lpt
		ocall_endclock();

		o++;
		Label_DN1.clear();
		Label_DN2.clear();
		Label_DN3.clear();
}
void ecall_orderby()
{

	int label=0;
	int timecnt=0;
	for (int i = 1; i <= m; i++)
	{

		printf("\n排序三个DN的Label %d，请输入 %d：\n",i,i);
		ocall_input(label);
		ALL_orderby();//对所有密文进行排序

		int kk;
		ocall_time(&kk);
		printf("排序Label %d用时 %d us\n" , i , kk);
		timecnt += kk ;
	}
	printf("\n排序所有Label 共用时 %d us\n" , timecnt);

	ocall_closedn1label();
	ocall_closedn2label();
	ocall_closedn3label();
}

void ecall_output()
{
	ocall_openoutputfile();
	int t = 0;
	for (int i = 1; i <= m; i++)
	{
		for (int j = 0; j < Lpt[i].size(); j++)
		{
			t++;
			ocall_writeoutputfile(Lpt[i][j]);

			char pt[17] = { 0 };//每一个明文数字
			ocall_sprintf(pt, Lpt[i][j]);
			char ct[17] = { 0 }; // 每一个AES密文

			Aes Encryption;
            Encryption.getKey(sk);
            Encryption.initAes();
			Encryption.encode((unsigned char*)pt,(unsigned char*)ct);
			ocall_writeoutputfile2(ct);
		}
	}
	ocall_closeoutputfile();

	
}
