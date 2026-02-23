#ifndef PA2_H
#define PA2_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Represents a node in the strictly binary tree
typedef struct TreeNode {
    int id;
    char cut_type;
    int width;
    int height;
    int x;
    int y;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

typedef struct StackNode {
    TreeNode *tree_node;
    struct StackNode *next;
} StackNode;

typedef struct Stack {
    StackNode *top;
} Stack;

TreeNode* create_leaf_node(int id, int width, int height);
TreeNode* create_internal_node(char cut_type);
void free_tree(TreeNode *root);

void push(Stack *s, TreeNode *node);
TreeNode* pop(Stack *s);
bool is_empty(Stack *s);

#endif