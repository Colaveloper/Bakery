
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

   int i;

   // printf(" mu:%d>am:%d, checking if baking is possible ", recipe->minUnbakeable, order->amount);
   Shelf *preShe, *curShe;
   Ingredient *curIng = recipe->ingredientList;
   while (curIng != NULL) {
      i = hash(curIng->shelf->name, WARE_SIZE);
      curShe = state.warehouse->buckets[i];
      preShe = NULL;
      while (curShe != NULL) {

         // ingredient missing
         if (curShe->total == 0) {
            state.baking = 0;
            recipe->minUnbakeable = 0;
            return state;
         }

         

         if (!strcmp(curIng->shelf->name, curShe->name)) {

            recipe->minUnbakeable = fmin(recipe->minUnbakeable, curShe->total / curIng->amount + 1);
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
      i = hash(curIng->shelf->name, WARE_SIZE);
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
