/**
 * @file hyrbtree.h
 * @brief Embedded Red-Black Tree Core Definitions
 * 
 * Type-agnostic R-B tree implementation featuring:
 * - Color-bit pointer compression
 * - Zero memory overhead per node
 * - Callback-driven key comparison
 * - Multi-index capable node embedding
 */

#ifndef HYRBTREE_H
#define HYRBTREE_H

#include <hystd.h>



/* Macro to extract node address from color-encoded pointer */
#define HYRBTREE_GET_NODE_ADDR(n)       ((void* )((hy_uptr_t)((n)->user_node) & ~(hy_uptr_t)(0x1)))

/* Macro to read node color (0=RED, 1=BLACK) */
#define HYRBTREE_READ_NODE_COLOR(n)     ((hy_uptr_t)((n)->user_node) & (hy_uptr_t)(0x1))
enum{
    HYRBTREE_NODE_RED,
    HYRBTREE_NODE_BLACK,
};



/**
 * @brief Operation status codes
 */
typedef enum{
    HYRBTREE_RET_OK,
    HYRBTREE_RET_ADD_NODE_ELEM_EXIST,
    HYRBTREE_RET_ADD_NODE_UNINITIALIZED,
    HYRBTREE_RET_DEL_NODE_ARGS_ERROR,
    HYRBTREE_RET_GET_NODE_NOT_FIND,
    HYRBTREE_RET_GET_NODE_TREE_NULL,
    HYRBTREE_RET_REPLACE_CMP_ERROR,
    HYRBTREE_RET_REPLACE_INIT_ERROR,
}hyrbtree_ret_t;



typedef struct hyrbnode_t{
    void *user_node;
    struct hyrbnode_t *parent_node;
    struct hyrbnode_t *left_node;
    struct hyrbnode_t *right_node;
}hyrbnode_t;

typedef struct{
    /**
     * @brief Callback to locate embedded node
     * @param user_node Container structure
     * @return Pointer to embedded hyrbnode_t
     */
    hyrbnode_t* (*get_rbnode)(void *user_node);
    
    /**
     * @brief Callback to extract key element
     * @param user_node Container structure
     * @return Pointer to comparable key
     */
    void* (*get_elem)(void *user_node);
    
    /**
     * @brief Callback for key comparison
     * @param elem1 First key
     * @param elem2 Second key
     * @return <0 if elem1 < elem2, 0 if equal, >0 otherwise
     */
    hy_i32_t (*cmp_elem)(void *elem1, void *elem2);
    
    hyrbnode_t *root_node;  ///< Root of tree (points to nil_node when empty)
    hyrbnode_t nil_node;    ///< Sentinel node (always black)
} hyrbtree_t;



/* Core API Functions */
void hyrbtree_init( hyrbtree_t *tree );
hyrbtree_ret_t hyrbtree_add_node( hyrbtree_t *tree,void *user_node,void **exist_node );
hyrbtree_ret_t hyrbtree_del_node( hyrbtree_t *tree,void *user_node );
hyrbtree_ret_t hyrbtree_get_node( hyrbtree_t *tree,void *elem,void **get_node );
hyrbtree_ret_t hyrbtree_replace_node( hyrbtree_t *tree,void *old_node,void *new_node );

#endif
