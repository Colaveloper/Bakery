#include <math.h>
#include <stdint.h> // TODO delete in final production, used only in prints
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 255    // TODO understand if strcmp compares all 255 chars
#define HASH_SIZE 8192 // TODO separate for the two hash-tables
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
   int name;
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
   int name;
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

typedef struct Pair {
   int hash;
   int ID;
} Pair;

Pair hash() {
   // using djb2
   Pair pair = {5381, 1111};
   char c;
   while ((c = getchar()) != ' ') {
      pair.hash = ((pair.hash << 5) + pair.hash) + c;
      pair.ID = ((pair.ID << 7) + pair.ID) + c;
   }
   pair.hash = pair.hash % HASH_SIZE;
   pair.ID = pair.ID % HASH_SIZE;
   printf("<%d,%d>", pair.hash, pair.ID);
   return pair;
}

Pair hashAlt() {
   // using djb2
   Pair pair = {5381, 1111};
   char c;
   while ((c = getchar()) != '\n') {
      pair.hash = ((pair.hash << 5) + pair.hash) + c;
      pair.ID = ((pair.ID << 7) + pair.ID) + c;
   }
   pair.hash = pair.hash % HASH_SIZE;
   pair.ID = pair.ID % HASH_SIZE;
   printf("<%d,%d>", pair.hash, pair.ID);
   return pair;
}

