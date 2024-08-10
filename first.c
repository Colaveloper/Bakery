#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN 255
#define HASH_SIZE 5 // TODO be careful this is large enough

// ingredient, amount ∈ Ingredient ∈ *Recipe[] ∈ cookbook
typedef struct Ingredient {
   char name[MAX_LEN];
   int amount;
   struct Ingredient *nextIngredient;
} Ingredient;
typedef struct Recipe {
   char name[MAX_LEN];
   Ingredient *ingredientList;
   struct Recipe *nextRecipe;
} Recipe;
typedef struct Cookbook {
   Recipe *buckets[HASH_SIZE]; // Array of lists of recipes
} Cookbook;

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
} Shelf;

// name, amount ∈ Order ∈ pendingOrders, readyOrders
typedef struct Order {
   int time;
   char name[MAX_LEN];
   int amount;
   int weight;
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
   Shelf *warehouse;
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
         printf("%10s:\t", curRec->name);
         Ingredient *curIng = curRec->ingredientList;
         while (curIng != NULL) {
            printf("%10s:%*d\t", curIng->name, 5, curIng->amount);
            curIng = curIng->nextIngredient;
         }
         curRec = curRec->nextRecipe;
         printf("\n");
      }
   }
}

void printWarehouse(Shelf *warehouse) {

   printf("\n");
   Shelf *curShe = warehouse;
   while (curShe != NULL) {
      printf("%10s:\t%*d:\t", curShe->name, 5, curShe->total);
      Bunch *curBun = curShe->bunchList;
      while (curBun != NULL) {
         printf("%*d:%*d\t\t", 5, curBun->expiration, 5, curBun->amount);
         curBun = curBun->nextBunch;
      }
      curShe = curShe->nextShelf;
      printf("\n");
   }
}

void printOrderList(OrderList pendingOrders) {
   printf("\n");
   Order *cur = pendingOrders.head;
   while (cur != NULL) {
      printf("at %*d\t%10s:\t%*d\t tot:%*d", 5, cur->time, cur->name, 5, cur->amount, 5, cur->weight);
      cur = cur->nextOrder;
      printf("\n");
   }
}

Cookbook *newRecipe(Cookbook *cookbook) {
   char name[MAX_LEN];
   if (scanf("%s", name) == 0) {
      printf("MUST HAVE A RECIPE TO ADD");
      return cookbook;
   }

   int i;
   i = hash(name);
   printf("[i=%d] ", i);

   Recipe *pre = NULL, *cur = cookbook->buckets[i];
   Recipe *newRecipe = (Recipe *)malloc(sizeof(Recipe));
   strcpy(newRecipe->name, name);
   newRecipe->nextRecipe = NULL;

   //// ------->

   if (cur == NULL) {
      cookbook->buckets[i] = newRecipe;
   } else {
      while (cur != NULL) {
         if (!strcmp(cur->name, name)) {
            char c;
            printf("ignorato\n");
            while ((c = getchar()) != '\n') {
               continue;
            }
            return cookbook; // nothing to do
         }
         pre = cur;
         cur = cur->nextRecipe;
      }
      // adding to the end
      pre->nextRecipe = newRecipe;
   }

   Ingredient *lastPair = NULL;
   char ingredient[MAX_LEN];
   int amount;

   while (scanf("%s %d", ingredient, &amount)) {
      Ingredient *newPair = (Ingredient *)malloc(sizeof(Ingredient));
      strcpy(newPair->name, ingredient);
      newPair->amount = amount;
      newPair->nextIngredient = lastPair;
      lastPair = newPair;
      char c = getchar();
      if (c == '\n' ||c == '\r' || c == EOF) {
         printf("exiting ");
         break;
      }
   }

   newRecipe->ingredientList = lastPair;
   //// <-------

   printf("aggiunta\n");
   return cookbook; // added to the end
}

Cookbook *removeRecipe(Cookbook *cookbook, State state) {
   char name[MAX_LEN];
   if (scanf("%s", name) == 0) {
      printf("MUST NAME A RECIPE TO REMOVE");
   }

   int i = hash(name);

   // printf(" removing %s from: ", name);
   // printf("\nPENDING ORDERS");
   // printOrderList(state.pendingOrders);
   // printf("\nREADY ORDERS");
   // printOrderList(state.readyOrders);

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

   Recipe *pre = NULL;
   Recipe *cur = cookbook->buckets[i];
   if (cur != NULL && !strcmp(cur->name, name)) {
      cookbook->buckets[i] = cur->nextRecipe;
      free(cur);
      printf("rimossa\n");
   } else {
      while (cur != NULL && strcmp(cur->name, name)) {
         pre = cur;
         cur = cur->nextRecipe;
      }

      if (cur == NULL) {
         printf("non presente\n");
      } else {
         pre->nextRecipe = cur->nextRecipe;
         free(cur);
         printf("rimossa\n");
      }
   }

   return cookbook;
}

