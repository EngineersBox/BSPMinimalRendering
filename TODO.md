# TODO

![TODO Status](https://img.shields.io/badge/TODO-outstanding-yellow?style=for-the-badge&logo=markdown)

## Raycaster

* [x] ~~Create generic method for both `checkVertical()` and `checkHorizontal()` called `checkDir()`~~
* [x] ~~Break `renderRays2Dto3D()` method up into smaller methods and generify where possible~~
* [x] Create texture hashing by string using polynomial rolling
* [x] ~~Change rendering to BMP columns~~ Changed to PNG
* [x] Add config files and reader
* [x] Change renderer to query wall texture
* [x] Fix HashTable (change to hashmap K,V pairs) and fix Texture
* [x] Change map file to JSON and map reader to parse JSON
* [x] Add collision detection with walls
* [x] Implement A* search algorithm
* [x] Add start + end locations in map definitions
* [ ] Change raycasting to query against walls from results of QSP rather than raymarching
* [x] Implement minmap and player position scaling
* [x] Fix ray rendering on minimap
* [x] Fix Linux + Windows GL window rendering (incorrect scaling)
* [x] Change TextureLoader to iterate through `resources/textures/*` and load all in there (verify existance and format)
* [x] Fix sprite rendering order, currently far sprites render on top of near, this should be inverted
* [ ] Finish StatsBar
* [x] Add animation frames to Enemies
* [ ] Add hit scan shooting when player clicks, call `hitScanCheck()` on enemies
* [ ] Fix wall textures vertical offset shifting on every second pixel
* [x] Integrate floor + ceiling rendering into y, windows height ,loop in `renderWalls()`. Use a full loop from 0 to height and boolean check if in start-end pos for walls

## Logging

* [x] OpenGL + GLUT ([OpenGL debug wiki](https://www.khronos.org/opengl/wiki/Debug_Output))
* [x] Player position
* [x] Texture processing
* [x] Map processing
* [ ] Add multiple verbosity levels
* [x] QSP tree building

## Map maker

* [ ] Ability to create $M\times N$ grids, where $M,N\in\Reals^{+}$
* [ ] Add walls by clicking on empty squares (white)
* [ ] Remove walls by clicking on walls (black)
* [ ] Set textures for a wall
* [ ] Set colours for a wall

## Testing

* [ ] A*
* [ ] BSP
* [ ] BMP read
* [x] INI read
* [x] JSON read
* [ ] map load
* [ ] texture load
* [x] hash table usage
* [x] hashing collision rate

## BSP Tree

* The root node is **ALWAYS** the closest to center point on the grid.
  * Root: wall with coords closest to $\lparen\lfloor{\frac{width}{2}\rfloor},\lfloor{\frac{height}{2}\rfloor}\rparen$
* Inserting nodes:
  * $L\to R$ or $R\to L$: X coordinate comparison first, then Y coordinate if inline
  * $U\to D$ or $D\to U$: Y coordinate comparison first, then X coordinate if inline

Example inserting procedure for $L\to R$:

* If coord.x is less then current.x node place on left
* If coord.x is greater than current.x node plane on right
* If coord.x is equal to current.y node:
  * If coord.y is less than current.y node place on left
  * If coord.y is greater than current.y node place on right

Tree building:

* $L\to R$ then $U\to D$ if from left
* $R\to L$ then $U\to D$ if from right
* $U\to D$ then $L\to R$ if from up
* $D\to U$ then $L\to R$ if from down

Only need to build $L\to R$ or $R\to L$ and $U\to D$ or $D\to U$ since reversing traversing sign in tree accounts for other case

* Traversing tree based on origin location relative to node
* Test rays against nodes
  * If ray intersects, take note of node
  * Remove ray from query pool
