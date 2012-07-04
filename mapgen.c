
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef struct{ int x, y; } Vector;
Vector vector( int x, int y )
{
    Vector v = { x, y };
    return v;
}

typedef struct
{
    Vector dimensions;
    char* tiles;
} Grid;

// Used in various places as the minimum dimension of a room.
const int MINLEN = 5; 

// A BSP node should have at least this much space.
int NODELEN = 13;

void init_grid( Grid* g, Vector dims )
{
    const int AREA = dims.x * dims.y;
    
    g->dimensions = dims;
    g->tiles = malloc( AREA );
    memset( g->tiles, '#', AREA );
}

void destroy_grid( Grid* g )
{
    free( g->tiles );
    g->tiles = 0;
}

char* grid_get( Grid g, Vector p )
{
    return g.tiles + g.dimensions.x*p.y + p.x;
}


void print_grid( Grid g )
{
    int y;
    for( y = 0; y < g.dimensions.y; y++ ) {
        Vector v = { 0, y };
        write( 0, grid_get(g,v), g.dimensions.x );
        write( 0, "\n", 1 );
    }
}

typedef struct {
    int left, right, up, down;
} Room;

int randr( int min, int max )
{
    if( min > max ) {
        int tmp = max;
        max = min;
        min = tmp;
    }

    return rand() % (max-min+1) + min;
}

Room random_room( Vector dims )
{
    Room r;
    r.left  = randr( 1, dims.x-MINLEN-1 );
    r.up    = randr( 1, dims.y-MINLEN-1 );
    r.right = randr( r.left+MINLEN, dims.x-1 );
    r.down  = randr( r.up+MINLEN, dims.y-1 );
    
    return r;
}

Room random_room_in_room( Room r )
{
    r.left  = randr( r.left, r.right-MINLEN );
    r.right = randr( r.left+MINLEN, r.right );
    r.up    = randr( r.up, r.down-MINLEN );
    r.down  = randr( r.up+MINLEN, r.down );
    return r;
}

Vector random_point( Room r )
{
    Vector v = { randr(r.left,r.right), randr(r.up,r.down) };
    return v;
}

void dig_room( Grid g, const Room r )
{
    int x, y;
    for( x = r.left; x < r.right+1; x++ ) {
        for( y = r.up; y < r.down+1; y++ ) {
            Vector v = { x, y };
            *grid_get( g, v ) = '.';
        }
    }
}

int towards( int x, int target )
{
    return x < target ? x+1 : x > target ? x-1 : x;
}

void dig_hallway( Grid g, Vector a, Vector b )
{
    // Dig from (ax,ay) to (bx,ay) to (bx,by).
    int x;
    for( x = a.x; x != b.x; x = towards(x,b.x) ) {
        Vector p = { x, a.y };
        *grid_get( g, p ) = '.';
    }
    
    int y;
    for( y = b.y; y != a.y; y = towards(y,a.y) ) {
        Vector p = { b.x, y };
        *grid_get( g, p ) = '.';
    }

    // Both loops stop just before (bx,ay); fix that.
    Vector p = { b.x, a.y };
    *grid_get( g, p ) = '.';
}

void splatter_pattern( Grid g, int n )
{
    Room* rooms = malloc( n * sizeof(Room) );
    
    const int N = n;
    for( n = 0; n < N; n++ )
        rooms[n] = random_room( g.dimensions );
    
    for( n = 0; n < N; n++ ) {
        dig_room( g, rooms[n] );
        if( N-n > 1 )
            dig_hallway( g,
                random_point( rooms[n] ),
                random_point( rooms[randr(n+1, N-1)] )
            );
    }
    
    free( rooms );
}

struct BspNode_t { 
    Room area; 
    struct BspNode_t *one, *two; 
};

typedef struct BspNode_t BspNode;

int leaf( BspNode* node )
{
    return !node->one || !node->two;
}
int too_small( BspNode* node )
{
    Room r = node->area;
    return r.right - r.left < NODELEN 
        || r.down  - r.up   < NODELEN;
}

