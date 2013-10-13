
Welcome to Game and Graphics Programing for iOS and Android using OpenGLES v2.0

To be able to recompile the book tutorials, exercises and the GFX 3D Engine make sure that you have properly installed the following software for your current development platform:

iOS: The latest iOS v5.0 (or up) SDK that you can download from http://developer.apple.com

Android: Go to http://developer.android.com/sdk/index.html and download/install the following:

	- Android SDK v2.3 (or up) 
	- Android NDK r5 (or up)
	
If you wish to use Eclipse, download the following:

	- ADT Plugin (from the Android developer website)
	- Eclipse Classic (http://www.eclipse.org/downloads/)
	- Sequoyah Android Native Code Support (http://eclipse.org/sequoyah/downloads/)

The architecture of this book SDK is very simple (and pretty much self explanatory), however for more information please refer to the directory list below:
_chapter#-#: Contain the final result that you should re-produce by reading the tutorial in the book. At anytime while reading this book, if you feel that the instructions are not clear, or if you are unsure where to insert some code, or even if you simply want to preview the final result of a tutorial ; open the directory. Inside the directory you can then find at the root the source files use by the tutorial (respectively named templateApp.cpp and templateApp.h) and two directories that contains the project files for iOS and Android, you can then load the project into your IDE and rebuild it from scratch.
common: Contain the free and open source GFX 3D engine (the mini game and graphics engine that you will be using in this book) source code of the version that was used to create the templates and tutorials for this book, along with the source of the libraries the engine depends on. The GFX 3D engine is a very small and lightweight graphic engine that is built with bits and pieces of my own professional engine. It is very small, fast, flexible and scalable; and will allow you to render state of the art graphics on your mobile device. If you wish to upgrade to the latest version or want more information about the engine visit http://gfx.sio2interactive.com.
data: In this directory you can find all the original assets that was used by each tutorial(s). Theses assets are either linked dynamically to the projects (in the case of iOS) or simply duplicated inside the assets directory of each Android tutorial. Please take note that all the original project scenes are available as a .blend (default file extension of Blender). It is not mandatory, but highly suggested that you download a copy of Blender for your platform; available at http://blender.org. So you can study the way the scenes are built and how the assets are linked, and exported to the Wavefront OBJ (the official 3d model exchange format used in the book).
EULA: In there you can find all the End User License Agreement for the different libraries that this book SDK is relying on. If you plan to release a commercial application using this book SDK, make sure that your application complies with all of them.
glsloptimizerCL: Contains the source to a simple yet powerful command line program that you can use to optimize your GLSL code (as demonstrated in the chapter about optimization). For more information about the capabilities of this tools visit https://github.com/aras-p/glsl-optimizer.
md5_exporter: A python script for Blender (v2.6x) that allows you to export bone animation sequences created in Blender to the MD5 version 10 file format (generously provided by Paul Zirkle), which is compatible with the loader included in the GFX 3D Engine.
template: The original template project that you will be using when creating a new project from scratch.
template_chapter#-#: In order to speed up and avoid redundancies you will duplicate theses directories throughout the book when 
needed to give you a head start by avoiding to rebuild everything from scratch using the default template project.

If you have questions or want more information feel free to contact me at sio2interactive@gmail.com

Cheers,

ROm
