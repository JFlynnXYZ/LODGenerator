# LODGenerator

![LODGenerator](http://jfdesigner.co.uk/wp-content/uploads/2015/10/LODGenerator-banner.png "LODGenerator")

**WARNING: Currently unstable and buggy, use at your own risk!**

LODGenerator is an application to load in an obj mesh and reduce the poly count by taking into acount the vertex positions of the model. It does this in the following steps:

  - You supplying an wavefront obj model
  - Set the number of faces you want the mesh to be reduced to
  - A result is calculated and can be saved

### Version
0.0.1

### Tech

Tested on in a Windows 7, 8.1 and 10 environment as well as Linux using Qt 5.4.

LODGenerator uses a few additional modules to work properly:

* **[OpenGL]**         - Used to render the mesh in realtime
* **[Qt 5.4]**         - Used to compile and create the makefile
* **[NGL]**            - The NCCA's graphics library for teaching at Bournemouth University. https://github.com/NCCA/NGL


### Installation

*Installation steps for Windows*

1. Install Qt 5.4 with MSVC2013 OpenGL 64bit compiler.
2. Install NGL, guide can be found for windows at https://docs.google.com/document/d/1QmMWPxFW7qy-eSDxDW5SSgPgB46Oj9wXuWVZhu8iNuI/edit
3. Build and run

After that, you should be good to go!

### How to Use

To use the program, simply run the executable, load in a mesh and set the specific number of faces wanted. Then export the file out.

For more detailed instructions, see https://github.com/JFDesigner/LODGenerator/blob/master/FinalSubmission/UserGuide.pdf

### To-Do

 - [BUG] Fix segmentation fault when trying to create a second LOD

## Contact

If you have problems or questions then please contact me by emailing me at jonflynn@jfdesigner.co.uk.

## Website

Visit my portfolio to see more of my work and interesting programs at http://jfdesigner.co.uk/

License
----

GPL V2
