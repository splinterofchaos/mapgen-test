
#pragma once

#include "Grid.h"
#include "random.h"

Room random_room();
Room random_room( Room );
Vector<int,2> random_point( const Room& );

void dig_room( const Room& );
void dig_hallway( Vector<int,2>, Vector<int,2> );

typedef Grid<char> MapGen;
extern MapGen mgMap;

