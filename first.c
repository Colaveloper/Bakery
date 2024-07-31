#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN 255

// ingredient, amount ∈ Ingredient ∈ Recipe ∈ cookbook
typedef struct Ingredient {
   char ingredient[MAX_LEN];
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
   Bunch *bunchList;
   struct Shelf *nextShelf;
} Shelf;

void printCookbook(Recipe *cookbook) {
   Recipe *curRec = cookbook;
   while (curRec != NULL) {
      printf("%s: ", curRec->name);
      Ingredient *curIng = curRec->ingredientList;
      while (curIng != NULL) {
         printf("%s %d, ", curIng->ingredient, curIng->amount);
         curIng = curIng->nextIngredient;
      }
      curRec = curRec->nextRecipe;
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
      strcpy(newPair->ingredient, ingredient);
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

void printWarehouse(Shelf *warehouse) {

   printf("\n");
   Shelf *curShe = warehouse;
   while (curShe != NULL) {
      printf("%s: ", curShe->name);
      Bunch *curBun = curShe->bunchList;
      while (curBun != NULL) {
         printf("(%d %d), ", curBun->expiration, curBun->amount);
         curBun = curBun->nextBunch;
      }
      curShe = curShe->nextShelf;
      printf("\n");
   }
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
            break;
         } else {
            pre = cur;
            cur = cur->nextShelf;
         }
      }
      if (cur == NULL) {
         Shelf *newShelf = (Shelf *)malloc(sizeof(Shelf));
         strcpy(newShelf->name, name);
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

int main() {
   int courierPeriod, maxPayload, time = 0;
   Recipe *cookbook = NULL;
   Shelf *warehouse = NULL;
   char command[MAX_LEN], commandLine[MAX_LEN];

   if (scanf("%d", &courierPeriod) == 0 || scanf("%d", &maxPayload) == 0) {
      printf("MUST SPECIFY COURIER PERIOD AND MAX PAYLOAD");
      return -1;
   }

   while (scanf("%s", command) > 0) {

      if (time && time % courierPeriod == 0) {
         printf("(courier) ");
      }

      if (!strcmp(command, "aggiungi_ricetta")) {

         cookbook = newRecipe(cookbook);

      } else if (!strcmp(command, "rimuovi_ricetta")) {

         cookbook = removeRecipe(cookbook);

      } else if (!strcmp(command, "rifornimento")) {

         warehouse = newBatch(warehouse);

      } else if (!strcmp(command, "ordine")) {

         printf("new order\n");
         if (fgets(commandLine, MAX_LEN, stdin) == 0) {
            return -1; // empty command argument error
         }
      }

      time++;
   }
   printf("\n");
   printCookbook(cookbook);
   printWarehouse(warehouse);
}