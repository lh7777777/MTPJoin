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

#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <vector>

using namespace BAT;

MBuffer* mbuffer;
BAddTree<int,k_r>* tree;

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

    for(int i=1;i<=EBUFFER_SIZE;i++)
    {
        MBuf_id mbuf_id;
        mbuf_id.page_id = (i-1)/4 + 1;
        mbuf_id.offset = (i-1)%4;
        node2page[i] = mbuf_id;
        //cout<<"pageid:"<<node2page[i].page_id<<"offset:"<<node2page[i].offset<<endl;
    }

    mbuffer = (MBuffer*) mb;
    //MBuffer* mbuffer;
    printf("mbuffer size: %d\n",mbuffer->size);
    tree = new BAddTree<int,k_r>(5,mbuffer);

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
    printf("\nebuffer size: %d\n",tree->ebuffer->size);
    printf("\ntree size: %d\n",tree->size());
    printf("start traversal:\n");
    tree->list_traversal(itf);
}
