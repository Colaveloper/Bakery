#include <math.h>
// #include <stddef.h>
#include <stdint.h> // TODO delete in final production, used only in prints
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 255 // TODO understand if strcmp compares all 255 chars
#define HASH_SIZE 3 // TODO be careful this is adequate, try 4096
#define INFINITE 2147483647

// expiration, amount ∈ Bunch ∈ Shelf ∈ warehouse
typedef struct Bunch {
   int expiration;
   int amount;
   struct Bunch *left;
   struct Bunch *right;
   struct Bunch *parent;
} Bunch;
typedef struct BunchMinHeap {
   Bunch *root;
   int size;
} BunchMinHeap;
typedef struct Shelf {
   char name[MAX_LEN];
   int total;
   BunchMinHeap *bunchMinHeap;
   struct Shelf *nextShelf;
   int usage;
} Shelf;
typedef struct Warehouse {
   Shelf *buckets[HASH_SIZE]; // Array of lists of shelves
} Warehouse;

// ingredient, amount ∈ Ingredient ∈ *Recipe[] ∈ cookbook
typedef struct Ingredient {
   int amount;
   Shelf *shelf;
   struct Ingredient *nextIngredient;
} Ingredient;
typedef struct Recipe {
   char name[MAX_LEN];
   int minUnbakeable;
   int usage;
   Ingredient *ingredientList;
   struct Recipe *nextRecipe;
} Recipe;
typedef struct Cookbook {
   Recipe *buckets[HASH_SIZE]; // Array of lists of recipes
} Cookbook;

// name, amount ∈ Order ∈ pendingOrders, readyOrders
typedef struct Order {
   int time;
   int amount;
   int weight;
   Recipe *recipe;
   struct Order *nextOrder;
} Order;
typedef struct OrderList {
   Order *head;
   Order *tail;
} OrderList;

// pendingOrders, warehouse ∈ state
typedef struct State {

   // basically supply and demand

   OrderList pendingOrders;
   OrderList readyOrders;
   Warehouse *warehouse;
   int baking;

   // baking semantics:
   // // -1 rifiutato
   // // 0  not satysfiable (will be in pendingOrders)
   // // 1  satysfying (will be in readyOrders)

} State;

int hash(char *str) {
   // using djb2
   unsigned long hash = 5381;
   int c;
   while ((c = *str++)) {
      hash = ((hash << 5) + hash) + c;
   }
   return hash % HASH_SIZE;
}

void printCookbook(Cookbook *cookbook) {
   printf("\n");
   Recipe *curRec;
   for (int i = 0; i < HASH_SIZE; i++) {
      curRec = cookbook->buckets[i];
      while (curRec != NULL) {
         printf("%10s\tmun:%d\tusg:%d ", curRec->name, curRec->minUnbakeable, curRec->usage);
         Ingredient *curIng = curRec->ingredientList;
         while (curIng != NULL) {
            printf("%10s:%*d\t", curIng->shelf->name, 5, curIng->amount);
            curIng = curIng->nextIngredient;
         }
         curRec = curRec->nextRecipe;
         printf("\n");
      }
   }
}

void printBunchHeap(Bunch *node, int depth) {
   if (node == NULL)
      return;

   // Print right child
   printBunchHeap(node->right, depth + 1);

   // Print current node
   for (int i = 0; i < depth; i++)
      printf("    ");
   printf("%d:%d\n", node->expiration, node->amount);

   // Print left child
   printBunchHeap(node->left, depth + 1);
}

void printWarehouse(Warehouse *warehouse) {

   printf("\n");
   Shelf *curShe;
   for (int i = 0; i < HASH_SIZE; i++) {
      curShe = warehouse->buckets[i];
      while (curShe != NULL) {
         printf("%s:\ttot:%*d\tusg:%d ", curShe->name, 5, curShe->total, curShe->usage);
         printf("\n");
         printBunchHeap(curShe->bunchMinHeap->root, 0);
         printf("\n");
         curShe = curShe->nextShelf;
      }
   }
}

