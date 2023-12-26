//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.hpp"
#include <stdio.h>
#include <cstring>
#include <cmath>
#include <stdint.h>
#undef uint64_t
#define uint64_t unsigned long
#include <stdlib.h>

//
// TODO:Student Information
//
const char *studentName = "PRASHIL PAREKH";
const char *studentID   = "A59026482";
const char *email       = "prparekh@ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;      // Number of sets in the I$
uint32_t icacheAssoc;     // Associativity of the I$
uint32_t icacheBlocksize; // Blocksize of the I$
uint32_t icacheHitTime;   // Hit Time of the I$

uint32_t dcacheSets;      // Number of sets in the D$
uint32_t dcacheAssoc;     // Associativity of the D$
uint32_t dcacheBlocksize; // Blocksize of the D$
uint32_t dcacheHitTime;   // Hit Time of the D$

uint32_t l2cacheSets;     // Number of sets in the L2$
uint32_t l2cacheAssoc;    // Associativity of the L2$
uint32_t l2cacheBlocksize;// Blocksize of the L2$
uint32_t l2cacheHitTime;  // Hit Time of the L2$
uint32_t inclusive;       // Indicates if the L2 is inclusive

uint32_t prefetch;        // Indicate if prefetching is enabled

uint32_t memspeed;        // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

uint64_t compulsory_miss;  // Compulsory misses on all caches
uint64_t other_miss;       // Other misses (Conflict / Capacity miss) on all caches

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//

//------------------------------------//
//          Cache Functions           //
//------------------------------------//


uint32_t *icache_tag;
uint32_t *icache_valid;
uint32_t *icache_lru;

uint32_t *dcache_tag;
uint32_t *dcache_valid;
uint32_t *dcache_lru;

uint32_t *l2cache_tag;
uint32_t *l2cache_valid;
uint32_t *l2cache_lru;

uint32_t *prefetcher_tag;
uint32_t *prefetcher_index;
uint32_t *prefetcher_adr;

uint32_t *prefetcher_h1_dcache;
uint32_t *prefetcher_h2_dcache;
uint32_t *prefetcher_adr_dcache;





// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;

  compulsory_miss = 0;
  other_miss = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //
    uint32_t icache_block_offset = log2(icacheBlocksize);
    uint32_t icache_index_bits = log2(icacheSets);
    uint32_t icache_tag_bits = 32 - icache_block_offset - icache_index_bits;
    uint32_t icache_lines = icacheSets*icacheAssoc;

    icache_valid = (uint32_t *)malloc(icache_lines * sizeof(uint32_t));
    icache_tag = (uint32_t *)malloc(icache_lines * sizeof(uint32_t));
    icache_lru = (uint32_t *)malloc(icache_lines * sizeof(uint32_t));
    
    
    for (int i=0;i<icache_lines;i++)
    {
        icache_tag[i] = 0;
        icache_lru[i] = i%icacheAssoc;
        icache_valid[i] = 0;
    }
    
    
    uint32_t dcache_block_offset = log2(dcacheBlocksize);
    uint32_t dcache_index_bits = log2(dcacheSets);
    uint32_t dcache_tag_bits = 32 - dcache_block_offset - dcache_index_bits;
    uint32_t dcache_lines = dcacheSets*dcacheAssoc;

    dcache_valid = (uint32_t *)malloc(dcache_lines * sizeof(uint32_t));
    dcache_tag = (uint32_t *)malloc(dcache_lines * sizeof(uint32_t));
    dcache_lru = (uint32_t *)malloc(dcache_lines * sizeof(uint32_t));
    
    
    for (int i=0;i<dcache_lines;i++)
    {
        dcache_tag[i] = 0;
        dcache_lru[i] = i%dcacheAssoc;
        dcache_valid[i] = 0;
    }
    
    uint32_t l2cache_block_offset = log2(l2cacheBlocksize);
    uint32_t l2cache_index_bits = log2(l2cacheSets);
    uint32_t l2cache_tag_bits = 32 - l2cache_block_offset - l2cache_index_bits;
    uint32_t l2cache_lines = l2cacheSets*l2cacheAssoc;

    l2cache_valid = (uint32_t *)malloc(l2cache_lines * sizeof(uint32_t));
    l2cache_tag = (uint32_t *)malloc(l2cache_lines * sizeof(uint32_t));
    l2cache_lru = (uint32_t *)malloc(l2cache_lines * sizeof(uint32_t));
    
    
    for (int i=0;i<l2cache_lines;i++)
    {
        l2cache_tag[i] = 0;
        l2cache_lru[i] = i%l2cacheAssoc;
        l2cache_valid[i] = 0;
    }
    
    uint32_t prefetch_num = 24;
    uint32_t prefetcher_lines = 1 << prefetch_num;
    
    prefetcher_tag = (uint32_t *)malloc(prefetcher_lines * sizeof(uint32_t));
    prefetcher_index = (uint32_t *)malloc(prefetcher_lines * sizeof(uint32_t));
    prefetcher_adr = (uint32_t *)malloc(prefetcher_lines * sizeof(uint32_t));
    
    for (int i=0;i<prefetcher_lines;i++)
    {
        prefetcher_tag[i] = 0;
        prefetcher_index[i] = 0;
        prefetcher_adr[i] = -1;
    }
    
    prefetcher_h1_dcache = (uint32_t *)malloc(2*prefetcher_lines * sizeof(uint32_t));
    prefetcher_h2_dcache = (uint32_t *)malloc(2*prefetcher_lines * sizeof(uint32_t));
    prefetcher_adr_dcache = (uint32_t *)malloc(2*prefetcher_lines * sizeof(uint32_t));
    
    for (int i=0;i<(2*prefetcher_lines);i++)
    {
        prefetcher_h1_dcache[i] = 0;
        prefetcher_h2_dcache[i] = 0;
        prefetcher_adr_dcache[i] = -1;
    }
    
    
    
}

