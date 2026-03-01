# Pipelang

## Build
```console
    # Pipelang uses a 'nob.h' single-header only library as a build system
    # This will compile the build script that also rebuilds itself if changed
    $ cc -o nob nob.c

    # This will build the project and make an executable 'pipe'
    $ ./nob

    # You can run the executable to get a help message
    $ ./pipe

    # You can just feed the script to executable
    $ ./pipe ./examples/images/01-rgb_to_grayscale.pipe
```