void printOrderList(OrderList pendingOrders) {
   printf("\n");
   Order *cur = pendingOrders.head;
   while (cur != NULL) {
      printf("at %*d\t%10s:\t%*d\t tot:%*d", 5, cur->time, cur->recipe->name, 5, cur->amount, 5, cur->weight);
      cur = cur->nextOrder;
      printf("\n");
   }
}

void swapKeys(Bunch *a, Bunch *b) {
   int temp = a->expiration;
   a->expiration = b->expiration;
   b->expiration = temp;
   temp = a->amount;
   a->amount = b->amount;
   b->amount = temp;
}

void heapifyUp(Bunch *node) {
   while (node->parent && node->expiration < node->parent->expiration) {
      swapKeys(node, node->parent);
      node = node->parent;
   }
}

void heapifyDown(Bunch *node) {
   Bunch *smallest = node;

   if (node->left && node->left->expiration < smallest->expiration)
      smallest = node->left;

   if (node->right && node->right->expiration < smallest->expiration)
      smallest = node->right;

   if (smallest != node) {
      swapKeys(node, smallest);
      heapifyDown(smallest);
   }
}

Bunch *getLastNode(BunchMinHeap *heap) {
   if (heap->size == 0)
      return NULL;

   int path[32], level = 0;
   int n = heap->size;

   // Generate path to the last node
   while (n > 1) {
      path[level++] = n % 2;
      n /= 2;
   }

   Bunch *current = heap->root;
   for (int i = level - 1; i >= 0; i--) {
      if (path[i] == 0)
         current = current->left;
      else
         current = current->right;
   }

   return current;
}

BunchMinHeap *insertBunch(BunchMinHeap *heap, int key, int amount) {
   // LOWEST EXPIRATION ON ROOT

   Bunch *newBunch = (Bunch *)malloc(sizeof(Bunch));
   Bunch *cur = heap->root;
   newBunch->expiration = key;
   newBunch->amount = amount;
   newBunch->left = NULL;
   newBunch->right = NULL;
   newBunch->parent = cur;

   // empty heap
   if (heap->root == NULL) {
      newBunch->parent = NULL;
      heap->root = newBunch;
      heap->size++;
      return heap;
   }

   int depth = (int)log2(heap->size) + 1;
   int *path = (int *)malloc(depth * sizeof(int));
   int level = 0;
   int n = heap->size;
   while (n > 1) {
      path[level++] = n % 2;
      n /= 2;
   }

   for (int i = level - 1; i >= 0; i--) {
      if (path[i] == 0) {
         if (cur->left == NULL)
            break;
         cur = cur->left;
      } else {
         if (cur->right == NULL)
            break;
         cur = cur->right;
      }
   }

   if (path[0] == 0)
      cur->left = newBunch;
   else
      cur->right = newBunch;

   heap->size++; // TODO check if meaningful

   heapifyUp(newBunch);

   // Free dynamically allocated memory
   free(path);

   // if found
   // cur->amount += amount;
   // else create
   return heap;
}

BunchMinHeap *extractBunch(BunchMinHeap *heap) {
   // used only when the required amount is more than stored in the root
   // only handles repositioning
   // Bunch* min = heap->root; // TODO use

   if (heap->size == 1) {
      free(heap->root);
      heap->root = NULL;
   } else {
      Bunch *lastNode = getLastNode(heap);

      // Move last node's key to root
      heap->root->amount = lastNode->amount;
      heap->root->expiration = lastNode->expiration;

      // Detach the last node
      if (lastNode->parent) {
         if (lastNode->parent->right == lastNode)
            lastNode->parent->right = NULL;
         else
            lastNode->parent->left = NULL;
      }

      free(lastNode);
      heapifyDown(heap->root);

      // TODO handle empty tree
   }
   return heap;
}

