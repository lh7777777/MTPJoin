
#include "en_mbuffer.h"

   // 将一个dirty的mbuffer写回磁盘
    void WriteBuffer(int buffer_id) {
        //LOG_DEBUG("MBuffer.WriteBuffer, buffer_id", buffer_id);
        //if (ds_mgr->WritePage(buffer2page[buffer_id], mbuf_pool[buffer_id]) == -1)
            //FAIL;
        //ds_mgr->WritePage(buffer2page[buffer_id], mbuf_pool[buffer_id]);
        ocall_writepage(buffer2page[buffer_id], &mbuf_pool[buffer_id]);
    }
    // 从磁盘读一个page到mbuffer
    void ReadPage(int page_id, MBuf_pool &thebuffer) {
        //LOG_DEBUG("MBuffer.ReadPage, page_id", page_id);
        //if (ds_mgr->ReadPage(page_id, thebuffer) == -1)
            //FAIL;
        //ds_mgr->ReadPage(page_id, thebuffer);
        ocall_readpage(page_id, &thebuffer);
    }

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
        //ds_mgr->WriteNewPage(tmp_buffer);
        ocall_writenewpage(&tmp_buffer);
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
        printf("2okk\n");
        if (page2buffer.find(page_id) != page2buffer.end()) { // 在缓冲区
            printf("MBuffer HIT!\n");
            IncHitNum();
            int mbid = page2buffer[page_id];
            //memset((mbuf_pool[mbid].field)+offset*1024,0,1024);
            memcpy((mbuf_pool[mbid].field)+offset*1024, thebuffer.field, 1024);

            SetDirty(mbid);
            LRUUpdate(mbid);
            //return;
        } else { // 不在缓冲区
            printf("%d %d %d %s",node_id,page_id,offset,"踢入MBuffer!\n");
            int target_bid = -1, ret = 0;
            bool do_insert = false;
            if (size >= MBUFFER_NUM_MAX_SIZE) { // 缓冲区满
                printf("mbuffer满了\n");
                target_bid = SelectVictim();
                if (mbuf_des[target_bid].dirty) { // dirty
                    printf("踢入disk\n");
                    ocall_writepage(buffer2page[target_bid], &mbuf_pool[target_bid]);
                    //ret = ds_mgr->WritePage(buffer2page[target_bid], mbuf_pool[target_bid]);
                    //if (ret == -1)
                        //FAIL;
                }
            } else { // 缓冲区未满
                target_bid = size;
                do_insert = true;
            }
            /*ret = ds_mgr->ReadPage(page_id, mbuf_pool[target_bid]);
            if (ret == -1)
                FAIL;*/
            //memset((mbuf_pool[target_bid].field)+offset*1024,0,1024);
            printf("size: %d\n",size);

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

    // 将所有的 dirty缓冲块写回磁盘
    void WriteDirtys() {
        //LOG_DEBUG("MBuffer.WriteDirtys");
        for (int bid = 0; bid < size; bid++) {
            if (mbuf_des[bid].dirty) {
                ocall_writepage(buffer2page[bid], &mbuf_pool[bid]);
                //int ret = ds_mgr->WritePage(buffer2page[bid], mbuf_pool[bid]);
                //if (ret == -1)
                    //FAIL;
                mbuf_des[bid].dirty = false;
            }
        }
    }