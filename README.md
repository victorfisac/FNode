<img src="https://github.com/victorfisac/FNode/blob/master/src/icon/fnode_256x256.png">

# FNode
Tool based in nodes to build GLSL shaders without any programming knowledge written in C using OpenGL and GLFW (raylib library). It contains a compilation pipeline to convert the nodes web into a GLSL vertex and fragment shader files.

This pipeline can be configured in the settings pop up to select the GLSL shading version. Besides, the program starts with an example project of how to build a vertex animated plant with lambert lighting.


Installation
-----

FNode requires raylib. To get it, follow the next steps:

* Go to [raylib](https://github.com/raysan5/raylib) and clone the repository.
* Ensure to pull the last changes from master branch.
* Use code inside examples header comments to compile and execute.


Notes
------

When running build pipeline, shader.vs and shader.fs are generated in output folder. Besides, a shader.fnode file is generated which contains all data from the shader editor. This data is loaded when the tool is opened again to continue editing the shader.

I recommend to take a look to the example shader to see how to handle with some vectors operations. By the way, observe that both vertex and fragment output nodes needs a 4 floats input (Vector4 or any operator node which returns 4 values).

_Note: the output shader is tweaked to work with raylib videogames programming library, but all variables have a familiar name to adapt them to your own engine._


Screenshots
------

<img src="https://github.com/victorfisac/FNode/blob/master/release/screenshots/screenshot_001.png">

<img src="https://github.com/victorfisac/FNode/blob/master/release/screenshots/screenshot_002.png">


Thanks to
------
- [raysan5](https://www.github.com/raysan5) for creating a improved interface mock-up.