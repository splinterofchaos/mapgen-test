
s" objects.fs" included 
s" random.fs"  included
     
\ Not your C's ++.
: ++ ( addr n-cells -- addr+n ) cells + ;

\ \ MAP -- The thing we're trying to gen. \ \
\ Internally stores a pointer to a structure: { width, height, "..." }
VARIABLE (*map*) : *map* (*map*) @ ;

char # CONSTANT WALL
char . CONSTANT FLOOR

: *height* *map* 0 ++ @ ; : *width*   *map* 1 ++ @ ;
: *width*! *map* 0 ++ ! ; : *height*! *map* 1 ++ ! ;

: *area* *width* *height* * ;

: *tiles* *map* 2 ++ ;
: *end*   *tiles* *area* ++ ;

\ Helper used by (emit) and (dig) below.
: ij>tile *width* * + *tiles* + ;

\ Constructs a map.
: <map> ( w h --)
    HERE (*map*) !
    , , \ *map*'s accessors can now be called.
    *area* chars allocate
    *tiles* *area* WALL fill ;

: (.map) ij>tile C@ emit ;
: .map *height* 0  DO cr *width* 0 DO I J (.map) LOOP LOOP ;

\ \ Room -- Represents an area in the map.
: <room> ( left right up down -- addr ) HERE >r , , , , r> ;
: sizeof-<room> 4 cells ;

object class 
    cell% field left cell% field right cell% field up cell% field down
    m: ( left right up down -- )
        this down ! this up ! this right ! this left !
    ;m overrides construct
end-class Room
: <room> room heap-new ;
: <0room> 0 0 0 0 <room> ;
: >room< free ;

: room! { r1 r2 -- r2=r1 }
    r1 right @ r2 right !  r1 left @ r2 left !
    r1   up  @ r2  up   !  r1 down @ r2 down ! ;

: (dig) ij>tile FLOOR swap C! ;
: dig { room }
    room down  @ 1+ room  up  @ DO
    room right @ 1+ room left @ DO
        I J (dig)
    LOOP LOOP ;

\ Seed rnd.
: sqr dup * ;
: cube dup sqr * ;
time&date + + + * sqr * cube seed . . 

: rand ( n -- 0<=x<=n) rnd swap mod ;
: randr ( min max -- x ) over - rand + ;

rnd rnd rnd rnd . . . . \ Shake up the generator a little.

4 CONSTANT MINLEN 
: rand-range ( min max -- min' max')
    over MINLEN + 1+ swap randr ( -- min max')
    swap over MINLEN - randr ( -- max' min')
    swap ;
: <random-room> ( -- room) 
    1 *width*  2 - rand-range
    1 *height* 2 - rand-range <room> ;

: splatter-pattern { rrooms n -- } 
    n 1- 0 DO 
        <random-room> rrooms I ++ <0room> over ! room!
    LOOP ;
    
: .room { r } ." < " r left ? r right ? ." , " r up ? r down ? ." >" ;

create rrooms 5 cells ALLOT 
    
\ 50 50 <map> 
\ 1 2 1 2 <room> dig 
\ rrooms splatter-pattern
\ rrooms 1 ++ @ .room
\ .map cr
cr

' disasm is discode    
