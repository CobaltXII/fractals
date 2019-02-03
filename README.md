# Fractals

A GPU-accelerated fractal explorer.

Most of the code included in this repository is a modified version of code from either my Shaker or Boiler repositories. I just ported the algorithms and code to work on the GPU, instead of the CPU. This enables the program to be fast enough to run in realtime.

## Building

You need to link OpenGL and SDL2 in order to build Shaker. The following command should suffice for most systems and compilers.

```bash
clang++ fractals.cpp -o fractals -O3 -lSDL2 -lGL
```

On Apple systems, you may need to use this command instead, based on your compiler vendor.

```bash
clang++ fractals.cpp -o fractals -O3 -lSDL2 -framework OpenGL
```

I don't use GNU/Linux or Windows, so I wouldn't know where to find the OpenGL or SDL include files. You'll probably have to modify the include paths. Other than that, Fractals is (probably) cross-compatible.

## Usage

Once you have compiled Fractals successfully, it is trivial to use it. Simply pass a filename as an argument to Fractals. You can optionally pass a width and a height (you must specify both if you are to specify any).

```bash
./fractals mandelbrot.glsl
```

or, this is also valid

```bash
./shaker mandelbrot.glsl 800 600
```

## Fractals

Currently, Fractals only supports the Mandelbrot, Julia and Burning Ship fractals.

## Technical

Fractals requires a machine that supports a minimum of OpenGL 3.2 Core. This is basically almost every machine. I chose to use OpenGL 3.2 Core for reasons specific to my machine.

## License

This repository is licensed under the MIT License.