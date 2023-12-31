#ifndef _CSCBF_
#define _CSCBF_
#include <vector>
#include <set>
#include <random>

#include <cstring>
#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <cmath>

#include "bitarray.h"
#include "MurmurHash3.h"


uint32_t concatenate(uint16_t i, uint16_t j) {
    return (uint32_t)i << 16 | j;
}

class CSCBF{
    public:
        int k;
        size_t r;
        size_t b;
        size_t capacity;
        size_t single_capacity;
        int set_num;
        size_t mask;
        size_t* copy_array;
        size_t* mask_array;
        size_t* result;
        int array_length, al_size;
        float time;
        size_t* seed;
        size_t* partition_seed;

        bitarray** CSCBF_array;
        std::set<size_t>* CSCBF_partition;

        vector<size_t> partitionHash(size_t set_id , string key){
            vector<size_t> Hash;
            size_t op;
            for(int i=0; i<this->r; i++){
                uint32_t i_j_combined = concatenate(i, set_id);
                MurmurHash3_x86_32(key.c_str(), key.size(), i_j_combined, &op);
                Hash.push_back(op);
            }
            return Hash;
        }

        vector<size_t> locationHash(string key){ 
            vector<size_t> Locations;
            size_t op;
            for(int i=0; i<this->k; i++){
                MurmurHash3_x86_128(key.c_str(), key.size(), this->seed[i], &op);
                Locations.push_back(op % this->single_capacity);
            }
            return Locations;
        }

        CSCBF(size_t R, size_t B, size_t total_usage, int k, int m){
            this->r = R;
            this->b = B;
            this->capacity = total_usage;
            this->single_capacity = size_t(this->capacity / (this->r));
            this->k = k;
            this->mask = 1ULL;
            this->set_num = m;

            this->seed = new size_t[this->k];
            this->partition_seed = new size_t[this->r];


            for (int i = 0; i < this->k; i++) {
                size_t generatedNumber;
                if ((sgxssl_read_rand(reinterpret_cast<uint8_t*>(&generatedNumber), sizeof(generatedNumber))) != SGX_SUCCESS) {
                    printf("Failed to generate random number");
                }
                this->seed[i] = generatedNumber;
            }
            for (int i = 0; i < this->r; i++) {
                size_t generatedNumber;
                if ((sgxssl_read_rand(reinterpret_cast<uint8_t*>(&generatedNumber), sizeof(generatedNumber))) != SGX_SUCCESS) {
                    printf("Failed to generate random number"); 
                }
                this->partition_seed[i] = generatedNumber;
            }

            this->CSCBF_array = new bitarray*[this->r];
            for(int r=0; r < this->r; r++){
                this->CSCBF_array[r] = new bitarray(this->single_capacity);
            }
            CSCBF_partition = new set<size_t>[this->r * this->b];

            this->array_length = (this->b + 63) >> 6;
            this->copy_array = new size_t[this->array_length];
            this->mask_array = new size_t[this->array_length];
            this->result = new size_t[this->array_length];

            this->al_size = this->array_length * 8;
            memset(this->copy_array, 0, this->al_size);
            memset(this->mask_array, 255, this->al_size);
            memset(this->result, 0, this->al_size);
            this->time = 0.0;
        }

        void insertion(string setID, string keys){

            size_t offset_location = 0;
            int set_id = atoi(setID.c_str());
            vector<size_t> hash_res = partitionHash(set_id,keys);
            vector<size_t> locations = locationHash(keys);
            for(int r=0; r < this->r; r++){
                for(auto &location : locations){
                    offset_location = (location + hash_res[r]) % this->single_capacity;
                    this->CSCBF_array[r]->setbit(offset_location);

                }
            }
        }


        int16_t query(size_t set_id,string query_key){

            int res=1,ret = 1;
            bitarray k1(this->set_num);
            int count;
            size_t offset_location = 0;

            vector<size_t> check_locations = locationHash(query_key);
            vector<size_t> offset = partitionHash(set_id,query_key);
            for(int r = 0;r<this->r;r++){
                for(auto &location:check_locations){
                    offset_location = (location + offset[r]) % this->single_capacity;
                    res = res&this->CSCBF_array[r]->checkbit(offset_location);
                }
                ret  = ret &res;
            }
            return ret;
        }

        int16_t obfquery(size_t set_id,string query_key){

            int res=1,ret = 1;
            int re = 0;
            size_t offset_location = 0;

            vector<vector<size_t> > testVector(this->r,vector<size_t>(this->single_capacity));

            vector<size_t> check_locations = locationHash(query_key);
            vector<size_t> offset = partitionHash(set_id,query_key);
            for(int r = 0;r<this->r;r++){
                for(auto &location:check_locations){
                    offset_location = (location + offset[r]) % this->single_capacity;
                    testVector[r][offset_location] = 1;
                }
            }

            for(int r = 0;r<this->r;r++){
                for(auto &location:check_locations){
                    offset_location = (location + offset[r]) % this->single_capacity;
                    testVector[r][offset_location] &= this->CSCBF_array[r]->checkbit(offset_location);
                }
                re += testVector[r][offset_location];
            }
            re -= this->k;
            return (re == 0);
        }

        void CopyArray(size_t* src_array, size_t* tgt_array, size_t begin_location){

            size_t start = begin_location / 64; 
            size_t start_offset = begin_location & 63;
            size_t end_offset = 64 - start_offset;

            int i = 0;
            while(i < this->array_length){
                tgt_array[i] = (src_array[start+1] << end_offset) | (src_array[start] >> start_offset);
                i++; start++; 
            }

        }

        void CopyArray2(size_t* src_array, size_t* tgt_array, size_t begin_location){

            size_t start = begin_location / 64;
            size_t start_offset = begin_location & 63;
            size_t end_offset = 64 - start_offset;
            size_t dis = this->array_length - 1;

            int i = 0;
            while(i < this->array_length){

                if(start < dis){
                    tgt_array[i] = (src_array[start+1] << end_offset) | (src_array[start] >> start_offset);
                    start ++;   
                }
                else if(start == dis){
                    tgt_array[i] = (src_array[0] << end_offset) | (src_array[start] >> start_offset);
                    start = 0;
                }
                i++;
            }
        }

};

#endif