State newBatch(State state, Cookbook *cookbook, int time) {

   int expiration, amount;
   char name[MAX_LEN];

   while (scanf("%s %d %d", name, &amount, &expiration)) {
      int i = hash(name);
      // printf(" <%s|%d>", name, i);
      Shelf *pre = NULL;
      Shelf *cur = state.warehouse->buckets[i];

      // looking for an existing shelf with that name
      while (cur != NULL) {
         if (!strcmp(cur->name, name)) {
            // TODO !!! if bunchMinHeap is null create one
            if (cur->bunchMinHeap == NULL) {
               cur->bunchMinHeap = (BunchMinHeap *)malloc(sizeof(BunchMinHeap));
            }
            cur->bunchMinHeap = insertBunch(cur->bunchMinHeap, expiration, amount);
            cur->total += amount;
            break;
         } else {
            pre = cur;
            cur = cur->nextShelf;
         }
      }
      if (cur == NULL) {

         // creating new shelf with one bunch
         Shelf *newShelf = (Shelf *)malloc(sizeof(Shelf));
         strcpy(newShelf->name, name);
         newShelf->total = amount;
         newShelf->nextShelf = NULL;

         // creating heap in shelf
         BunchMinHeap *heap = (BunchMinHeap *)malloc(sizeof(BunchMinHeap));
         heap->size = 1;

         newShelf->bunchMinHeap = heap;

         // creating first bunch in heap root
         Bunch *firstBunch = (Bunch *)malloc(sizeof(Bunch));
         firstBunch->expiration = expiration;
         firstBunch->amount = amount;

         heap->root = firstBunch;

         if (state.warehouse->buckets[i] == NULL) {
            state.warehouse->buckets[i] = newShelf;
         } else {
            pre->nextShelf = newShelf;
         }
      }
      char c = getchar();
      if (c == '\n' || c == '\r' || c == EOF) {
         break;
      }
   }

   // Warehouse *warehouse = state.warehouse;
   // printf("\nWAREHOUSE RECEIVED NEW BATCH; NOW CHECKING FOR POSSIBLE BAKING");
   // printWarehouse(warehouse);

   int i;
   // Order *prePen = NULL;
   Order *curPen = state.pendingOrders.head;
   // Order *preRea = NULL, *curRea;
   // Order *nexPen; // TODO restore
   Recipe *recipe;

   // any recipe isn't unbakeable
   for (int i = 0; i < HASH_SIZE; i++) {
      recipe = cookbook->buckets[i];
      while (recipe != NULL) {
         recipe->minUnbakeable = INFINITE;
         recipe = recipe->nextRecipe;
      }
   }

   while (curPen != NULL) {

      i = hash(curPen->recipe->name);
      recipe = cookbook->buckets[i];
      while (recipe != NULL) {
         if (!strcmp(recipe->name, curPen->recipe->name)) {
            break;
         }
         recipe = recipe->nextRecipe;
      }
      // recipe == NULL is impossible
      // TODO RESTORE
      //    state = tryBaking(curPen, recipe, state, time);
      //    int baking = state.baking;
      //    nexPen = curPen->nextOrder;
      //    if (baking) {
      //       printf(" transferring %d ", curPen->time);
      //       removing curPen from pending
      //       if (prePen == NULL) {
      //          state.pendingOrders.head = curPen->nextOrder;
      //          if (state.pendingOrders.head == NULL) {
      //             state.pendingOrders.tail = NULL;
      //          }
      //       } else {
      //          prePen->nextOrder = curPen->nextOrder;
      //          if (state.pendingOrders.tail == curPen) {
      //             state.pendingOrders.tail = prePen;
      //          }
      //       }
      //       adding curPen in ready
      //       curRea = state.readyOrders.head;
      //       if (curRea == NULL) {
      //          curPen->nextOrder = NULL;
      //          state.readyOrders.head = curPen;
      //          state.readyOrders.tail = curPen;
      //       } else {
      //          //
      //          while (curRea != NULL) {
      //             if (curRea->time > curPen->time) {
      //                break;
      //             }
      //             preRea = curRea;
      //             curRea = curRea->nextOrder;
      //          }

      //          curPen->nextOrder = curRea;

      //          if (preRea == NULL) {
      //             state.readyOrders.head = curPen;
      //          } else {
      //             preRea->nextOrder = curPen;
      //          }

      //          if (curRea == NULL) {
      //             state.readyOrders.tail = curPen;
      //          }
      //          //
      //       }
      //       prePen unchanged

      //       printf("\nPENDING ORDERS");
      //       printOrderList(state.pendingOrders);
      //       printf("\nREADY ORDERS");
      //       printOrderList(state.readyOrders);
      //    } else {
      //       prePen = curPen;
      //    }
      //    curPen = nexPen;
   }
   printf("rifornito\n");
   return state;
}

