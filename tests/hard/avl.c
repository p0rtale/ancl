#include "include/std.h"

struct Node {
    int key;
    struct Node *left;
    struct Node *right;
    int height;
};

struct Node* create_node(int key) {
    struct Node* node = malloc(sizeof(struct Node));

    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;

    return node;
}

void free_node(struct Node* node) {
    free(node);
}

int maximum(int a, int b) {
    return (a > b) ? a : b;
}

int height(struct Node *N) {
    if (N == NULL) {
        return 0;
    }
    return N->height;
}

int get_balance(struct Node *N) {
    if (N == NULL) {
        return 0;
    }
    return height(N->left) - height(N->right);
}

struct Node *left_rotate(struct Node *x) {
    struct Node *y = x->right;
    struct Node *temp = y->left;

    y->left = x;
    x->right = temp;

    x->height = maximum(height(x->left), height(x->right)) + 1;
    y->height = maximum(height(y->left), height(y->right)) + 1;

    return y;
}

struct Node *right_rotate(struct Node *y) {
    struct Node *x = y->left;
    struct Node *temp = x->right;

    x->right = y;
    y->left = temp;

    y->height = maximum(height(y->left), height(y->right)) + 1;
    x->height = maximum(height(x->left), height(x->right)) + 1;

    return x;
}

struct Node* insert(struct Node* node, int key) {
    if (node == NULL) {
        return create_node(key);
    }

    if (key < node->key) {
        node->left = insert(node->left, key);
    } else if (key > node->key) {
        node->right = insert(node->right, key);
    } else {
        return node;
    }

    node->height = 1 + maximum(height(node->left), height(node->right));

    int balance = get_balance(node);

    if (balance > 1 && key < node->left->key) {
        return right_rotate(node);
    }

    if (balance < -1 && key > node->right->key) {
        return left_rotate(node);
    }

    if (balance > 1 && key > node->left->key) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    if (balance < -1 && key < node->right->key) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

void preorder_traversal(struct Node *root) {
	if (root != NULL) {
		printf("%d ", root->key);
		preorder_traversal(root->left);
		preorder_traversal(root->right);
	}
}

int main() {
    struct Node *root = NULL;

    root = insert(root, 9);
    root = insert(root, 5);
	root = insert(root, 10);
	root = insert(root, 0);
	root = insert(root, 6);
	root = insert(root, 11);
	root = insert(root, -1);
	root = insert(root, 1);
	root = insert(root, 2);

	/*
        9
        /  \
        1    10
        /  \     \
        0    5     11
        /    /  \
        -1   2    6
	*/

    preorder_traversal(root);
    printf("\n");

    return EXIT_SUCCESS;
}
