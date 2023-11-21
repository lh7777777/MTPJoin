#if !defined(BUFFER_MGR_H)
#define BUFFER_MGR_H

#include "mbuf_header.h"
#include "ebuf_header.h"

#include "Enclave_t.h"
#include "Enclave.h"

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


//map<int,MBuf_id> node2page;//ebuffer到mbuffer的地址转换表

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

};

#endif // BUFFER_MGR_H