// Clean Up the Cache Hierarchy
//
void
clean_cache()
{
  //
  //TODO: Clean Up Cache Simulator Data Structures
  //
    free(icache_tag);
    free(icache_lru);
    free(icache_valid);
    
    free(dcache_tag);
    free(dcache_lru);
    free(dcache_valid);
    
    free(l2cache_tag);
    free(l2cache_lru);
    free(l2cache_valid);
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
  
    uint32_t icache_block_offset = log2(icacheBlocksize);
    uint32_t icache_index_bits = log2(icacheSets);
    uint32_t icache_tag_bits = 32 - icache_block_offset - icache_index_bits;
    uint32_t icache_lines = icacheSets*icacheAssoc;
    
    icacheRefs = icacheRefs + 1;
    uint32_t adr_offset = 0;
    adr_offset = addr & (icacheBlocksize - 1);
    uint32_t adr_index = (addr >> icache_block_offset) & ((1 << icache_index_bits) - 1);
    uint32_t adr_tag = (addr >> (icache_block_offset + icache_index_bits)) & ((1 << icache_tag_bits) - 1);
    //printf("start %u  %u  %u  %u  %u\n",addr, adr_offset, adr_index, adr_tag, icache_lines );
    int c_hit = 0;
    int hit_index;
    int hit_index_lru;
    
    
    
    for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
    {
        //printf("start %d  %u  %u  %u \n",i,icache_valid[i], icache_tag[i], icache_lru[i]);
    }
    
    for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
    {
        hit_index = -1;
        hit_index_lru = -1;
        if ((icache_tag[i] == adr_tag) && (icache_valid[i] == 1))
        {
            c_hit = 1;
            hit_index = i;
            hit_index_lru = icache_lru[i];
            break;
        }
    }
    
    //printf("c_HIT = %d \n",c_hit);
    uint32_t ac_time = 0;
    if (c_hit == 1)
    {
        
        for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
        {
            if ((i != hit_index) && (icache_lru[i] > hit_index_lru))
            {
                icache_lru[i] = icache_lru[i] - 1;
            }
        }
        
        for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
        {
            if (i == hit_index)
            {
                icache_lru[i] = icacheAssoc-1;
            }
        }
    }
    
    if (c_hit == 0)
    {
        icacheMisses = icacheMisses + 1;
        ac_time = l2cache_access(addr,0); // access the l2 cache if this is a miss
        int new_index = -1;
        for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
        {
            if (icache_lru[i] == 0)
            {
                if (icache_valid[i] == 0)
                {
                    compulsory_miss = compulsory_miss + 1;
                }
                if (icache_valid[i] == 1)
                {
                    other_miss = other_miss + 1;
                }
                icache_valid[i] = 1;
                icache_tag[i] = adr_tag;
                new_index = i;
                icache_lru[i] = icacheAssoc-1;
            }
        }
        
        for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
        {
            if (i != new_index)
            {
                icache_lru[i] = icache_lru[i] - 1;
            }
        }
        
    }
    
    for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
    {
        //printf("end %d %u  %u  %u \n",i,icache_valid[i], icache_tag[i], icache_lru[i]);
    }
    
    
  
  return icacheHitTime + ac_time;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //
    
    uint32_t dcache_block_offset = log2(dcacheBlocksize);
    uint32_t dcache_index_bits = log2(dcacheSets);
    uint32_t dcache_tag_bits = 32 - dcache_block_offset - dcache_index_bits;
    uint32_t dcache_lines = dcacheSets*dcacheAssoc;
    
    dcacheRefs = dcacheRefs + 1;
    uint32_t adr_offset = 0;
    adr_offset = addr & (dcacheBlocksize - 1);
    uint32_t adr_index = (addr >> dcache_block_offset) & ((1 << dcache_index_bits) - 1);
    uint32_t adr_tag = (addr >> (dcache_block_offset + dcache_index_bits)) & ((1 << dcache_tag_bits) - 1);
    //printf("start %u  %u  %u  %u  %u\n",addr, adr_offset, adr_index, adr_tag, dcache_lines );
    int c_hit = 0;
    int hit_index;
    int hit_index_lru;
    
    
    
    for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
    {
        //printf("start %d  %u  %u  %u \n",i,dcache_valid[i], dcache_tag[i], dcache_lru[i]);
    }
    
    for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
    {
        hit_index = -1;
        hit_index_lru = -1;
        if ((dcache_tag[i] == adr_tag) && (dcache_valid[i] == 1))
        {
            c_hit = 1;
            hit_index = i;
            hit_index_lru = dcache_lru[i];
            break;
        }
    }
    
    //printf("c_HIT = %d \n",c_hit);
    uint32_t ac_time=0;
    if (c_hit == 1)
    {
        
        for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
        {
            if ((i != hit_index) && (dcache_lru[i] > hit_index_lru))
            {
                dcache_lru[i] = dcache_lru[i] - 1;
            }
        }
        
        for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
        {
            if (i == hit_index)
            {
                dcache_lru[i] = dcacheAssoc-1;
            }
        }
    }
    
    if (c_hit == 0)
    {
        dcacheMisses = dcacheMisses + 1;
        ac_time = l2cache_access(addr,1); // access the l2 cache if this is a miss
        int new_index = -1;
        for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
        {
            if (dcache_lru[i] == 0)
            {
                if (dcache_valid[i] == 0)
                {
                    compulsory_miss = compulsory_miss + 1;
                }
                if (dcache_valid[i] == 0)
                {
                    other_miss = other_miss + 1;
                }
                dcache_valid[i] = 1;
                dcache_tag[i] = adr_tag;
                new_index = i;
                dcache_lru[i] = dcacheAssoc-1;
            }
        }
        
        for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
        {
            if (i != new_index)
            {
                dcache_lru[i] = dcache_lru[i] - 1;
            }
        }
        
    }
    
    for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
    {
        //printf("end %d %u  %u  %u \n",i,dcache_valid[i], dcache_tag[i], dcache_lru[i]);
    }
    
  return dcacheHitTime + ac_time;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//



uint32_t
l2cache_access(uint32_t addr, int cache_type)
{
  //
  //TODO: Implement L2$
  //
    uint32_t l2cache_block_offset = log2(l2cacheBlocksize);
    uint32_t l2cache_index_bits = log2(l2cacheSets);
    uint32_t l2cache_tag_bits = 32 - l2cache_block_offset - l2cache_index_bits;
    uint32_t l2cache_lines = l2cacheSets*l2cacheAssoc;
    
    l2cacheRefs = l2cacheRefs + 1;
    uint32_t adr_offset = 0;
    adr_offset = addr & (l2cacheBlocksize - 1);
    uint32_t adr_index = (addr >> l2cache_block_offset) & ((1 << l2cache_index_bits) - 1);
    uint32_t adr_tag = (addr >> (l2cache_block_offset + l2cache_index_bits)) & ((1 << l2cache_tag_bits) - 1);
    //printf("start %u  %u  %u  %u  %u\n",addr, adr_offset, adr_index, adr_tag, l2cache_lines );
    int c_hit = 0;
    int hit_index;
    int hit_index_lru;
    
    
    
    for (int i=adr_index*l2cacheAssoc;i<l2cacheAssoc+adr_index*l2cacheAssoc;i++)
    {
        //printf("start %d  %u  %u  %u \n",i,l2cache_valid[i], l2cache_tag[i], l2cache_lru[i]);
    }
    
    for (int i=adr_index*l2cacheAssoc;i<l2cacheAssoc+adr_index*l2cacheAssoc;i++)
    {
        hit_index = -1;
        hit_index_lru = -1;
        if ((l2cache_tag[i] == adr_tag) && (l2cache_valid[i] == 1))
        {
            c_hit = 1;
            hit_index = i;
            hit_index_lru = l2cache_lru[i];
            break;
        }
    }
    
    //printf("c_HIT = %d \n",c_hit);
    uint32_t ac_time = 0;
    if (c_hit == 1)
    {
        if (cache_type == 0)
        {
            icachePenalties = icachePenalties + l2cacheHitTime;
        }
        if (cache_type == 1)
        {
            dcachePenalties = dcachePenalties + l2cacheHitTime;
        }
        for (int i=adr_index*l2cacheAssoc;i<l2cacheAssoc+adr_index*l2cacheAssoc;i++)
        {
            if ((i != hit_index) && (l2cache_lru[i] > hit_index_lru))
            {
                l2cache_lru[i] = l2cache_lru[i] - 1;
            }
        }
        
        for (int i=adr_index*l2cacheAssoc;i<l2cacheAssoc+adr_index*l2cacheAssoc;i++)
        {
            if (i == hit_index)
            {
                l2cache_lru[i] = l2cacheAssoc-1;
            }
        }
    }
    
    if (c_hit == 0)
    {
        ac_time = memspeed;
        l2cachePenalties = l2cachePenalties + memspeed;
        if (cache_type == 0)
        {
            icachePenalties = icachePenalties + l2cacheHitTime + memspeed;
        }
        if (cache_type == 1)
        {
            dcachePenalties = dcachePenalties + l2cacheHitTime + memspeed;
        }
        l2cacheMisses = l2cacheMisses + 1;
        int new_index = -1;
        for (int i=adr_index*l2cacheAssoc;i<l2cacheAssoc+adr_index*l2cacheAssoc;i++)
        {
            if (l2cache_lru[i] == 0)
            {
                if (l2cache_valid[i] == 0)
                {
                    compulsory_miss = compulsory_miss + 1;
                }
                if (l2cache_valid[i] == 1)
                {
                    other_miss = other_miss + 1;
                }
                l2cache_valid[i] = 1;
                l2cache_tag[i] = adr_tag;
                new_index = i;
                l2cache_lru[i] = l2cacheAssoc-1;
            }
        }
        
        for (int i=adr_index*l2cacheAssoc;i<l2cacheAssoc+adr_index*l2cacheAssoc;i++)
        {
            if (i != new_index)
            {
                l2cache_lru[i] = l2cache_lru[i] - 1;
            }
        }
        
    }
    
    for (int i=adr_index*l2cacheAssoc;i<l2cacheAssoc+adr_index*l2cacheAssoc;i++)
    {
        //printf("end %d %u  %u  %u \n",i,l2cache_valid[i], l2cache_tag[i], l2cache_lru[i]);
    }

    return ac_time + l2cacheHitTime;
 
}

uint32_t prev_tag = 0;
uint32_t prev_index = 0;
uint32_t prev_pc = 0;

// Predict an address to prefetch on icache with the information of last icache access:
// 'pc':     Program Counter of the instruction of last icache access
// 'addr':   Accessed Address of last icache access
// 'r_or_w': Read/Write of last icache access
uint32_t
icache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
    uint32_t predict_adr;
    uint32_t prefetch_num = 24;
    uint32_t prefetcher_lines = 1 << prefetch_num;
    
    uint32_t pc_index = pc & ((1 << prefetch_num) - 1);  // find the PC index of the prefetcher data structure.
    
    uint32_t icache_block_offset = log2(icacheBlocksize);
    uint32_t icache_index_bits = log2(icacheSets);
    uint32_t icache_tag_bits = 32 - icache_block_offset - icache_index_bits;
    uint32_t icache_lines = icacheSets*icacheAssoc;
    
    // Find the addr tag and index of the input address
    uint32_t adr_offset = 0;
    adr_offset = addr & (icacheBlocksize - 1);
    uint32_t adr_index = (addr >> icache_block_offset) & ((1 << icache_index_bits) - 1);
    uint32_t adr_tag = (addr >> (icache_block_offset + icache_index_bits)) & ((1 << icache_tag_bits) - 1);
    
    // Check if the tag and index matches with the prefetcher DS
    
    if ((adr_tag == prefetcher_tag[pc_index]) && (adr_index == prefetcher_index[pc_index]))
    {
        if (prefetcher_adr[pc_index] == -1)
        {
            predict_adr = addr + icacheBlocksize;
        }
        else
        {
            predict_adr = prefetcher_adr[pc_index];
        }
    }
    
    //train the prefetcher
    
    int do_nothing = 0;
    if ((prev_tag == adr_tag) && (prev_index == adr_index))
    {
        do_nothing = 1;
    }
    
    if (do_nothing == 0)
    {
        uint32_t prev_pc_index = prev_pc & ((1 << prefetch_num) - 1);
        prefetcher_tag[prev_pc_index] = prev_tag;
        prefetcher_index[prev_pc_index] = prev_index;
        prefetcher_adr[prev_pc_index] = addr;
        
        prev_tag = adr_tag;
        prev_index = adr_index;
        prev_pc = pc;
    }
    
    // put the to-be-prefetched address in the prefetcher DS.
    
    
  return predict_adr; // Next line prefetching
  //
  //TODO: Implement a better prefetching strategy
  //
}

