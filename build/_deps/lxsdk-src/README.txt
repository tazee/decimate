MODO plug-in Software Developer Kit
===================================
The MODO Plug-in Software Developer Kit provides an application programming interface (API) and numerous source code samples that enable developers to read and write MODO scene, image, and movie files, introduce new commands and items into MODO, as well as plug-in scripting support (with an example Lua interpreter).

The MODO SDK is based on COM, with C++ wrappers for many of the published APIs, and is available for Mac OS X, Windows 64-bit and Linux versions of MODO.

What you can do with the MODO SDK
---------------------------------
Export data from MODO into the rest of your pipeline or tool suite (e.g. game engine). Control how MODO data looks to your application, for seamless downstream use in your asset pipeline
Dump the contents of an LXO file into human-readable form
Import data from another application into MODO. Read a proprietary in-house format or tackle an industry standard format
Copy the contents of one LXO to another, applying custom operations as needed
Introduce new procedural textures into MODO like pudding or boiling mud
Add new Commands to MODO that execute native C++-based actions
Introduce new types of geometry into MODO like nuts and bolts and screws
Create new tools for MODO users that provide new ways to create and edit geometry
Add new Replicator point generators to MODO. For example, you could create a condensation plug-in that places replicators, as you’d see on a sweating drinking glass
Design innovative new ways for users to pick colors in MODO


Building the samples
====================

Caveat: The project files provided are not guaranteed to be compatible with all developer configurations. You may have to make modifications to build on your system.

Windows
-------
VisualStudio 2010 project files and solution are provided, except where stated, for each of the samples. See <sdkroot>/samples/VisualC/AllSamples.sln.

Linux
-----
Makefiles are provided for building with GCC 4.1.x, except where stated, for each of the samples. See <sdkroot>/samples/Makefiles.

MacOS
-----
A single Xcode project for building all the samples, except where stated, is provided. See <sdkroot>/samples/xcode_project.

