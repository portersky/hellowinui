# hellowinui

A minimal C++23 Hello World application built with CMake and Ninja.

## Setup

Requirements:

- CMake 3.21 or newer
- Ninja
- A C++23-compatible compiler

`fmt` is fetched automatically during the first CMake configuration, so
network access is required.

## Build

Configure the project:

```sh
cmake -S . -B build -GNinja
```

Build the application:

```sh
ninja -C build
```

## Run

On Unix-like systems:

```sh
./build/hellowinui
```

On Windows:

```text
build\hellowinui.exe
```