Bunch *newBunch(Bunch *bunch, int expiration, int amount) {
   // ASSUMING BUNCH IS NOT EMPTY
   // LOWEST EXPIRATION IN HEAD
   Bunch *pre = NULL, *cur = bunch;
   while (cur != NULL) {
      if (expiration == cur->expiration) { // can return
         cur->amount += amount;
         return bunch;
      } else if (expiration < cur->expiration) { // must add bunch before cur
         break;
      } else { // go on until cur is NULL, then add bunch before cur
         pre = cur;
         cur = cur->nextBunch;
      }
   }

   Bunch *newBunch = (Bunch *)malloc(sizeof(Bunch));
   newBunch->expiration = expiration;
   newBunch->amount = amount;
   newBunch->nextBunch = cur;
   if (pre == NULL) {
      bunch = newBunch;
   } else {
      pre->nextBunch = newBunch;
   }

   return bunch;
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

// State tryBaking(Order *order, Recipe *cookbook, State state) {
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
//
//    // printf(" %d", order->time);
//
//    state.baking = 0;
//
//    Recipe *recipe = cookbook;
//    while (recipe != NULL) {
//       if (!strcmp(recipe->name, order->name)) {
//          break;
//       }
//       recipe = recipe->nextRecipe;
//    }
//
//    if (recipe == NULL) {
//       // rifiutato
//       state.baking = -1;
//       return state;
//    }
//
//    // printf(" TB ");
//
//    // cleanWarehouse
//    // checking if baking is possible
//    Shelf *curShe;
//    Ingredient *curIng = recipe->ingredientList;
//    while (curIng != NULL) {
//       curShe = state.warehouse;
//       while (curShe != NULL) {
//          if (!strcmp(curIng->name, curShe->name)) {
//             if (curIng->amount * order->amount <= curShe->total) {
//                break; // Enough curIng, check next ingredient
//             }
//             // printf("not enough %s to bake %s\n", curShe->name, order->name);
//             return state;
//          }
//          curShe = curShe->nextShelf;
//       }
//       if (curShe == NULL) {
//          // printf("%s not present at all to bake %s\n", curIng->name, order->name);
//          // printf(" state.baking==%d ", state.baking);
//          return state;
//       }
//       curIng = curIng->nextIngredient;
//    }
//
//    // printf("(baking %s)\n", order->name);
//    state.baking = 1; // ARRIVATI QUI
//    // printf(" B ");
//
//    curIng = recipe->ingredientList;
//    Shelf *preShe;
//    Bunch *delBun;
//    int required, weight = 0;
//    while (curIng != NULL) { // TODO be sure you can always get in here the first time
//       curShe = state.warehouse;
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
//                         state.warehouse = curShe->nextShelf;
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

// State newBatch(State state, Recipe *cookbook) {
//
//    int expiration, amount;
//    char name[MAX_LEN];
//
//    while (scanf("%s %d %d", name, &amount, &expiration)) {
//       Shelf *pre, *cur = state.warehouse;
//       while (cur != NULL) {
//          if (!strcmp(cur->name, name)) {
//             cur->bunchList = newBunch(cur->bunchList, expiration, amount);
//             cur->total += amount;
//             break;
//          } else {
//             pre = cur;
//             cur = cur->nextShelf;
//          }
//       }
//       if (cur == NULL) {
//          Shelf *newShelf = (Shelf *)malloc(sizeof(Shelf));
//          strcpy(newShelf->name, name);
//          newShelf->total = amount;
//          newShelf->nextShelf = NULL;
//          Bunch *newBunch = (Bunch *)malloc(sizeof(Bunch));
//          newShelf->bunchList = newBunch;
//          newShelf->bunchList->expiration = expiration;
//          newShelf->bunchList->amount = amount;
//          newShelf->bunchList->nextBunch = NULL;
//          if (state.warehouse == NULL) {
//             state.warehouse = newShelf;
//          } else {
//             pre->nextShelf = newShelf;
//          }
//       }
//       char c = getchar();
//       if (c == '\n' ||c == '\n' || c == EOF) {
//          break;
//       }
//    }
//
//    Order *prePen = NULL, *curPen = state.pendingOrders.head;
//    Order *preRea = NULL, *curRea;
//    Order *nexPen;
//    while (curPen != NULL) {
//       state = tryBaking(curPen, cookbook, state);
//       int baking = state.baking;
//       nexPen = curPen->nextOrder;
//       if (baking) {
//          // printf(" transferring %d ", curPen->time);
//          // removing curPen from pending
//          if (prePen == NULL) {
//             state.pendingOrders.head = curPen->nextOrder;
//             if (state.pendingOrders.head == NULL) {
//                state.pendingOrders.tail = NULL;
//             }
//          } else {
//             prePen->nextOrder = curPen->nextOrder;
//             if (state.pendingOrders.tail == curPen) {
//                state.pendingOrders.tail = prePen;
//             }
//          }
//          // adding curPen in ready
//          curRea = state.readyOrders.head;
//          if (curRea == NULL) {
//             curPen->nextOrder = NULL;
//             state.readyOrders.head = curPen;
//             state.readyOrders.tail = curPen;
//          } else {
//             ////
//             while (curRea != NULL) {
//                if (curRea->time > curPen->time) {
//                   break;
//                }
//                preRea = curRea;
//                curRea = curRea->nextOrder;
//             }
//
//             curPen->nextOrder = curRea;
//
//             if (preRea == NULL) {
//                state.readyOrders.head = curPen;
//             } else {
//                preRea->nextOrder = curPen;
//             }
//
//             if (curRea == NULL) {
//                state.readyOrders.tail = curPen;
//             }
//             ////
//          }
//          // prePen unchanged
//
//          // printf("\nPENDING ORDERS");
//          // printOrderList(state.pendingOrders);
//          // printf("\nREADY ORDERS");
//          // printOrderList(state.readyOrders);
//       } else {
//          prePen = curPen;
//       }
//       curPen = nexPen;
//    }
//
//    printf("rifornito\n");
//    return state;
// }
//
// State newOrder(Recipe *cookbook, State state, int time) {
//
//    Order *newOrder = (Order *)malloc(sizeof(Order));
//
//    if (scanf("%s %d", newOrder->name, &newOrder->amount) == 0) {
//       printf("EXPECTED ORDER NAME AND AMOUNT");
//       return state;
//    }
//
//    newOrder->nextOrder = NULL;
//    newOrder->time = time;
//    newOrder->weight = -1;
//
//    // printf("Receiving order of %s; ", newOrder->name);
//    state = tryBaking(newOrder, cookbook, state);
//    int baking = state.baking;
//    // printf(" state.baking==%d ", baking);
//
//    switch (baking) {
//    case -1:
//       printf("rifiutato\n");
//       break;
//    case 0: // baking newOrder in the future
//       printf("accettato\n");
//       state.pendingOrders = appendOrder(newOrder, state.pendingOrders);
//       break;
//    case 1: // baked immediately!
//       printf("accettato\n");
//       if (state.readyOrders.tail == NULL) {
//          state.readyOrders.head = newOrder;
//          state.readyOrders.tail = newOrder;
//       } else {
//          // newOrder has the least priority
//          state.readyOrders.tail->nextOrder = newOrder;
//          state.readyOrders.tail = newOrder;
//       }
//       break;
//    default:
//       printf("UNKNOWN BAKING CODE");
//       break;
//    }
//
//    // printf("\nPENDING ORDERS");
//    // printOrderList(state.pendingOrders);
//    // printf("\nREADY ORDERS");
//    // printOrderList(state.readyOrders);
//    // printCookbook(cookbook);
//
//    return state;
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
//       Order *loadingOrders = NULL, *curLoa, *preLoa;
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
//          loadingOrders = loadingOrders->nextOrder;
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

int main() { // TODO make cookbook testable commenting all instances of previous implementation

   int courierPeriod, maxPayload, time = 0;
   char command[MAX_LEN];
   Cookbook *cookbook = (Cookbook *)malloc(sizeof(Cookbook));
   for (int i = 0; i < HASH_SIZE; i++) {
      cookbook->buckets[i] = NULL;
   }
   State state = {{NULL, NULL}, {NULL, NULL}, NULL, 0};

   if (scanf("%d", &courierPeriod) == 0 || scanf("%d", &maxPayload) == 0) {
      perror("MUST SPECIFY COURIER PERIOD AND MAX PAYLOAD");
      return -1;
   }

   while (scanf("%s", command) > 0) {
      // printf("\n[%d] ", time);

      if (time && time % courierPeriod == 0) {
         // state = loadOrders(state, maxPayload);
      }

      if (!strcmp(command, "aggiungi_ricetta")) {
         // printf("[newRecipe at %d] ", time);
         cookbook = newRecipe(cookbook);

      } else if (!strcmp(command, "rimuovi_ricetta")) {
         // printf("[removeRecipe] ");
         cookbook = removeRecipe(cookbook, state);

      } else if (!strcmp(command, "rifornimento")) {
         // printf("[newBatch] ");
         // state = newBatch(state, cookbook);

      } else if (!strcmp(command, "ordine")) {
         // printf("[newOrder] ");
         // state = newOrder(cookbook, state, time);
      }
      // printf("\nPENDING ORDERS");
      // printOrderList(state.pendingOrders);
      // if (state.pendingOrders.tail)
      //    printf("tail: %d", state.pendingOrders.tail->time);
      // printf("\nREADY ORDERS");
      // printOrderList(state.readyOrders);
      // if (state.readyOrders.tail)
      //    printf("tail: %d", state.readyOrders.tail->time);
      time++;
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