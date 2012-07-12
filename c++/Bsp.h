
#pragma once

#include "mapgen.h"

#include <memory>

struct Bsp
{
    static const int NODELEN;

    Room area;
    std::unique_ptr<Bsp> one, two; 

    Bsp( int depth );
    Bsp( Room, int depth );
    Bsp( Bsp&& );

    bool leaf() const; 

  private:
    void _init( int depth );
};

void dig( const Bsp& );
Vector<int,2> random_point( const Bsp& );
