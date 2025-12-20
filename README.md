## easyFEM

EasyFEM lets you solve nonstationary heat reansfer problem using finite element method to showcase its principles.

## ToDo

    Making FEM solver work on different thread than main app

    Adding status bar
    
    Visualising results

    Adding ability to specify boundary conditions

    Refining mesh generation

## RaylibImGUI Template
Using template from http://keasigmadelta.com/kea-campus/

## To repair
There are severall things to repair like:
    - Dense mesh doesn't work (result is nan everywhere)
    - Meshes sometimes are not properly constructed (lines intersect)
    - For less dense mesh results are incorect as it appears as temperature is not travelling through mesh (only edges are heated)

## How to build 

You need a c/c++ compiler, Raylib and CMAKE.

    mkdir build

    cd build

    cmake ..

    cmake --build .

    ./Debug/easyFEM.exe
