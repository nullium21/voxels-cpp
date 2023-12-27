# `voxels-cpp`

This project is a simple voxel raytracer written in C++23 for educational purposes.

## Running

To run the renderer (`voxels` CMake target), there needs to be a `suzanne-svo.bin` file in the current working directory.
That contains the Sparse Voxel Octree used for raytracing relatively fast; without the file it will just segfault.

To create the `suzanne-svo.bin`, you have to run `svogen` with a `suzanne.obj` in the current working directory; the program will create `suzanne-points.bin` with the point cloud data, and `suzanne-svo.bin` with the octree.

Both file formats are documented in ImHex patterns: [`points.pat` for `*-points.bin`](./svogen/points.pat) and [`svo.pat` for `*-svo.bin`](./svogen/svo.pat).

## Used Code

This project uses some open-source libraries under permissive licenses to help:

- [`svogen/tiny_obj_loader.h`](./svogen/tiny_obj_loader.h): [(source; MIT)](https://github.com/tinyobjloader/tinyobjloader/blob/release/tiny_obj_loader.h) single-file Wavefront .OBJ model loader, used for transforming meshes into point-cloud/SVO form
- [`svogen/voxelizer.h`](./svogen/voxelizer.h): [(source; MIT)](https://github.com/karimnaaji/voxelizer/blob/master/voxelizer.h) single-file mesh->voxels (in this case, mesh->point cloud) converter library with configurable resolution
- [`glm/`](./glm/): [(source; MIT)](https://github.com/g-truc/glm) header-only library used for maths, such as vectors
- [`stb/stb_image_write.h`](./stb/stb_image_write.h): [(source; public domain)](https://github.com/nothings/stb/blob/master/stb_image_write.h) single-file library for writing various image formats, PNG used here for displaying results

## Used Documentation, Blog Posts, etc.

The code for determining ray-AABB(voxel) collisions is borrowed from [this gamedev stackexchange answer](https://gamedev.stackexchange.com/a/18459) and seems to be an implementation of the *slab method* first documented in *Graphics Gems, 1990, pages 395-396*.
A useful blog post from 2011 by *Tavian Barnes* is available [at Tavian's website](https://tavianator.com/2011/ray_box.html) explaining this method and providing example implementations for 2D.

**TODO:** Another blog post by *Tavian Barnes* [describes](https://tavianator.com/2022/ray_box_boundary.html) how this can be implemented more efficiently, but isn't implemented yet.
