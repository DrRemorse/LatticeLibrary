library
=======

c++ opengl game library

Requirements
---
- OpenGL drivers
- g++ -std=c++11
- make


Required libraries and packages
---
- GLFW3: http://www.glfw.org/

On linux you may need these additional packages:
- libglu1-mesa-dev
- xorg-dev

Optional libraries for certain parts of the library:
- BASS: http://www.un4seen.com/bass.html (for sound & music)
- lzo2: http://www.oberhumer.com/opensource/lzo/ (for compression)
- tcc: http://bellard.org/tcc/ (for C-scripting)


Compile the library
---
Using your favorite terminal (eg. cmd); type 'make clean all'. With mingw you must type: mingw32-make clean all.
The make process should produce liblibrary.a in the ./Debug folder. If you are just using this for the first time, or you are having problems building, feel free to edit the makefile (using any text editor) and change the BUILDOPT variable to simply -ggdb3 to get a proper debugging build. The line should look like this:
- BUILDOPT = -ggdb3

If that produces an error, try using -g instead: BUILDOPT = -g

If you get real build errors, please submit a bug report or contact me on EFnet @ #textella.

Setting up your project
---
Create new project, add libglfw3.a, liblzo2.a (and so on) to linker libraries. The linker commandline might look like this in the end:
- Windows: -static -llibrary -lpthread -lbassdll -lglfw3 -lgdi32 -lopengl32 -llzo2 -lws2_32 -ltcc
- Linux: -Llib -llibrary -lbass -lGL -lglfw3 -lX11 -lXxf86vm -lXrandr -lXi -lpthread -Wl,-rpath,.

Remember to read the documentation for each library. The 'library' project uses: BASS, TCC, GLFW3, sockets, threads.
Finally, you can add DLL search paths using -Wl,-rpath,{PATH_HERE}. An example path might be '../lib', or simply '.'.

Copy the contents of the 'inc' if you want to, however you can also get those include files from their respective origins. Copy the contents of the 'include' folder to your own include folder, so that you can access the library headers. If your project compiles, you are good to go.

Dynamic libraries
---
- BASS is only available in dynamic library form: libbass.so/bass.dll
- TCC is available statically, but I had issues compiling it for 64bit with mingw64, so I settled for libtcc.dll/libtcc.so


Example usage
---
See src folder for the tests which are very simplistic. Regretfully there is not much documentation, but there are some tests, the code is very straightforward and some headers have small examples within them. Keep in mind this library will not (and is not intended to) teach you the fundamentals of OpenGL, audio or network programming.