Cookbook *newRecipe(Cookbook *cookbook, Warehouse *warehouse) {

   char name[MAX_LEN];
   if (scanf("%s", name) == 0) {
      printf("MUST HAVE A RECIPE TO ADD");
      return cookbook;
   }

   int i = hash(name);
   Recipe *pre = NULL, *cur = cookbook->buckets[i];
   Recipe *newRecipe;

   if (cur == NULL) {

      // the bucket was empty
      newRecipe = (Recipe *)malloc(sizeof(Recipe));
      strcpy(newRecipe->name, name);
      newRecipe->nextRecipe = NULL;
      cookbook->buckets[i] = newRecipe;

   } else {

      // searching for recipes with that name
      while (cur != NULL) {

         if (!strcmp(cur->name, name)) {

            printf("ignorato\n");

            // ignoring until EOL
            char c = 'c';
            while (c != '\n' && c != '\r' && c != EOF) {
               c = getchar();
               continue;
            }

            return cookbook;
         }
         pre = cur;
         cur = cur->nextRecipe;
      }

      // creating at the end
      newRecipe = (Recipe *)malloc(sizeof(Recipe));
      strcpy(newRecipe->name, name);
      newRecipe->nextRecipe = NULL;
      pre->nextRecipe = newRecipe;
   }

   // now reading the ingredients
   Ingredient *lastPair = NULL;
   Ingredient *newIng;
   char ingredient[MAX_LEN];
   int amount;
   Shelf *preShe, *curShe;

   while (scanf("%s %d", ingredient, &amount)) {

      newIng = (Ingredient *)malloc(sizeof(Ingredient));
      newIng->amount = amount;

      int j = hash(ingredient);
      preShe = NULL;
      curShe = warehouse->buckets[j];

      // printf(" <curShe=%s> ", curShe->name);

      // searching for ingredients to connect
      if (curShe == NULL) {

         // the bucket was empty
         // creating a new empty shelf
         curShe = (Shelf *)malloc(sizeof(Shelf));
         strcpy(curShe->name, ingredient);
         curShe->nextShelf = NULL;
         curShe->total = 0;
         curShe->usage = 1;
         curShe->bunchMinHeap = (BunchMinHeap *)malloc(sizeof(BunchMinHeap));
         warehouse->buckets[j] = curShe;

      } else {

         // searching for ingredients with the same name
         while (curShe != NULL) {

            // printf(" <%s,%s> ", curShe->name, ingredient);

            if (!strcmp(curShe->name, ingredient)) {

               // the ingredient already exists in warehouse
               curShe->usage++;
               break;
            } else {
               preShe = curShe;
               curShe = curShe->nextShelf;
            }
         }

         if (curShe == NULL) {
            // creating at the end
            curShe = (Shelf *)malloc(sizeof(Shelf));
            strcpy(curShe->name, ingredient);
            curShe->nextShelf = NULL;
            curShe->nextShelf = NULL;
            curShe->total = 0;
            curShe->usage = 1;
            curShe->bunchMinHeap = (BunchMinHeap *)malloc(sizeof(BunchMinHeap));

            if (preShe == NULL) {
               warehouse->buckets[j] = curShe;
            } else {
               preShe->nextShelf = curShe;
            }
         }
      }

      // connecting the ingredient
      newIng->shelf = curShe;

      // pushing on top of ingredient list
      newIng->nextIngredient = lastPair;
      lastPair = newIng;

      // check if cur is the last ingredient
      char c = getchar();
      if (c == '\n' || c == '\r' || c == EOF) {
         break;
      }
   }

   // attaching ingredient list to recipe
   newRecipe->ingredientList = lastPair;

   // we now assume the recipe not to be unbakeable
   // TODO make this variable actually tell if recipe is bakeable
   newRecipe->minUnbakeable = INFINITE;

   printf("aggiunta\n");
   return cookbook;
}

