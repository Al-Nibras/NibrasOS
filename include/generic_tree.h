#ifndef __GENERIC_TREE_H
#define __GENERIC_TREE_H

#ifndef __TYPES_H
    #include <types.h>
#endif

#ifndef __LIST_H
    #include <list.h>
#endif

typedef struct gtreenode {
    list_t * children;
    char  value[12];
}gtreenode_t;

typedef struct gtree {
    gtreenode_t * root;
}gtree_t;

gtree_t * tree_create();

gtreenode_t * treenode_create(const char * value);

gtreenode_t * tree_insert(gtree_t * tree, gtreenode_t * subroot, const char * value);

gtreenode_t * tree_find_parent(gtree_t * tree, gtreenode_t * remove_node, int * child_index);

gtreenode_t * tree_find_parent_recur(gtree_t * tree, gtreenode_t * remove_node, gtreenode_t * subroot, int * child_index);

void tree_remove(gtree_t * tree, gtreenode_t * remove_node);

void tree2list_recur(gtreenode_t * subroot, list_t * list);

void tree2list(gtree_t * tree, list_t * list);

void tree2array(gtree_t * tree, char ** array, size_t size);

void tree2array_recur(gtreenode_t * subroot, char ** array, size_t size);

#endif