// Predict an address to prefetch on dcache with the information of last dcache access:
// 'pc':     Program Counter of the instruction of last dcache access
// 'addr':   Accessed Address of last dcache access
// 'r_or_w': Read/Write of last dcache access

uint32_t prev_adr = 0;
uint32_t
dcache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
    
    uint32_t predict_adr;
    uint32_t prefetch_num = 24;
    uint32_t prefetcher_lines = 1 << prefetch_num;
    
    uint32_t pc_index = pc & ((1 << prefetch_num) - 1);  // find the PC index of the prefetcher data structure.
    
    uint32_t dcache_block_offset = log2(dcacheBlocksize);
    uint32_t dcache_index_bits = log2(dcacheSets);
    uint32_t dcache_tag_bits = 32 - dcache_block_offset - dcache_index_bits;
    uint32_t dcache_lines = dcacheSets*dcacheAssoc;
    
    // Find the addr tag and index of the input address
    uint32_t adr_offset = 0;
    adr_offset = addr & (dcacheBlocksize - 1);
    uint32_t adr_index = (addr >> dcache_block_offset) & ((1 << dcache_index_bits) - 1);
    uint32_t adr_tag = (addr >> (dcache_block_offset + dcache_index_bits)) & ((1 << dcache_tag_bits) - 1);
    
    // give the output stride + adr
    int add=0;
    if (r_or_w == 'R')
    {
        add = 0;
    }
    if (r_or_w == 'W')
    {
        add = 1;
    }
    
    if (prefetcher_h1_dcache[pc_index+add] == 0)
    {
        predict_adr = addr + dcacheBlocksize;
    }
    
    else if (prefetcher_h2_dcache[pc_index+add] == 0)
    {
        predict_adr = addr + prefetcher_h1_dcache[pc_index+add];
    }
    
    else if (prefetcher_h2_dcache[pc_index+add] > (1<<20))
    {
        predict_adr = addr + prefetcher_h1_dcache[pc_index+add];
    }
    
    else
    {
        predict_adr = addr + prefetcher_h2_dcache[pc_index+add];
    }
    
    
    //train the prefetcher
    
    int stride = addr - prefetcher_adr_dcache[pc_index+add];
    prefetcher_h2_dcache[pc_index+add] = prefetcher_h1_dcache[pc_index+add];
    prefetcher_h1_dcache[pc_index+add] = stride;
    prefetcher_adr_dcache[pc_index+add] = addr;
    
    // put the to-be-prefetched address in the prefetcher DS.
    
  return predict_adr; // Next line prefetching
  //
  //TODO: Implement a better prefetching strategy
  //
}

