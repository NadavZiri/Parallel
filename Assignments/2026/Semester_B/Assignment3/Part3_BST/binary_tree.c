#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "binary_tree.h"

/* Allocate and return a new node with the given data. */
TreeNode *createNode(int data) {
    TreeNode *newNode = (TreeNode *)malloc(sizeof(TreeNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    
    /* Initialize the OpenMP lock for this specific node */
    omp_init_lock(&newNode->lock);
    
    return newNode;
}

/* Insert `data` into the tree. Returns the (possibly updated) root. */
TreeNode *insertNode(TreeNode *root, int data) {
    /* Edge Case: The tree is completely empty.
       We need a critical section just to safely create the first root node. */
    if (root == NULL) {
        #pragma omp critical
        {
            if (root == NULL) {
                root = createNode(data);
            }
        }
        return root;
    }

    /* Start Hand-over-Hand Locking */
    TreeNode *curr = root;
    omp_set_lock(&curr->lock); // Lock the root

    while (true) {
        if (data < curr->data) {
            if (curr->left == NULL) {
                /* Found the spot. Since we hold curr's lock, 
                   no other thread can insert here right now. */
                curr->left = createNode(data);
                omp_unset_lock(&curr->lock); // Release parent lock
                break;
            } else {
                /* Move down left: Lock child, then unlock parent */
                TreeNode *next = curr->left;
                omp_set_lock(&next->lock);
                omp_unset_lock(&curr->lock);
                curr = next;
            }
        } 
        else if (data > curr->data) {
            if (curr->right == NULL) {
                /* Found the spot. */
                curr->right = createNode(data);
                omp_unset_lock(&curr->lock); // Release parent lock
                break;
            } else {
                /* Move down right: Lock child, then unlock parent */
                TreeNode *next = curr->right;
                omp_set_lock(&next->lock);
                omp_unset_lock(&curr->lock);
                curr = next;
            }
        } 
        else {
            /* Duplicate data found. Do nothing, just release the lock and exit. */
            omp_unset_lock(&curr->lock);
            break;
        }
    }

    return root;
}

/* Delete `data` from the tree if present. Returns the (possibly updated) root. */
TreeNode *deleteNode(TreeNode *root, int data) {
    if (root == NULL) return NULL;

    TreeNode *parent = NULL;
    TreeNode *curr = root;

    omp_set_lock(&curr->lock);

    /* 1. Traversal: Find the node while maintaining a 2-node locked window */
    while (curr->data != data) {
        TreeNode *next = (data < curr->data) ? curr->left : curr->right;

        if (next == NULL) {
            /* Target data not found in the tree. Unlock and exit. */
            omp_unset_lock(&curr->lock);
            if (parent != NULL) omp_unset_lock(&parent->lock);
            return root;
        }

        omp_set_lock(&next->lock);

        /* Hand-over-hand: Release the grandparent, but keep parent and child */
        if (parent != NULL) {
            omp_unset_lock(&parent->lock);
        }
        parent = curr;
        curr = next;
    }

    /* * 2. Target Found. 
     * We currently hold curr's lock AND parent's lock (if parent exists).
     */

    /* Case A: The node has 0 or 1 child */
    if (curr->left == NULL || curr->right == NULL) {
        TreeNode *child = (curr->left != NULL) ? curr->left : curr->right;

        if (parent == NULL) {
            /* Edge Case: Deleting the root node. */
            TreeNode *newRoot = child;
            omp_unset_lock(&curr->lock);
            omp_destroy_lock(&curr->lock);
            free(curr);
            return newRoot; 
        } else {
            /* Standard internal node deletion */
            if (parent->left == curr) {
                parent->left = child;
            } else {
                parent->right = child;
            }

            /* Safe to destroy because the locked parent shielded this node */
            omp_unset_lock(&curr->lock);
            omp_destroy_lock(&curr->lock);
            free(curr);
            
            omp_unset_lock(&parent->lock);
            return root;
        }
    } 
    /* Case B: The node has 2 children */
    else {
        /* * We hold 'parent' and 'curr'. We must find the inorder successor 
         * (the smallest node in the right subtree). 
         */
        TreeNode *succParent = curr;
        TreeNode *succ = curr->right;
        omp_set_lock(&succ->lock);

        while (succ->left != NULL) {
            TreeNode *next = succ->left;
            omp_set_lock(&next->lock);
            
            if (succParent != curr) {
                omp_unset_lock(&succParent->lock);
            }
            succParent = succ;
            succ = next;
        }

        /* * Now holding locks for: parent (maybe), curr, succParent (if != curr), and succ.
         * Copy the successor's data into the current node. 
         */
        curr->data = succ->data;

        /* Remove the successor. It is guaranteed to have NO left child. */
        TreeNode *succChild = succ->right;
        if (succParent->left == succ) {
            succParent->left = succChild;
        } else {
            succParent->right = succChild; /* Happens if succ is exactly curr->right */
        }

        /* Cleanup the successor node */
        omp_unset_lock(&succ->lock);
        omp_destroy_lock(&succ->lock);
        free(succ);

        /* Release the remaining locks upwards */
        if (succParent != curr) {
            omp_unset_lock(&succParent->lock);
        }
        
        omp_unset_lock(&curr->lock);
        
        if (parent != NULL) {
            omp_unset_lock(&parent->lock);
        }

        return root;
    }
}

/* Returns true iff `data` is currently in the tree. */
bool searchNode(TreeNode *root, int data) {
    /* Handle empty tree edge case safely */
    if (root == NULL) {
        return false;
    }

    TreeNode* curr = root;
    omp_set_lock(&curr->lock); /* Don't forget the & */

    while (curr != NULL) {
        if (data == curr->data) {
            omp_unset_lock(&curr->lock);
            return true;
        } 
        
        /* Determine which way to go */
        TreeNode* next = (data < curr->data) ? curr->left : curr->right;

        /* Hand-over-hand locking */
        if (next != NULL) {
            omp_set_lock(&next->lock);
        }
        
        omp_unset_lock(&curr->lock);
        curr = next;
    }
    
    return false;
}

/* Returns the node holding the smallest value in the tree, or NULL if empty. */
TreeNode *findMin(TreeNode *root) {
    /* Edge case: empty tree */
    if (root == NULL) {
        return NULL;
    }
    
    TreeNode *curr = root;
    
    /* Acquire the lock on the root before traversing */
    omp_set_lock(&curr->lock);
    
    /* Hand-over-hand traversal down the left side of the tree */
    while (curr->left != NULL) {
        TreeNode *next = curr->left;
        
        /* Lock the child, then unlock the parent */
        omp_set_lock(&next->lock);
        omp_unset_lock(&curr->lock);
        
        curr = next;
    }
    
    /* We have reached the leftmost node (the minimum).
     * We must release its lock before returning so we don't block
     * other threads from accessing or deleting it later. */
    omp_unset_lock(&curr->lock);
    
    return curr;
}

/* Traversals: print every value to stdout, separated by single spaces. */
void inorderTraversal(TreeNode *root) {
    if (root != NULL) {
        inorderTraversal(root->left);
        printf("%d ", root->data);
        inorderTraversal(root->right);
    }
}

/* Free every node in the tree (and destroy any locks you added). */
void freeTree(TreeNode *root) {
    if (root == NULL) {
        return;
    }

    /* 1. Free the left subtree */
    freeTree(root->left);

    /* 2. Free the right subtree */
    freeTree(root->right);

    /* 3. Destroy the OpenMP lock for this specific node */
    omp_destroy_lock(&root->lock);

    /* 4. Free the memory allocated for the node */
    free(root);
}