
#include "mapgen.h"
#include "random.h"
#include "Vector.h"
#include "Grid.h"
#include "Bsp.h"

#include <vector>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>

MapGen mgMap;

void random_range( size_t& min, size_t& max )
{
    min = random( min, max - Room::MINLEN );
    max = random( min + Room::MINLEN, max );
}

Room random_room( Room r )
{
    random_range( r.left, r.right );
    random_range( r.up, r.down );
    return r;
}

Room random_room()
{
    Room r = { 1, mgMap.width-2, 1, mgMap.height-2 };
    return random_room( r );
}

Vector<int,2> random_point( const Room& r )
{
    return vector( random(r.left, r.right), random(r.up, r.down) );
}

void dig_room( const Room& r )
{
    std::fill( mgMap.reg_begin(r), mgMap.reg_end(r), '.' );
}

template< typename T >
T towards( T x, const T& target )
{
    return x < target ? ++x 
         : x > target ? --x 
         :                x;
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

typedef std::vector< Room > RoomV;
RoomV rooms;

bool inc_arg( int& argc, char**& argv )
{
    argv++;
    return --argc > 0;
}

// Get argument-taking option arg and return it in argv;
bool get_arg( const char* const arg, int& argc, char**& argv )
{
    return argc > 0 and strcmp(arg, *argv) == 0 and inc_arg(argc,argv);
}

template< typename T >
void minmax( T& a, T& b )
{
    if( a > b )
        std::swap( a, b );
}

std::vector<Room> splatter_pattern( int n )
{
    std::vector< Room > rooms(n);
    typedef Room (*RoomGenType) ();
    std::generate( rooms.begin(), rooms.end(), (RoomGenType)random_room );

    for( size_t i=0; i < rooms.size(); i++ ) {
        dig_room( rooms[i] );
        if( rooms.size()-i > 1 )
            dig_hallway ( 
                random_point( rooms[i] ), 
                random_point( rooms[random(i+1,rooms.size()-1)] ) 
            );
    }

    return rooms;
}

#include "Bsp.h"
Bsp bsp_pattern( int depth )
{
    Bsp bsp( depth );
    dig( bsp );
    return bsp;
}

int main( int argc, char** argv )
{
    enum Pattern_t{ SPLATTER, BSP };
    struct {
        Pattern_t pattern;
        unsigned int rooms;
        unsigned int width, height;
        unsigned int spawnPoints;
    } opts = { BSP, 3, 80, 60, 5 };

    while( inc_arg(argc,argv) )
    {
        if( get_arg("-w", argc, argv) ) {
            opts.width = atoi( *argv );
        } else if( get_arg("-h", argc, argv) ) {
            opts.height = atoi( *argv );
        } else if( get_arg("-n", argc, argv) ) {
            opts.rooms = atoi( *argv );
        } else if( get_arg("-X", argc, argv) ) {
            opts.spawnPoints = atoi( *argv );
        } else if( get_arg("--pattern", argc, argv) ) {
            if( strcmp(*argv, "bsp") == 0 )
                opts.pattern = BSP;
            else if( strcmp(*argv, "splatter") == 0 ) 
                opts.pattern = SPLATTER;
            else
                fprintf( stderr, "--splatter: Unknown option '%s'\n", *argv );
        }
    }

    mgMap.reset( opts.width, opts.height, '#' );

    std::vector< Vector<int,2> > spawnPoints;
    Vector<int,2> spawnPoint( 0, 0 );

    switch( opts.pattern )
    {
      case SPLATTER: 
        {
            std::vector<Room> rooms = splatter_pattern( opts.rooms ); 
            while( opts.spawnPoints-- )
                spawnPoints.push_back ( 
                    random_point( rooms[ random(0, rooms.size()-1) ] )
                );
        }
      break;
      case BSP:      
        Bsp bsp = bsp_pattern( opts.rooms );
        spawnPoint = random_point( bsp );
        while( opts.spawnPoints-- )
            spawnPoints.push_back( random_point(bsp) );
      break;
    }

    for( size_t y=0; y < mgMap.height; y++ ) {
        std::copy( mgMap.row_begin(y), mgMap.row_end(y), 
                   std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;
    }

    for( auto pt : spawnPoints )
        std::cout << "X " << pt.x() << ' ' << pt.y() << std::endl;
}
