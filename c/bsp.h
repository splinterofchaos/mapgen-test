
#include "mapgen.h"

struct Bsp_t { 
    Room area; 
    struct Bsp_t *one, *two; 
};

typedef struct Bsp_t Bsp;

// Returns 1 if node has no children.
int leaf( Bsp* );

Bsp* new_bsp_node( Room, int depth );
void delete_bsp_node( Bsp* );

Vector bsp_dig( Grid, Bsp* );
