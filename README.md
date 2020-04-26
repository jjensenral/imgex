# IMGEX

## NAME

  imgex - persist your collages of photos organised into windows

## DESCRIPTION

This application, designed for the [X Window System](www.x.org), opens screen-sized window.  The idea is to load image files from a given location - designed to support removable media, like a camera's memory card - and let the user arrange selected images into collages.  These can then be persisted, so they can be rebuilt later.

The implementation does not store the finished collage as an image.  Rather, it reloads the constituent images and remembers how they were processed and where they are to be placed in the target window.

This is an early version which merely has the core functionality - open and perform the most basic manipulation of images.

## IMPLEMENTATION

The software is licensed under the MIT license.

It is written in C++ - requiring C++20 - and, apart from STL and X11, depends only on [DevIL](http://openil.sourceforge.net/).

## Why X

and why not a toolkit, particularly a cross platform one?  The intention was to have a simple way of constructing images which could even be placed in
the root window, as a background (provided the window manager doesn't clobber it).

## VERSIONS

### 0.01

Basic functionality - loads three fixed images in a single window which can be manipulated with the mouse.
