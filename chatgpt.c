#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Node structure for the tree-based min-heap
typedef struct MinHeapNode {
   int key;                    // The key of the node
   struct MinHeapNode *left;   // Pointer to the left child
   struct MinHeapNode *right;  // Pointer to the right child
   struct MinHeapNode *parent; // Pointer to the parent node
} MinHeapNode;

// MinHeap structure that maintains the root of the tree and its size
typedef struct MinHeap {
   MinHeapNode *root; // Root node of the heap
   int size;          // Number of elements in the heap
} MinHeap;

// Function to create a new min-heap node
MinHeapNode *newMinHeapNode(int key) {
   MinHeapNode *node = (MinHeapNode *)malloc(sizeof(MinHeapNode));
   node->key = key;
   node->left = node->right = node->parent = NULL;
   return node;
}

// Function to create a new empty min-heap
MinHeap *createMinHeap() {
   MinHeap *heap = (MinHeap *)malloc(sizeof(MinHeap));
   heap->root = NULL;
   heap->size = 0;
   return heap;
}

// Function to swap the keys of two nodes
void swapKeys(MinHeapNode *a, MinHeapNode *b) {
   int temp = a->key;
   a->key = b->key;
   b->key = temp;
}

// Function to heapify down after extraction
void heapifyDown(MinHeapNode *node) {
   MinHeapNode *smallest = node;

   if (node->left && node->left->key < smallest->key)
      smallest = node->left;

   if (node->right && node->right->key < smallest->key)
      smallest = node->right;

   if (smallest != node) {
      swapKeys(node, smallest);
      heapifyDown(smallest);
   }
}

// Function to heapify up after insertion


// Function to insert a new key into the min-heap
void insertMinHeap(MinHeap* heap, int key) {
    MinHeapNode* newNode = newMinHeapNode(key);
    heap->size++;

    if (heap->root == NULL) {
        heap->root = newNode;
        return;
    }

    // Dynamic path allocation
    int depth = (int)log2(heap->size) + 1;
    int* path = (int*)malloc(depth * sizeof(int));
    int level = 0;
    int n = heap->size;

    // Generate path to the new node
    while (n > 1) {
        path[level++] = n % 2;
        n /= 2;
    }

    MinHeapNode* current = heap->root;
    for (int i = level - 1; i >= 0; i--) {
        if (path[i] == 0) {
            if (current->left == NULL)
                break;
            current = current->left;
        } else {
            if (current->right == NULL)
                break;
            current = current->right;
        }
    }

    newNode->parent = current;
    if (path[0] == 0)
        current->left = newNode;
    else
        current->right = newNode;

    heapifyUp(newNode);

    // Free dynamically allocated memory
    free(path);
}

// Helper function to get the last node in the heap
MinHeapNode *getLastNode(MinHeap *heap) {
   if (heap->size == 0)
      return NULL;

   int path[32], level = 0;
   int n = heap->size;

   // Generate path to the last node
   while (n > 1) {
      path[level++] = n % 2;
      n /= 2;
   }

   MinHeapNode *current = heap->root;
   for (int i = level - 1; i >= 0; i--) {
      if (path[i] == 0)
         current = current->left;
      else
         current = current->right;
   }

   return current;
}

// Function to extract the minimum element (root) from the heap
int extractMin(MinHeap *heap) {
   if (heap->size == 0) {
      printf("Heap is empty\n");
      return -1;
   }

   int min = heap->root->key;

   if (heap->size == 1) {
      free(heap->root);
      heap->root = NULL;
   } else {
      MinHeapNode *lastNode = getLastNode(heap);

      // Move last node's key to root
      heap->root->key = lastNode->key;

      // Detach the last node
      if (lastNode->parent) {
         if (lastNode->parent->right == lastNode)
            lastNode->parent->right = NULL;
         else
            lastNode->parent->left = NULL;
      }

      free(lastNode);
      heapifyDown(heap->root);
   }

   heap->size--;
   return min;
}

// Function to find the node with a given key (used for deletion)
MinHeapNode *findNode(MinHeapNode *root, int key) {
   if (root == NULL || root->key == key)
      return root;

   MinHeapNode *left = findNode(root->left, key);
   if (left)
      return left;

   return findNode(root->right, key);
}

// Function to delete a key from the min-heap
void deleteKey(MinHeap *heap, int key) {
   MinHeapNode *nodeToDelete = findNode(heap->root, key);
   if (nodeToDelete == NULL) {
      printf("Key not found\n");
      return;
   }

   if (heap->size == 1) {
      free(heap->root);
      heap->root = NULL;
      heap->size--;
      return;
   }

   MinHeapNode *lastNode = getLastNode(heap);

   // Replace nodeToDelete's key with lastNode's key
   nodeToDelete->key = lastNode->key;

   // Detach the last node
   if (lastNode->parent) {
      if (lastNode->parent->right == lastNode)
         lastNode->parent->right = NULL;
      else
         lastNode->parent->left = NULL;
   }

   free(lastNode);
   heapifyDown(nodeToDelete);
   heap->size--;
}

// Function to get the minimum element from the heap
int getMin(MinHeap *heap) {
   if (heap->size == 0) {
      printf("Heap is empty\n");
      return -1;
   }
   return heap->root->key;
}

// Function to free all nodes in the heap
void freeHeap(MinHeapNode *node) {
   if (node == NULL)
      return;
   freeHeap(node->left);
   freeHeap(node->right);
   free(node);
}

// Function to print the min-heap structure
void printHeap(MinHeapNode *node, int depth) {
   if (node == NULL)
      return;

   // Print right child
   printHeap(node->right, depth + 1);

   // Print current node
   for (int i = 0; i < depth; i++)
      printf("    ");
   printf("%d\n", node->key);

   // Print left child
   printHeap(node->left, depth + 1);
}

// Main function to test the tree-based min-heap implementation
int main() {
   MinHeap *heap = createMinHeap();

   // Insert 10 values into the heap
   insertMinHeap(heap, 10);
   insertMinHeap(heap, 4);
   insertMinHeap(heap, 15);
   insertMinHeap(heap, 20);
   insertMinHeap(heap, 8);
   insertMinHeap(heap, 12);
   insertMinHeap(heap, 6);
   insertMinHeap(heap, 3);
   insertMinHeap(heap, 9);
   insertMinHeap(heap, 2);

   printf("Min-Heap after insertion:\n");
   printHeap(heap->root, 0);

   // Extract and print each value from the heap
   printf("\nExtracting values from Min-Heap:\n");
   while (heap->size > 0) {
      int minValue = extractMin(heap);
      printf("%d ", minValue);
   }
   printf("\n");

   // Free memory
   freeHeap(heap->root);
   free(heap);

   return 0;
}
