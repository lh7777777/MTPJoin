#if !defined(MHEADER_H)
#define MHEADER_H

//#include "logger.h"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <string>
#include <time.h>
#include <unordered_map>
#include <map>
#include <vector>

const int PAGE_SIZE = 4 * 1024;
const int MBUFFER_SIZE = PAGE_SIZE;
const int MBUFFER_NUM_MAX_SIZE = 1024;//最多1024个mbuffer


struct MBuf_pool {
    char field[MBUFFER_SIZE] = {};//buffer pool存储的加密数据
    int size = MBUFFER_SIZE;

    // 将 src 中的内容复制到 field 中
    void set_field(MBuf_pool &src_buffer) {
        memcpy(field, src_buffer.field, size);
    }
};
struct MBuf_id{
    int page_id;
    int offset;
    MBuf_id(){}
    MBuf_id(int page_id,int offset){this->page_id=page_id;this->offset=offset;}
};
struct k_r{
    int k;
    int rid;
    int key()const{
        return k;
    }
    k_r(){}
    k_r(int k,int r){this->k=k;r=rid;}
};
struct MBuf_des {
    int page_id;
    int buffer_id;
    int count;
    bool dirty;
    int next_buffer_id, prev_buffer_id;

    MBuf_des() { page_id = buffer_id = next_buffer_id = prev_buffer_id = 1, count = 0, dirty = false; }
    void update(int pid, int bid, int cnt, bool dirty) {
        //LOG_DEBUG("MBuf_des.update");
        page_id = pid, buffer_id = bid, count = cnt, this->dirty = dirty;
    }
};

#endif // MHEADER_H