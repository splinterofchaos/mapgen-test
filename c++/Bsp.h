
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

    bool leaf() const; 

  private:
    void _init( int depth );
};

void dig( const Bsp& );