// Perform a prefetch operation to I$ for the address 'addr'
void
icache_prefetch(uint32_t addr)
{
  //
  //TODO: Implement I$ prefetch operation
  //
    
    uint32_t icache_block_offset = log2(icacheBlocksize);
    uint32_t icache_index_bits = log2(icacheSets);
    uint32_t icache_tag_bits = 32 - icache_block_offset - icache_index_bits;
    uint32_t icache_lines = icacheSets*icacheAssoc;
    
    uint32_t adr_offset = 0;
    adr_offset = addr & (icacheBlocksize - 1);
    uint32_t adr_index = (addr >> icache_block_offset) & ((1 << icache_index_bits) - 1);
    uint32_t adr_tag = (addr >> (icache_block_offset + icache_index_bits)) & ((1 << icache_tag_bits) - 1);
    //printf("start %u  %u  %u  %u  %u\n",addr, adr_offset, adr_index, adr_tag, icache_lines );
    int c_hit = 0;
    int hit_index;
    int hit_index_lru;
    
    
    
    for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
    {
        //printf("start %d  %u  %u  %u \n",i,icache_valid[i], icache_tag[i], icache_lru[i]);
    }
    
    for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
    {
        hit_index = -1;
        hit_index_lru = -1;
        if ((icache_tag[i] == adr_tag) && (icache_valid[i] == 1))
        {
            c_hit = 1;
            hit_index = i;
            hit_index_lru = icache_lru[i];
            break;
        }
    }
    
    //printf("c_HIT = %d \n",c_hit);
    if (c_hit == 1)
    {
        
        for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
        {
            if ((i != hit_index) && (icache_lru[i] > hit_index_lru))
            {
                icache_lru[i] = icache_lru[i] - 1;
            }
        }
        
        for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
        {
            if (i == hit_index)
            {
                icache_lru[i] = icacheAssoc-1;
            }
        }
    }
    
    if (c_hit == 0)
    {
        int new_index = -1;
        for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
        {
            if (icache_lru[i] == 0)
            {
                icache_valid[i] = 1;
                icache_tag[i] = adr_tag;
                new_index = i;
                icache_lru[i] = icacheAssoc-1;
            }
        }
        
        for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
        {
            if (i != new_index)
            {
                icache_lru[i] = icache_lru[i] - 1;
            }
        }
        
    }
    
    for (int i=adr_index*icacheAssoc;i<icacheAssoc+adr_index*icacheAssoc;i++)
    {
        //printf("end %d %u  %u  %u \n",i,icache_valid[i], icache_tag[i], icache_lru[i]);
    }
    
    
}