Cookbook *removeRecipe(Cookbook *cookbook, Warehouse *warehouse) {
   char name[MAX_LEN];
   if (scanf("%s", name) == 0) {
      printf("MUST NAME A RECIPE TO REMOVE");
   }

   int i = hash(name);
   Recipe *pre = NULL, *cur = cookbook->buckets[i];

   // the bucket can't be empty
   // searching for a recipe with that name
   while (cur != NULL) {

      if (!strcmp(cur->name, name)) {
         // recipe recovered
         break;
      }
      pre = cur;
      cur = cur->nextRecipe;
   }

   if (cur == NULL) {

      // the recipe was already absent
      printf("non presente\n");
      return cookbook;
   }

   if (cur->usage) {

      // there are orders using this recipe
      printf("ordini in sospeso\n");
      return cookbook;
   }

   // removing all ingredients
   Ingredient *delIng;
   while (cur->ingredientList != NULL) {
      delIng = cur->ingredientList;
      delIng->shelf->usage--;

      // TODO removing unused empty shelves
      // if (delIng->shelf->usage == 0 && delIng->shelf->total == 0) {
      // }

      cur->ingredientList = cur->ingredientList->nextIngredient;
   }

   // removing recipe
   if (pre == NULL) {

      // removing the first
      cookbook->buckets[i] = cur->nextRecipe;
   } else {

      // removing any other
      pre->nextRecipe = cur->nextRecipe;
   }

   free(cur);
   printf("rimossa\n");
   return cookbook;
}

int main() {

   int courierPeriod, maxPayload, time = 0;
   char command[MAX_LEN];
   Cookbook *cookbook = (Cookbook *)malloc(sizeof(Cookbook));
   Warehouse *warehouse = (Warehouse *)malloc(sizeof(Warehouse));
   for (int i = 0; i < HASH_SIZE; i++) {
      cookbook->buckets[i] = NULL;
      warehouse->buckets[i] = NULL;
   }
   State state = {{NULL, NULL}, {NULL, NULL}, warehouse, 0};

   if (scanf("%d", &courierPeriod) == 0 || scanf("%d", &maxPayload) == 0) {
      perror("MUST SPECIFY COURIER PERIOD AND MAX PAYLOAD");
      return -1;
   }

   while (scanf("%s", command) > 0) {

      if (time && time % courierPeriod == 0) {
         // state = loadOrders(state, maxPayload);
      }

      if (!strcmp(command, "aggiungi_ricetta")) {
         // printf("[newRecipe at %d] ", time);
         cookbook = newRecipe(cookbook, state.warehouse);

      } else if (!strcmp(command, "rimuovi_ricetta")) {
         // printf("[removeRecipe] ");
         cookbook = removeRecipe(cookbook, state.warehouse);

      } else if (!strcmp(command, "rifornimento")) {
         // printf("[newBatch] ");
         state = newBatch(state, cookbook, time);

      } else if (!strcmp(command, "ordine")) {
         printf("[newOrder] ");
         // state = newOrder(cookbook, state, time);

      } else {
         printf("ERROR: UNKNOWN COMMAND %s", command);
      }

      // // printf(" <%s> ", command);
      // // printf("'%s'", state.warehouse->buckets[3]->name);
      // printf("\nCOOKBOOK");
      // printCookbook(cookbook);
      // printf("\nWAREHOUSE AFTER");
      // printWarehouse(state.warehouse);
      // printf("\nPENDING ORDERS");
      // printOrderList(state.pendingOrders);
      // // if (state.pendingOrders.tail)
      // //    printf(" pending tail: %d ", state.pendingOrders.tail->time);
      // printf("\nREADY ORDERS");
      // printOrderList(state.readyOrders);
      // // if (state.readyOrders.tail)
      // // printf(" ready tail: %d ", state.readyOrders.tail->time);
      // printf("\n\n");
      // time++;
      // printf("It's [%d]\n", time);
   }
   if (time && time % courierPeriod == 0) {
      // state = loadOrders(state, maxPayload);
   }
   // printf("\n");
   printf("\nCOOKBOOK");
   printCookbook(cookbook);
   printf("\nWAREHOUSE");
   printWarehouse(state.warehouse);
   // printf("\nPENDING ORDERS");
   // printOrderList(state.pendingOrders);
   // printf("\nREADY ORDERS");
   // printOrderList(state.readyOrders);
}