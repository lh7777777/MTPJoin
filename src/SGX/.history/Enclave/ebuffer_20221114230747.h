#if !defined(EBUFFER_H)
#define EBUFFER_H

#include "ebuf_header.h"
#include "mbuffer.h"
//#include "baddtree.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <string>
#include <time.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>

using std::string;
using std::to_string;
//using namespace BAT;
using std::unordered_map;
using std::map;
using namespace std;

//#define BUFFER_NUM_MAX_SIZE 1024

//const int PAGE_SIZE = 4 * 1024;


class EBuffer {
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
    //BAddTree<int,int> tree;

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
        LOG_DEBUG("EBuffer.SelectVictim, victim", tail_bid);
        return tail_bid;
    }
    void SetDirty(int buffer_id) {
        LOG_DEBUG("EBuffer.SetDirty");
        ebuf_des[buffer_id].dirty = true;
    }
    // 将一个dirty的ebuffer写回mbuffer
    void WriteBuffer(int buffer_id) {
        LOG_DEBUG("EBuffer.WriteBuffer, buffer_id", buffer_id);
        mbuffer->UpdatePage(buffer2node[buffer_id], ebuf_pool[buffer_id]);      
    }
    // 从mbuffer读一个page到ebuffer
    void ReadPage(int node_id, EBuf_pool &thebuffer) {
        LOG_DEBUG("EBuffer.ReadPage, node_id", node_id);
        if (mbuffer->FixPage(node_id, thebuffer) == -1)
            FAIL;
    }
    // LRU -- 更新了一个缓冲块（可能淘汰了旧的）
    //对于新插入的缓冲块，使用 [LRUInsert]
    void LRUUpdate(int readed_bid) {
        LOG_DEBUG("EBuffer.LRUUpdate");
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
        LOG_DEBUG("EBuffer.LRUInsert");
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
        LOG_DEBUG("~EBuffer");
        delete[] ebuf_pool;
    }
    //
    /*void InsertIndex(int page_id,int key,int rid){
        //bool flag=tree.insert(key,rid);
        LOG_DEBUG("EBuffer.InsertIndex");
        Buffer thebuffer;
        thebuffer=;
        //IncTotPageUpdateNum();
        if (mbuffer2ebuffer.find(page_id) != mbuffer2ebuffer.end()) { // 在缓冲区
            printf("HIT!\t");
            IncHitNum();
            int tmp_frm_id = mbuffer2ebuffer[page_id];
            buffer[tmp_frm_id] = thebuffer;
            SetDirty(tmp_frm_id);
            LRUUpdate(tmp_frm_id);
            //return page2buffer[page_id];
        }
        //printf("\t\t");
        // 不在缓冲区
        int target_bid = size + 1;
        bool do_insert = false; // 确定执行更新还是插入操作

        if (size >= BUFFER_NUM_MAX_SIZE) { // 缓冲区满
            target_bid = SelectVictim();
            if (buf_des[target_bid].dirty == 1) { // dirty
                WriteBuffer(target_bid);
            }
            int target_pid=ebuffer2mbuffer[target_bid];
            if(buf_des[target_bid].dirty){
                if(buffer_mgr->Find(target_pid)){
                    // update part of mbuffer maybe need new function
                    buffer_mgr->UpdatePage(target_pid,buffer[target_bid]);
                    buffer_mgr->UnfixPage(target_pid);
                } else {
                    
                }
            } else {
                
            }
        } else { // 缓冲区未满
            target_bid = size;
            do_insert = true;
        }
        buffer[target_bid] = thebuffer;
        buf_des[target_bid].update(page_id, target_bid, 0, false);
        SetDirty(target_bid);
        //tree.insert(key,rid);
        do_insert ? LRUInsert(target_bid) : LRUUpdate(target_bid);
        do_insert ? size += 1 : 0;
        // 更新哈希表
        mbuffer2ebuffer[page_id] = target_bid;
        ebuffer2mbuffer[target_bid] = page_id;

    }*/
    /*
    // 读取node_id的内容，返回ebufferpool
    char* FixPage(int node_id, int prot) {
        LOG_DEBUG("EBuffer.FixPage, node_id", node_id);
        IncTotPageUpdateNum();
        //if (mbuffer2ebuffer.find(node_id) != mbuffer2ebuffer.end()) 
        if (node2buffer.find(node_id) != node2buffer.end()) { // 在缓冲区
            printf("EBuffer HIT!\t");
            IncHitNum();
            int bfid = node2buffer[node_id];
            return ebuf_pool[bfid].field;
        }
        printf("\t\t");
        // 不在缓冲区
        int target_bid = size + 1;
        bool do_insert = false; // 确定执行更新还是插入操作

        if (size >= EBUFFER_NUM_MAX_SIZE) { // 缓冲区满
            target_bid = SelectVictim();
            //int target_pid=node2page[target_bid];
            if (ebuf_des[target_bid].dirty == 1) { // dirty
                WriteBuffer(target_bid);
                //buffer_mgr->UpdatePage(target_pid, buffer[target_bid]);
            }
        } else { // 缓冲区未满
            target_bid = size;
            do_insert = true;
        }

        // 读数据
        EBuf_pool tmp_buffer;
        ReadPage(node_id,tmp_buffer);

        // 写数据到缓冲区，更新 Buf_des 和 LRU
        //tmp_buffer.set_field(buffer[target_bid]);
        ebuf_pool[target_bid].set_field(tmp_buffer);
        ebuf_des[target_bid].update(node_id, target_bid, 0, false);
        do_insert ? LRUInsert(target_bid) : LRUUpdate(target_bid);
        do_insert ? size += 1 : 0;

        // 更新哈希表
        //mbuffer2ebuffer[node_id] = target_bid;
        //node2buffer[target_bid] = node_id;
        node2buffer[node_id] = target_bid;
        buffer2node[target_bid] = node_id;

        return ebuf_pool[target_bid].field;
        //return target_bid;
    }*/

    // 写入一个新记录到mbuffer
    /*
    void FixNewPage(EBuf_pool &tmp_buffer) {
        LOG_DEBUG("EBuffer.FixNewPage");
        if (ds_mgr->WriteNewPage(tmp_buffer) == -1)
            FAIL;  
    }
    */

    // 更新某个page
    /*
    void UpdatePage(int page_id, Buffer thebuffer) {
        LOG_DEBUG("BufferMgr.UpdatePage, page_id", page_id);
        IncTotPageUpdateNum();
        if (mbuffer2ebuffer.find(page_id) != mbuffer2ebuffer.end()) { // 在缓冲区
            printf("HIT!\t");
            IncHitNum();
            int tmp_frm_id = mbuffer2ebuffer[page_id];
            buffer[tmp_frm_id] = thebuffer;
            SetDirty(tmp_frm_id);
            LRUUpdate(tmp_frm_id);
            return;
        } else { // 不在缓冲区
            printf("\t\t");
            int target_bid = -1, ret = 0;
            bool do_insert = false;
            if (size == BUFFER_NUM_MAX_SIZE) { // 缓冲区满
                target_bid = SelectVictim();
                if (buf_des[target_bid].dirty) { // dirty
                    ret = ds_mgr->WritePage(ebuffer2mbuffer[target_bid], buffer[target_bid]);
                    if (ret == -1)
                        FAIL;
                }
            } else { // 缓冲区未满
                target_bid = size;
                do_insert = true;
            }
            ret = ds_mgr->ReadPage(page_id, buffer[target_bid]);
            if (ret == -1)
                FAIL;
            buffer[target_bid] = thebuffer;
            buf_des[target_bid].update(page_id, target_bid, 0, false);
            SetDirty(target_bid);
            do_insert ? LRUInsert(target_bid) : LRUUpdate(target_bid);
            do_insert ? size += 1 : 0;
            // 更新哈希表
            mbuffer2ebuffer[page_id] = target_bid;
            ebuffer2mbuffer[target_bid] = page_id;
        }
    }*/

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
        LOG_DEBUG("EBuffer.WriteDirtys");
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
