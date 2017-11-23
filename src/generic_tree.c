/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-04-22T11:59:17+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: generic_tree.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:52:32+03:00
 */



#include <generic_tree.h>

gtree_t * tree_create() {
    gtree_t* tree = kmalloc(sizeof(gtree_t));
    memset(tree,0,sizeof(gtree_t));
    return tree;
}

gtreenode_t * treenode_create(const char * value) {
    gtreenode_t * n = kmalloc(sizeof(gtreenode_t));
    memset(n,0,sizeof(gtreenode_t));
    strcpy(n->value,value);
    n->children = list_create();
    return n;
}

gtreenode_t * tree_insert(gtree_t * tree, gtreenode_t * subroot, const char * value) {
    // Create a treenode
    gtreenode_t * treenode = kmalloc(sizeof(gtreenode_t));
    memset(treenode,0,sizeof(gtreenode_t));
    treenode->children = list_create();
    strcpy(treenode->value,value);

    // Insert it
    if(!tree->root) {
        tree->root = treenode;
        return treenode;
    }
    list_insert_front(subroot->children, treenode);
    return treenode;
}

gtreenode_t * tree_find_parent(gtree_t * tree, gtreenode_t * remove_node, int * child_index) {
    // If subroot is the parent
    if(remove_node == tree->root) return NULL;
    return tree_find_parent_recur(tree, remove_node, tree->root, child_index);
}

gtreenode_t * tree_find_parent_recur(gtree_t * tree, gtreenode_t * remove_node, gtreenode_t * subroot, int * child_index) {
    int idx;
    if((idx = list_contain(subroot->children, remove_node)) != -1) {
        *child_index = idx;
        return subroot;
    }
    foreach(child, subroot->children) {
        gtreenode_t * ret = tree_find_parent_recur(tree, remove_node, child->val, child_index);
        if(ret != NULL) {
            return ret;
        }
        // ret is NULL, keep searching.
    }
    return NULL;
}

void tree_remove(gtree_t * tree, gtreenode_t * remove_node) {
    // Search for tree's parent and remove the node from parent's children list
    // If parent is NULL, then just set tree->root to NULL(yeah, I dont care about mem leaks)
    int child_index = -1;
    gtreenode_t * parent = tree_find_parent(tree, remove_node, &child_index);
    // Do treenode remove in here:
    if(parent != NULL) {
        gtreenode_t * freethis = list_remove_by_index(parent->children, child_index);
        // Free tree node here
        kfree(freethis);
    }
}

void tree2list_recur(gtreenode_t * subroot, list_t * list) {
    if(subroot== NULL)
        return;
    foreach(child, subroot->children) {
        gtreenode_t * curr_treenode = (gtreenode_t*)child->val;
        void * curr_val = curr_treenode->value;
        list_insert_back(list, curr_val);
        tree2list_recur(child->val, list);
    }
}

void tree2list(gtree_t * tree, list_t * list) {
    tree2list_recur(tree->root, list);
}

void tree2array(gtree_t * tree, char ** array, size_t size) {
    tree2array_recur(tree->root, array, size);
}

void tree2array_recur(gtreenode_t * subroot, char ** array, size_t size) {
    if(subroot== NULL || !size)
        return;
    char * curr_val = subroot->value;
    array[size-1] = curr_val;
    size--;
    foreach(child, subroot->children) {
        tree2array_recur(child, array, size);
    }
}
void tree_print_recur(gtreenode_t * subroot, char ** array, size_t size) {
    if(subroot== NULL || !size)
        return;
    char * curr_val = subroot->value;
    printf("%s ",curr_val);
    array[size-1] = curr_val;
    size--;
    foreach(child, subroot->children) {
        tree2array_recur(child, array, size);
    }
    printf("\n");
}
