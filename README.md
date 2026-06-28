# EasyHeatTransfer2D
**A C++ FEM Solver for 2D Heat Transfer Analysis (Transient & Steady-State)**

EasyHeatTransfer2D is a computational application written in C++ that solves two-dimensional heat transfer problems using the Finite Element Method (FEM). The system uses an interactive graphical user interface built with ImGui and Raylib, allowing users to define simulations fully within the program environment.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build Status](https://img.shields.io/travis/user/repo/build.svg?label=Build)](link-to-ci)
[![GUI Framework](https://img.shields.io/badge/Framework-ImGui%2BRaylib-red.svg)]()

---

## Description
This project models the heat conduction equation in a discretized domain over time and at steady state. The core computation involves formulating and solving large, sparse systems of linear equations derived from established FEM principles.

The simulation workflow is self-contained: users interact with the GUI to define geometries, boundary conditions, source terms, and numerical schemes directly within the application.

## Core Scientific Features
*   **Transient Analysis:** Solves for time-dependent temperature distributions ($\partial T/\partial t$) using advanced numerical methods.
*   **Steady-State Analysis:** Calculates the equilibrium temperature distribution by solving the system when time dependence is null ($\frac{\partial T}{\partial t} = 0$). This determines the final, stable temperature field given boundary and source conditions.
*   **Mesh Input:** Reads structured mesh data (`Nodes`, `Elements`) from various formats, including Abaqus `.inp` files, supporting both triangular and quadrilateral element types.
*   **Time Integration Control:** Implements three specific numerical methods for transient time advancement:
    *   Explicit Euler Scheme
    *   Implicit Euler Scheme
    *   Crank-Nicolson Scheme (Averages the spatial derivatives at $t$ and $t+\Delta t$).

## Architectural Breakdown (FEM Cycle)
The simulation relies on a three-stage pipeline to manage data flow:

1.  **Mesh Assembly:** Geometric input is processed to generate global element connectivity. This step determines which degrees of freedom (DOFs) are coupled together across the domain elements.
2.  **Matrix Calculation & Aggregation:** The core logic, managed by the `Solution` class (`src/mes/mes.h`), calculates local stiffness and capacity matrices ($\mathbf{H}_{local}, \mathbf{C}_{local}$) for every element. These matrices are then systematically aggregated into large-scale **Global System Matrices** ($\mathbf{H}_{Global}, \mathbf{C}_{Global}$), using Eigen for efficient linear algebra operations.
3.  **Solver:** The solution method depends on the problem type:
    *   **Transient Mode:** Advances time by solving $\mathbf{A} \cdot \Delta T = \mathbf{b}$ iteratively over defined time steps ($\Delta t$).
    *   **Steady-State Mode:** Solves the system directly for the equilibrium state, requiring only one solution pass.

## Getting Started

### Prerequisites
*   C/C++ Compiler (e.g., GCC or MSVC).
*   CMake build system generator.

### Build Instructions
1.  **Navigate:** Change directory to the project root (`EasyHeatTransfer2D`).
2.  **Execute:** Use the provided script:
    ```bash
    # Windows
    .\build.bat 
    
    # Linux/macOS
    chmod +x build.sh
    ./build.sh
    ```

Upon successful compilation, running the executable will launch the ImGui GUI environment, allowing immediate interaction with the simulation parameters and selection of whether to solve for a steady-state or transient solution.
