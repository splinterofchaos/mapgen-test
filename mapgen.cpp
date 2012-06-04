
#include "random.h"
#include "Vector.h"
#include "Grid.h"

#include <vector>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>


typedef Grid<char> MapGen;
MapGen mgMap;

struct Room : public Region
{
    Room( size_t, size_t, size_t, size_t );
    Room() {}
};

Room::Room( size_t l, size_t r, size_t u, size_t d )
    : Region(l,r,u,d)
{
}

void random_range( size_t& min, size_t& max )
{
    const size_t MIN_LENGTH = 4;

    min = random( min, max - MIN_LENGTH );
    max = random( min + MIN_LENGTH, max );
}

Room random_room()
{
    Room r = { 1, mgMap.width-1, 1, mgMap.height-2 };
    random_range( r.left, r.right );
    random_range( r.up, r.down );
    return r;
}

void dig_room( const Region& r )
{
    std::fill( mgMap.reg_begin(r), mgMap.reg_end(r), '.' );
}

typedef std::vector< Room > RoomV;
RoomV rooms;

bool inc_arg( int& argc, char**& argv )
{
    argv++;
    return --argc > 0;
}

bool get_arg( const char* const arg, int& argc, char**& argv )
{
    return argc > 0 and strcmp(arg, *argv) == 0 and inc_arg(argc,argv);
}

Vector<int,2> random_point( const Region& r )
{
    return vector( random(r.left, r.right), random(r.up, r.down) );
}

template< typename T >
void minmax( T& a, T& b )
{
    if( a > b )
        std::swap( a, b );
}

template< typename T >
T towards( T x, const T& target )
{
    return x < target ? ++x : x > target ? --x : x;
}

void dig_hallway( Vector<int,2> a, Vector<int,2> b )
{
    // Dig from (ax,ay) to (bx,ay) to (bx,by).
    auto row = mgMap.row_begin( a.y() );
    for( int i = a.x(); i != b.x(); i = towards(i,b.x()) )
        *(row + i) = '.';

    auto col = mgMap.col_begin( b.x() );
    for( int i = b.y(); i != a.y(); i = towards(i,a.y()) )
        *(col + i) = '.';

    // Both loops end before (bx,ay) gets dug; fix that.
    mgMap.get( b.x(), a.y() ) = '.';
}

void splatter_pattern( int n )
{
    std::vector< Region > rooms;
    while( n-- )
        rooms.push_back( random_room() );

    for( size_t i=0; i < rooms.size(); i++ ) {
        dig_room( rooms[i] );
        if( rooms.size()-i > 1 )
            dig_hallway ( 
                random_point( rooms[i] ), 
                random_point( rooms[random(i+1,rooms.size()-1)] ) 
            );
    }
}

int main( int argc, char** argv )
{
    struct {
        unsigned int rooms = 3;
        unsigned int width=80, height=60;
    } opts;

    while( inc_arg(argc,argv) )
    {
        if( get_arg("-d", argc, argv) ) {
            if( (*argv)[strlen(*argv)-1] != ')'
                or sscanf(*argv, "(%u,%u)", 
                           &opts.width, &opts.height) != 2 ) {
                fprintf( stderr, "Option -d requires arguments in "
                                 "form (width,height).\n" );
                exit( 1 );
            }
        } else if( get_arg("-n", argc, argv) ) {
            opts.rooms = atoi( *argv );
        }

    }

    mgMap.reset( opts.width, opts.height, '#' );

    splatter_pattern( opts.rooms );

    for( size_t y=0; y < mgMap.height; y++ ) {
        std::copy( mgMap.row_begin(y), mgMap.row_end(y), 
                   std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;
    }
    
    std::cout << std::endl;
}
