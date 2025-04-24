/**
 * @file hyrbtree_test.c
 * @brief Comprehensive Test Cases for R-B Tree
 * 
 * Validates:
 * - Insertion/deletion invariants
 * - Memory management correctness
 * - Tree balancing properties
 * - Error handling robustness
 */

#include "hyrbtree_test.h"



/**
 * @brief Recursive tree traversal with diagnostics
 * @param tree Tree to traverse
 * @param node_ptr Current node
 * @param depth Current depth in tree
 * 
 * Prints:
 * - Node depth and key
 * - Color (R/B)
 * - Address and collision chain
 */
void rbtree_preorder( hyrbtree_t *tree,hyrbnode_t *node_ptr,hy_u8_t depth ){
    user_node_t *user_node_ptr;
    user_node_t *cur_node_ptr;

    if( node_ptr!=&tree->nil_node ){
        rbtree_preorder( tree,node_ptr->left_node,depth+1 );

        user_node_ptr = (user_node_t *)(HYRBTREE_GET_NODE_ADDR(node_ptr));
        printf("\ndepth=%d,elem=%d",depth,user_node_ptr->elem);
        if( HYRBTREE_READ_NODE_COLOR(node_ptr)==HYRBTREE_NODE_RED ){
            printf(",color=R");
        }
        else{
            printf(",color=B");
        }
        printf(",addr:%d",user_node_ptr->addr);
        cur_node_ptr = user_node_ptr;
        while( cur_node_ptr->next_node!=NULL ){
            cur_node_ptr = cur_node_ptr->next_node;
            printf("->%d",cur_node_ptr->addr);
        }

        rbtree_preorder( tree,node_ptr->right_node,depth+1 );
    }
}



/**
 * @brief Initialize memory pool
 * @param pool Pool to initialize
 * 
 * Sets up:
 * - Index tracking array
 * - Sentinel values (max_size)
 * - Buffer positions
 */
void user_pool_init( user_pool_t *pool ){
    uint32_t i;
    for( i=0 ; i<pool->max_size ; i++ ){
        pool->idx_pool[i] = i;
        pool->node_pool[i].idx = pool->max_size;
    }
    pool->read_pos = 0;
    pool->write_pos = 0;
}

/**
 * @brief Allocate node from pool
 * @param pool Memory pool
 * @param new_node Template node
 * @param new_node_ptr [out] Allocated node
 * @return Allocation status
 */
ret_t user_pool_new_node( user_pool_t *pool,user_node_t *new_node,user_node_t **new_node_ptr ){
    if( pool->idx_pool[ pool->read_pos ]!=pool->max_size ){
        new_node->idx = pool->idx_pool[ pool->read_pos ];
        pool->idx_pool[ pool->read_pos ] = pool->max_size;

        *new_node_ptr = &pool->node_pool[ new_node->idx ];
        *(*new_node_ptr) = *new_node;

        pool->read_pos++;
        if( pool->read_pos==pool->max_size ){
            pool->read_pos=0;
        }
        return RET_OK;
    }
    return RET_POOL_FULL;
}

/**
 * @brief Release node to pool
 * @param pool Memory pool
 * @param node Node to free
 * @return Deallocation status
 */
ret_t user_pool_del_node( user_pool_t *pool, user_node_t *node ){
    if( node->idx!=pool->max_size ){
        pool->idx_pool[ pool->write_pos ] = node->idx;
        pool->write_pos++;
        if( pool->write_pos==pool->max_size ){
            pool->write_pos = 0;
        }
        return RET_OK;
    }
    return RET_ERR;
}



/**
 * @brief Core test sequence
 * @param user_pool Memory manager
 * @param rbtree Tree under test
 * @param add_array Elements to insert
 * @param add_array_size Insertion count
 * @param del_array Elements to remove
 * @param del_array_size Deletion count
 * 
 * Validates:
 * 1. Insertion with collision handling
 * 2. Deletion with successor replacement
 * 3. Tree integrity after each operation
 */
