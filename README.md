# Voxel constructor

Voxel constructor is a tool for building voxel-like models. The construction consists of utilizing building blocks ("voxels") for constructing the model. The blocks can be placed, rotated, and removed [planned: painting]. 

## Building
Mac OS X: Currently Mac OS X (10.9) is supported via a Makefile. To build and test, in a terminal change to the project directory, and run:
```unix
make
./test
```

Linux: whilst the software has been built with Linux support in mind, the current build does not have all the necessary Linux libraries included, and requires some modifications to the Makefile.

Windows: the software has not been tested under Windows, and likely needs minor modifications to run. 

## Instructions
* Adding blocks: From the left-side panel, select "Add". Now click on the model to add a block.
* Removing blocks: Same as for adding, only this time select "Remove".
* Rotating blocks: Prior to placing a block, the rotation can be chosen by clicking "Rotate" (increase the size of the window if the "Rotate" button is not visible). Currently the only indication of the rotation is a value displayed on the "Rotate" button. Planned: displaying a wireframe or semi-transparent block prior to placement.
* Selecting blocks: Chosen from the left-side panel. Note: "r" stands for "reverse", and indicates the block is upside-down. "Inv" stands for "inverse", and indicates the block is a cube with the respective shape subtracted from it.
* Painting: currently not supported! The random colours are there as a place holder, demonstrating that the painting does function, but is not user-controllable. 
* Rotating mesh: Click the area outside the mesh and hold to rotate.

###Exporting
The model can be exported to a Wavefront .obj model. This is done by clicking the text field at the bottom of the screen. Type "export", then hit return.

IMPORTANT!!!
Due to an as-of-yet unresolved bug, the software *will* crash if the mesh contains any overlapping edges. If, for example, two cube shapes are placed diagonally next to one another, such that they are connected by a single edge, this will not be exportable. The reason behind this is that internally, a half-edge data-structure is used to represent the mesh. This means that each half-edge has information about one face, and thus each edge is connected to two faces. In the case of an overlapping edge, the edge is connected to four faces, which cannot be represented by a half-edge data structure, hence the error.

##Miscellaneous
- The GUI is a placeholder, and does not work as one would expect. The buttons toggle on and off irrespective of the actual state of the tool in use. Additionally, the menubar is currently just for show.
- Random crashing! The program does occasionally segfault without warning. Requires further testing.
- Just to make things clear, here's a disclamer: do NOT use this software for production purposes! It is buggy, and will result in frustrations. It was built for personal usage, and the intention was not to release it to the public (hence the lack of documentation in the source code).