// Perform a prefetch operation to D$ for the address 'addr'
void
dcache_prefetch(uint32_t addr)
{
  //
  //TODO: Implement D$ prefetch operation
  //
    
    uint32_t dcache_block_offset = log2(dcacheBlocksize);
    uint32_t dcache_index_bits = log2(dcacheSets);
    uint32_t dcache_tag_bits = 32 - dcache_block_offset - dcache_index_bits;
    uint32_t dcache_lines = dcacheSets*dcacheAssoc;
    
    uint32_t adr_offset = 0;
    adr_offset = addr & (dcacheBlocksize - 1);
    uint32_t adr_index = (addr >> dcache_block_offset) & ((1 << dcache_index_bits) - 1);
    uint32_t adr_tag = (addr >> (dcache_block_offset + dcache_index_bits)) & ((1 << dcache_tag_bits) - 1);
    //printf("start %u  %u  %u  %u  %u\n",addr, adr_offset, adr_index, adr_tag, dcache_lines );
    int c_hit = 0;
    int hit_index;
    int hit_index_lru;
    
    
    
    for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
    {
        //printf("start %d  %u  %u  %u \n",i,dcache_valid[i], dcache_tag[i], dcache_lru[i]);
    }
    
    for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
    {
        hit_index = -1;
        hit_index_lru = -1;
        if ((dcache_tag[i] == adr_tag) && (dcache_valid[i] == 1))
        {
            c_hit = 1;
            hit_index = i;
            hit_index_lru = dcache_lru[i];
            break;
        }
    }
    
    if (c_hit == 1)
    {
        
        for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
        {
            if ((i != hit_index) && (dcache_lru[i] > hit_index_lru))
            {
                dcache_lru[i] = dcache_lru[i] - 1;
            }
        }
        
        for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
        {
            if (i == hit_index)
            {
                dcache_lru[i] = dcacheAssoc-1;
            }
        }
    }
    
    if (c_hit == 0)
    {
        int new_index = -1;
        for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
        {
            if (dcache_lru[i] == 0)
            {
                dcache_valid[i] = 1;
                dcache_tag[i] = adr_tag;
                new_index = i;
                dcache_lru[i] = dcacheAssoc-1;
            }
        }
        
        for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
        {
            if (i != new_index)
            {
                dcache_lru[i] = dcache_lru[i] - 1;
            }
        }
        
    }
    
    for (int i=adr_index*dcacheAssoc;i<dcacheAssoc+adr_index*dcacheAssoc;i++)
    {
        //printf("end %d %u  %u  %u \n",i,dcache_valid[i], dcache_tag[i], dcache_lru[i]);
    }
    
}
