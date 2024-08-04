#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN 255

// ingredient, amount ∈ Ingredient ∈ Recipe ∈ cookbook
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
} State;

void printCookbook(Recipe *cookbook) {
   printf("\n");
   Recipe *curRec = cookbook;
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
      printf("%*d\t%10s:\t%*d\t", 5, cur->time, cur->name, 5, cur->amount);
      cur = cur->nextOrder;
      printf("\n");
   }
}

Recipe *newRecipe(Recipe *cookbook) {
   char name[MAX_LEN];
   if (scanf("%s", name) == 0) {
      printf("MUST HAVE A RECIPE TO REMOVE");
      return cookbook;
   }

   Recipe *cur, *pre = cookbook;
   Recipe *newRecipe = (Recipe *)malloc(sizeof(Recipe));
   strcpy(newRecipe->name, name);
   newRecipe->nextRecipe = NULL;

   if (pre == NULL) {
      cookbook = newRecipe;
   } else {
      cur = pre->nextRecipe;
      while (cur != NULL) {
         if (!strcmp(pre->name, name)) {
            printf("ignorato\n");
            return cookbook; // nothing to do
         }
         pre = cur;
         cur = cur->nextRecipe;
      }
      pre->nextRecipe = newRecipe;
   }

   char commandLine[MAX_LEN];
   // TODO allow for longer commands
   if (fgets(commandLine, MAX_LEN, stdin) == 0) {
      printf("NEW RECIPE MUST SPECIFY A RECIPE");
      return cookbook;
   }

   Ingredient *lastPair = NULL;
   char ingredient[MAX_LEN];
   int amount;
   char *ptr = commandLine;

   while (sscanf(ptr, "%s %d", ingredient, &amount) == 2) {
      Ingredient *newPair = (Ingredient *)malloc(sizeof(Ingredient));
      strcpy(newPair->name, ingredient);
      newPair->amount = amount;
      newPair->nextIngredient = lastPair;
      lastPair = newPair;
      ptr++;
      while (*ptr != ' ')
         ptr++;
      ptr++;
      while (*ptr != ' ')
         ptr++;
   }

   newRecipe->ingredientList = lastPair;
   printf("aggiunta\n");
   return cookbook; // added to the end
}

Recipe *removeRecipe(Recipe *cookbook) {
   char name[MAX_LEN];
   if (scanf("%s", name) == 0) {
      printf("MUST NAME A RECIPE TO REMOVE");
   }

   Recipe *pre = NULL;
   Recipe *cur = cookbook;

   if (cur != NULL && !strcmp(cur->name, name)) {
      // TODO check for in sospeso:
      //    if name exists in pendingOrders
      //       Print "ordine in attesa"
      //    else
      cookbook = cur->nextRecipe;
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
      if (expiration == cur->expiration) {
         cur->amount += amount;
         break; // exit
      } else if (expiration < cur->expiration) {
         break;
      } else {
         pre = cur;
         cur = cur->nextBunch;
      }
   }
   if (expiration != cur->expiration) {
      Bunch *newBunch = (Bunch *)malloc(sizeof(Bunch));
      newBunch->expiration = expiration;
      newBunch->amount = amount;
      newBunch->nextBunch = cur;
      if (pre == NULL) {
         bunch = newBunch;
      } else {
         pre->nextBunch = newBunch;
      }
   }
   return bunch;
}