void printCookbook(Cookbook *cookbook) {
   printf("\n");
   Recipe *curRec;
   for (int i = 0; i < HASH_SIZE; i++) {
      curRec = cookbook->buckets[i];
      while (curRec != NULL) {
         printf("%d\tmun:%d\tusg:%d ", curRec->name, curRec->minUnbakeable, curRec->usage);
         Ingredient *curIng = curRec->ingredientList;
         while (curIng != NULL) {
            printf("%d:%*d\t", curIng->shelf->name, 5, curIng->amount);
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
         printf("%d:\ttot:%*d\tusg:%d ", curShe->name, 5, curShe->total, curShe->usage);
         printf("\n");
         printBunchHeap(curShe->bunchMinHeap->root, 0);
         printf("\n");
         curShe = curShe->nextShelf;
      }
   }
}

void printSimpleWarehouse(Warehouse *warehouse) {
   Shelf *curShe;
   for (int i = 0; i < HASH_SIZE; i++) {
      curShe = warehouse->buckets[i];
      while (curShe != NULL) {
         if (curShe->total != 0) {
            printf("'%d':\t%*d:\t", curShe->name, 5, curShe->total);
            printf("\n");
         }
         curShe = curShe->nextShelf;
      }
   }
}

void printOrderList(OrderList pendingOrders) {
   printf("\n");
   Order *cur = pendingOrders.head;
   while (cur != NULL) {
      printf("at %*d\t%d:\t%*d\t tot:%*d", 5, cur->time, cur->recipe->name, 5, cur->amount, 5, cur->weight);
      cur = cur->nextOrder;
      // printf("\n");
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

   int depth = (int)log2(heap->size) + 1;
   int *path = (int *)malloc(depth * sizeof(int)); // not leaking
   int level = 0;
   int n = heap->size;

   // Generate path to the last node
   while (n > 1) {
      path[level] = n % 2;
      level++;
      n /= 2;
   }

   Bunch *current = heap->root;
   for (int i = level - 1; i >= 0; i--) {
      if (path[i] == 0)
         current = current->left;
      else
         current = current->right;
   }

   // Free dynamically allocated memory
   free(path);

   return current;
}

BunchMinHeap *insertBunch(BunchMinHeap *heap, int key, int amount) {
   // LOWEST EXPIRATION ON ROOT

   Bunch *newBunch = (Bunch *)malloc(sizeof(Bunch));
   newBunch->expiration = key;
   newBunch->amount = amount;
   newBunch->left = NULL;
   newBunch->right = NULL;
   newBunch->parent = NULL;

   heap->size++;

   if (heap->root == NULL) {
      heap->root = newBunch;
      return heap;
   }

   int depth = (int)log2(heap->size) + 1;
   int *path = (int *)malloc(depth * sizeof(int)); // not leaking
   int level = 0;
   int n = heap->size;

   // Generate path to the new node
   while (n > 1) {
      path[level++] = n % 2;
      n /= 2;
   }

   Bunch *current = heap->root;
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

   newBunch->parent = current;
   if (path[0] == 0)
      current->left = newBunch;
   else
      current->right = newBunch;

   heapifyUp(newBunch);

   // Free dynamically allocated memory
   free(path);

   return heap;
}

BunchMinHeap *deleteMinBunch(BunchMinHeap *heap) {
   // used only when the required amount is more than stored in the root
   // only handles repositioning

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
   }
   return heap;
}

Cookbook *newRecipe(Cookbook *cookbook, Warehouse *warehouse) {

   if (getchar() != ' ') {
      printf("BAAAD");
   }

   Pair i = hash();
   Recipe *pre = NULL, *cur = cookbook->buckets[i.hash];
   Recipe *newRecipe;

   if (cur == NULL) {

      // the bucket was empty
      newRecipe = (Recipe *)malloc(sizeof(Recipe));
      newRecipe->name = i.ID;
      newRecipe->nextRecipe = NULL;
      cookbook->buckets[i.hash] = newRecipe;

   } else {

      // searching for recipes with that name
      while (cur != NULL) {

         if (cur->name == i.ID) {

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
      newRecipe->name = i.ID;
      newRecipe->nextRecipe = NULL;
      pre->nextRecipe = newRecipe;
   }

   // now reading the ingredients
   Ingredient *lastPair = NULL;
   Ingredient *newIng;
   char c;
   int amount;
   Shelf *preShe, *curShe;

   while ((c = getchar()) != '\n') {

      Pair j = hash();
      preShe = NULL;
      curShe = warehouse->buckets[j.hash];

      newIng = (Ingredient *)malloc(sizeof(Ingredient));
      if (scanf("%d", &amount) == 0) {
         printf("BADBADBAD");
      }
      newIng->amount = amount;

      // printf(" <curShe=%s> ", curShe->name);

      // searching for ingredients to connect
      if (curShe == NULL) {

         // the bucket was empty
         // creating a new empty shelf
         curShe = (Shelf *)malloc(sizeof(Shelf));
         curShe->name = j.ID;
         curShe->nextShelf = NULL;
         curShe->total = 0;
         curShe->usage = 1;
         curShe->bunchMinHeap = (BunchMinHeap *)malloc(sizeof(BunchMinHeap));
         warehouse->buckets[j.hash] = curShe;

      } else {

         // searching for ingredients with the same name
         while (curShe != NULL) {

            // printf(" <%s,%s> ", curShe->name, ingredient);

            if (curShe->name == j.ID) {

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
            curShe->name = j.ID;
            curShe->nextShelf = NULL;
            curShe->nextShelf = NULL;
            curShe->total = 0;
            curShe->usage = 1;
            curShe->bunchMinHeap = (BunchMinHeap *)malloc(sizeof(BunchMinHeap));

            if (preShe == NULL) {
               warehouse->buckets[j.hash] = curShe;
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
   newRecipe->minUnbakeable = INFINITE;

   printf("aggiunta\n");
   return cookbook;
}

Cookbook *removeRecipe(Cookbook *cookbook, Warehouse *warehouse) {

   if (getchar() != ' ') {
      printf("BAAAD");
   }

   Pair j = hashAlt();
   Recipe *preRec = NULL, *curRec = cookbook->buckets[j.hash];

   // the bucket can't be empty
   // searching for a recipe with that name
   while (curRec != NULL) {

      if (curRec->name == j.ID) {
         // recipe recovered
         break;
      }
      preRec = curRec;
      curRec = curRec->nextRecipe;
   }

   if (curRec == NULL) {

      // the recipe was already absent
      printf("non presente\n");
      return cookbook;
   }

   if (curRec->usage) {

      // there are orders using this recipe
      printf("ordini in sospeso\n");
      return cookbook;
   }

   // removing all ingredients
   Ingredient *delIng;
   while (curRec->ingredientList != NULL) {
      delIng = curRec->ingredientList;
      delIng->shelf->usage--;

      // removing unused empty shelves // TODO RESTORE
      // if (delIng->shelf->usage == 0 && delIng->shelf->total == 0) {

      //    // search the previous shelf
      //    int i = hash(delIng->shelf->name);
      //    Shelf *curShe = warehouse->buckets[i];
      //    Shelf *preShe = NULL;

      //    while (curShe != delIng->shelf) {
      //       preShe = curShe;
      //       curShe = curShe->nextShelf;
      //    }

      //    if (preShe == NULL) {
      //       // first shelf
      //       warehouse->buckets[i] = curShe->nextShelf;
      //    } else {
      //       // any other
      //       preShe->nextShelf = curShe->nextShelf;
      //    }
      //    free(curShe->bunchMinHeap);
      //    free(curShe);
      // }

      curRec->ingredientList = curRec->ingredientList->nextIngredient;
      free(delIng);
   }

   // removing recipe
   if (preRec == NULL) {

      // removing the first
      cookbook->buckets[j.hash] = curRec->nextRecipe;
   } else {

      // removing any other
      preRec->nextRecipe = curRec->nextRecipe;
   }

   free(curRec);
   printf("rimossa\n");
   return cookbook;
}

OrderList appendOrder(Order *order, OrderList pendingOrders) {

   if (pendingOrders.tail != NULL) {
      pendingOrders.tail->nextOrder = order;
      pendingOrders.tail = pendingOrders.tail->nextOrder;
   } else {
      pendingOrders.head = order;
      pendingOrders.tail = order;
   }
   return pendingOrders;
}

State tryBaking(Order *order, Recipe *recipe, State state, int time) {
   // DOES NOT MODIFY PENDING LIST NOR READY LIST
   // ONLY MODIFIES WAREHOUSE AND STATE.BAKING
   // DOES NOT PRINT ANYTHING

   // printf("trying to bake %s with:", order->name);
   // printWarehouse(state.warehouse);
   // printf("\nPENDING ORDERS");
   // printOrderList(state.pendingOrders);
   // printf("\nREADY ORDERS");
   // printOrderList(state.readyOrders);
   // printf(" %d", order->time);

   Pair i;

   // printf(" mu:%d>am:%d, checking if baking is possible ", recipe->minUnbakeable, order->amount);
   Shelf *preShe, *curShe;
   Ingredient *curIng = recipe->ingredientList;
   while (curIng != NULL) {
      i = hash(curIng->shelf->name);
      curShe = state.warehouse->buckets[i];
      preShe = NULL;
      while (curShe != NULL) {

         // ingredient missing
         if (curShe->total == 0) {
            state.baking = 0;
            recipe->minUnbakeable = 0;
            return state;
         }

         // clensing from expired
         if (curShe->bunchMinHeap->root->expiration <= time) {
            while (curShe->bunchMinHeap->root != NULL) {
               if (curShe->bunchMinHeap->root->expiration <= time) {
                  curShe->total -= curShe->bunchMinHeap->root->amount;
                  curShe->bunchMinHeap = deleteMinBunch(curShe->bunchMinHeap);
                  curShe->bunchMinHeap->size--;
               } else {
                  break;
               }
            }
            if (curShe->bunchMinHeap->size == 0 && curShe->usage == 0) {
               // Removing entire shelf
               if (preShe == NULL) {
                  // First shelf
                  state.warehouse->buckets[i] = curShe->nextShelf;
               } else {
                  preShe->nextShelf = curShe->nextShelf;
               }
               free(curShe->bunchMinHeap);
               free(curShe);
               // if we removed a necessary ingredient, we can't bake
               state.baking = 0;
               recipe->minUnbakeable = 0;
               return state;
            }
         }


         if (!strcmp(curIng->shelf->name, curShe->name)) {

            recipe->minUnbakeable = fmin(recipe->minUnbakeable, curShe->total/curIng->amount+1);
            // printf(" mu: %d ", recipe->minUnbakeable);
            if (order->amount < recipe->minUnbakeable) {
               break; // Enough curIng, check next ingredient
            }
            state.baking = 0;
            return state;
            
         }
         preShe = curShe;
         curShe = curShe->nextShelf;
      }

      // shelf missing
      if (curShe == NULL) {
         // printf("%s not present at all to bake %s\n", curIng->shelf->name, order->recipe->name);
         // printf(" state.baking==%d ", state.baking);
         state.baking = 0;
         recipe->minUnbakeable = 0;
         return state;
      }
      curIng = curIng->nextIngredient;
   }

   // printf(" (baking %s) \n", order->recipe->name);
   state.baking = 1; // ARRIVATI QUI
   recipe->minUnbakeable -= order->amount;

   // printf("\nWAREHOUSE CLEANED");
   // printWarehouse(state.warehouse);

   curIng = recipe->ingredientList;
   int required, weight = 0;
   while (curIng != NULL) {
      i = hash(curIng->shelf->name);
      curShe = state.warehouse->buckets[i];
      preShe = NULL;
      weight += curIng->amount * order->amount;

      // this "while" should terminate only thorugh "break"
      // since it's guaranteed to have all the required ingredients
      while (curShe != NULL) {
         if (!strcmp(curIng->shelf->name, curShe->name)) {
            // Removing used ingredients
            required = curIng->amount * order->amount;
            while (required != 0) {
               if (curShe->bunchMinHeap->root->amount <= required) {
                  required -= curShe->bunchMinHeap->root->amount;
                  curShe->total -= curShe->bunchMinHeap->root->amount;
                  curShe->bunchMinHeap = deleteMinBunch(curShe->bunchMinHeap);
                  curShe->bunchMinHeap->size--;
                  // curShe->bunchList can become NULL
                  // but we don't want empty shelves
                  if (curShe->bunchMinHeap->size == 0 && curShe->usage == 0) {
                     // Removing entire shelf
                     if (preShe == NULL) {
                        // First shelf
                        state.warehouse->buckets[i] = curShe->nextShelf;
                     } else {
                        // Any other shelf
                        preShe->nextShelf = curShe->nextShelf;
                     }
                     free(curShe->bunchMinHeap);
                     free(curShe);
                     break;
                  }
               } else {
                  curShe->bunchMinHeap->root->amount -= required;
                  curShe->total -= required;
                  // required = 0; useless
                  break;
               }
            }
            break;
         }
         preShe = curShe;
         curShe = curShe->nextShelf;
      }
      ////
      curIng = curIng->nextIngredient;
   }

   order->weight = weight;

   return state;
};

State newBatch(State state, Cookbook *cookbook, int time) {

   int expiration, amount;
   char c;

   if (getchar() != ' ') {
      printf("BAAAD");
   }
   
   // refilling warehouse (very efficient)
   while ((c = getchar()) != '\n') {
      Pair i = hash();
      Shelf *pre = NULL;
      Shelf *cur = state.warehouse->buckets[i.hash];

      scanf("%d %d", &amount, &expiration);

      // looking for an existing shelf with that name
      while (cur != NULL) {
         if (cur->name = i.ID) {
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
         newShelf->name = i.ID;
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

         if (state.warehouse->buckets[i.hash] == NULL) {
            state.warehouse->buckets[i.hash] = newShelf;
         } else {
            pre->nextShelf = newShelf;
         }
      }
   }

   printf("rifornito\n");

   // printf("\nWAREHOUSE RECEIVED NEW BATCH; NOW CHECKING FOR POSSIBLE BAKING");
   // printWarehouse(state.warehouse);

   Order *prePen = NULL;
   Order *curPen = state.pendingOrders.head;
   Order *preRea = NULL, *curRea;
   Order *nexPen;
   Recipe *recipe;

   // any recipe isn't unbakeable // TODO reduce this weight
   for (int i = 0; i < HASH_SIZE; i++) {
      recipe = cookbook->buckets[i];
      while (recipe != NULL) {
         recipe->minUnbakeable = INFINITE;
         recipe = recipe->nextRecipe;
      }
   }

   // try baking all pending
   while (curPen != NULL) {
      recipe = curPen->recipe;
      nexPen = curPen->nextOrder;
      // printf(" o:%d ", curPen->time);
      if (recipe->minUnbakeable <= curPen->amount) {
         // printf(" mu:%d<=am:%d, not even trying to bake %s\n", recipe->minUnbakeable, curPen->amount, recipe->name);
         state.baking = 0;
      } else {
         // printf(" mu:%d>am:%d, trying to bake %s\n", recipe->minUnbakeable, curPen->amount, recipe->name);
         // recipe == NULL is impossible
         state = tryBaking(curPen, recipe, state, time);
      }
      if (state.baking) {
         // printf(" transferring %d ", curPen->time);
         // removing curPen from pending
         if (prePen == NULL) {
            state.pendingOrders.head = nexPen;
            if (state.pendingOrders.head == NULL) {
               state.pendingOrders.tail = NULL;
            }
         } else {
            prePen->nextOrder = nexPen;
            if (state.pendingOrders.tail == curPen) {
               state.pendingOrders.tail = prePen;
            }
         }
         // adding curPen in ready
         curRea = state.readyOrders.head;
         if (curRea == NULL) {
            curPen->nextOrder = NULL;
            state.readyOrders.head = curPen;
            state.readyOrders.tail = curPen;
         } else {
            while (curRea != NULL) {
               if (curRea->time > curPen->time) {
                  break;
               }
               preRea = curRea;
               curRea = curRea->nextOrder;
            }

            curPen->nextOrder = curRea;

            if (preRea == NULL) {
               state.readyOrders.head = curPen;
            } else {
               preRea->nextOrder = curPen;
            }

            if (curRea == NULL) {
               state.readyOrders.tail = curPen;
            }
         }

         // printf("\nPENDING ORDERS");
         // printOrderList(state.pendingOrders);
         // printf("\nREADY ORDERS");
         // printOrderList(state.readyOrders);
         prePen = prePen;
      } else {
         if (recipe->minUnbakeable > curPen->amount) {
            // smallest unbakeable order yet
            recipe->minUnbakeable = curPen->amount;
            // printf(" mu reduced to %d\n", recipe->minUnbakeable);
         }
         prePen = curPen;
      }
      curPen = nexPen;
      if (nexPen == NULL) {
         break;
      } else {
         nexPen = nexPen->nextOrder;
      }
   }
   return state;
}



State loadOrders(State state, int payloadLeft) {
   // printf("\n");
   // printf("\nWAREHOUSE");
   // printWarehouse(state.warehouse);
   // printf("\nPENDING ORDERS");
   // printOrderList(state.pendingOrders);
   // printf("\nREADY ORDERS");
   // printOrderList(state.readyOrders);
   if (state.readyOrders.head == NULL) {
      printf("camioncino vuoto\n");
   } else {
      Order *loadingOrders = NULL, *curLoa, *preLoa, *delOrd;
      Order *curRea = state.readyOrders.head;
      while (curRea != NULL) {
         // loading the van with ready orders by weight, by date
         payloadLeft -= state.readyOrders.head->weight;
         if (payloadLeft < 0) {
            break;
         }
         state.readyOrders.head = state.readyOrders.head->nextOrder;
         curLoa = loadingOrders;
         preLoa = NULL;
         while (curLoa != NULL) {
            if (curRea->weight > curLoa->weight) {
               break;
            } else if (curRea->weight == curLoa->weight) {
               if (curRea->time < curLoa->time) {
                  break;
               }
            }
            preLoa = curLoa;
            curLoa = curLoa->nextOrder;
         }
         if (preLoa == NULL) {
            curRea->nextOrder = curLoa;
            loadingOrders = curRea;
         } else {
            preLoa->nextOrder = curRea;
            curRea->nextOrder = curLoa;
         }
         curRea = state.readyOrders.head;
      }
      if (curRea == NULL) {
         state.readyOrders.tail = NULL;
      }
      while (loadingOrders != NULL) {
         printf("%d %s %d\n", loadingOrders->time, loadingOrders->recipe->name, loadingOrders->amount);
         delOrd = loadingOrders;
         loadingOrders = loadingOrders->nextOrder;
         delOrd->recipe->usage--;
         free(delOrd);
      }
   }
   // printf("\n");
   // printf("\nWAREHOUSE");
   // printWarehouse(state.warehouse);
   // printf("\nPENDING ORDERS");
   // printOrderList(state.pendingOrders);
   // printf("\nREADY ORDERS");
   // printOrderList(state.readyOrders);
   return state;
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
         // state = newBatch(state, cookbook, time);

      } else if (!strcmp(command, "ordine")) {
         // printf("[newOrder] ");
         // state = newOrder(cookbook, state, time);

      } else {
         printf("ERROR: UNKNOWN COMMAND %s", command);
      }

      // printf(" <%s> ", command);
      // // // printf("'%s'", state.warehouse->buckets[3]->name);
      // // printf("\nCOOKBOOK");
      // // printCookbook(cookbook);
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

      // printSimpleWarehouse(state.warehouse);
      time++;

      // printf("It's [%d]\n", time);
   }
   if (time && time % courierPeriod == 0) {
      // state = loadOrders(state, maxPayload);
   }

   // printf("\n");
   printf("\nCOOKBOOK");
   printCookbook(cookbook);
   // printf("\nWAREHOUSE");
   // printWarehouse(state.warehouse);
   // printf("\nPENDING ORDERS");
   // printOrderList(state.pendingOrders);
   // printf("\nREADY ORDERS");
   // printOrderList(state.readyOrders);
}