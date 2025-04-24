/**
 * @file hyrbtree.c
 * @brief Embedded Red-Black Tree Implementation
 */

#include "hyrbtree.h"



/* Macro to set node color by bit manipulation */
#define HYRBTREE_SET_NODE_RED(n)        {n->user_node = (void *)((hy_uptr_t)(n->user_node) & ~(hy_uptr_t)(0x1));}
#define HYRBTREE_SET_NODE_BLACK(n)      {n->user_node = (void *)((hy_uptr_t)(n->user_node) | (hy_uptr_t)(0x1));}

/* Rotation cases for insertion balancing */
enum{
    RBNODE_ADD_ROTATE_LL,
    RBNODE_ADD_ROTATE_RL,
    RBNODE_ADD_ROTATE_LR,
    RBNODE_ADD_ROTATE_RR,
};

/* Rotation cases for deletion balancing */
enum{
    RBNODE_DEL_LEFT_SILING,
    RBNODE_DEL_ROTATE_LL_0,
    RBNODE_DEL_ROTATE_LR,
    RBNODE_DEL_ROTATE_LL_1,
    RBNODE_DEL_RIGHT_SILING,
    RBNODE_DEL_ROTATE_RL,
    RBNODE_DEL_ROTATE_RR_0,
    RBNODE_DEL_ROTATE_RR_1,
};



/**
 * @brief Initialize a Red-Black Tree
 * @param tree Pointer to the tree structure
 * 
 * Sets root to nil node and initializes nil node as black.
 */
void hyrbtree_init( hyrbtree_t *tree ){
    tree->root_node = &tree->nil_node;
    tree->nil_node.user_node = (void *)((hy_uptr_t)(tree->nil_node.user_node) | (hy_uptr_t)(0x1));
}



/**
 * @brief Perform left rotation
 * @param node Pivot node for rotation
 * 
 * Used during tree rebalancing. Modifies tree topology while preserving BST properties.
 */
static void hyrbtree_left_rotate_node( hyrbnode_t *node ){
    hyrbnode_t *center_node;

    center_node = node->right_node;
    if( node==node->parent_node->left_node ){
        node->parent_node->left_node = center_node;
    }
    else{
        node->parent_node->right_node = center_node;
    }
    center_node->parent_node = node->parent_node;

    node->parent_node = center_node;
    node->right_node = center_node->left_node;

    center_node->left_node = node;
    node->right_node->parent_node = node;
}

/**
 * @brief Perform right rotation
 * @param node Pivot node for rotation
 * 
 * Mirror operation of left_rotate_node.
 */
static void hyrbtree_right_rotate_node( hyrbnode_t *node ){
    hyrbnode_t *center_node;

    center_node = node->left_node;
    if( node==node->parent_node->left_node ){
        node->parent_node->left_node = center_node;
    }
    else{
        node->parent_node->right_node = center_node;
    }
    center_node->parent_node = node->parent_node;

    node->parent_node = center_node;
    node->left_node = center_node->right_node;

    center_node->right_node = node;
    node->left_node->parent_node = node;
}



/**
 * @brief Rebalance tree after insertion
 * @param rbtree Tree structure
 * @param cur_node Newly inserted node
 * 
 * Handles 4 rotation cases to maintain R-B properties.
 */
