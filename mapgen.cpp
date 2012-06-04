
#include "random.h"
#include "Vector.h"
#include "Grid.h"

#include <vector>
#include <iostream>
#include <cstdio>
#include <cstring>


typedef Grid<char> MapGen;
MapGen mgMap;

struct Room : public Region
{
    Room* connections[3] = {nullptr,nullptr,nullptr};

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
    for( auto it = mgMap.reg_begin(r); 
         it != mgMap.reg_end(r); it++ )
        *it = '.';
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

    while( opts.rooms-- )
        dig_room( random_room() );

    for( size_t y=0; y < mgMap.height; y++ ) {
        std::copy( mgMap.row_begin(y), mgMap.row_end(y), 
                   std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;
    }
    
    std::cout << std::endl;
}
