
#include "bsp.h"

#include <stdlib.h> // Malloc and free.

int leaf( Bsp* node )
{
    return !node->one || !node->two;
}

// Initialize a node as a leaf.
void _bsp_leaf_init( Bsp* node )
{
    // Nullify the children, just in case.
    delete_bsp_node( node->one );
    delete_bsp_node( node->two );
    node->one = node->two = 0;

    node->area = random_room_in_room( node->area );
}

int _split_pos( int min, int max )
{
    int range = max - min;
    if( range > 4 ) {
        min += range / 4;
        max -= range / 4;
    }
    return randr( min, max );
}

// Split r in two, horizontally.
// (Used by new_bsp_node.)
void _hsplit( Room r, Room* r1, Room* r2 )
{
    int split = _split_pos( r.up+NODELEN, r.down-NODELEN );
    r1->left  = r2->left  = r.left;
    r1->right = r2->right = r.right;
    r1->up   = r.up;   r1->down = split - 1;
    r2->down = r.down; r2->up   = split + 1;
}

// Vertical.
void _vsplit( Room r, Room* r1, Room* r2 )
{
    int split = _split_pos( r.left+NODELEN, r.right-NODELEN );
    r1->up   = r2->up   = r.up;
    r1->down = r2->down = r.down;
    r1->left  = r.left;  r1->right = split - 1;
    r2->right = r.right; r2->left  = split + 1; 
}

Bsp* new_bsp_node( Room area, int depth )
{
    int width  = area.right - area.left + 1;
    int height = area.down  - area.up   + 1;

    Bsp* node = malloc( sizeof(Bsp) );
    node->area = area;
    node->one = node->two = 0;

    if( depth <= 1 ) { 
        _bsp_leaf_init( node );
        return node;
    }

    enum { VERT, HOR };
    int splitOrientation = randr( VERT, HOR );
    
    const float MAXRATIO = 1.2f;

    int notWideEnough = width  < NODELEN * 2;
    int notTallEnough = height < NODELEN * 2;
    int tooWide = width  > height * MAXRATIO;
    int tooTall = height > width  * MAXRATIO;

    if( notWideEnough && notTallEnough ) {
        _bsp_leaf_init( node );
        return node;
    } else if( notWideEnough || tooTall ) {
        splitOrientation = HOR;
    } else if( notTallEnough || tooWide ) {
        splitOrientation = VERT;
    }

    Room r1, r2;
    if( splitOrientation ) _hsplit( area, &r1, &r2 );
    else                   _vsplit( area, &r1, &r2 );
    node->one = new_bsp_node( r1, depth-1 );
    node->two = new_bsp_node( r2, depth-1 );

    // node may not have spawned children for a number of reasons.
    // Just make sure it leaves in a valid state.
    if( leaf(node) )
        _bsp_leaf_init( node );

    return node;
}

void delete_bsp_node( Bsp* node )
{
    if( node ) {
        delete_bsp_node( node->one );
        delete_bsp_node( node->two );
        free( node );
    }
}

// Dig and connect each room.
// Returns a position within the hallway between the two rooms.
Vector bsp_dig( Grid g, Bsp* node )
{
    if( leaf(node) ) {
        dig_room( g, node->area );
        return random_point( node->area );
    } else {
        Bsp *one = node->one, *two = node->two;
        Vector vone = bsp_dig( g, one );
        Vector vtwo = bsp_dig( g, two );
        dig_hallway( g, vone, vtwo );
        
        // We need a point within the hallway to return. 
        // dig_hallway guarantees (x1,y1) -> (x2,y1) -> (x2,y2).
        // First choose (x1,y1)->(x2,y1) or (x2,y1)->(x2,y2).
        Vector v;
        if( randr(0,1) )
            v = vector( vtwo.x, randr(vone.y, vtwo.y) );
        else
            v = vector( randr(vone.x, vtwo.x), vone.y );
        return v;
    }
}
