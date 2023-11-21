#if !defined(BUFFER_MGR_H)
#define BUFFER_MGR_H

#include "mbuf_header.h"
#include "ds_mgr.h"
#include "ebuf_header.h"

//#include <algorithm>
//#include <cstring>
//#include <cstdio>
//#include <string>
//#include <time.h>
//#include <unordered_map>
//#include <map>
//#include <vector>


//#define BUFFER_NUM_MAX_SIZE 1024

map<int,MBuf_id> node2page;//ebuffer到mbuffer的地址转换表


//const bool create_file = false;
//DataStorageMgr ds_mgr = DataStorageMgr(create_file);//Disk
DataStorageMgr* ds_mgr;

struct MBuffer {
private:
    unordered_map<int, int> page2buffer;
    int buffer2page[MBUFFER_NUM_MAX_SIZE];

    MBuf_des mbuf_des[MBUFFER_NUM_MAX_SIZE];
    MBuf_pool *mbuf_pool;
    int head_bid, tail_bid, size;

    //DataStorageMgr *ds_mgr;

    // 读/写页的操作数
    int tot_page_update_num;
    // 缓存命中次数
    int hit_num;
/*
    // 选择一个受害者
    int SelectVictim() {
        //LOG_DEBUG("MBuffer.SelectVictim, victim", tail_bid);
        return tail_bid;
    }
    void SetDirty(int buffer_id) {
        //LOG_DEBUG("MBuffer.SetDirty");
        mbuf_des[buffer_id].dirty = true;
    }
    // 将一个dirty的mbuffer写回磁盘
    void WriteBuffer(int buffer_id) {
        //LOG_DEBUG("MBuffer.WriteBuffer, buffer_id", buffer_id);
        //if (ds_mgr->WritePage(buffer2page[buffer_id], mbuf_pool[buffer_id]) == -1)
            //FAIL;
        ds_mgr->WritePage(buffer2page[buffer_id], mbuf_pool[buffer_id]);
    }
    // 从磁盘读一个page到mbuffer
    void ReadPage(int page_id, MBuf_pool &thebuffer) {
        //LOG_DEBUG("MBuffer.ReadPage, page_id", page_id);
        //if (ds_mgr->ReadPage(page_id, thebuffer) == -1)
            //FAIL;
        ds_mgr->ReadPage(page_id, thebuffer);
    }
    // LRU -- 更新了一个缓冲块（可能淘汰了旧的）
    //对于新插入的缓冲块，使用 [LRUInsert]
    void LRUUpdate(int readed_bid) {
        //LOG_DEBUG("MBuffer.LRUUpdate");
        if (tail_bid == readed_bid) { // 选了尾巴
            // 从尾巴处删除
            tail_bid = mbuf_des[readed_bid].prev_buffer_id;
            mbuf_des[tail_bid].next_buffer_id = tail_bid;
        } else { // 更新了中间某个缓冲块
            int prev = mbuf_des[readed_bid].prev_buffer_id;
            int next = mbuf_des[readed_bid].next_buffer_id;
            // 删除它
            mbuf_des[prev].next_buffer_id = next;
            mbuf_des[next].prev_buffer_id = prev;
        }
        // 加到首部
        mbuf_des[head_bid].prev_buffer_id = readed_bid;
        mbuf_des[readed_bid].next_buffer_id = head_bid;
        mbuf_des[readed_bid].prev_buffer_id = readed_bid;
        head_bid = readed_bid;
    }
    // LRU -- 插入了一个新缓冲块
    void LRUInsert(int inserted_bid) {
        //LOG_DEBUG("MBuffer.LRUInsert");
        if (size == 0) { // LRU 链表为空
            head_bid = tail_bid = inserted_bid;
            mbuf_des[head_bid].next_buffer_id = mbuf_des[head_bid].prev_buffer_id = inserted_bid;
        } else { // 链表不为空
            mbuf_des[head_bid].prev_buffer_id = inserted_bid;
            mbuf_des[inserted_bid].next_buffer_id = head_bid;
            mbuf_des[inserted_bid].prev_buffer_id = inserted_bid;
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
    }*/

public:
    //MBuffer(){}
    MBuffer(/*DataStorageMgr *ds_mgr*/) {
        memset(buffer2page, -1, sizeof(buffer2page));
        memset(mbuf_des, -1, sizeof(mbuf_des));
        head_bid = tail_bid = -1;
        size = 01;
        //this->ds_mgr = ds_mgr;
        mbuf_pool = new MBuf_pool[MBUFFER_NUM_MAX_SIZE];

        hit_num = tot_page_update_num = 0;
    }
    ~MBuffer() {
        //LOG_DEBUG("~MBuffer");
        delete[] mbuf_pool;
    }
/*
    // 读取page_id的内容到ebufferpool
    int FixPage(int node_id, EBuf_pool &thebuffer) {
        //cout<<"读mbuffer！"<<endl;
        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;
        int offset = mbuf_id.offset;

        //LOG_DEBUG("MBuffer.FixPage, page_id", page_id);
        //LOG_DEBUG("MBuffer.FixPage, offset", offset);

        IncTotPageUpdateNum();
        if (page2buffer.find(page_id) != page2buffer.end()) { // 在缓冲区
            printf("MBuffer HIT!\t");
            IncHitNum();
            int mbid = page2buffer[page_id];
            //memset(thebuffer.field,0,1024);
            memcpy(thebuffer.field, (mbuf_pool[mbid].field)+offset*1024, 1024);
            return 0;
        }
        printf("\t\t");
        // 不在缓冲区
        int target_bid = size;
        bool do_insert = false; // 确定执行更新还是插入操作

        if (size >= MBUFFER_NUM_MAX_SIZE) { // 缓冲区满
            target_bid = SelectVictim();
            if (mbuf_des[target_bid].dirty == 1) { // dirty
                WriteBuffer(target_bid);
            }
        } else { // 缓冲区未满
            target_bid = size;
            do_insert = true;
        }

        // 读数据
        MBuf_pool tmp_buffer;
        ReadPage(page_id, tmp_buffer);
        // 写数据到缓冲区，更新 Buf_des 和 LRU
        //tmp_buffer.set_field(mbuf_pool[target_bid]);
        mbuf_pool[target_bid].set_field(tmp_buffer);        
        mbuf_des[target_bid].update(page_id, target_bid, 0, false);
        do_insert ? LRUInsert(target_bid) : LRUUpdate(target_bid);
        do_insert ? size += 1 : 0;

        //memset(thebuffer.field,0,1024);
        memcpy(thebuffer.field, (mbuf_pool[target_bid].field)+offset*1024, 1024);

        // 更新哈希表
        page2buffer[page_id] = target_bid;
        buffer2page[target_bid] = page_id;
        return 0;
    }
    // 写入一个新记录到磁盘
    void FixNewPage(MBuf_pool &tmp_buffer) {
        //LOG_DEBUG("MBuffer.FixNewPage");
        //if (ds_mgr->WriteNewPage(tmp_buffer) == -1)
            //FAIL;
        ds_mgr->WriteNewPage(tmp_buffer);
    }

    // 更新某个page
    void UpdatePage(int node_id, EBuf_pool &thebuffer) {

        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;
        int offset = mbuf_id.offset;

        //LOG_DEBUG("MBuffer.UpdatePage, page_id", page_id);
        //LOG_DEBUG("MBuffer.UpdatePage, offset", offset);

        IncTotPageUpdateNum();
        if (page2buffer.find(page_id) != page2buffer.end()) { // 在缓冲区
            printf("MBuffer HIT!\t");
            IncHitNum();
            
            //int tmp_frm_id = page2buffer[page_id];
            //mbuf_pool[tmp_frm_id] = thebuffer;
            int mbid = page2buffer[page_id];
            //memset((mbuf_pool[mbid].field)+offset*1024,0,1024);
            memcpy((mbuf_pool[mbid].field)+offset*1024, thebuffer.field, 1024);

            SetDirty(mbid);
            LRUUpdate(mbid);
            return;
        } else { // 不在缓冲区
            printf("%d %d %s",node_id,page_id,"踢入MBuffer!\n");
            int target_bid = -1, ret = 0;
            bool do_insert = false;
            if (size == MBUFFER_NUM_MAX_SIZE) { // 缓冲区满
                target_bid = SelectVictim();
                if (mbuf_des[target_bid].dirty) { // dirty
                    ret = ds_mgr->WritePage(buffer2page[target_bid], mbuf_pool[target_bid]);
                    //if (ret == -1)
                        //FAIL;
                }
            } else { // 缓冲区未满
                target_bid = size;
                do_insert = true;
            }
            //ret = ds_mgr->ReadPage(page_id, mbuf_pool[target_bid]);
            //if (ret == -1)
                //FAIL;
            //memset((mbuf_pool[target_bid].field)+offset*1024,0,1024);
            memcpy((mbuf_pool[target_bid].field)+offset*1024, thebuffer.field, 1024);

            mbuf_des[target_bid].update(page_id, target_bid, 0, false);
            SetDirty(target_bid);
            do_insert ? LRUInsert(target_bid) : LRUUpdate(target_bid);
            do_insert ? size += 1 : 0;
            // 更新哈希表
            page2buffer[page_id] = target_bid;
            buffer2page[target_bid] = page_id;
        }
    }
    // count减一
    void UnfixPage(int node_id) {
        //LOG_DEBUG("MBuffer.UnfixPage");

        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;
        //int offset = mbuf_id.offset;

        if (page2buffer.find(page_id) != page2buffer.end())
            mbuf_des[page2buffer[page_id]].count -= 1;
    }
    // 获取命中率
    double GetHitRate() {
        return tot_page_update_num == 0 ? -1 : hit_num / (double)tot_page_update_num;
    }
    // 获取IO总次数
    long GetIONumTot() {
        return ds_mgr->GetTotalIO();
    }

    // 将所有的 dirty缓冲块写回磁盘
    void WriteDirtys() {
        //LOG_DEBUG("MBuffer.WriteDirtys");
        for (int bid = 0; bid < size; bid++) {
            if (mbuf_des[bid].dirty) {
                int ret = ds_mgr->WritePage(buffer2page[bid], mbuf_pool[bid]);
                //if (ret == -1)
                    //FAIL;
                mbuf_des[bid].dirty = false;
            }
        }
    }*/
};

#endif // BUFFER_MGR_H
