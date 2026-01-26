[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/6itnWvSn)
# CMP5359 Computer Graphics 
## Coursework and Lab Starter Repo
### Using this repo

This repo should be used for all of your work for the CMP5359 Computer Graphics module. It is where you should work on the labs, and also submit your final coursework by creating a GitHub release.

Please remember you are marked on regularly committing to this repository - this will form an important part of your Milestone marks!

### Structure

The `Labs` folder contains starter code for your lab activities. These can be compiled using CMake as detailed below.

The `Coursework` folder contains folders and starter code for your three coursework tasks, the Pathtracer, Raytracer and Rasteriser. You may choose to use this starter code, or write your own. See the Submitting and CMake sections below for how to format your submission, and how to compile the code.

### Compiling with CMake

Each of the subfolders, e.g. `Labs/Week1` or `Coursework/Rasteriser` contains a CMake project - note each contains a `CMakeLists.txt` file. To compile each, open up the CMake-GUI application. If you're working on your own machine, you can get CMake here https://cmake.org/download/ . Get the Binary version (probably `Windows x64 Installer`, but versions for Mac and Linux are available).

There are two boxes at the top where you enter the *source* and *build* directories. 

In the "Where is the source code" box at the top, enter the full path to the folder containing `CMakeLists.txt`. You can also use the "Browse Source" button to find the folder in a GUI window. For example, this might be `C:\Users\student\Documents\GitHub\CMP5359\Labs\Week1`.

In the "Where to build the binaries" directory, select a folder called "build" within the source directory. For example, this might be `C:\Users\student\Documents\GitHub\CMP5359\Labs\Week1\build`

**Very Important**: You *must* select a build folder within each project as the "Where to build the binaries" option. If you select the source directory or any other directory you will run into multiple issues, including the app being unable to find files, and clogging up your GitHub with lots of unnecessary files. 

Click "Configure", "Generate" and then open in Visual Studio, either using the third button below, or by going into the build folder and opening the `.sln` file.

When running, remember that you will likely need to select a project in the Solution Explorer on the right, and then right click and "Set as Startup Project" before running and debugging.

### Submitting

Your submitted coursework code should be contained in the `Coursework` directory, as follows:

* `Coursework/Rendered_Images`: Should contained 4 image files - your reference screenshot, and rendered images from your 3 rendering approaches (rasteriser, raytracer, pathtracer).
* `Coursework/Rasteriser`: C++ Code for your rasteriser.
* `Coursework/Raytracer`: C++ Code for your raytracer.
* `Coursework/Pathtracer`: Blender file (.blend format) containing your path-traced scene. I recommend leaving this outside the repo until you are ready, to avoid saving multiple versions of this large binary file.

I must be able to compile and run your code on the lab machines. I recommend building on the starter code and using CMake, but use of Visual Studio is acceptable (if you do, make sure you're using relative directories, i.e. `$(SolutionDir)` when setting up include directories so it will compile on other computers).
