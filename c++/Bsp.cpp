
#include "Bsp.h"

const int Bsp::NODELEN = 9;

void _init_leaf( Bsp& node )
{
    node.one.release();
    node.two.release();
    node.area = random_room( node.area );
}

void Bsp::_init( int depth )
{
    int width  = area.right - area.left + 1;
    int height = area.down  - area.up   + 1;

    if( depth <= 1 ) {
        _init_leaf( *this );
        return;
    }

    enum{ VERT, HOR };
    int splitOrientation = random( VERT, HOR );

    const float MAXRATIO = 1.2f;

    bool notWideEnough = width  < NODELEN * 2;
    bool notTallEnough = height < NODELEN * 2;
    bool tooWide = width  > height * MAXRATIO;
    bool tooTall = height > width  * MAXRATIO;

    if( notWideEnough and notTallEnough ) {
        _init_leaf( *this );
        return;
    } else if( notWideEnough or tooTall ) {
        splitOrientation = HOR;
    } else if( notTallEnough or tooWide ) {
        splitOrientation = VERT;
    }

    std::pair<Room,Room> childArea;
    childArea = splitOrientation == HOR ? 
        hsplit( area, NODELEN ) : vsplit( area, NODELEN );

    one.reset( new Bsp(childArea.first,  depth-1) );
    two.reset( new Bsp(childArea.second, depth-1) );

    // Just in case.
    if( leaf() )
        _init_leaf( *this );
}

Bsp::Bsp( int depth )
    : area (
        1, mgMap.width  - 2, 
        1, mgMap.height - 2
    )
{
    _init( depth );
}

Bsp::Bsp( Room a, int depth )
    : area( a )
{
    _init( depth );
}

bool Bsp::leaf() const
{
    return not (one and two);
}

Vec _dig( const Bsp& node )
{
    if( node.leaf() ) {
        dig_room( node.area );
        return random_point( node.area );
    }

    Vec vone = _dig( *node.one ),
        vtwo = _dig( *node.two );
    dig_hallway( vone, vtwo );

    // We need a point within the hallway to return. 
    // dig_hallway guarantees (x1,y1) -> (x2,y1) -> (x2,y2).
    // First choose (x1,y1)->(x2,y1) or (x2,y1)->(x2,y2).
    Vec v;
    if( random(0,1) )
        v = vector( vtwo.x(), random(vone.y(), vtwo.y()) );
    else
        v = vector( random(vone.x(), vtwo.x()), vone.y() );
    return v;
}

void dig( const Bsp& node )
{
    _dig( node );
}
