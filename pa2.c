#include "pa2.h"
#include "string.h"

// Creates and initializes a leaf node
TreeNode* create_leaf_node(int id, int width, int height) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (node == NULL) {
        fprintf(stderr, "Malloc failed for leaf node\n");
        exit(EXIT_FAILURE);
    }
    node->id = id;
    node->cut_type = 'L'; // L for leaf block
    node->width = width;
    node->height = height;
    node->x = 0;
    node->y = 0;
    node->left = NULL; // leaves have no children
    node->right = NULL; // leaves have no children
    return node;
}

// Creates a node to represent either horizontal or vertical cut
TreeNode* create_internal_node(char cut_type) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (node == NULL) {
        fprintf(stderr, "Malloc failed for internal node\n");
        exit(EXIT_FAILURE);
    }
    // set dimensions to 0 for now
    node->id = 0;
    node->cut_type = cut_type;
    node->width = 0;
    node->height = 0;
    node->x = 0;
    node->y = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Frees tree memory to prevent memory leaks
void free_tree(TreeNode *root) {
    if (root == NULL) {
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// Allocates new StackNode, points to tree node, place at top of stack
void push(Stack *s, TreeNode *node) {
    StackNode *new_node = (StackNode *)malloc(sizeof(StackNode));
    if (new_node == NULL) {
        fprintf(stderr, "Malloc failed for stack node\n");
        exit(EXIT_FAILURE);
    }
    new_node->tree_node = node;
    new_node->next = s->top;
    s->top = new_node;
}

// Removes and frees StackNode
TreeNode* pop(Stack *s) {
    if (is_empty(s)) {
        return NULL;
    }
    StackNode *temp = s->top;
    TreeNode *tree_node = temp->tree_node;
    s->top = s->top->next;
    free(temp);
    return tree_node;
}

// Check if there are any items left
bool is_empty(Stack *s) {
    return s->top == NULL;
}

// Read pre-order traversal from input file and build strictly binary tree
TreeNode* Load_Tree(const char* filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open input file %s\n", filename);
        return NULL;
    }

    Stack s;
    s.top = NULL;
    TreeNode *root = NULL;
    char buffer[256];

    while (fscanf(fp, "%s", buffer) == 1) {
        TreeNode *new_node = NULL;

        // Internal node is either H or V
        if (buffer[0] == 'H' || buffer[0] == 'V') {
            new_node = create_internal_node(buffer[0]);
        } 
        // Leaf node is formatted as (width, height)
        else {
            int id, width, height;
            if (sscanf(buffer, "%d(%d,%d)", &id, &width, &height) == 3) {
                new_node = create_leaf_node(id, width, height);
            } else {
                fprintf(stderr, "Error parsing leaf node: %s\n", buffer);
                fclose(fp);
                return NULL;
            }
        }

        // Tree attachment logic
        if (root == NULL) {
            root = new_node;
        } else {
            if (is_empty(&s)) {
                fprintf(stderr, "Invalid tree\n");
                break;
            }
            
            TreeNode *parent = s.top->tree_node; 

            if (parent->left == NULL) {
                parent->left = new_node;
            } else if (parent->right == NULL) {
                parent->right = new_node;
                // Parent node has both children required by strictly binary tree 
                pop(&s); 
            }
        }

        // If new node is an internal node, it needs children, so push it onto the stack
        if (new_node->cut_type == 'H' || new_node->cut_type == 'V') {
            push(&s, new_node);
        }
    }

    fclose(fp);
    return root;
}

// Processes using post-order traversal to determine dimensions of the smallest rectangular room
void Calculate_Dimensions(TreeNode *root) {
    if (root == NULL) {
        return;
    }

    Stack s;
    s.top = NULL;
    TreeNode *current = root;
    TreeNode *last_visited = NULL;

    while (current != NULL || !is_empty(&s)) {
        if (current != NULL) {
            push(&s, current);
            current = current->left;
        } else {
            TreeNode *peek_node = s.top->tree_node;
            
            // If right child exists and traversing node from left child, move right
            if (peek_node->right != NULL && last_visited != peek_node->right) {
                current = peek_node->right;
            } else {
                if (peek_node->cut_type == 'H') {
                    // For horizontal cut, rooms are stacked top and bottom
                    int w_left = peek_node->left->width;
                    int w_right = peek_node->right->width;
                    
                    peek_node->width = (w_left > w_right) ? w_left : w_right;
                    peek_node->height = peek_node->left->height + peek_node->right->height;
                } 
                else if (peek_node->cut_type == 'V') {
                    // For vertical cut, the rooms are side-by-side
                    int h_left = peek_node->left->height;
                    int h_right = peek_node->right->height;
                    
                    peek_node->width = peek_node->left->width + peek_node->right->width;
                    peek_node->height = (h_left > h_right) ? h_left : h_right;
                }
                
                last_visited = pop(&s);
            }
        }
    }
}

// Processes using pre-order traversal
void Calculate_Coordinates(TreeNode *root) {
    if (root == NULL) {
        return;
    }

    // Root of entire packing is always at (0,0)
    root->x = 0;
    root->y = 0;

    Stack s;
    s.top = NULL;
    push(&s, root);

    while (!is_empty(&s)) {
        TreeNode *curr = pop(&s);

        // If leaf, there are no children to process
        if (curr->cut_type == 'L') {
            continue;
        }

        TreeNode *left_child = curr->left;   // Represents "Top"  or "Left"
        TreeNode *right_child = curr->right; // Represents "Bottom" or "Right"

        if (left_child == NULL || right_child == NULL) {
            continue; 
        }

        if (curr->cut_type == 'H') {
            // Bottom room stays at the parent's (x, y)
            
            // Bottom Child (Right)
            right_child->x = curr->x;
            right_child->y = curr->y;

            // Top Child (Left)
            left_child->x = curr->x;
            left_child->y = curr->y + right_child->height;
        } 
        else if (curr->cut_type == 'V') {
            // Left room stays at the parent's (x, y)

            // Left Child (Left)
            left_child->x = curr->x;
            left_child->y = curr->y;

            // Right Child (Right)
            right_child->x = curr->x + left_child->width;
            right_child->y = curr->y;
        }

        // Push children to stack to process them next.
        push(&s, right_child);
        push(&s, left_child);
    }
}

// Traverse tree in post-order and print node
void Save_PostOrder(TreeNode *root, const char *filename) {
    // File to write output
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Can't open output file %s\n", filename);
        return;
    }

    if (root == NULL) {
        fclose(fp);
        return;
    }

    Stack s;
    s.top = NULL;
    TreeNode *current = root;
    TreeNode *last_visited = NULL;

    while (current != NULL || !is_empty(&s)) {
        if (current != NULL) {
            push(&s, current);
            current = current->left;
        } else {
            TreeNode *peek_node = s.top->tree_node;
            
            if (peek_node->right != NULL && last_visited != peek_node->right) {
                current = peek_node->right;
            } else {
                // Visit Node
                if (peek_node->cut_type == 'L') {
                    fprintf(fp, "%d(%d,%d)\n", peek_node->id, peek_node->width, peek_node->height);
                } else {
                    fprintf(fp, "%c\n", peek_node->cut_type);
                }
                
                last_visited = pop(&s);
            }
        }
    }
    fclose(fp);
}