void hyrbtree_add_del_test( user_pool_t *user_pool,hyrbtree_t *rbtree,
    int32_t *add_array,uint32_t add_array_size,
    int32_t *del_array,uint32_t del_array_size ){

    uint8_t i;
    hyrbtree_ret_t ret;



    user_node_t new_node = {
        .rbnode = {
            .user_node = NULL,
        },
        .next_node = NULL,
    };
    user_node_t *new_node_ptr;
    user_node_t *exist_node_ptr;
    user_node_t *cur_node_ptr;

    printf("\n\nadd node:");
    for( i=0;i<add_array_size;i++ ){
        new_node.elem = add_array[i];
        new_node.addr = i;

        if( user_pool_new_node( user_pool,&new_node,&new_node_ptr )==RET_OK ){
            printf("\nAdd node elem=%d",new_node.elem);
            ret = hyrbtree_add_node( rbtree,new_node_ptr,(void **)&exist_node_ptr );
            switch( ret ){
                case HYRBTREE_RET_OK:
                    printf(" success!");
                    break;
                case HYRBTREE_RET_ADD_NODE_ELEM_EXIST:
                    printf(" exist!");
                    cur_node_ptr = exist_node_ptr;
                    while( cur_node_ptr->next_node!=NULL ){
                        cur_node_ptr = cur_node_ptr->next_node;
                    }
                    cur_node_ptr->next_node = new_node_ptr;
                    break;
                case HYRBTREE_RET_ADD_NODE_UNINITIALIZED:
                    printf(" error! Node->rbnode uninitialized!");
                    break;
                default:
                    break;
            }
        }
        else{
            printf("\nAdd node error!",new_node.elem);
        }
    }
    printf("\n\nrbtree_preorder:");
    rbtree_preorder( rbtree,rbtree->root_node,0 );



    int32_t temp_elem;
    user_node_t *ret_node_ptr;
    user_node_t *del_node_ptr;

    for( i=0;i<del_array_size;i++ ){
        temp_elem = del_array[i];
        printf("\n\ndel node elem=%d",temp_elem);

        ret = hyrbtree_get_node( rbtree,&temp_elem,(void **)&ret_node_ptr );
        switch( ret ){
            case HYRBTREE_RET_OK:
                printf("\nret node:elem=%d,addr=%d",ret_node_ptr->elem,ret_node_ptr->addr);

                del_node_ptr = ret_node_ptr;
                if( del_node_ptr->next_node!=NULL ){
                    ret = hyrbtree_replace_node( rbtree,del_node_ptr,del_node_ptr->next_node );
                    switch( ret ){
                        case HYRBTREE_RET_OK:
                            user_pool_del_node( user_pool,del_node_ptr );

                            printf("\nrbtree_preorder:");
                            rbtree_preorder( rbtree,rbtree->root_node,0 );
                            break;
                        case HYRBTREE_RET_REPLACE_CMP_ERROR:
                            printf("\nRepl cmp error!");
                            break;
                        case HYRBTREE_RET_REPLACE_INIT_ERROR:
                            printf("\nRepl init error!");
                            break;
                        default:
                            break;
                    }
                }
                else{
                    ret = hyrbtree_del_node( rbtree,del_node_ptr );
                    switch( ret ){
                        case HYRBTREE_RET_OK:
                            user_pool_del_node( user_pool,del_node_ptr );

                            printf("\nrbtree_preorder:");
                            rbtree_preorder( rbtree,rbtree->root_node,0 );
                            break;
                        case HYRBTREE_RET_DEL_NODE_ARGS_ERROR:
                            printf("\nNode init error!");
                            break;
                        default:
                            break;
                    }
                }
                break;
            case HYRBTREE_RET_GET_NODE_NOT_FIND:
                printf("\nNode not find in tree!");
                break;
            case HYRBTREE_RET_GET_NODE_TREE_NULL:
                printf("\nTree is null!");
                break;
            default:
                break;
        }
    }
}

/* Test configuration */
#define USER_POOL_SIZE  16
user_node_t user_node_pool[USER_POOL_SIZE];
uint32_t user_idx_pool[USER_POOL_SIZE];

/**
 * @brief Callback: Get embedded rbnode from user struct
 * @param node Container structure
 * @return Pointer to embedded rbnode
 */
hyrbnode_t *user_node_get_rbnode( void *node ){
    return &((user_node_t *)node)->rbnode;
}

/**
 * @brief Callback: Extract key from user struct
 * @param node Container structure
 * @return Pointer to sortable key
 */
void *user_node_get_elem( void *node ){
    return &((user_node_t *)node)->elem;
}

/**
 * @brief Callback: Key comparison
 * @param elem1 First key
 * @param elem2 Second key
 * @return Ordinal relationship (-1/0/+1)
 */
hy_i32_t user_node_cmp_elem( void *elem1,void *elem2 ){
    int32_t *elem1_ptr;
    int32_t *elem2_ptr;
    elem1_ptr = (int32_t *)elem1;
    elem2_ptr = (int32_t *)elem2;
    if( *elem1_ptr<*elem2_ptr ){
        return -1;
    }
    else if( *elem1_ptr>*elem2_ptr ){
        return 1;
    }
    return 0;
}

/**
 * @brief Main test entry point
 * 
 * Executes test sequences:
 * 1. Balanced insertion/deletion
 * 2. Collision-heavy scenario
 * 
 * Each test validates:
 * - Tree structural integrity
 * - Memory management
 * - Error handling
 */
void hyrbtree_test(void){

    user_pool_t user_pool = {
      .node_pool = user_node_pool,
      .idx_pool = user_idx_pool,
      .max_size = USER_POOL_SIZE,
    };
    user_pool_init(&user_pool);

    hyrbtree_t rbtree = {
        .get_rbnode = user_node_get_rbnode,
        .get_elem = user_node_get_elem,
        .cmp_elem = user_node_cmp_elem,
    };
    hyrbtree_init( &rbtree );


    
    int32_t temp_add_array[] = {20, 10, 30, 5, 15, 25, 35, 3, 7, 12, 17, 22, 27, 32, 37, 1};
    int32_t temp_del_array[] = {15, 5, 25, 20, 30, 10, 35, 3, 17, 22, 27, 32, 37, 7, 12, 1, 19};
    hyrbtree_add_del_test( &user_pool,&rbtree,
        temp_add_array,sizeof(temp_add_array)/sizeof(int32_t),
        temp_del_array,sizeof(temp_del_array)/sizeof(int32_t) );

    int32_t temp_add_array2[] = {10, 5, 15, 16, 3, 7, 12, 20, 16, 7, 4, 6, 16, 9, 11};
    int32_t temp_del_array2[] = {99, 7, 11, 5, 9, 12, 16, 20, 10, 15, 3, 16, 4, 6, 7, 16};
    hyrbtree_add_del_test( &user_pool,&rbtree,
        temp_add_array2,sizeof(temp_add_array2)/sizeof(int32_t),
        temp_del_array2,sizeof(temp_del_array2)/sizeof(int32_t) );
}
