# Sandscreen

**Sandscreen** is a basic terminal screensaver built on [ncurses](https://invisible-mirror.net/archives/ncurses/).

It runs a cellular automaton sand simulation that clears and re-runs once complete.

# Usage

Using **Sandscreen** is very simple at this time. Just run the binary and it will display the simulation in your terminal. I hope to add further command-line options down the line, but I haven't done it quite yet. To quit the program, you can either terminate it or press "q".

# Building

**Sandscreen** uses the CMake build system generator. A script to build the program is provided. An example for how to build manually is below:

```
cmake -B build -S .
cmake --build build
```

# License

This program is licensed under the MIT License. For further details, read the LICENSE file.
