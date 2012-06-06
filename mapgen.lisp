
(defun args()
  (or #+SBCL *posix-argv*
      #+CLISP *args*
      nil))
(declaim (optimize (speed 0) (space 0) (debug 3)))

(defun make-map(x y)
    (make-array (list x y) :initial-element #\#))

(defun print-map (array)
  (loop for i below (array-total-size array) do
        (when (zerop (mod i (array-dimension array 0)))
          (princ #\Newline))
        (princ (row-major-aref array i))))

(defstruct area left right up down)

(defun area (left right up down)
  (make-area :left left :right right :up up :down down))

(defun randr (min max) (+ min (random (1+(- max min)))))

(defun random-area (width height)
  (let* ((MIN-LEN 4)
         (l (randr 1 (- width  MIN-LEN 2)))
         (u (randr 1 (- height MIN-LEN 2)))
         (r (randr (+ MIN-LEN l) (- width  2)))
         (d (randr (+ MIN-LEN u) (- height 2))))
    (area l r u d)))

(defun random-point (room)
  (vector (randr (area-left room) (area-right room))
          (randr (area-up   room) (area-down  room))))

(defun dig-at (m x y)
  (setf (row-major-aref m (+ x (* y (array-dimension m 0)))) #\.))

(defun dig-room (map room)
  (let ((width (array-dimension map 0)))
    (loop for j from (area-up room) below (1+ (area-down room)) do
          (loop for i from (area-left room) below (1+ (area-right room))
                do (dig-at map i j)))))

(defun dig-hallway (map a b)
  ; Dig from (ax,ay) -> (bx,ay) -> (bx,by)
  (let ((ax (elt a 0))
        (bx (elt b 0))
        (ay (elt a 1))
        (by (elt b 1)))
    (loop for x from (min ax bx) to (max ax bx) do
          (dig-at map x ay))
    (loop for y from (min ay by) to (max ay by) do
          (dig-at map bx y))))

(defun splatter-pattern (map n)
  (let* ((height (array-dimension map 1))
         (width  (array-dimension map 0))
         (rooms (loop for i from 1 to n 
                      collect (random-area width height))))
    (loop for i from 0 below (length rooms) do
          (dig-room map (elt rooms i))
          (when (> (-(length rooms)i) 1)
            (let* ((a (random-point (elt rooms i)))
                   (b (random-point
                       (elt rooms (randr (1+ i)
                                         (- (length rooms) 1))))))
              (dig-hallway map a b))))))

(setf *random-state* (make-random-state t))
(let* ((rooms 5)
       (width 80)
       (height 60)
       (map (make-map width height)))
  (splatter-pattern map 5)
  (print-map map))
