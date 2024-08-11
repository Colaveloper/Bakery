#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 101  // Preferably a prime number

// Define the Recipe struct DONE
typedef struct Recipe {
    char name[100];
    char ingredients[500];
    char instructions[1000];
    struct Recipe *next;  // Pointer for the linked list
} Recipe;

// Define the Hash Table DONE
typedef struct HashTable {
    Recipe *buckets[TABLE_SIZE];  // Array of pointers to linked lists
} HashTable;

// Hash function (djb2) DONE
unsigned int hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}

// Initialize the hash table DONE
HashTable* createHashTable() {
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    if (table == NULL) {
        perror("Failed to create hash table");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}

// Insert a recipe into the hash table DONE
void insertRecipe(HashTable *table, const char *name, const char *ingredients, const char *instructions) {
    unsigned int index = hash(name);
    Recipe *newRecipe = (Recipe *)malloc(sizeof(Recipe));
    if (newRecipe == NULL) {
        perror("Failed to insert recipe");
        exit(EXIT_FAILURE);
    }
    strcpy(newRecipe->name, name);
    strcpy(newRecipe->ingredients, ingredients);
    strcpy(newRecipe->instructions, instructions);
    ////
    newRecipe->next = table->buckets[index];
    table->buckets[index] = newRecipe;
    ////
}

// Print all recipes in the hash table DONE
void printHashTable(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Recipe *current = table->buckets[i];
        if (current != NULL) {
            printf("Bucket %d:\n", i);
            while (current != NULL) {
                printf("  Recipe Name: %s\n", current->name);
                printf("  Ingredients: %s\n", current->ingredients);
                printf("  Instructions: %s\n\n", current->instructions);
                current = current->next;
            }
        }
    }
}

// Search for a recipe by name 
Recipe* searchRecipe(HashTable *table, const char *name) {
    unsigned int index = hash(name);
    Recipe *current = table->buckets[index];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Delete a recipe by name DONE
void deleteRecipe(HashTable *table, const char *name) {
    unsigned int index = hash(name);
    Recipe *current = table->buckets[index];
    Recipe *previous = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (previous == NULL) {
                table->buckets[index] = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            printf("Recipe '%s' deleted.\n", name);
            return;
        }
        previous = current;
        current = current->next;
    }

    printf("Recipe '%s' not found.\n", name);
}

// Free the hash table
void freeHashTable(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Recipe *current = table->buckets[i];
        while (current != NULL) {
            Recipe *toDelete = current;
            current = current->next;
            free(toDelete);
        }
    }
    free(table);
}

int main() {
    HashTable *cookbook = createHashTable();

    insertRecipe(cookbook, "Pancakes", "Flour, Eggs, Milk", "Mix and cook");
    insertRecipe(cookbook, "Omelette", "Eggs, Salt", "Beat eggs, cook in pan");

    Recipe *r = searchRecipe(cookbook, "Pancakes");
    if (r != NULL) {
        printf("Found recipe for %s: %s\n", r->name, r->ingredients);
    }

    deleteRecipe(cookbook, "Omelette");

    printHashTable(cookbook);
    freeHashTable(cookbook);

    return 0;
}
