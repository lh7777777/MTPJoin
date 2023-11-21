#if !defined(BUFFER_MGR_H)
#define BUFFER_MGR_H

#include "mbuf_header.h"
#include "ebuf_header.h"
#include "BAddTreeNode.h"

#include "Enclave_t.h"
//#include "Enclave.h"
#include "Ocall_wrappers.h"

#include <algorithm>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>
#include <queue>


using namespace BATN;

map<int,MBuf_id> node2page;//ebuffer到mbuffer的地址转换表

char key[AES_BLOCK_SIZE];

struct MBuffer {
//private:
public:
    //unordered_map<int, int> page2buffer;
    //int buffer2page[MBUFFER_NUM_MAX_SIZE];
    //MBuf_des mbuf_des[MBUFFER_NUM_MAX_SIZE];
    //MBuf_pool *mbuf_pool;
    //MB* mb;
    int head_bid, tail_bid, size;

    // 读/写页的操作数
    int tot_page_update_num;
    // 缓存命中次数
    int hit_num;

    // 选择一个受害者
    int SelectVictim() {
        //LOG_DEBUG("MBuffer.SelectVictim, victim", tail_bid);
        return tail_bid;
    }
    void SetDirty(int buffer_id,MB* mb) {
        //LOG_DEBUG("MBuffer.SetDirty");
        //mbuf_des[buffer_id].dirty = true;
        mb->mbuf_des[buffer_id].dirty = true;
    }
    // 将一个dirty的mbuffer写回磁盘
    void WriteBuffer(int buffer_id,MB* mb) {
        //LOG_DEBUG("MBuffer.WriteBuffer, buffer_id", buffer_id);
        //if (ds_mgr->WritePage(buffer2page[buffer_id], mbuf_pool[buffer_id]) == -1)
            //FAIL;
        //ds_mgr->WritePage(buffer2page[buffer_id], mbuf_pool[buffer_id]);
        ocall_writepage(mb->buffer2page[buffer_id], mb->mbuf_pool[buffer_id].field);
    }
    // 从磁盘读一个page到mbuffer
    void ReadPage(int page_id, MBuf_pool &thebuffer) {
        //LOG_DEBUG("MBuffer.ReadPage, page_id", page_id);
        //if (ds_mgr->ReadPage(page_id, thebuffer) == -1)
            //FAIL;
        //ds_mgr->ReadPage(page_id, thebuffer);
        ocall_readpage(page_id, thebuffer.field);
    }
    // LRU -- 更新了一个缓冲块（可能淘汰了旧的）
    //对于新插入的缓冲块，使用 [LRUInsert]
    void LRUUpdate(int readed_bid,MB* mb) {
        //LOG_DEBUG("MBuffer.LRUUpdate");
        if (tail_bid == readed_bid) { // 选了尾巴
            // 从尾巴处删除
            tail_bid = mb->mbuf_des[readed_bid].prev_buffer_id;
            mb->mbuf_des[tail_bid].next_buffer_id = tail_bid;
        } else { // 更新了中间某个缓冲块
            int prev = mb->mbuf_des[readed_bid].prev_buffer_id;
            int next = mb->mbuf_des[readed_bid].next_buffer_id;
            // 删除它
            mb->mbuf_des[prev].next_buffer_id = next;
            mb->mbuf_des[next].prev_buffer_id = prev;
        }
        // 加到首部
        mb->mbuf_des[head_bid].prev_buffer_id = readed_bid;
        mb->mbuf_des[readed_bid].next_buffer_id = head_bid;
        mb->mbuf_des[readed_bid].prev_buffer_id = readed_bid;
        head_bid = readed_bid;
    }
    // LRU -- 插入了一个新缓冲块
    void LRUInsert(int inserted_bid,MB* mb) {
        //LOG_DEBUG("MBuffer.LRUInsert");
        if (size == 0) { // LRU 链表为空
            head_bid = tail_bid = inserted_bid;
            mb->mbuf_des[head_bid].next_buffer_id = mb->mbuf_des[head_bid].prev_buffer_id = inserted_bid;
        } else { // 链表不为空
            mb->mbuf_des[head_bid].prev_buffer_id = inserted_bid;
            mb->mbuf_des[inserted_bid].next_buffer_id = head_bid;
            mb->mbuf_des[inserted_bid].prev_buffer_id = inserted_bid;
            head_bid = inserted_bid;
        }
    }
    // 命中缓存，加一
    void IncHitNum() {
        hit_num += 1;
    }
    // 执行了依次读/写页
    void IncTotPageUpdateNum() {
        tot_page_update_num += 1;
    }

public:
    //MBuffer(){}
    MBuffer() {
        //memset(buffer2page, -1, sizeof(buffer2page));
        //memset(mbuf_des, -1, sizeof(mbuf_des));
        //mbuf_pool = new MBuf_pool[MBUFFER_NUM_MAX_SIZE];

        //this->mb = mb;
        //mb = new MB;
        head_bid = tail_bid = -1;
        size = 0;
        hit_num = tot_page_update_num = 0;
    }
    /*~MBuffer() {
        //LOG_DEBUG("~MBuffer");
        delete[] mbuf_pool;
    }*/


