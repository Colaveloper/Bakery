// #include <math.h>
// #include <stddef.h>
#include <stdint.h> // TODO delete in final production, used only in prints
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 255
#define HASH_SIZE 3 // TODO be careful this is adequate, try 4096
#define INFINITE 2147483647

// expiration, amount ∈ Bunch ∈ Shelf ∈ warehouse
typedef struct Bunch {
   int expiration;
   int amount;
   struct Bunch *nextBunch;
} Bunch;
typedef struct Shelf {
   char name[MAX_LEN];
   int total;
   Bunch *bunchList;
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
         printf("%10s\t%d\t%d", curRec->name, curRec->minUnbakeable, curRec->usage);
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

void printWarehouse(Warehouse *warehouse) {

   printf("\n");
   Shelf *curShe;
   for (int i = 0; i < HASH_SIZE; i++) {
      curShe = warehouse->buckets[i];
      while (curShe != NULL) {
         printf("%s:\t%*d:\t%d", curShe->name, 5, curShe->total, curShe->usage);
         Bunch *curBun = curShe->bunchList;
         while (curBun != NULL) {
            printf("%*d:%*d\t\t", 5, curBun->expiration, 5, curBun->amount);
            curBun = curBun->nextBunch;
         }
         curShe = curShe->nextShelf;
         printf("\n");
      }
   }
}

// void printOrderList(OrderList pendingOrders) {
//    printf("\n");
//    Order *cur = pendingOrders.head;
//    while (cur != NULL) {
//       printf("at %*d\t%10s:\t%*d\t tot:%*d", 5, cur->time, cur->name, 5, cur->amount, 5, cur->weight);
//       cur = cur->nextOrder;
//       printf("\n");
//    }
// }

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

      // searching for recipes with the same name
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
         curShe->bunchList = NULL;

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
            curShe->bunchList = NULL;

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

// Cookbook *removeRecipe(Cookbook *cookbook, State state) {
// char name[MAX_LEN];
// if (scanf("%s", name) == 0) {
//    printf("MUST NAME A RECIPE TO REMOVE");
// }
//
// // printf(" removing %s from: ", name);
// // printf("\nPENDING ORDERS");
// // printOrderList(state.pendingOrders);
// // printf("\nREADY ORDERS");
// // printOrderList(state.readyOrders);
//
// Order *curOrd = state.pendingOrders.head;
// while (curOrd != NULL) {
//    if (!strcmp(curOrd->name, name)) {
//       printf("ordini in sospeso\n");
//       return cookbook;
//    }
//    curOrd = curOrd->nextOrder;
// }
// curOrd = state.readyOrders.head;
// while (curOrd != NULL) {
//    if (!strcmp(curOrd->name, name)) {
//       printf("ordini in sospeso\n");
//       return cookbook;
//    }
//    curOrd = curOrd->nextOrder;
// }
//
// int i = hash(name);
//
// Recipe *pre = NULL;
// Recipe *cur = cookbook->buckets[i];
// Ingredient *delIng;
// if (cur != NULL && !strcmp(cur->name, name)) {
//    cookbook->buckets[i] = cur->nextRecipe;
//    while (cur->ingredientList != NULL) {
//       delIng = cur->ingredientList;
//       cur->ingredientList = cur->ingredientList->nextIngredient;
//       free(delIng);
//    }
//    free(cur);
//    printf("rimossa\n");
// } else {
//    while (cur != NULL && strcmp(cur->name, name)) {
//       pre = cur;
//       cur = cur->nextRecipe;
//    }
//
//    if (cur == NULL) {
//       printf("non presente\n");
//    } else {
//       pre->nextRecipe = cur->nextRecipe;
//
//       while (cur->ingredientList != NULL) {
//          delIng = cur->ingredientList;
//          cur->ingredientList = cur->ingredientList->nextIngredient;
//          free(delIng);
//       }
//       free(cur);
//       printf("rimossa\n");
//    }
// }
//
// return cookbook;
// }

// Bunch *newBunch(Bunch *bunch, int expiration, int amount) {
//    // ASSUMING BUNCH IS NOT EMPTY
//    // LOWEST EXPIRATION IN HEAD
//    Bunch *pre = NULL, *cur = bunch;
//    while (cur != NULL) {
//       if (expiration == cur->expiration) { // can return
//          cur->amount += amount;
//          return bunch;
//       } else if (expiration < cur->expiration) { // must add bunch before cur
//          break;
//       } else { // go on until cur is NULL, then add bunch before cur
//          pre = cur;
//          cur = cur->nextBunch;
//       }
//    }
//
//    Bunch *newBunch = (Bunch *)malloc(sizeof(Bunch));
//    newBunch->expiration = expiration;
//    newBunch->amount = amount;
//    newBunch->nextBunch = cur;
//    if (pre == NULL) {
//       bunch = newBunch;
//    } else {
//       pre->nextBunch = newBunch;
//    }
//
//    return bunch;
// }

// OrderList appendOrder(Order *order, OrderList pendingOrders) {

// if (pendingOrders.tail != NULL) {
//    pendingOrders.tail->nextOrder = order;
//    pendingOrders.tail = pendingOrders.tail->nextOrder;
// } else {
//    pendingOrders.head = order;
//    pendingOrders.tail = order;
// }
// return pendingOrders;
// }

// State tryBaking(Order *order, Recipe *recipe, State state, int time) {
//    // DOES NOT MODIFY PENDING LIST NOR READY LIST
//    // ONLY MODIFIES WAREHOUSE AND STATE.BAKING
//    // DOES NOT PRINT ANYTHING
//
//    // printf("trying to bake %s with:", order->name);
//    // printWarehouse(state.warehouse);
//    // printf("\nPENDING ORDERS");
//    // printOrderList(state.pendingOrders);
//    // printf("\nREADY ORDERS");
//    // printOrderList(state.readyOrders);
//    // printf(" %d", order->time);
//
//    if (recipe->minUnbakeable <= order->amount) {
//       state.baking = 0;
//       return state;
//    }
//
//    int i;
//
//    // checking if baking is possible
//    Bunch *delBun;
//    Shelf *preShe, *curShe;
//    Ingredient *curIng = recipe->ingredientList;
//    while (curIng != NULL) {
//       i = hash(curIng->name);
//       curShe = state.warehouse->buckets[i];
//       preShe = NULL;
//       while (curShe != NULL) {
//          if (curShe->bunchList->expiration <= time) {
//             // removing expired
//             while (curShe->bunchList != NULL) {
//                if (curShe->bunchList->expiration <= time) {
//                   delBun = curShe->bunchList;
//                   curShe->bunchList = curShe->bunchList->nextBunch;
//                   curShe->total -= delBun->amount;
//                   free(delBun);
//                } else {
//                   break;
//                }
//             }
//             if (curShe->bunchList == NULL) {
//                // Removing entire shelf
//                if (preShe == NULL) {
//                   // First shelf
//                   state.warehouse->buckets[i] = curShe->nextShelf;
//                } else {
//                   preShe->nextShelf = curShe->nextShelf;
//                }
//                // if we removed a necessary ingredient, we can't bake
//                free(curShe);
//                state.baking = 0;
//                return state;
//             }
//          }
//          if (!strcmp(curIng->name, curShe->name)) {
//             if (curIng->amount * order->amount <= curShe->total) {
//                break; // Enough curIng, check next ingredient
//             }
//             // printf("not enough %s to bake %s\n", curShe->name, order->name);
//             state.baking = 0;
//             return state;
//          }
//          preShe = curShe;
//          curShe = curShe->nextShelf;
//       }
//       if (curShe == NULL) {
//          // printf("%s not present at all to bake %s\n", curIng->name, order->name);
//          // printf(" state.baking==%d ", state.baking);
//          state.baking = 0;
//          return state;
//       }
//       curIng = curIng->nextIngredient;
//    }
//
//    // printf("(baking %s)\n", order->name);
//    state.baking = 1; // ARRIVATI QUI
//
//    // printf("\nWAREHOUSE CLEANED");
//    // printWarehouse(state.warehouse);
//
//    curIng = recipe->ingredientList;
//    int required, weight = 0;
//    while (curIng != NULL) { // TODO be sure you can always get in here the first time
//       i = hash(curIng->name);
//       curShe = state.warehouse->buckets[i];
//       preShe = NULL;
//       weight += curIng->amount * order->amount;
//
//       // this "while" should terminate only thorugh "break"
//       // since it's guaranteed to have all the required ingredients
//       while (curShe != NULL) {
//          if (!strcmp(curIng->name, curShe->name)) {
//             // Removing used ingredients
//             required = curIng->amount * order->amount;
//             while (required != 0) {
//                if (curShe->bunchList->amount <= required) {
//                   required -= curShe->bunchList->amount;
//                   curShe->total -= curShe->bunchList->amount;
//                   ////
//                   delBun = curShe->bunchList;
//                   curShe->bunchList = curShe->bunchList->nextBunch;
//                   free(delBun);
//                   // curShe->bunchList can become NULL
//                   // but we don't want empty shelves
//                   if (curShe->bunchList == NULL) {
//                      // Removing entire shelf
//                      if (preShe == NULL) {
//                         // First shelf
//                         state.warehouse->buckets[i] = curShe->nextShelf;
//                         // DANGER
//                      } else {
//                         preShe->nextShelf = curShe->nextShelf;
//                      }
//                      free(curShe);
//                      break;
//                   }
//                } else {
//                   curShe->bunchList->amount -= required;
//                   curShe->total -= required;
//                   // required = 0; useless
//                   break;
//                }
//             }
//             break;
//          }
//          preShe = curShe;
//          curShe = curShe->nextShelf;
//       }
//       ////
//       curIng = curIng->nextIngredient;
//    }
//
//    order->weight = weight;
//
//    return state;
// };

// State newBatch(State state, Cookbook *cookbook, int time) {
//
// int expiration, amount;
// char name[MAX_LEN];
//
// while (scanf("%s %d %d", name, &amount, &expiration)) {
//    int i = hash(name);
//    // printf(" <%s|%d>", name, i);
//    Shelf *pre = NULL;
//    Shelf *cur = state.warehouse->buckets[i];
//    while (cur != NULL) {
//       if (!strcmp(cur->name, name)) {
//          cur->bunchList = newBunch(cur->bunchList, expiration, amount);
//          cur->total += amount;
//          break;
//       } else {
//          pre = cur;
//          cur = cur->nextShelf;
//       }
//    }
//    if (cur == NULL) {
//       Shelf *newShelf = (Shelf *)malloc(sizeof(Shelf));
//       strcpy(newShelf->name, name);
//       newShelf->total = amount;
//       newShelf->nextShelf = NULL;
//       Bunch *newBunch = (Bunch *)malloc(sizeof(Bunch));
//       newShelf->bunchList = newBunch;
//       newShelf->bunchList->expiration = expiration;
//       newShelf->bunchList->amount = amount;
//       newShelf->bunchList->nextBunch = NULL;
//       if (state.warehouse->buckets[i] == NULL) {
//          state.warehouse->buckets[i] = newShelf;
//       } else {
//          pre->nextShelf = newShelf;
//       }
//    }
//    char c = getchar();
//    if (c == '\n' || c == '\r' || c == EOF) {
//       break;
//    }
// }
//
// // Warehouse *warehouse = state.warehouse;
// // printf("\nWAREHOUSE RECEIVED NEW BATCH; NOW CHECKING FOR POSSIBLE BAKING");
// // printWarehouse(warehouse);
//
// int i;
// Order *prePen = NULL, *curPen = state.pendingOrders.head;
// Order *preRea = NULL, *curRea;
// Order *nexPen;
// Recipe *recipe;
//
// // any recipe isn't unbakeable
// for (int i = 0; i < HASH_SIZE; i++) {
//    recipe = cookbook->buckets[i];
//    while (recipe != NULL) {
//       recipe->minUnbakeable = INFINITE;
//       recipe = recipe->nextRecipe;
//    }
// }
//
// while (curPen != NULL) {
//
//    i = hash(curPen->name);
//    recipe = cookbook->buckets[i];
//    while (recipe != NULL) {
//       if (!strcmp(recipe->name, curPen->name)) {
//          break;
//       }
//       recipe = recipe->nextRecipe;
//    }
//    // recipe == NULL is impossible
//    state = tryBaking(curPen, recipe, state, time);
//    int baking = state.baking;
//    nexPen = curPen->nextOrder;
//    if (baking) {
//       // printf(" transferring %d ", curPen->time);
//       // removing curPen from pending
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
//       // adding curPen in ready
//       curRea = state.readyOrders.head;
//       if (curRea == NULL) {
//          curPen->nextOrder = NULL;
//          state.readyOrders.head = curPen;
//          state.readyOrders.tail = curPen;
//       } else {
//          ////
//          while (curRea != NULL) {
//             if (curRea->time > curPen->time) {
//                break;
//             }
//             preRea = curRea;
//             curRea = curRea->nextOrder;
//          }
//
//          curPen->nextOrder = curRea;
//
//          if (preRea == NULL) {
//             state.readyOrders.head = curPen;
//          } else {
//             preRea->nextOrder = curPen;
//          }
//
//          if (curRea == NULL) {
//             state.readyOrders.tail = curPen;
//          }
//          ////
//       }
//       // prePen unchanged
//
//       // printf("\nPENDING ORDERS");
//       // printOrderList(state.pendingOrders);
//       // printf("\nREADY ORDERS");
//       // printOrderList(state.readyOrders);
//    } else {
//       prePen = curPen;
//    }
//    curPen = nexPen;
// }
//
// printf("rifornito\n");
// return state;
// }

// State newOrder(Cookbook *cookbook, State state, int time) {

// Order *newOrder = (Order *)malloc(sizeof(Order));

// if (scanf("%s %d", newOrder->name, &newOrder->amount) == 0) {
//    printf("EXPECTED ORDER NAME AND AMOUNT");
//    return state;
// }

// newOrder->nextOrder = NULL;
// newOrder->time = time;
// newOrder->weight = -1;
// int i = hash(newOrder->name);
// Recipe *recipe = cookbook->buckets[i];

// while (recipe != NULL) {
//    if (!strcmp(recipe->name, newOrder->name)) {
//       break;
//    }
//    recipe = recipe->nextRecipe;
// }
// if (recipe == NULL) {
//    state.baking = -1;
// } else if (recipe->minUnbakeable <= newOrder->amount) {
//    state.baking = 0;
// } else {
//    printf("Receiving order of %s; ", newOrder->name);
//    state = tryBaking(newOrder, recipe, state, time);
// }

// int baking = state.baking;
// printf(" state.baking==%d ", baking);

// switch (baking) {
// case -1:
//    printf("rifiutato\n");
//    free(newOrder);
//    break;
// case 0: // baking newOrder in the future
//    printf("accettato\n");
//    if (recipe->minUnbakeable > newOrder->amount) {
//       recipe->minUnbakeable = newOrder->amount;
//    }
//    state.pendingOrders = appendOrder(newOrder, state.pendingOrders);
//    break;
// case 1: // baked immediately!
//    printf("accettato\n");
//    if (state.readyOrders.tail == NULL) {
//       state.readyOrders.head = newOrder;
//       state.readyOrders.tail = newOrder;
//    } else {
//       newOrder has the least priority
//       state.readyOrders.tail->nextOrder = newOrder;
//       state.readyOrders.tail = newOrder;
//    }
//    break;
// default:
//    printf("UNKNOWN BAKING CODE");
//    break;
// }

// printf("\nPENDING ORDERS");
// printOrderList(state.pendingOrders);
// printf("\nREADY ORDERS");
// printOrderList(state.readyOrders);
// printCookbook(cookbook);

// return state;
// }

// State loadOrders(State state, int payloadLeft) {
//    // printf("\n");
//    // printf("\nWAREHOUSE");
//    // printWarehouse(state.warehouse);
//    // printf("\nPENDING ORDERS");
//    // printOrderList(state.pendingOrders);
//    // printf("\nREADY ORDERS");
//    // printOrderList(state.readyOrders);
//    if (state.readyOrders.head == NULL) {
//       printf("camioncino vuoto\n");
//    } else {
//       Order *loadingOrders = NULL, *curLoa, *preLoa, *delOrd;
//       Order *curRea = state.readyOrders.head;
//       while (curRea != NULL) {
//          // loading the van with ready orders by weight, by date
//          payloadLeft -= state.readyOrders.head->weight;
//          if (payloadLeft < 0) {
//             break;
//          }
//          state.readyOrders.head = state.readyOrders.head->nextOrder;
//          curLoa = loadingOrders;
//          preLoa = NULL;
//          while (curLoa != NULL) {
//             if (curRea->weight > curLoa->weight) {
//                break;
//             } else if (curRea->weight == curLoa->weight) {
//                if (curRea->time < curLoa->time) {
//                   break;
//                }
//             }
//             preLoa = curLoa;
//             curLoa = curLoa->nextOrder;
//          }
//          if (preLoa == NULL) {
//             curRea->nextOrder = curLoa;
//             loadingOrders = curRea;
//          } else {
//             preLoa->nextOrder = curRea;
//             curRea->nextOrder = curLoa;
//          }
//          curRea = state.readyOrders.head;
//       }
//       if (curRea == NULL) {
//          state.readyOrders.tail = NULL;
//       }
//       while (loadingOrders != NULL) {
//          printf("%d %s %d\n", loadingOrders->time, loadingOrders->name, loadingOrders->amount);
//          delOrd = loadingOrders;
//          loadingOrders = loadingOrders->nextOrder;
//          free(delOrd);
//       }
//    }
//    // printf("\n");
//    // printf("\nWAREHOUSE");
//    // printWarehouse(state.warehouse);
//    // printf("\nPENDING ORDERS");
//    // printOrderList(state.pendingOrders);
//    // printf("\nREADY ORDERS");
//    // printOrderList(state.readyOrders);
//    return state;
// }

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
         printf("[removeRecipe] ");
         // cookbook = removeRecipe(cookbook, state);

      } else if (!strcmp(command, "rifornimento")) {
         printf("[newBatch] ");
         // state = newBatch(state, cookbook, time);

      } else if (!strcmp(command, "ordine")) {
         printf("[newOrder] ");
         // state = newOrder(cookbook, state, time);

      } else {
         printf("ERROR: UNKNOWN COMMAND");
      }

      // // printf(" <%s> ", command);
      // printf("at [%d] ", time);
      // // printf("'%s'", state.warehouse->buckets[3]->name);
      // printf("\nCOOKBOOK");
      // printCookbook(cookbook);
      // printf("\nWAREHOUSE AFTER");
      // printWarehouse(state.warehouse);
      // printf("\nPENDING ORDERS");
      // printOrderList(state.pendingOrders);
      // // if (state.pendingOrders.tail)
      // //    printf("tail: %d", state.pendingOrders.tail->time);
      // printf("\nREADY ORDERS");
      // printOrderList(state.readyOrders);
      // // if (state.readyOrders.tail)
      // // printf("tail: %d", state.readyOrders.tail->time);
      time++;
   }
   if (time && time % courierPeriod == 0) {
      // state = loadOrders(state, maxPayload);
   }
   printf("\n");
   printf("\nCOOKBOOK");
   printCookbook(cookbook);
   printf("\nWAREHOUSE");
   printWarehouse(state.warehouse);
   // printf("\nPENDING ORDERS");
   // printOrderList(state.pendingOrders);
   // printf("\nREADY ORDERS");
   // printOrderList(state.readyOrders);
}