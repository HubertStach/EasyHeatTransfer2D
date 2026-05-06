## EasyHeatTransfer2D

EasyHeatTransfer2D lets you solve nonstationary heat transfer problem using finite element method, on mesh created by the user.

Mesh is created by using advancing front method [1] from cloud of points created by inserting nodes in triangles from Delonay triangulation [2] (section 2.2.1 Point insertion).
It also allows to load 2D triangular, mixed or quad meshes fom Abaqus (.inp file). It doesn't support holes in domain and to have boundaries correctly set you need to create section "BC".

It supports 3 types of time discretisation: explicit Euler, implicit Euler and Crank-Nicolson.


## How to build 

You need C/C++ compiler and Cmake. To build run:

On windows

    ./build.bat

On linux (you might need to use sudo)

    chmod +x build.sh
    ./build.sh

## ToDo

    Add Dirichlet B.C.

    Create proper documentation

## RaylibImGUI Template
Using template from http://keasigmadelta.com/kea-campus/

## Sources
[1] Lo, S. H.. “A NEW MESH GENERATION SCHEME FOR ARBITRARY PLANAR DOMAINS.” International Journal for Numerical Methods in Engineering 21 (1985): 1403-1426.

[2] Owen, Steven. (2000). A Survey of Unstructured Mesh Generation Technology. 7th International Meshing Roundtable. 3.
