# FNode
Tool based in nodes to build GLSL shaders without any programming knowledge written in C using OpenGL and GLFW (raylib library).

Output shader
------
When running build pipeline, shader.vs and shader.fs are generated in output folder. Besides, a shader.fnode file is generated which contains all data from the shader editor. This data is loaded when the tool is opened again to continue editing the shader.

I recommend to take a look to the example shader to see how to handle with some vectors operations. By the way, observe that both vertex and fragment output nodes needs a 4 floats input (Vector4 or any operator node which returns 4 values).

_Note: the output shader is tweaked to work with raylib videogames programming library, but all variables have a familiar name to adapt them to your own engine._

Credits
------
Víctor Fisac - www.victorfisac.com
