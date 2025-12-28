## easyFEM

EasyFEM lets you solve nonstationary heat transfer problem using finite element method, on mesh created by the user.

Mesh is created by subdividing triangles from delonay triangulation.

## ToDo
    Visualising results

    Adding ability to specify boundary conditions
    
    Making FEM solver work on different thread than main app

    Adding status bar

## RaylibImGUI Template
Using template from http://keasigmadelta.com/kea-campus/

## How to build 

You need a c/c++ compiler, Raylib and CMAKE.

    mkdir build

    cd build

    cmake ..

    cmake --build .

    ./Debug/easyFEM.exe