    // 读取page_id的内容到ebufferpool
    int FixPage(int node_id, EBuf_pool &thebuffer,MB* mb) {
        //cout<<"读mbuffer！"<<endl;
        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;
        int offset = mbuf_id.offset;

        printf("找的pageid,offset,bid：%d %d %d\n",page_id,offset,mb->page2buffer[page_id]);

        /*char dstStringTemp[1024];
        memset((char*)dstStringTemp, 0 ,1024);*/

        IncTotPageUpdateNum();
        if (mb->page2buffer.find(page_id) != mb->page2buffer.end()) { // 在缓冲区
            printf("MBuffer HIT!\t");
            IncHitNum();
            int mbid = mb->page2buffer[page_id];
            memcpy(thebuffer.field, (mb->mbuf_pool[mbid].field)+offset*1024, 1024);

            /*memcpy(dstStringTemp, (mb->mbuf_pool[mbid].field)+offset*1024, 1024);
            if(!aes_decrypt(dstStringTemp,key,thebuffer.field,1024))
            {
                printf("decrypt error\n");
            }*/
            return 0;
        }
        
        // 不在缓冲区
        int target_bid = size;
        bool do_insert = false; // 确定执行更新还是插入操作

        if (size >= MBUFFER_NUM_MAX_SIZE) { // 缓冲区满
            target_bid = SelectVictim();
            if (mb->mbuf_des[target_bid].dirty == 1) { // dirty
                WriteBuffer(target_bid,mb);
            }
        } else { // 缓冲区未满
            target_bid = size;
            do_insert = true;
        }

        // 读数据
        //MBuf_pool tmp_buffer;
        //ReadPage(page_id, tmp_buffer);
        //mbuf_pool[target_bid].set_field(tmp_buffer);  
        ReadPage(page_id, mb->mbuf_pool[target_bid]);

        mb->mbuf_des[target_bid].update(page_id, target_bid, 0, false);
        do_insert ? LRUInsert(target_bid,mb) : LRUUpdate(target_bid,mb);
        do_insert ? size += 1 : 0;

        memcpy(thebuffer.field, (mb->mbuf_pool[target_bid].field)+offset*1024, 1024);

        /*memcpy(dstStringTemp, (mb->mbuf_pool[target_bid].field)+offset*1024, 1024);
        if(!aes_decrypt(dstStringTemp,key,thebuffer.field,1024))
        {
            printf("decrypt error\n");
        }*/

        // 更新哈希表
        mb->page2buffer[page_id] = target_bid;
        mb->buffer2page[target_bid] = page_id;
        return 0;
    }

