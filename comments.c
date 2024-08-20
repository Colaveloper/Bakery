


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
      // printf(" mu:%d<=am:%d, not even trying\n", recipe->minUnbakeable, newOrder->amount);
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
         // printf(" mu reduced to %d\n", recipe->minUnbakeable);
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

