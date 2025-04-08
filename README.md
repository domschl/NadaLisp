# NadaLisp Project

## Overview
NadaLisp is a simple C project that demonstrates the basic structure of a C program. It includes a main function that prints "Hello, World!" to the console.

## Project Structure
```
nada-lisp
├── src
│   └── NadaLisp.c
├── include
│   └── NadaLisp.h
├── CMakeLists.txt
├── .gitignore
└── README.md
```

## Building the Project
To build the NadaLisp project, follow these steps:

1. Ensure you have CMake installed on your system.
2. Open a terminal and navigate to the project directory.
3. Create a build directory:
   ```
   mkdir build
   cd build
   ```
4. Run CMake to configure the project:
   ```
   cmake ..
   ```
5. Compile the project:
   ```
   make
   ```

## Running the Project
After building the project, you can run the executable:
```
./NadaLisp
```
This will output:
```
Hello, World!
```

## License
This project is licensed under the MIT License. See the LICENSE file for more details.