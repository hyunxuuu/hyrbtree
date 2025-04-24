/**
 * @file hyrbtree_test.h
 * @brief Red-Black Tree Test Harness
 * 
 * Provides verification infrastructure for:
 * - Tree operations validation
 * - Memory pool management
 * - Edge case testing
 */

#ifndef HYRBTREE_TEST_H
#define HYRBTREE_TEST_H

#include <stdio.h>
#include "hyrbtree.h"



/** Test operation status codes */
typedef uint8_t         ret_t;
enum{
    RET_OK,
    RET_ERR,
    RET_POOL_FULL,
};



/**
 * @brief Test node structure
 * 
 * Demonstrates multi-index capability by:
 * - Embedding rbnode for tree management
 * - Supporting linked list for hash collisions
 */
typedef struct user_node_t{
    uint32_t idx;

    uint32_t addr;
    int32_t elem;
    hyrbnode_t rbnode;

    struct user_node_t *next_node;
}user_node_t;

/**
 * @brief Bounded memory pool manager
 * 
 * Implements ring buffer semantics to:
 * - Prevent memory leaks in embedded environments
 * - Simulate constrained memory conditions
 * - Track allocation patterns
 */
typedef struct{
    uint32_t max_size;

    user_node_t *node_pool;
    uint32_t *idx_pool;

    uint32_t read_pos;
    uint32_t write_pos;
}user_pool_t;


    
/** Entry point for test suite execution */
void hyrbtree_test( void );

#endif