void delete_bsp_node( BspNode* node );

void bsp_leaf_init( BspNode* node )
{
    // Delete will do nothing on an already-nulled node.
    // This just ensures they ARE nulled.
    delete_bsp_node( node->one );
    delete_bsp_node( node->two );
    node->one = node->two = 0;
    node->area = random_room_in_room( node->area );
}

// Split r in two, horizontally.
// (Used by new_bsp_node.)
void hsplit( Room r, Room* r1, Room* r2 )
{
    int split = randr( r.up+NODELEN, r.down-NODELEN );
    r1->left  = r2->left  = r.left;
    r1->right = r2->right = r.right;
    r1->up   = r.up;   r1->down = split - 1;
    r2->down = r.down; r2->up   = split + 1;
}

// Vertical.
void vsplit( Room r, Room* r1, Room* r2 )
{
    int split = randr( r.left+NODELEN, r.right-NODELEN );
    r1->up   = r2->up   = r.up;
    r1->down = r2->down = r.down;
    r1->left  = r.left;  r1->right = split - 1;
    r2->right = r.right; r2->left  = split + 1; 
}

BspNode* new_bsp_node( Room area )
{
    // A small node can't contain a room. 
    if( area.right - area.left < NODELEN
        || area.down - area.up < NODELEN )
        // This node's parent will become a leaf.
        return 0;

    BspNode* node = malloc( sizeof(BspNode) );
    node->area = area;
    
    // Nodes that can't be split become leaves.
    if( area.right - area.left < NODELEN+3
        || area.down - area.up < NODELEN+3 ) {
        bsp_leaf_init( node );
        return node;
    }

    // Split this node.
    Room r1, r2;
    if( area.right-area.left > area.down-area.up )
         vsplit( area, &r1, &r2 );
    else hsplit( area, &r1, &r2 );
    node->one = new_bsp_node( r1 );
    node->two = new_bsp_node( r2 );

    // Since the recursive calls may return zero, 
    // we may need to return a leaf.
    if( leaf(node) )
        bsp_leaf_init( node );

    return node;
}

void delete_bsp_node( BspNode* node )
{
    if( node ) {
        delete_bsp_node( node->one );
        delete_bsp_node( node->two );
        free( node );
    }
}

// Dig and connect each room.
// Returns a position within the hallway between the two rooms.
Vector bsp_dig( Grid g, BspNode* node )
{
    if( leaf(node) ) {
        dig_room( g, node->area );
        return random_point( node->area );
    } else {
        BspNode *one = node->one, *two = node->two;
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

void bsp_pattern( Grid g )
{
    Room bounds = { 1, g.dimensions.x-2, 1, g.dimensions.y-2 };
    BspNode* bsp = new_bsp_node( bounds );
    bsp_dig( g, bsp );
    delete_bsp_node( bsp );
}

int inc_arg( int* argc, char*** argv )
{
    (*argv)++;
    return --(*argc) > 0;
}

int get_arg( const char* const arg, int* argc, char*** argv )
{
    return *argc > 0 && strcmp(arg, **argv) == 0 && inc_arg(argc,argv);
}

int main( int argc, char** argv )
{
    struct {
        size_t rooms;
        Vector dimensions;
    } opts = {3,{80,60}};

    while( inc_arg(&argc,&argv) )
    {
        if( get_arg("-d", &argc, &argv) ) {
            if( (*argv)[strlen(*argv)-1] == ')'
                && sscanf(*argv, "(%u,%u)", 
                           &opts.dimensions.x, &opts.dimensions.y) != 2 ) {
                fprintf( stderr, "Option -d requires arguments in "
                                 "form (width,height).\n" );
                exit( 1 );
            }
        } else if( get_arg("-n", &argc, &argv) ) {
            opts.rooms = atoi( *argv );
        }

    }
    
    srand( time(0) );
    
    Grid map;
    init_grid( &map, opts.dimensions );
     
    //splatter_pattern( map, opts.rooms );
    bsp_pattern( map );
    
    print_grid( map );
    
    destroy_grid( &map );
    
    return 0;
}
