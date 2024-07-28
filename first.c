#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN 255

typedef struct IngredientAndQuantity {
   char ingredient[MAX_LEN];
   int quantity;
   struct IngredientAndQuantity *nextIngredient;
} IngredientAndQuantity;

typedef struct Recipe {
   char name[MAX_LEN];
   IngredientAndQuantity *ingredientList;
   struct Recipe *nextRecipe;
} Recipe;

int courierPeriod, maxPayload, time = 0;
Recipe *cookbook = NULL;

int newRecipe() {
   char name[MAX_LEN];
   if (scanf("%s", name) == 0) {
      return -1; // no name error
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
            printf("ignorato");
            return 0; // nothing to do
         }
         pre = cur;
         cur = cur->nextRecipe;
      }
      pre->nextRecipe = newRecipe;
   }

   char commandLine[MAX_LEN];
   if (fgets(commandLine, MAX_LEN, stdin) == 0) {
      return -1; // empty command argument error
   }

   IngredientAndQuantity *lastPair = NULL;
   char ingredient[MAX_LEN];
   int quantity;
   char *ptr = commandLine;

   while (sscanf(ptr, "%s %d", ingredient, &quantity) == 2) {
      IngredientAndQuantity *newPair = (IngredientAndQuantity *)malloc(sizeof(IngredientAndQuantity));
      strcpy(newPair->ingredient, ingredient);
      newPair->quantity = quantity;
      newPair->nextIngredient = lastPair;
      lastPair = newPair;
      ptr++;
      while (*ptr != ' ')
         ptr++;
      ptr++;
      while (*ptr != ' ')
         ptr++;

      printf("(%s, %d) ", ingredient, quantity);
   }

   newRecipe->ingredientList = lastPair;
   return 1; // added to the end
}

int printCookbook() {
   Recipe *curRec = cookbook;
   while (curRec != NULL) {
      printf("\n%s: ", curRec->name);
      IngredientAndQuantity *curIAQ = curRec->ingredientList;
      while (curIAQ != NULL) {
         printf("%s %d, ", curIAQ->ingredient, curIAQ->quantity);
         curIAQ = curIAQ->nextIngredient;
      }
      curRec = curRec->nextRecipe;
   }
   return 0;
}

int main() {
   char command[MAX_LEN], commandLine[MAX_LEN];

   printf("<><><><><><><><><><><><>\n");

   if (scanf("%d", &courierPeriod) == 0 || scanf("%d", &maxPayload) == 0) {
      return -1; // missing initialization error
   }

   while (scanf("%s", command) > 0) {

      if (time && time % courierPeriod == 0) {
         printf("(courier) ");
      }

      if (!strcmp(command, "aggiungi_ricetta")) {
         printf("new recipe ");
         newRecipe();
         printf("\n");
      } else if (!strcmp(command, "rimuovi_ricetta")) {
         printf("remove recipe\n");
         if (fgets(commandLine, MAX_LEN, stdin) == 0) {
            return -1; // empty command argument error
         }
      } else if (!strcmp(command, "rifornimento")) {
         printf("new batch\n");
         if (fgets(commandLine, MAX_LEN, stdin) == 0) {
            return -1; // empty command argument error
         }
      } else if (!strcmp(command, "ordine")) {
         printf("new order\n");
         if (fgets(commandLine, MAX_LEN, stdin) == 0) {
            return -1; // empty command argument error
         }
      } else {
         return -1; // unknown command error
      }

      time++;
   }

   printCookbook();
}