# TODO

![TODO Status](https://img.shields.io/badge/TODO-outstanding-yellow?style=for-the-badge&logo=markdown)

* Create generic method for both `checkVertical()` and `checkHorizontal()` called `checkDir()`
* ~~Break `renderRays2Dto3D()` method up into smaller methods and generify where possible~~
* ~~Create texture hashing by string using polynomial rolling~~
* *Change rendering to BMP columns* (WIP)
* ~~Add config files and reader~~
* Implement logging:
  * ~~OpenGL + GLUT ([OpenGL debug wiki](https://www.khronos.org/opengl/wiki/Debug_Output))~~
  * Player position
  * ~~Texture processing~~
  * ~~Map processing~~
* ~~Change renderer to query wall texture~~
* ~~Fix HashTable (change to hashmap K,V pairs) and fix Texture~~
* Create test cases for HashTable, A* and BSP Tree (Convex hull)
* ~~Change map file to JSON and map reader to parse JSON~~
* ~~Add collision detection with walls~~
* ~~Implement A* search algorithm~~
* ~~Add start + end locations in map definitions~~
* Implement BSP search tree based on first and last rays
* Change raycasting to query against walls from results of BSP rather than raymarching