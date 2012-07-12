
#include "mapgen.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h> // For write.

const int MINLEN = 6; 
const int NODELEN = 9;

Vector vector( int x, int y )
{
    Vector v = { x, y };
    return v;
}

Grid new_grid( Vector dims )
{
    Grid g = { dims, 0 };

    const size_t AREA = (size_t) (dims.x * dims.y);
    g.tiles = malloc( AREA );
    memset( g.tiles, (int)'#', AREA );
    return g;
}

void destroy_grid( Grid* g )
{
    if( g ) {
        if( g->tiles )
            free( g->tiles );
        g->tiles = 0;
    }
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

#include "bsp.h"
void bsp_pattern( Grid g, int depth )
{
    Room bounds = { 1, g.dimensions.x-2, 1, g.dimensions.y-2 };
    Bsp* bsp = new_bsp_node( bounds, depth );
    bsp_dig( g, bsp );
    delete_bsp_node( bsp );
}

int inc_arg( int* argc, char*** argv )
{
    (*argv)++;
    return --(*argc) > 0;
}

// If argv equals the argument-taking option arg, returns true and increments
// argv to point to the argument.
int get_arg( const char* const arg, int* argc, char*** argv )
{
    return *argc > 0 && strcmp(arg, **argv) == 0 && inc_arg(argc,argv);
}

int main( int argc, char** argv )
{
    enum Pattern{ SPLATTER, BSP };
    struct {
        enum Pattern pattern;

        union {
            int rooms; // Number of rooms (SPLATTER).
            int depth; // Depth of tree (BSP).
        };

        Vector dimensions;
    } opts = {BSP,{4},{80,60}};

    while( inc_arg(&argc,&argv) )
    {
        if( get_arg("-w", &argc, &argv) ) {
            opts.dimensions.x = atoi( *argv );
        } else if( get_arg("-h", &argc, &argv) ) {
            opts.dimensions.y = atoi( *argv );
        } else if( get_arg("-n", &argc, &argv) ) {
            opts.rooms = atoi( *argv );
        } else if( get_arg("--pattern", &argc, &argv) ) {
            if( strcmp(*argv, "splatter") == 0 )
                opts.pattern = SPLATTER;
            else if( strcmp(*argv, "bsp") == 0 )
                opts.pattern = BSP;
            else
                fprintf( stderr, "Unknown pattern: '%s'\n", *argv );
        }
    }
    
    srand( time(0) );
    
    Grid map = new_grid( opts.dimensions );
     
    switch( opts.pattern ) {
      case SPLATTER: splatter_pattern( map, opts.rooms ); break;
      case BSP:      bsp_pattern( map, opts.depth );      break;
    }
    
    print_grid( map );
    
    destroy_grid( &map );
    
    return 0;
}
