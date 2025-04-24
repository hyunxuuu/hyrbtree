[English](README.md)|[中文](README_zh.md)

#   编码
UTF-8

#   Hyrbtree
Hyrbtree是一个基于C的实现红黑树.该项目采用注册回调与节点内嵌的设计策略.这可以实现泛型支持并将内存管理的控制权交付给用户.

#   特点
-   通过注册回调函数完成用户类型的抽象.
-   采用节点内嵌的方式将内存管理完全解耦.
-   仅需必要的标准库.无需额外的外部依赖.
-   编译体积极小.仅需3kB不到的闪存.
-   MIT许可.可自由商用.

#   快速上手
1.  检查并替换 hystd.h 里设备支持的现有类型
    ```
    #define HY_NULL                             (NULL)

    typedef uintptr_t                           hy_uptr_t;
    typedef uint8_t 							hy_u8_t;
    typedef int32_t								hy_i32_t;
    ```
1.  在定义用户结构体时添加键值与红黑节点(以及用于解决冲突时构建单向链表的next_node字段)
    ```
    typedef struct user_node_t{
        ...
        int32_t elem;       // rbnode elem
        hyrbnode_t rbnode;  // rbtree manage

        uint32_t addr;
        user_node_t *next_node;
        ...
    }user_node_t;
    ```
1.  实现在用户类型中获取红黑节点.获取键值与比较键值的回调函数
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
1.  声明并初始化红黑树
    ```
    hyrbtree_t rbtree = {
        .get_rbnode = user_node_get_rbnode,
        .get_elem = user_node_get_elem,
        .cmp_elem = user_node_cmp_elem,
    };
    hyrbtree_init( &rbtree );
    ```
1.  对红黑树进行新增,查询,替换或删除的操作:
    ```
    /**
     *  hyrbtree_ret_t ret;
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
1.  实现对红黑树的中序遍历(包含对单向链表的遍历):
    ```
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
    ```
    
#   测试说明
在Example目录下包含测试文件(hyrbtree_test.h,hyrbtree_test).下面对测试文件进行简述:
1.  测试所用的用户类型user_node_t.其成员说明如下:
    1.  idx:表示当前用户节点位于用户节点池的位置.
    1.  addr:用于可视化节点冲突(同一elem键值下不同的addr).
    1.  elem:红黑树节点的键值.
    1.  rbnode:红黑树节点的实例.
    1.  next_node:指向单向链表的下一个元素.默认值为NULL.
1.  使用静态内存池管理用户节点.基于下标索引池idx_pool在用户节点池node_pool上完成静态内存分配.
1.  rbtree_preorder函数用于对红黑树进行中序遍历.其包含以下重要信息:
    1.  使用HYRBTREE_GET_NODE_ADDR宏从红黑节点中提取用户节点的地址.
    1.  使用HYRBTREE_READ_NODE_COLOR宏从红黑节点中提取节点颜色.
    1.  从同一elem键值下用户节点构成的单向链表中遍历addr字段.
1.  hyrbtree_add_del_test函数将进行红黑树节点新增与删除的测试.其测试说明如下:
    1.  add_array内的各元素将作为用户节点elem的键值.在完成用户节点分配后将其添加至红黑树.
    1.  del_array内的各元素将依次作为查询红黑树时的键值.在查询成功后删除该用户节点.
    1.  若在添加用户节点中发生冲突.则将已存在的用户节点与新的用户节点构建单向链表.
    1.  若在删除用户节点中发现单向链表的后继节点不为空.则替换为后继用户节点再将其删除.

#   优势
-   红黑节点直接嵌入用户数据结构中.无需额外内存分配.
-   通过回调函数实现键值提取和比较.支持任意数据类型.
-   单个用户结构体可嵌入多个红黑树节点.简化关联关系的构建.
-   将读写锁完全解耦.支持用户根据实际需求选择锁策略.
-   极简的API设计.核心API仅5个函数(初始化/增/删/查/替换)

#   许可协议
Hyrbtree 遵循 MIT 开源协议.
