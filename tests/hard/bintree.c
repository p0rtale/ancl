#include "include/std.h"

struct Node {
	int value;
	struct Node *left;
	struct Node *right;
};

struct Node *create_node(int value) {
	struct Node* node = malloc(sizeof(struct Node));
	node->value = value;
	node->left = NULL;
	node->right = NULL;

	return node;
}

void free_node(struct Node *node) {
    free(node);
}

struct Node *insert_left(struct Node *root, int value) {
	root->left = create_node(value);
	return root->left;
}

struct Node *insert_right(struct Node *root, int value) {
	root->right = create_node(value);
	return root->right;
}

void inorder_traversal(struct Node *root) {
	if (root == NULL) {
		return;
	}

	inorder_traversal(root->left);
	printf("%d ->", root->value);
	inorder_traversal(root->right);
}

void preorder_traversal(struct Node *root) {
	if (root == NULL) {
		return;
	}

	printf("%d ->", root->value);
	preorder_traversal(root->left);
	preorder_traversal(root->right);
}

void postorder_traversal(struct Node *root) {
	if (root == NULL) {
		return;
	}
	postorder_traversal(root->left);
	postorder_traversal(root->right);
	printf("%d ->", root->value);
}

int main() {
	struct Node *root = create_node(1);
	insert_left(root, 2);
	insert_right(root, 3);
	insert_left(root->left, 4);


	printf("Inorder traversal \n");
	inorder_traversal(root);

	printf("\nPreorder traversal \n");
	preorder_traversal(root);

	printf("\nPostorder traversal \n");
	postorder_traversal(root);

    return EXIT_SUCCESS;
}
