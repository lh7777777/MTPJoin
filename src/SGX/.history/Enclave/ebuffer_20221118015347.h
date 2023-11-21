#if !defined(EBUFFER_H)
#define EBUFFER_H

#include "ebuf_header.h"
#include "mbuffer.h"
//#include <algorithm>
//#include <cstring>
//#include <cstdio>
//#include <string>
//#include <time.h>
//#include <unordered_map>
//#include <map>
//#include <vector>
//#include <iostream>

using std::string;
using std::to_string;
using std::unordered_map;
using std::map;
using namespace std;


struct EBuffer {
//private:
public:
    //unordered_map<int, int> mbuffer2ebuffer; //？？？
    unordered_map<int,int> node2buffer;//哈希表
    int buffer2node[EBUFFER_NUM_MAX_SIZE];//回到node，直接调用ebuffer.nodeid
    //int node2page[EBUFFER_NUM_MAX_SIZE]; 
    

    EBuf_des ebuf_des[EBUFFER_NUM_MAX_SIZE];

    //BAddTreeNode<int,k_r> *buffer;
    EBuf_pool *ebuf_pool;
    int head_bid, tail_bid, size;

    //DataStorageMgr *ds_mgr;
    MBuffer *mbuffer;
    //map<int,mbuf_id> node2page;//ebuffer到mbuffer的地址转换表

    //用来建立ebuffer和nodeid之间的关系
    int blank_id;

    // 读/写页的操作数
    int tot_page_update_num;
    // 缓存命中次数
    int hit_num;

    // 选择一个受害者
    int SelectVictim() {
        //LOG_DEBUG("EBuffer.SelectVictim, victim", tail_bid);
        return tail_bid;
    }
    void SetDirty(int buffer_id) {
        //LOG_DEBUG("EBuffer.SetDirty");
        ebuf_des[buffer_id].dirty = true;
    }
    // 将一个dirty的ebuffer写回mbuffer
    void WriteBuffer(int buffer_id) {
        //LOG_DEBUG("EBuffer.WriteBuffer, buffer_id", buffer_id);
        mbuffer->UpdatePage(buffer2node[buffer_id], ebuf_pool[buffer_id]);      
    }
    // 从mbuffer读一个page到ebuffer
    void ReadPage(int node_id, EBuf_pool &thebuffer) {
        //LOG_DEBUG("EBuffer.ReadPage, node_id", node_id);
        if (mbuffer->FixPage(node_id, thebuffer) == -1)
            FAIL;
    }
    // LRU -- 更新了一个缓冲块（可能淘汰了旧的）
    //对于新插入的缓冲块，使用 [LRUInsert]
    void LRUUpdate(int readed_bid) {
        //LOG_DEBUG("EBuffer.LRUUpdate");
        if (tail_bid == readed_bid) { // 选了尾巴
            // 从尾巴处删除
            tail_bid = ebuf_des[readed_bid].prev_buffer_id;
            ebuf_des[tail_bid].next_buffer_id = tail_bid;
        } else { // 更新了中间某个缓冲块
            int prev = ebuf_des[readed_bid].prev_buffer_id;
            int next = ebuf_des[readed_bid].next_buffer_id;
            // 删除它
            ebuf_des[prev].next_buffer_id = next;
            ebuf_des[next].prev_buffer_id = prev;
        }
        // 加到首部
        ebuf_des[head_bid].prev_buffer_id = readed_bid;
        ebuf_des[readed_bid].next_buffer_id = head_bid;
        ebuf_des[readed_bid].prev_buffer_id = readed_bid;
        head_bid = readed_bid;
    }
    // LRU -- 插入了一个新缓冲块
    void LRUInsert(int inserted_bid) {
        //LOG_DEBUG("EBuffer.LRUInsert");
        if (size == 0) { // LRU 链表为空
            head_bid = tail_bid = inserted_bid;
            ebuf_des[head_bid].next_buffer_id = ebuf_des[head_bid].prev_buffer_id = inserted_bid;
        } else { // 链表不为空
            ebuf_des[head_bid].prev_buffer_id = inserted_bid;
            ebuf_des[inserted_bid].next_buffer_id = head_bid;
            ebuf_des[inserted_bid].prev_buffer_id = inserted_bid;
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

//public:
    EBuffer(MBuffer *mbuffer) {
        memset(buffer2node, -1, sizeof(buffer2node));
        //node2buffer[1] = 0;

        memset(ebuf_des, -1, sizeof(ebuf_des));
        head_bid = tail_bid = -1;
        size = 0;
        this->mbuffer = mbuffer;
        ebuf_pool = new EBuf_pool[EBUFFER_NUM_MAX_SIZE];
        hit_num = tot_page_update_num = 0;
        //cout<<"EBuffer!"<<endl;
    }
    ~EBuffer() {
        //LOG_DEBUG("~EBuffer");
        delete[] ebuf_pool;
    }

    // 写入一个新记录到磁盘
    /*
    void FixNewPage(EBuf_pool &tmp_buffer) {
        LOG_DEBUG("EBuffer.FixNewPage");
        if (ds_mgr->WriteNewPage(tmp_buffer) == -1)
            FAIL;  
    }
    */

    // 获取命中率
    /*
    double GetHitRate() {
        return tot_page_update_num == 0 ? -1 : hit_num / (double)tot_page_update_num;
    }*/

    // 获取IO总次数
    /*
    long GetIONumTot() {
        return ds_mgr->GetTotalIO();
    }*/

    // 将所有的dirty缓冲块写回mbuffer
    void WriteDirtys() {
        //LOG_DEBUG("EBuffer.WriteDirtys");
        for (int bid = 0; bid < size; bid++) {
            if (ebuf_des[bid].dirty) {
                //int ret =
                mbuffer->UpdatePage(buffer2node[bid], ebuf_pool[bid]);
                //if (ret == -1)
                    //FAIL;
                ebuf_des[bid].dirty = false;
            }
        }
        mbuffer->WriteDirtys();
    }
};

#endif // BUFFER_MGR_H
