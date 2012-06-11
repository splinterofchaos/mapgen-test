! Copyright (C) 2012 Your name.
! See http://factorcode.org/license.txt for BSD license.
USING: 
    kernel io strings namespaces shuffle command-line fry 
    memory system
    grouping sequences combinators make 
    math math.ranges math.order
    generalizations 
    random ;
IN: mapgen

SYMBOL: *map-width*  80 *map-width*  set-global
SYMBOL: *map-height* 50 *map-height* set-global

: *width*   ( -- n ) *map-width*  get ;
: *height*  ( -- n ) *map-height* get ;
: *width*!  ( n -- ) *map-width*  set ;
: *height*! ( n -- ) *map-height* set ;

: *area* ( -- n ) *width* *height* * ;

CONSTANT: WALL  CHAR: #
CONSTANT: FLOOR CHAR: .

CONSTANT: MINLEN 4

SYMBOL: *map* ! A string representing a map. 

: map-rows   ( -- *map*>group  ) *map* get *width* group ;
: map>string ( --      >string ) map-rows "\n" join ;
: .map       ( --              ) map>string print ;

: i'-map ( i j -- i' map   ) *width* * + *map* get ;
: map[]  ( i j -- map[i,j] ) i'-map nth ;
: map[]! ( elt i j --      ) i'-map set-nth ;

: left  ( room -- n ) [ 0 ] dip nth ;
: right ( room -- n ) [ 1 ] dip nth ;
: up    ( room -- n ) [ 2 ] dip nth ;
: down  ( room -- n ) [ 3 ] dip nth ;

: left!  ( elt room -- ) [ 0 ] dip set-nth ;
: right! ( elt room -- ) [ 1 ] dip set-nth ;
: up!    ( elt room -- ) [ 2 ] dip set-nth ;
: down!  ( elt room -- ) [ 3 ] dip set-nth ;

: left-right ( room -- l r ) [ left ] [ right ] bi ;
: up-down    ( room -- u d ) [  up  ] [ down  ] bi ;
: width  ( room -- w ) left-right swap - ;
: height ( room -- h )  up-down   swap - ;

: area ( room -- n ) 
[ width  1 + ] [ height 1 + ] bi * ;

: (room-map) ( room quot: ( i j -- ) -- )
! Set up the range in back.
[ [ up-down [a,b] ] keep ] dip 
'[  ! Outer loop ( j -- )
    ! Capture the room and put the range behind j.
    _ left-right [a,b] swap
    ! Capture the quot, curry, and loop.
    _ curry each ] 
each ; inline recursive

: (dig) ( i  j -- ) [ FLOOR ] 2dip map[]! ;
: dig   ( room -- ) [ (dig) ] (room-map)  ;

: randr ( min max -- n ) dupd swap - 1 + random + ;

: randrange ( min max minlen -- n n ) 
-rot [ 2dup + ] dip ! mlen min min+mlen max
randr dup           ! mlen min max' max'
[ rot - randr ] dip ;

: randpoint ( room -- x y )
[ left-right randr ] [ up-down randr ] bi ;

: random-room ( -- room )
1 *width*  2 - MINLEN randrange
1 *height* 2 - MINLEN randrange
{ } 4sequence ;

: (ord-range) ( x1 x2 -- range ) [ min ] [ max ] 2bi [a,b] ;

: (dig-hallway) ( x0 y x1 quot: ( x y -- ) -- )
! dig-hallway's helper. Digs a line on one axis.
! quot should either be (dig) or swap (dig).
swapd [ (ord-range) ] 2dip curry each ; inline recursive

: dig-hallway ( x y u v -- )  
3dup [ swap (dig) ] (dig-hallway)
drop [   (dig)    ] (dig-hallway) ;

: (random-rooms) ( n -- seq )
[ random-room ] replicate ; inline recursive

: (connect-rooms) ( i room -- )
! Connect room_i with room_j where i < j.
[ nth randpoint ]
[ [ 1 + ] dip [ length 1 - randr ] keep nth randpoint ]
2bi dig-hallway ;

: scatter-pattern ( n -- )
[ [0,b) ] keep
(random-rooms) '[ _
    ! Dig and connect room.
    [ nth dig ] 
    [ [ length 1 - < ] 2keep 
      '[ _ _ (connect-rooms) ] 
      when ]
    2bi 
] each ;

SYMBOL: nrooms 5 nrooms set
*area* WALL <string> *map* set-global

: main ( -- ) 
    ! command-line get parse-command-line
    ! "n" get dup '[ _ nrooms set ] when
    ! nrooms get scatter-pattern .map ;
    5 scatter-pattern .map ;

MAIN: main 
