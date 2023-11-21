#if !defined(MHEADER_H)
#define MHEADER_H

//#include "logger.h"

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>

using std::string;
using std::to_string;
using std::unordered_map;
using std::map;
using namespace std;

const int MBUFFER_SIZE = 4 * 1024;
const int MBUFFER_NUM_MAX_SIZE = 1024;//最多1024个mbuffer

struct k_r{
    int k;
    int rid;
    int key()const{
        return k;
    }
    k_r(){}
    k_r(int k,int rid){this->k=k;this->rid=rid;}
};


struct MBuf_id{
    int page_id;
    int offset;
    MBuf_id(){}
    MBuf_id(int page_id,int offset){this->page_id=page_id;this->offset=offset;}
};




#endif // MHEADER_H
