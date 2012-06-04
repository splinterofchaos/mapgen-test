
import System.Random

import System.Console.GetOpt
import System.Environment(getArgs, getProgName)

type Coord = (Int,Int)
type Range = (Int,Int)
type Area  = (Coord,Coord) -- Upper-left and lower-right bounds.

data Tile = TFloor | TWall
instance Show Tile where
  show TFloor = "."
  show TWall  = "#"

type MRow = [Tile]
type RMap = [MRow] 
  
show_map :: RMap -> String
show_map rmap = 
  unlines $ map (foldl accum "") rmap
  where accum line tile = line ++ show tile

make_map :: Coord -> RMap
make_map (x,y) = replicate y (replicate x TWall)

split_gap :: Int -> Int -> [a] -> ([a],[a],[a])
split_gap start size lst = (before, middle, after)
  where 
    (before,rest) = splitAt start lst
    (middle,after) = splitAt size rest

dig_row :: Range -> MRow -> MRow
dig_row (start,end) row = 
  before ++ replicate size TFloor ++ after
  where 
    size = end - start
    (before,_,after) = split_gap start size row

dig_room :: Area -> RMap -> RMap
dig_room ((x,y),(u,v)) rmap =
  ybefore ++ map (dig_row (x,u)) rows ++ yend
  where 
    (ybefore,rows,yend) = split_gap y (v-y) rmap

to_range a b max minLen = (a',b')
  where
    a' = a `mod` (max-minLen) + 1
    range = max - a' - minLen
    b' = (if range > 0 then b `mod` range else 0) 
         + a' + minLen - 1
random_room :: (RandomGen r) => r -> Coord -> (r,Area)
random_room gen (w,h) = 
  (g'''',((x',y'),(u',v'))) -- Note the reordering of xuyv to xyuv.
  where 
    minLen = 3
    (x,g')    = next gen
    (y,g'')   = next g'
    (u,g''')  = next g''
    (v,g'''') = next g'''
    ((x',u'),(y',v')) = (to_range x u w minLen, 
                         to_range y v h minLen)
    
data Options = Options {optRooms::Integer,optDimensions::Coord}

defaults :: Options
defaults = Options {optRooms=5,optDimensions=(80,60)}

options = 
  [Option ['n'] ["rooms"] 
      (ReqArg (\s op-> return op{optRooms=read s::Integer}) "ROOMS")
      "Number of rooms to dig.",
   Option ['d'] ["dimensions"]
      (ReqArg (\s op-> case reads s :: [(Coord,String)] of
                  ((dims,_):_) -> 
                    return op { optDimensions = dims }
                  otherwise -> 
                    error "Dimensions must be in format (width,height)") 
              "DIMENSIONS") 
      "Dimensions of map."]

random_rooms :: (Num a, RandomGen r) => a -> r -> (Coord) -> (r,[Area])
random_rooms n gen dims = random_rooms' n gen dims []

random_rooms' :: (Num a, RandomGen r) => 
                 a -> r -> (Coord) -> [Area] -> (r,[Area])
random_rooms' 0 gen dims rooms = (gen,rooms)
random_rooms' n gen dims rooms = 
  random_rooms' (n-1) gen' dims (r:rooms)
  where (gen',r) = random_room gen dims
        
dig_rooms :: [Area] -> RMap -> RMap 
dig_rooms [] m = m
dig_rooms (r:rooms) m = dig_rooms rooms (dig_room r m)

main = do
  argv <- getArgs
  let (actions,noops,msgs) = getOpt RequireOrder options argv
  ops <- foldl (>>=) (return defaults) actions
  let Options { optRooms=rooms, optDimensions=dimensions } = ops
  
  gen <- newStdGen
  let m = make_map dimensions
      (g',rrooms) = random_rooms rooms gen dimensions
      m' = dig_rooms rrooms m 
  putStrLn $ show_map m'
