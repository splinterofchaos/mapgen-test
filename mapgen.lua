
FLOOR = string.byte('.')
WALL  = string.byte('#')

function Dungeon( w, h )
   local d = {width=w, height=h, grid={}}

   for j = 1, h do
      d.grid[j] = {}
      for i = 1, w do
         d.grid[j][i] = WALL
   end end
   
   return setmetatable( d, {
       __index = function( d, j )
                    return d.grid[j]
                 end,
       
       __tostring = function( self )
                       local s = ''
                       for _,row in pairs(self.grid) do
                           for _,c in pairs(row) do
                               s = s .. string.char(c)
                           end

                           s = s .. '\n'
                       end
                       return s
                    end,
    })
end

-- Delay the initialization of the dungeon as we don't know its dimensions.
dungeon = {} 

function random_range( min, max, minlen )
    min = math.random( min, max - minlen )
    max = math.random( min + minlen, max )
    return min, max
end

function random_room()
    local MINLEN = 4
    l, r = random_range( 2, dungeon.width-1,  MINLEN )
    u, d = random_range( 2, dungeon.height-1, MINLEN )
    return {left=l, right=r, up=u, down=d}
end

function dig( room )
    for j = room.up, room.down do for i = room.left, room.right do
        dungeon[j][i] = FLOOR
    end end 
end

function minmax(x,y) return math.min(x,y), math.max(x,y); end

function dig_hallway( a, b )
    -- Dig from (ax,ay) to (bx,ay) to (bx,by)
    for x = math.min(a.x,b.x), math.max(a.x,b.x) do dungeon[a.y][x] = FLOOR; end
    for y = math.min(a.y,b.y), math.max(a.y,b.y) do dungeon[y][b.x] = FLOOR; end
end

function random_point( room )
    return { x = math.random(room.left,room.right),
             y = math.random(room.up,room.down) }
end

function splatter( n )
    local rooms = {}
    for i = 1, n do rooms[i] = random_room(); end

    for i = 1, n do
        dig( rooms[i] )
        if i < n then
            dig_hallway (
                random_point( rooms[i] ),
                random_point( rooms[math.random(i+1,n)] )
            )
        end 
    end  
end

math.randomseed( os.time() )
dungeon = Dungeon( 20, 20 )
splatter( 5 )
print( dungeon ) 

