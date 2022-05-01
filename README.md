# IMGEX

## NAME

  imgex - persist your collages of photos organised into windows

## STATUS

Experimental/work in progress.

This is an early version which merely has the core functionality - open and perform the most basic manipulation of images.

## DESCRIPTION

Build collages of images for, say, your desktop background.

This application, originally designed for the [X Window System](www.x.org) but now using Qt (Qt 5 currently), opens a screen-sized window.  The idea is to load image files from a given location - designed to support removable media, like a camera's memory card - and let the user arrange selected images into collages.  These can then be persisted, so they can be rebuilt later.  By default, only the steps taken to create the image are recorded; the image itself is not saved.  These steps can then be replayed later, assuming the original images remain available.

## IMPLEMENTATION

The software is licensed under the MIT license.

It is written in C++ - requiring C++20 - and depends only on [Qt](https://www.qt.io/).

## Why X

and why not a toolkit, particularly a cross platform one?  The intention was to have a simple way of constructing images which could even be placed in
the root window, as a background (provided the window manager doesn't clobber it).

## VERSIONS

### 0.01

Basic functionality - loads three fixed images in a single window which can be manipulated with the mouse.

Use the mouse wheel to rescale the images and click an image to move it around.
