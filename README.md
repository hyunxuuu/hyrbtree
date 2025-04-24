[English](README.md)|[中文](README_zh.md)

#   Encoding
UTF-8

#   Hyrbtree
Hyrbtree is a C-based implementation of Red-Black Tree. The project adopts a design strategy of callback registration and node embedding, which enables generic support and delegates memory management control to users.

#   Features
-   Abstracts user types through registered callback functions.
-   Completely decouples memory management using node embedding.
-   Requires only necessary standard libraries with no additional external dependencies.
-   Extremely small compiled size - less than 3kB of flash memory.
-   MIT licensed - free for commercial use.

#   Getting Started
1.  Check and replace the existing types in hystd.h according to your device support
    ```
    #define HY_NULL                             (NULL)

    typedef uintptr_t                           hy_uptr_t;
    typedef uint8_t 							hy_u8_t;
    typedef int32_t								hy_i32_t;
    ```
2.  Add key value and Red-Black node (and the next_node field for building singly linked lists in case of collisions) when defining your user structure
    ```
    typedef struct user_node_t{
        ...
        int32_t elem;
        hyrbnode_t rbnode;

        user_node_t *next_node;
        ...
    }user_node_t;
    ```
3.  Implement callback functions to get Red-Black node, get key value, and compare key values in your user type
    ```
    hyrbnode_t *user_node_get_rbnode( void *node ){
        return &((user_node_t *)node)->rbnode;
    }
    void *user_node_get_elem( void *node ){
        return &((user_node_t *)node)->elem;
    }
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
    ```
4.  Declare and initialize the Red-Black tree
    ```
    hyrbtree_t rbtree = {
        .get_rbnode = user_node_get_rbnode,
        .get_elem = user_node_get_elem,
        .cmp_elem = user_node_cmp_elem,
    };
    hyrbtree_init( &rbtree );
    ```
5.  Perform operations on the Red-Black tree: add, query, replace, or delete nodes
    ```
    /**
     *  hyrbtree_t *rbtree;
     *  user_node_t *new_node_ptr;
     *  user_node_t *exist_node_ptr;
     *  int32_t temp_elem;
     *  user_node_t *ret_node_ptr;
     *  user_node_t *del_node_ptr;
     */
    ret = hyrbtree_add_node( rbtree,new_node_ptr,(void **)&exist_node_ptr );
    ret = hyrbtree_get_node( rbtree,&temp_elem,(void **)&ret_node_ptr );
    ret = hyrbtree_replace_node( rbtree,del_node_ptr,del_node_ptr->next_node );
    ret = hyrbtree_del_node( rbtree,del_node_ptr );
    ```

#   Testing Instructions
The Example directory contains test files (hyrbtree_test.h, hyrbtree_test). Here's a brief description:

1.  The user type used for testing is user_node_t, with the following members:
    1.  idx: indicates the position of the current user node in the user node pool.
    1.  addr: used to visualize node collisions (different addr under the same elem key).
    1.  elem: the key value of the Red-Black tree node.
    1.  rbnode: instance of the Red-Black tree node.
    1.  next_node: points to the next element in the singly linked list. Default value is NULL.

1.  Uses a static memory pool to manage user nodes. Static memory allocation is performed on the user node pool node_pool based on the index pool idx_pool.

1.  The rbtree_preorder function is used for in-order traversal of the Red-Black tree, containing the following important information:
    1.  Uses the HYRBTREE_GET_NODE_ADDR macro to extract the user node address from the Red-Black node.
    1.  Uses the HYRBTREE_READ_NODE_COLOR macro to extract the node color from the Red-Black node.
    1.  Traverses the addr field from the singly linked list of user nodes under the same elem key.

1. The hyrbtree_add_del_test function performs addition and deletion tests on Red-Black tree nodes:
    1.  Elements in add_array will be used as key values for user node elem. After allocating user nodes, they will be added to the Red-Black tree.
    1.  Elements in del_array will be used as key values for querying the Red-Black tree. Upon successful query, the corresponding user node will be deleted.
    1.  If collisions occur when adding user nodes, builds a singly linked list between the existing user node and the new user node.
    1.  If a successor node is found in the singly linked list when deleting a user node, replaces it with the successor user node before deletion.

#   Advantages
-   Red-Black nodes are directly embedded in user data structures, eliminating the need for additional memory allocation.
-   Supports any data type through callback functions for key extraction and comparison.
-   A single user structure can embed multiple Red-Black tree nodes, simplifying the construction of relationships.
-   Completely decouples read-write locks, allowing users to choose lock strategies according to actual needs.
-   Minimalist API design with only 5 core functions (initialization/add/delete/query/replace).

#   License
Hyrbtree follows the MIT license.
