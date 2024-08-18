
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

   if (recipe->minUnbakeable <= order->amount) {
      state.baking = 0;
      return state;
   }

   int i;

   // checking if baking is possible
   Bunch *delBun;
   Shelf *preShe, *curShe;
   Ingredient *curIng = recipe->ingredientList;
   while (curIng != NULL) {
      i = hash(curIng->shelf->name);
      curShe = state.warehouse->buckets[i];
      preShe = NULL;
      while (curShe != NULL) {
         if (curShe->total == 0) {

            // completly missing an ingredient
            state.baking = 0;
            return state;
         }
         if (curShe->bunchList->expiration <= time) {
            // removing expired
            while (curShe->bunchList != NULL) {
               if (curShe->bunchList->expiration <= time) {
                  delBun = curShe->bunchList;
                  curShe->bunchList = curShe->bunchList->nextBunch;
                  curShe->total -= delBun->amount;
                  free(delBun);
               } else {
                  break;
               }
            }
            if (curShe->bunchList == NULL) {
               // TODO handling empty shelves
               // Removing entire shelf
               // if (preShe == NULL) {
               //    // First shelf
               //    state.warehouse->buckets[i] = curShe->nextShelf;
               // } else {
               //    preShe->nextShelf = curShe->nextShelf;
               // }
               // // if we removed a necessary ingredient, we can't bake
               // free(curShe);
               state.baking = 0;
               return state;
            }
         }
         if (!strcmp(curIng->shelf->name, curShe->name)) {
            if (curIng->amount * order->amount <= curShe->total) {
               break; // Enough curIng, check next ingredient
            }
            // printf("not enough %s to bake %s\n", curShe->name, order->name);
            state.baking = 0;
            return state;
         }
         preShe = curShe;
         curShe = curShe->nextShelf;
      }
      if (curShe == NULL) {
         // printf("%s not present at all to bake %s\n", curIng->name, order->name);
         // printf(" state.baking==%d ", state.baking);
         state.baking = 0;
         return state;
      }
      curIng = curIng->nextIngredient;
   }

   // printf(" (baking %s) \n", order->recipe->name);
   state.baking = 1; // ARRIVATI QUI

   // printf("\nWAREHOUSE CLEANED");
   // printWarehouse(state.warehouse);

   curIng = recipe->ingredientList;
   int required, weight = 0;
   while (curIng != NULL) { // TODO be sure you can always get in here the first time
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
               if (curShe->bunchList->amount <= required) {
                  required -= curShe->bunchList->amount;
                  curShe->total -= curShe->bunchList->amount;
                  ////
                  delBun = curShe->bunchList;
                  curShe->bunchList = curShe->bunchList->nextBunch;
                  free(delBun);
                  // curShe->bunchList can become NULL
                  // but we don't want empty shelves
                  if (curShe->bunchList == NULL) {
                     // Removing entire shelf
                     if (preShe == NULL) {
                        // First shelf
                        state.warehouse->buckets[i] = curShe->nextShelf;
                        // DANGER
                     } else {
                        preShe->nextShelf = curShe->nextShelf;
                     }
                     // TODO restore free
                     // free(curShe);
                     break;
                  }
               } else {
                  curShe->bunchList->amount -= required;
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

State newOrder(Cookbook *cookbook, State state, int time) {

   Order *newOrder = (Order *)malloc(sizeof(Order));
   newOrder->nextOrder = NULL;
   newOrder->time = time;
   newOrder->weight = -1;

   char name[MAX_LEN];
   if (scanf("%s %d", name, &newOrder->amount) == 0) {
      printf("EXPECTED ORDER NAME AND AMOUNT");
      return state;
   }

   // searching for a recipe to connect

   int i = hash(name);
   Recipe *recipe = cookbook->buckets[i];

   while (recipe != NULL) {
      if (!strcmp(recipe->name, name)) {
         break;
      }
      recipe = recipe->nextRecipe;
   }
   if (recipe == NULL) {

      // recipe not found
      state.baking = -1;
   } else if (recipe->minUnbakeable <= newOrder->amount) {

      // we couldn't bake smaller orders of the same recipe
      state.baking = 0;
      recipe->usage++;
      newOrder->recipe = recipe;
   } else {

      // let's try
      recipe->usage++;
      newOrder->recipe = recipe;
      // printf("Receiving order of %s; ", newOrder->recipe->name);
      state = tryBaking(newOrder, recipe, state, time);
   }

   switch (state.baking) {
   case -1:
      printf("rifiutato\n");
      free(newOrder);
      break;
   case 0: // baking newOrder in the future
      printf("accettato\n");
      // printf("baking in the future\n");
      if (recipe->minUnbakeable > newOrder->amount) {
         recipe->minUnbakeable = newOrder->amount;
      }
      state.pendingOrders = appendOrder(newOrder, state.pendingOrders);
      break;
   case 1: // baked immediately!
      printf("accettato\n");
      // printf("baked immediately\n");
      if (state.readyOrders.tail == NULL) {
         state.readyOrders.head = newOrder;
         state.readyOrders.tail = newOrder;
      } else {
         // newOrder has the least priority
         state.readyOrders.tail->nextOrder = newOrder;
         state.readyOrders.tail = newOrder;
      }
      break;
   default:
      printf("UNKNOWN BAKING CODE");
      break;
   }

   // printf("\nPENDING ORDERS");
   // printOrderList(state.pendingOrders);
   // printf("\nREADY ORDERS");
   // printOrderList(state.readyOrders);
   // printCookbook(cookbook);

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
