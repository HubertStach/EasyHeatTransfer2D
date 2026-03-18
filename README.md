## easyFEM

EasyFEM lets you solve nonstationary heat transfer problem using finite element method, on mesh created by the user.

Mesh is created by subdividing triangles from delonay triangulation.

## ToDo
    
    Add ability to change solver type (explicit Euler, implicit Euler, Crank-Nicolson)

    Create bash script for simpler building

    Create proper documentation


## RaylibImGUI Template
Using template from http://keasigmadelta.com/kea-campus/

## How to build 

You need a c/c++ compiler, Raylib and CMAKE.

    mkdir build

    cd build

    cmake ..

    cmake --build .

    ./Debug/easyFEM.exe