Shelf *newBatch(Shelf *warehouse) {
   char commandLine[MAX_LEN];
   if (fgets(commandLine, MAX_LEN, stdin) == 0) {
      printf("BATCH ANNOUNCED BUT NOT DELIVERED");
   }

   int expiration, amount;
   char name[MAX_LEN], *ptr = commandLine;

   while (sscanf(ptr, "%s %d %d", name, &expiration, &amount) == 3) {
      Shelf *pre, *cur = warehouse;
      if (cur == NULL) {
      }
      while (cur != NULL) {
         if (!strcmp(cur->name, name)) {
            cur->bunchList = newBunch(cur->bunchList, expiration, amount);
            cur->total += amount;
            break;
         } else {
            pre = cur;
            cur = cur->nextShelf;
         }
      }
      if (cur == NULL) {
         Shelf *newShelf = (Shelf *)malloc(sizeof(Shelf));
         strcpy(newShelf->name, name);
         newShelf->total = amount;
         newShelf->nextShelf = NULL;
         Bunch *newBunch = (Bunch *)malloc(sizeof(Bunch));
         newShelf->bunchList = newBunch;
         newShelf->bunchList->expiration = expiration;
         newShelf->bunchList->amount = amount;
         newShelf->bunchList->nextBunch = NULL;
         if (warehouse == NULL) {
            warehouse = newShelf;
         } else {
            pre->nextShelf = newShelf;
         }
      }
      ptr++;
      while (*ptr != ' ')
         ptr++;
      ptr++;
      while (*ptr != ' ')
         ptr++;
      ptr++;
      while (*ptr != ' ')
         ptr++;
   }

   printf("rifornito\n");
   return warehouse;
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

State tryBaking(Order *order, Recipe *cookbook, State state) {

   Recipe *recipe = cookbook;
   while (recipe != NULL) {
      if (!strcmp(recipe->name, order->name)) {
         break;
      }
      recipe = recipe->nextRecipe;
   }

   if (recipe == NULL) {
      printf("rifiutato\n");
      return state;
   }

   // cleanWarehouse

   Shelf *curShe;
   Ingredient *curIng = recipe->ingredientList;
   while (curIng != NULL) {
      curShe = state.warehouse;
      while (curShe != NULL) {
         if (!strcmp(curIng->name, curShe->name)) {
            if (curIng->amount <= curShe->total) {
               break; // Enough curIng, check next ingredient
            }
            printf("not enough %s to bake %s\n", curShe->name, order->name);
            state.pendingOrders = appendOrder(order, state.pendingOrders);
            return state;
         }
         curShe = curShe->nextShelf;
      }
      if (curShe == NULL) {
         printf("%s not present at all\n", curIng->name);
         state.pendingOrders = appendOrder(order, state.pendingOrders);
         return state;
      }
      curIng = curIng->nextIngredient;
   }

   printf("baking %s\n", order->name);

   curIng = recipe->ingredientList;
   Shelf *preShe, *delShe; 
   Bunch *delBun;
   int required, delta;
   while (curIng != NULL) {
      curShe = state.warehouse;
      preShe = NULL;
      while (1) {
         if (!strcmp(curIng->name, curShe->name)) {
            // Removing used ingredients
            required = curIng->amount;
            while (required != 0) {
               if (curShe->bunchList->amount <= required) {
                  delta = required - curShe->bunchList->amount;
                  required -= delta;
                  curShe->total -= delta;
                  delBun = curShe->bunchList;
                  curShe->bunchList = curShe->bunchList->nextBunch; // curShe->bunchList can become NULL
                  free(delBun);
                  if (curShe->bunchList == NULL) {
                     // Removing entire shelf
                     if (preShe == NULL) {
                        // First shelf
                        state.warehouse = curShe->nextShelf;
                     }
                     delShe = curShe;              
                     curShe = curShe->nextShelf;                     
                     free(delShe);
                  }
               } else {
                  delta = curShe->bunchList->amount - required;
                  required -= delta;
                  curShe->bunchList->amount -= delta;
                  curShe->total -= delta;
                  break;
               }
            }
            break;
         }
         preShe = curShe;
         curShe = curShe->nextShelf;
      }
      curIng = curIng->nextIngredient;
   }

   // Adding to tail of readyOrders

   if (state.readyOrders.tail == NULL) {
      state.readyOrders.head = order;
      state.readyOrders.tail = order;
   } else {
      state.readyOrders.tail = order;
      state.readyOrders.tail = state.readyOrders.tail->nextOrder;
   }

   return state;
};

State newOrder(Recipe *cookbook, State state, int time) {

   Order *newOrder = (Order *)malloc(sizeof(Order));

   if (scanf("%s %d", newOrder->name, &newOrder->amount) == 0) {
      printf("EXPECTED ORDER NAME AND AMOUNT");
      return state;
   }

   newOrder->nextOrder = NULL;
   newOrder->time = time;

   return tryBaking(newOrder, cookbook, state);
}

int main() {

   int courierPeriod, maxPayload, time = 0;
   char command[MAX_LEN];
   Recipe *cookbook = NULL;
   State state = {{NULL, NULL}, {NULL, NULL}, NULL};

   if (scanf("%d", &courierPeriod) == 0 || scanf("%d", &maxPayload) == 0) {
      printf("MUST SPECIFY COURIER PERIOD AND MAX PAYLOAD");
      return -1;
   }

   while (scanf("%s", command) > 0) {
      printf("\n(%d) ", time);

      if (time && time % courierPeriod == 0) {
         printf("(courier) ");
      }

      if (!strcmp(command, "aggiungi_ricetta")) {

         cookbook = newRecipe(cookbook);

      } else if (!strcmp(command, "rimuovi_ricetta")) {

         cookbook = removeRecipe(cookbook);
         // TODO check for in sospeso

      } else if (!strcmp(command, "rifornimento")) {

         state.warehouse = newBatch(state.warehouse);

         // Go through pendingOrders
         // If any order can be prepared
         // bake(that order)

      } else if (!strcmp(command, "ordine")) {

         state = newOrder(cookbook, state, time);
      }

      time++;
   }
   printf("\n");
   printCookbook(cookbook);
   printWarehouse(state.warehouse);
   printOrderList(state.pendingOrders);
   printOrderList(state.readyOrders);
}