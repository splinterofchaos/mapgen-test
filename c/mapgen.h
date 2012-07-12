
#pragma once

typedef struct{ int x, y; }Vector;
Vector vector( int x, int y ); // Defined in mapgen.c.

typedef struct
{
    Vector dimensions;
    char* tiles;
} Grid;

// Used in various places as the minimum dimension of a room.
extern const int MINLEN; 

// A BSP node should have at least this much space.
// It must provide enough space for a random room.
extern const int NODELEN;

typedef struct { int left, right, up, down; }Room;

int randr( int, int );
// Create a room within the bounds of the map.
Room random_room( Vector dims );
// Create a room within the bounds of another room.
Room random_room_in_room( Room r );
// Create a point within a room.
Vector random_point( Room r );

void dig_room( Grid, const Room r );
// Draw a hallway between two points.
void dig_hallway( Grid, Vector, Vector );