void hyrbtree_add_balance( hyrbtree_t *rbtree , hyrbnode_t *cur_node ){
    hyrbnode_t *parent_rbnode;
    hyrbnode_t *grandpa_rbnode;
    hyrbnode_t *uncle_rbnode;
    hy_u8_t balance_case;

    HYRBTREE_SET_NODE_RED(cur_node);
    parent_rbnode = cur_node->parent_node;
    grandpa_rbnode = parent_rbnode->parent_node;
    balance_case = 0;

    while(1){
        if( parent_rbnode==grandpa_rbnode->left_node ){
            uncle_rbnode = grandpa_rbnode->right_node;
        }
        else{
            uncle_rbnode = grandpa_rbnode->left_node;
            balance_case = balance_case+1;
        }

        if( HYRBTREE_READ_NODE_COLOR(uncle_rbnode)==HYRBTREE_NODE_RED ){
            HYRBTREE_SET_NODE_BLACK(parent_rbnode);
            HYRBTREE_SET_NODE_BLACK(uncle_rbnode);
            cur_node = grandpa_rbnode;
            if( cur_node!=rbtree->root_node ){
                HYRBTREE_SET_NODE_RED(cur_node);

                parent_rbnode = cur_node->parent_node;
                if( HYRBTREE_READ_NODE_COLOR(parent_rbnode)==HYRBTREE_NODE_RED ){
                    grandpa_rbnode = parent_rbnode->parent_node;
                    balance_case = 0;
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }
        else{
            if( cur_node!=parent_rbnode->left_node ){
                balance_case = balance_case+2;
            }
            switch( balance_case ){
                case RBNODE_ADD_ROTATE_LL:
                    HYRBTREE_SET_NODE_BLACK(parent_rbnode);
                    hyrbtree_right_rotate_node(grandpa_rbnode);
                    break;
                case RBNODE_ADD_ROTATE_RL:
                    HYRBTREE_SET_NODE_BLACK(cur_node);
                    hyrbtree_right_rotate_node(parent_rbnode);
                    hyrbtree_left_rotate_node(grandpa_rbnode);
                    break;
                case RBNODE_ADD_ROTATE_LR:
                    HYRBTREE_SET_NODE_BLACK(cur_node);
                    hyrbtree_left_rotate_node(parent_rbnode);
                    hyrbtree_right_rotate_node(grandpa_rbnode);
                    break;
                case RBNODE_ADD_ROTATE_RR:
                    HYRBTREE_SET_NODE_BLACK(parent_rbnode);
                    hyrbtree_left_rotate_node(grandpa_rbnode);
                    break;
                default:
                    break;
            }
            HYRBTREE_SET_NODE_RED(grandpa_rbnode);
            
            if( grandpa_rbnode==rbtree->root_node ){
                if( balance_case%3==0 ){
                    rbtree->root_node = parent_rbnode;
                }
                else{
                    rbtree->root_node = cur_node;
                }
                rbtree->root_node->parent_node = &rbtree->nil_node;
                rbtree->nil_node.left_node = rbtree->root_node;
            }
            break;
        }
    }
}

/**
 * @brief Insert a node into the tree
 * @param tree Tree structure
 * @param user_node User data containing embedded rbnode
 * @param exist_node [out] Returns existing node if key exists
 * @return Operation status code
 * 
 * Performs standard BST insertion followed by rebalancing.
 * Returns:
 * - HYRBTREE_RET_OK: Success
 * - HYRBTREE_RET_ADD_NODE_ELEM_EXIST: Key collision
 * - HYRBTREE_RET_ADD_NODE_UNINITIALIZED: Invalid node
 */
hyrbtree_ret_t hyrbtree_add_node( hyrbtree_t *tree,void *user_node,void **exist_node ){
    hyrbnode_t *add_node;
    hyrbnode_t *cur_node;
    hyrbnode_t *parent_node;
    hy_i32_t result;
    void *add_node_elem;
    void *cur_node_elem;

    add_node = tree->get_rbnode(user_node);
    if( HYRBTREE_GET_NODE_ADDR(add_node)==HY_NULL ){

        add_node->left_node = &tree->nil_node;
        add_node->right_node = &tree->nil_node;

        if( tree->root_node!=&tree->nil_node ){

            add_node_elem = tree->get_elem(user_node);
            cur_node = tree->root_node;
            
            while(1){
                cur_node_elem = tree->get_elem(HYRBTREE_GET_NODE_ADDR(cur_node));
                parent_node = cur_node;

                result = tree->cmp_elem(add_node_elem,cur_node_elem);
                if( result<0 ){
                    cur_node = cur_node->left_node;
                    if( cur_node==&tree->nil_node ){
                        parent_node->left_node = add_node;
                        add_node->parent_node = parent_node;
                        break;
                    }
                }
                else if( result>0 ){
                    cur_node = cur_node->right_node;
                    if( cur_node==&tree->nil_node ){
                        parent_node->right_node = add_node;
                        add_node->parent_node = parent_node;
                        break;
                    }
                }
                else{
                    *exist_node = HYRBTREE_GET_NODE_ADDR(cur_node);
                    return HYRBTREE_RET_ADD_NODE_ELEM_EXIST;
                }
            }

            add_node->user_node = user_node;

            if( HYRBTREE_READ_NODE_COLOR(add_node->parent_node)==HYRBTREE_NODE_RED ){
                hyrbtree_add_balance(tree,add_node);
            }
        }
        else{
            add_node->user_node = user_node;
            
            tree->root_node = add_node;
            HYRBTREE_SET_NODE_BLACK(tree->root_node);
            tree->root_node->parent_node = &tree->nil_node;
            tree->nil_node.left_node = tree->root_node;
        }
        return HYRBTREE_RET_OK;
    }
    return HYRBTREE_RET_ADD_NODE_UNINITIALIZED;
}



/**
 * @brief Replace node with its in-order successor
 * @param tree Tree structure
 * @param node Node to be replaced
 * 
 * Helper function for deletion. Preserves color properties during replacement.
 */
void hyrbtree_replace_successor( hyrbtree_t *tree,hyrbnode_t *node ){
    hyrbnode_t *cur_node;

    if( node->right_node!=&tree->nil_node ){

        cur_node = node->right_node;
        while( cur_node->left_node!=&tree->nil_node ){
            cur_node = cur_node->left_node;
        }

        if( HYRBTREE_READ_NODE_COLOR(node)==HYRBTREE_NODE_RED ){
            if( HYRBTREE_READ_NODE_COLOR(cur_node)!=HYRBTREE_NODE_RED ){
                HYRBTREE_SET_NODE_BLACK(node);
                HYRBTREE_SET_NODE_RED(cur_node);
            }
        }
        else{
            if( HYRBTREE_READ_NODE_COLOR(cur_node)==HYRBTREE_NODE_RED ){
                HYRBTREE_SET_NODE_RED(node);
                HYRBTREE_SET_NODE_BLACK(cur_node);
            }
        }

        cur_node->left_node = node->left_node;
        node->left_node->parent_node = cur_node;
        node->left_node = &tree->nil_node;

        if( node->parent_node->left_node==node ){
            node->parent_node->left_node = cur_node;
        }
        else{
            node->parent_node->right_node = cur_node;
        }

        if( cur_node!=node->right_node ){
            node->right_node->parent_node = cur_node;
            cur_node->parent_node->left_node = node;
            cur_node->right_node->parent_node = node;

            tree->nil_node.parent_node = cur_node->parent_node;
            tree->nil_node.right_node = cur_node->right_node;
            cur_node->parent_node = node->parent_node;
            cur_node->right_node = node->right_node;
            node->parent_node = tree->nil_node.parent_node;
            node->right_node = tree->nil_node.right_node;
        }
        else{
            tree->nil_node.right_node = cur_node->right_node;
            cur_node->parent_node = node->parent_node;
            cur_node->right_node = node;
            node->parent_node = cur_node;
            node->right_node = tree->nil_node.right_node;
        }
        
        if( node==tree->root_node ){
            tree->root_node = cur_node;
        }
    }
}

/**
 * @brief Rebalance tree after deletion
 * @param rbtree Tree structure
 * @param cur_node Node being removed
 * 
 * Handles 8 possible deletion cases to maintain R-B properties.
 */
void hyrbtree_del_balance( hyrbtree_t *rbtree , hyrbnode_t *cur_node ){
    hyrbnode_t *parent_rbnode;
    hyrbnode_t *sibling_rbnode;
    hy_u8_t balance_case;
    
    if( cur_node->right_node!=&rbtree->nil_node ){
        if( cur_node->parent_node->left_node==cur_node ){
            cur_node->parent_node->left_node = cur_node->right_node;
        }
        else{
            cur_node->parent_node->right_node = cur_node->right_node;
        }
        cur_node->right_node->parent_node = cur_node->parent_node;
        HYRBTREE_SET_NODE_BLACK(cur_node->right_node);
    }
    else if( cur_node->left_node!=&rbtree->nil_node ){
        if( cur_node->parent_node->left_node==cur_node ){
            cur_node->parent_node->left_node = cur_node->left_node;
        }
        else{
            cur_node->parent_node->right_node = cur_node->left_node;
        }
        cur_node->left_node->parent_node = cur_node->parent_node;
        HYRBTREE_SET_NODE_BLACK(cur_node->left_node);
        if( cur_node==rbtree->root_node ){
            rbtree->root_node = cur_node->left_node;
            rbtree->root_node->parent_node = &rbtree->nil_node;
            rbtree->nil_node.left_node = rbtree->root_node;
        }
    }
    else if( HYRBTREE_READ_NODE_COLOR(cur_node)==HYRBTREE_NODE_RED ){
        if( cur_node->parent_node->left_node==cur_node ){
            cur_node->parent_node->left_node = &rbtree->nil_node;
        }
        else{
            cur_node->parent_node->right_node = &rbtree->nil_node;
        }
    }
    else{
        if( cur_node==rbtree->root_node ){
            rbtree->root_node = &rbtree->nil_node;
            return ;
        }

        balance_case = 0;
        parent_rbnode = cur_node->parent_node;
        
        if( cur_node==parent_rbnode->left_node ){
            parent_rbnode->left_node = &rbtree->nil_node;
        }
        else{
            parent_rbnode->right_node = &rbtree->nil_node;
        }
        cur_node = &rbtree->nil_node;

        while(1){
            if( cur_node==parent_rbnode->left_node ){
                sibling_rbnode = parent_rbnode->right_node;
                balance_case = 4;
            }
            else{
                sibling_rbnode = parent_rbnode->left_node;
            }

            if( HYRBTREE_READ_NODE_COLOR(sibling_rbnode)!=HYRBTREE_NODE_RED ){
                if( HYRBTREE_READ_NODE_COLOR(sibling_rbnode->left_node)==HYRBTREE_NODE_RED ){
                    balance_case = balance_case+1;
                }
                if( HYRBTREE_READ_NODE_COLOR(sibling_rbnode->right_node)==HYRBTREE_NODE_RED ){
                    balance_case = balance_case+2;
                }

                switch( balance_case ){
                    case RBNODE_DEL_LEFT_SILING:
                    case RBNODE_DEL_RIGHT_SILING:
                        HYRBTREE_SET_NODE_RED(sibling_rbnode);
                        if( parent_rbnode!=rbtree->root_node ){
                            if( HYRBTREE_READ_NODE_COLOR(parent_rbnode)!=HYRBTREE_NODE_RED ){
                                cur_node = parent_rbnode;
                                parent_rbnode = cur_node->parent_node;
                                balance_case = 0;
                                continue;
                            }
                            else{
                                HYRBTREE_SET_NODE_BLACK(parent_rbnode);
                            }
                        }
                        return;

                    case RBNODE_DEL_ROTATE_LR:
                        if( HYRBTREE_READ_NODE_COLOR(parent_rbnode)==HYRBTREE_NODE_RED ){
                            HYRBTREE_SET_NODE_RED(sibling_rbnode->right_node);
                        }
                        else{
                            HYRBTREE_SET_NODE_BLACK(sibling_rbnode->right_node);
                        }
                        HYRBTREE_SET_NODE_BLACK(parent_rbnode);
                        hyrbtree_left_rotate_node(sibling_rbnode);
                        hyrbtree_right_rotate_node(parent_rbnode);
                        break;

                    case RBNODE_DEL_ROTATE_LL_0:
                    case RBNODE_DEL_ROTATE_LL_1:
                        HYRBTREE_SET_NODE_BLACK(sibling_rbnode->left_node);
                        if( HYRBTREE_READ_NODE_COLOR(parent_rbnode)==HYRBTREE_NODE_RED ){
                            HYRBTREE_SET_NODE_RED(sibling_rbnode);
                        }
                        else{
                            HYRBTREE_SET_NODE_BLACK(sibling_rbnode);
                        }
                        HYRBTREE_SET_NODE_BLACK(parent_rbnode);
                        hyrbtree_right_rotate_node(parent_rbnode);
                        break;

                    case RBNODE_DEL_ROTATE_RL:
                        if( HYRBTREE_READ_NODE_COLOR(parent_rbnode)==HYRBTREE_NODE_RED ){
                            HYRBTREE_SET_NODE_RED(sibling_rbnode->left_node);
                        }
                        else{
                            HYRBTREE_SET_NODE_BLACK(sibling_rbnode->left_node);
                        }
                        HYRBTREE_SET_NODE_BLACK(parent_rbnode);
                        hyrbtree_right_rotate_node(sibling_rbnode);
                        hyrbtree_left_rotate_node(parent_rbnode);
                        break;

                    case RBNODE_DEL_ROTATE_RR_0:
                    case RBNODE_DEL_ROTATE_RR_1:
                        HYRBTREE_SET_NODE_BLACK(sibling_rbnode->right_node);
                        if( HYRBTREE_READ_NODE_COLOR(parent_rbnode)==HYRBTREE_NODE_RED ){
                            HYRBTREE_SET_NODE_RED(sibling_rbnode);
                        }
                        else{
                            HYRBTREE_SET_NODE_BLACK(sibling_rbnode);
                        }
                        HYRBTREE_SET_NODE_BLACK(parent_rbnode);
                        hyrbtree_left_rotate_node(parent_rbnode);
                        break;
                    
                    default:
                        break;
                }
                
                if( parent_rbnode==rbtree->root_node ){
                    if( balance_case%3==2 ){
                        rbtree->root_node = sibling_rbnode->parent_node;
                    }
                    else{
                        rbtree->root_node = sibling_rbnode;
                    }
                    rbtree->root_node->parent_node = &rbtree->nil_node;
                    rbtree->nil_node.left_node = rbtree->root_node;
                }
                break;
            }
            else{
                HYRBTREE_SET_NODE_BLACK(sibling_rbnode);
                HYRBTREE_SET_NODE_RED(parent_rbnode);
                switch( balance_case ){
                    case RBNODE_DEL_LEFT_SILING:
                        hyrbtree_right_rotate_node(parent_rbnode);
                        break;
                    case RBNODE_DEL_RIGHT_SILING:
                        hyrbtree_left_rotate_node(parent_rbnode);
                        break;
                    default:
                        break;
                }
                balance_case = 0;

                if( parent_rbnode==rbtree->root_node ){
                    rbtree->root_node = sibling_rbnode;
                    rbtree->root_node->parent_node = &rbtree->nil_node;
                    rbtree->nil_node.left_node = rbtree->root_node;
                }
            }
        }
    }
}

/**
 * @brief Remove a node from the tree
 * @param tree Tree structure
 * @param user_node User data containing embedded rbnode
 * @return Operation status code
 * 
 * Returns:
 * - HYRBTREE_RET_OK: Success
 * - HYRBTREE_RET_DEL_NODE_ARGS_ERROR: Invalid node
 */
hyrbtree_ret_t hyrbtree_del_node( hyrbtree_t *tree,void *user_node ){
    hyrbnode_t *node;

    node = tree->get_rbnode(user_node);
    if( HYRBTREE_GET_NODE_ADDR(node)==user_node ){
        hyrbtree_replace_successor( tree,node );
        hyrbtree_del_balance( tree,node );
        node->user_node = HY_NULL;
        return HYRBTREE_RET_OK;
    }
    return HYRBTREE_RET_DEL_NODE_ARGS_ERROR;
}



/**
 * @brief Search for a node by key
 * @param tree Tree structure
 * @param get_node_elem Key to search for
 * @param get_node [out] Found node
 * @return Operation status code
 * 
 * Returns:
 * - HYRBTREE_RET_OK: Found
 * - HYRBTREE_RET_GET_NODE_NOT_FIND: Not found
 * - HYRBTREE_RET_GET_NODE_TREE_NULL: Empty tree
 */
hyrbtree_ret_t hyrbtree_get_node( hyrbtree_t *tree,void *get_node_elem,void **get_node ){
    hyrbnode_t *cur_node;
    void *cur_node_elem;
    hy_i32_t result;

    if( tree->root_node!=&tree->nil_node ){
        cur_node = tree->root_node;
        while(1){
            cur_node_elem = tree->get_elem(HYRBTREE_GET_NODE_ADDR(cur_node));
            result = tree->cmp_elem(get_node_elem,cur_node_elem);
            if( result<0 ){
                cur_node = cur_node->left_node;
                if( cur_node==&tree->nil_node ){
                    return HYRBTREE_RET_GET_NODE_NOT_FIND;
                }
            }
            else if( result>0 ){
                cur_node = cur_node->right_node;
                if( cur_node==&tree->nil_node ){
                    return HYRBTREE_RET_GET_NODE_NOT_FIND;
                }
            }
            else{
                *get_node = HYRBTREE_GET_NODE_ADDR(cur_node);
                return HYRBTREE_RET_OK;
            }
        }
    }
    return HYRBTREE_RET_GET_NODE_TREE_NULL;
}



/**
 * @brief Replace a node while preserving tree structure
 * @param tree Tree structure
 * @param old_node Existing node
 * @param new_node Replacement node
 * @return Operation status code
 * 
 * Performs in-place replacement if keys match.
 * Returns:
 * - HYRBTREE_RET_OK: Success
 * - HYRBTREE_RET_REPLACE_CMP_ERROR: Key mismatch
 * - HYRBTREE_RET_REPLACE_INIT_ERROR: Invalid nodes
 */
hyrbtree_ret_t hyrbtree_replace_node( hyrbtree_t *tree,void *old_node,void *new_node ){
    hyrbnode_t *old_rbnode;
    hyrbnode_t *new_rbnode;
    void *old_elem;
    void *new_elem;
    hy_i32_t result;

    old_rbnode = tree->get_rbnode(old_node);
    new_rbnode = tree->get_rbnode(new_node);

    if( HYRBTREE_GET_NODE_ADDR(old_rbnode)==old_node && 
        HYRBTREE_GET_NODE_ADDR(new_rbnode)==HY_NULL ){

        old_elem = tree->get_elem(old_node);
        new_elem = tree->get_elem(new_node);

        result = tree->cmp_elem(old_elem,new_elem);
        if( result==0 ){
            new_rbnode->user_node = new_node;
            if( HYRBTREE_READ_NODE_COLOR(old_rbnode)==HYRBTREE_NODE_RED ){
                HYRBTREE_SET_NODE_RED(new_rbnode);
            }
            else{
                HYRBTREE_SET_NODE_BLACK(new_rbnode);
            }
            old_rbnode->user_node = HY_NULL;

            new_rbnode->parent_node = old_rbnode->parent_node;
            new_rbnode->left_node = old_rbnode->left_node;
            new_rbnode->right_node = old_rbnode->right_node;

            if( old_rbnode->parent_node->left_node==old_rbnode ){
                old_rbnode->parent_node->left_node = new_rbnode;
            }
            else{
                old_rbnode->parent_node->right_node = new_rbnode;
            }
            old_rbnode->left_node->parent_node = new_rbnode;
            old_rbnode->right_node->parent_node = new_rbnode;

            return HYRBTREE_RET_OK;
        }
        return HYRBTREE_RET_REPLACE_CMP_ERROR;
    }
    return HYRBTREE_RET_REPLACE_INIT_ERROR;
}