    // 更新某个page
    void UpdatePage(int node_id, EBuf_pool &thebuffer,MB* mb) {
        
        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;
        int offset = mbuf_id.offset;

        /*char dstStringTemp[1024];
        memset((char*)dstStringTemp, 0 ,1024);
        //int sour_len = strlen((char*)sourceStringTemp);
        if(!aes_encrypt(thebuffer.field,key,dstStringTemp,1024))
        {
            printf("encrypt error\n");
        }

        printf("enc %d:",strlen((char*)dstStringTemp));
        for(int i= 0;dstStringTemp[i];i+=1){
            printf("%x",(unsigned char)dstStringTemp[i]);
        }
        printf("\n");*/

        //LOG_DEBUG("MBuffer.UpdatePage, page_id", page_id);
        //LOG_DEBUG("MBuffer.UpdatePage, offset", offset);

        IncTotPageUpdateNum();
        if (mb->page2buffer.find(page_id) != mb->page2buffer.end()) { // 在缓冲区
            printf("MBuffer HIT!\n");
            IncHitNum();
            int mbid = mb->page2buffer[page_id];
            memcpy((mb->mbuf_pool[mbid].field)+offset*1024, thebuffer.field, 1024);
            //memcpy((mb->mbuf_pool[mbid].field)+offset*1024, dstStringTemp, 1024);

            SetDirty(mbid,mb);
            LRUUpdate(mbid,mb);
            //return;
        } else { // 不在缓冲区
            printf("%d %d %d %s",node_id,page_id,offset,"踢入MBuffer!\n");
            int target_bid = -1, ret = 0;
            bool do_insert = false;
            if (size >= MBUFFER_NUM_MAX_SIZE) { // 缓冲区满
                printf("mbuffer满了\n");
                target_bid = SelectVictim();
                if (mb->mbuf_des[target_bid].dirty) { // dirty
                    printf("踢入disk\n");
                    ocall_writepage(mb->buffer2page[target_bid], mb->mbuf_pool[target_bid].field);
                    //ret = ds_mgr->WritePage(buffer2page[target_bid], mbuf_pool[target_bid]);
                    //if (ret == -1)
                        //FAIL;
                }
            } else { // 缓冲区未满
                target_bid = size;
                do_insert = true;
            }

            printf("size: %d\n",size);
            
			//BAddTreeLeafNode<int, k_r>* node = (BAddTreeLeafNode<int, k_r>*) thebuffer.field;
            //printf("找到的叶子nodeid: %d\n",node->node_id);

            memcpy((mb->mbuf_pool[target_bid].field)+offset*1024, thebuffer.field, 1024);
            //memcpy((mb->mbuf_pool[target_bid].field)+offset*1024, dstStringTemp, 1024);
            mb->mbuf_des[target_bid].update(page_id, target_bid, 0, false);
            SetDirty(target_bid,mb);
            do_insert ? LRUInsert(target_bid,mb) : LRUUpdate(target_bid,mb);
            do_insert ? size += 1 : 0;
            // 更新哈希表
            mb->page2buffer[page_id] = target_bid;
            mb->buffer2page[target_bid] = page_id;
        }
    }
    // count减一
    void UnfixPage(int node_id,MB* mb) {
        //LOG_DEBUG("MBuffer.UnfixPage");

        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;
        //int offset = mbuf_id.offset;

        if (mb->page2buffer.find(page_id) != mb->page2buffer.end())
            mb->mbuf_des[mb->page2buffer[page_id]].count -= 1;
    }
    // 获取命中率
    double GetHitRate() {
        return tot_page_update_num == 0 ? -1 : hit_num / (double)tot_page_update_num;
    }
    // 获取IO总次数
    //long GetIONumTot() {
    //    return ds_mgr->GetTotalIO();
    //}

    // 将所有的 dirty缓冲块写回磁盘
    void WriteDirtys(MB* mb) {
        //LOG_DEBUG("MBuffer.WriteDirtys");
        for (int bid = 0; bid < size; bid++) {
            if (mb->mbuf_des[bid].dirty) {
                ocall_writepage(mb->buffer2page[bid], mb->mbuf_pool[bid].field);
                //int ret = ds_mgr->WritePage(buffer2page[bid], mbuf_pool[bid]);
                //if (ret == -1)
                    //FAIL;
                mb->mbuf_des[bid].dirty = false;
            }
        }
    }
};

#endif // BUFFER_MGR_H
