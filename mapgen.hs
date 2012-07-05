
import System.Random

import System.Console.GetOpt
import System.Environment(getArgs, getProgName)
import Control.Monad.State

-- Thanks to monoidal for this trick.
-- https://gist.github.com/2877633
type Rand = State StdGen
rand m = do a <- state next
            return (a `mod` m)

type Coord = (Int,Int)
type Range = (Int,Int)
type Area  = (Coord,Coord) -- Upper-left and lower-right bounds.

data Tile = TFloor | TWall
instance Show Tile where
  show TFloor = "."
  show TWall  = "#"

type MRow = [Tile]
type RMap = [MRow] 
  
showMap :: RMap -> String
showMap = unlines . map (>>= show)

makeMap :: Coord -> RMap
makeMap (x,y) = replicate y (replicate x TWall)

splitGap :: Int -> Int -> [a] -> ([a],[a],[a])
splitGap start size lst = (before, middle, after)
  where 
    (before,rest) = splitAt start lst
    (middle,after) = splitAt (abs size) rest

digRow :: Range -> MRow -> MRow
digRow (start,end) row = 
  before ++ replicate size TFloor ++ after
  where
    size = end - start + 1
    (before,_,after) = splitGap start size row

digRoom :: RMap -> Area -> RMap
digRoom rmap ((x,y),(u,v)) =
  ybefore ++ map (digRow (x,u)) rows ++ yend
  where 
    (ybefore,rows,yend) = splitGap y (v-y+1) rmap

randomRoom :: Coord -> Rand Area
randomRoom (w,h) = do
  (x,u) <- gen_range w
  (y,v) <- gen_range h
  return ((x,y),(u,v))
  where 
    minlen = 3
    gen_range m = do
        a <- rand (m-minlen-1)
        let a' = a + 1
            brange = m - a' - minlen
        b <- rand (max 1 brange)
        let b' = b + a' + minlen - 1
        return (a',b')
    
randomPoint :: Area -> Rand Coord
randomPoint ((x,y),(u,v)) = 
  do x' <- rand (u-x+1)
     y' <- rand (v-y+1)
     return (x' + x, y' + y)

digHallway :: RMap -> Area -> RMap
digHallway m ((x,y),(u,v)) = 
    -- Dig from (x,y) to (u,y) to (u,v).
    foldl digRoom m 
        [((u,min y v),(u,max y v)),((min x u,y),(max x u,y))]

digRandomHallways :: RMap -> [Area] -> Rand RMap
digRandomHallways m rooms
  | length rooms < 2 = return m
  | otherwise = 
    do n <- rand (length $ tail rooms)
       end1 <- randomPoint (head rooms)
       end2 <- randomPoint (tail rooms!!n)
       let m' = digHallway m (end1, end2)
       digRandomHallways m' (tail rooms)
    
splatter :: Int -> RMap -> Rand RMap
-- Splatter n random rooms onto m.
splatter n m = 
  do rooms <- replicateM n $ randomRoom (length (head m),length m)
     digRandomHallways (foldl digRoom m rooms) rooms
  where 
    center ((x,y),(u,v)) = ((x+u) `quot` 2, (y+v) `quot` 2)
    
data Options = Options {optRooms::Int,optDimensions::Coord}

defaults :: Options
defaults = Options {optRooms=5,optDimensions=(80,60)}

options = 
  [Option "n" ["rooms"] 
      (ReqArg (\s op-> return op{optRooms=read s::Int}) "ROOMS")
      "Number of rooms to dig.",
   Option "d" ["dimensions"]
      (ReqArg (\s op-> case reads s :: [(Coord,String)] of
                  ((dims,_):_) -> 
                    return op { optDimensions = dims }
                  _ -> error "Dimensions must be in format (width,height)") 
              "DIMENSIONS") 
      "Dimensions of map."]

main = do
  -- Parse command line.
  argv <- getArgs
  let (actions,noops,msgs) = getOpt RequireOrder options argv
  ops <- foldl (>>=) (return defaults) actions
  let Options { optRooms=rooms, optDimensions=dimensions } = ops
  
  gen <- newStdGen
  putStrLn . showMap $ evalState (splatter rooms $ makeMap dimensions) gen