// Traverse tree in post-order and output to out_file2
void Save_Dimensions(TreeNode *root, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Can't open output file %s\n", filename);
        return;
    }

    if (root == NULL) {
        fclose(fp);
        return;
    }

    Stack s;
    s.top = NULL;
    TreeNode *current = root;
    TreeNode *last_visited = NULL;

    while (current != NULL || !is_empty(&s)) {
        if (current != NULL) {
            push(&s, current);
            current = current->left;
        } else {
            TreeNode *peek_node = s.top->tree_node;
            
            if (peek_node->right != NULL && last_visited != peek_node->right) {
                current = peek_node->right;
            } else {
                // Visit Node
                if (peek_node->cut_type == 'L') {
                    fprintf(fp, "%d(%d,%d)\n", peek_node->id, peek_node->width, peek_node->height);
                } else {
                    fprintf(fp, "%c(%d,%d)\n", peek_node->cut_type, peek_node->width, peek_node->height);
                }
                
                last_visited = pop(&s);
            }
        }
    }
    fclose(fp);
}

// Traverses tree in pre-order to match the original input file's block ordering
void Save_Packing(TreeNode *root, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Can't open output file %s\n", filename);
        return;
    }

    if (root == NULL) {
        fclose(fp);
        return;
    }

    Stack s;
    s.top = NULL;
    push(&s, root);

    while (!is_empty(&s)) {
        TreeNode *curr = pop(&s);

        // If leaf, print details
        if (curr->cut_type == 'L') {
            fprintf(fp, "%d((%d,%d)(%d,%d))\n", curr->id, curr->width, curr->height, curr->x, curr->y);
        }

        // Push children (Right then Left for pre-order)
        if (curr->right != NULL) push(&s, curr->right);
        if (curr->left != NULL) push(&s, curr->left);
    }
    fclose(fp);
}

// Main Function

int main(int argc, char **argv) {
    // Check argument count
    if (argc != 5) {
        fprintf(stderr, "Usage: ./pa2 in_file out_file1 out_file2 out_file3\n");
        return EXIT_FAILURE;
    }

    // Load Tree
    TreeNode *root = Load_Tree(argv[1]);
    if (root == NULL) {
        return EXIT_FAILURE;
    }

    // Calculate Dimensions
    Calculate_Dimensions(root);

    // Calculate Coordinates
    Calculate_Coordinates(root);

    // Generate Outputs
    Save_PostOrder(root, argv[2]);

    // Dimensions (post-order)
    Save_Dimensions(root, argv[3]);

    // out_file3: Packing coordinates (Pre-order/Input order)
    Save_Packing(root, argv[4]);

    free_tree(root);

    return EXIT_SUCCESS;
}