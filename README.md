## easyFEM

EasyFEM lets you solve nonstationary heat transfer problem using finite element method, on mesh created by the user.

Mesh is created by using advancing front method [1]

## ToDo
        
    Add loading problem from .txt file

    Add quad elements
    
    Add loading and transfering abaqus mesh .tmp to .txt file

    Add Dirichlet B.C.

    Maybe add renumbering algorithm and faster solver

    Create proper documentation


## How to build 

You need C/C++ compiler and Cmake. To build run:

On windows

    ./build.bat

On linux

    ./build.sh

## RaylibImGUI Template
Using template from http://keasigmadelta.com/kea-campus/

## Sources
[1] Lo, S. H.. “A NEW MESH GENERATION SCHEME FOR ARBITRARY PLANAR DOMAINS.” International Journal for Numerical Methods in Engineering 21 (1985): 1403-1426.