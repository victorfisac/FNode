/**********************************************************************************************
*
*   FNode 1.0 - Node based shading library
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2016 Victor Fisac
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

//----------------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------------
#define FNODE_IMPLEMENTATION
#include "fnode.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define     UI_PADDING                  5                                   // Interface bounds padding with background
#define     UI_PADDING_SCROLL           20                                  // Interface scroll bar padding
#define     UI_BUTTON_HEIGHT            30                                  // Interface bounds height
#define     UI_SCROLL                   20                                  // Interface scroll sensitivity
#define     UI_GRID_ALPHA               0.25f                               // Interface canvas background grid lines alpha
#define     VISOR_MODEL_SCALE           11.0f                               // Visor model scale
#define     VISOR_MODEL_ROTATION        0.0f                                // Visor model rotation speed
#define     VISOR_BORDER                2                                   // Visor window border width
#define     VERTEX_PATH                 "output/shader.vs"                  // Vertex shader output path
#define     FRAGMENT_PATH               "output/shader.fs"                  // Fragment shader output path
#define     DATA_PATH                   "output/shader.fnode"               // Shader data output path
#define     MAX_TEXTURES                30                                  // Shader maximum texture units
#define     COMPILE_DURATION            120                                 // Shader compile result duration
#define     MODEL_PATH                  "res/meshes/plant.obj"              // Example model file path
#define     MODEL_TEXTURE_DIFFUSE       "res/textures/plant_color.png"      // Example model color texture file path
#define     MODEL_TEXTURE_WINDAMOUNT    "res/textures/plant_motion.png"     // Example model motion texture file path
#define     FXAA_VERTEX                 "res/shaders/fxaa.vs"               // Visor FXAA vertex shader path
#define     FXAA_FRAGMENT               "res/shaders/fxaa.fs"               // Visor FXAA fragment shader path
#define     FXAA_SCREENSIZE_UNIFORM     "viewportSize"                      // Visor FXAA shader screen size uniform location name
#define     WINDOW_ICON                 "res/fnode_icon.png"                // FNode icon for window initialization

//----------------------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------------------
Vector2 mousePosition = { 0, 0 };           // Current mouse position
Vector2 lastMousePosition = { 0, 0 };       // Previous frame mouse position
Vector2 mouseDelta = { 0, 0 };              // Current frame mouse position increment since previous frame
Vector2 currentOffset = { 0, 0 };           // Current selected node offset between mouse position and node shape
float modelRotation = 0.0f;                 // Current model visualization rotation angle
int scrollState = 0;                        // Current mouse drag interface scroll state
Vector2 canvasSize;                         // Interface screen size
float menuScroll = 10.0f;                   // Current interface scrolling amount
Vector2 scrollLimits = { 10, 1385 };        // Interface scrolling limits
Rectangle menuScrollRec = { 0, 0, 0, 0 };   // Interface scroll rectangle bounds
Vector2 menuScrollLimits = { 5, 685 };      // Interface scroll rectangle position limits
Rectangle canvasScroll = { 0, 0, 0, 0 };    // Interface scroll rectangle bounds
Model model;                                // Visor default model for shader visualization
RenderTexture2D visorTarget;                // Visor model visualization render target
Shader fxaa;                                // Canvas and visor anti-aliasing postprocessing shader
int fxaaUniform = -1;                       // FXAA shader viewport size uniform location point
Shader shader;                              // Visor model shader
int viewUniform = -1;                       // Created shader view direction uniform location point
int transformUniform = -1;                  // Created shader model transform uniform location point
int timeUniformV = -1;                      // Created shader current time uniform location point in vertex shader
int timeUniformF = -1;                      // Created shader current time uniform location point in fragment shader
bool loadedShader = false;                  // Current loaded custom shader state
float currentTime = 0;                      // Current global time to send to shader as attribute
char **droppedFiles;                        // Current dropped files paths
Texture2D textures[MAX_TEXTURES] = { 0 };   // Shader texture unit textures
int loadedFiles = 0;                        // Loaded textures count
bool usedUnits[MAX_TEXTURES] = { false };   // Shader compiling used texture units
bool fullVisor = false;                     // Visor full screen state
bool help = false;                          // Display help message state
bool visorState = false;                    // Visor camera control state
bool settings = false;                      // Interface settings window state
ShaderVersion version = GLSL_330;           // Current shader version setting
bool backfaceCulling = false;               // Current shader backface culling state
int compileState = -1;                      // Compile state (awiting, successful, failed)
int framesCounter = 0;                      // Global frames counter
int compileFrame = 0;                       // Compile time frames count
Texture2D iconTex;                          // FNode icon texture used in help message

//----------------------------------------------------------------------------------
// Functions Declaration
//----------------------------------------------------------------------------------
void CheckPreviousShader();                                 // Check if there are a compatible shader in output folder
void UpdateMouseData();                                     // Updates current mouse position and delta position
void UpdateInputsData();                                    // Updates current inputs states
void UpdateScroll();                                        // Updates mouse scrolling for menu and canvas drag
void UpdateNodesEdit();                                     // Check node data values edit input
void UpdateNodesDrag();                                     // Check node drag input
void UpdateNodesLink();                                     // Check node link input
void UpdateCommentCreationEdit();                           // Check comment creation and shape edit input
void UpdateCommentsDrag();                                  // Check comment drag input
void UpdateCommentsEdit();                                  // Check comment text edit input
void UpdateShaderData();                                    // Update required values to created shader for geometry data calculations
void CompileShader();                                       // Compiles all node structure to create the GLSL fragment shader in output folder
void CheckConstant(FNode node, FILE *file);                 // Check nodes searching for constant values to define them in shaders
void CompileNode(FNode node, FILE *file, bool fragment);    // Compiles a specific node checking its inputs and writing current node operation in shader
void AlignAllNodes();                                       // Aligns all created nodes
void ClearUnusedNodes();                                    // Destroys all unused nodes
void ClearGraph();                                          // Destroys all created nodes and its linked lines
void DrawCanvas();                                          // Draw canvas space to create nodes
void DrawCanvasGrid(int divisions);                         // Draw canvas grid with a specific number of divisions for horizontal and vertical lines
void DrawVisor();                                           // Draws a visor with default model rotating and current shader
void DrawInterface();                                       // Draw interface to create nodes
char *GetFileExtension(char *filename);                     // Returns the extension of a file
bool CheckFileExtension(char *filename, char *extension);   // Check filename for specific extension
bool CheckTextureExtension(char *filename);                 // Check filename for compatible texture extensions

//----------------------------------------------------------------------------------
// Functions Definition
//----------------------------------------------------------------------------------
// Check if there are a compatible shader in output folder
void CheckPreviousShader(bool makeGraph)
{
    Shader previousShader = LoadShader(VERTEX_PATH, FRAGMENT_PATH);
    if (previousShader.id > 0)
    {
        shader = previousShader;
        model.material.shader = shader;
        viewUniform = GetShaderLocation(shader, "viewDirection");
        transformUniform = GetShaderLocation(shader, "modelMatrix");
        timeUniformV = GetShaderLocation(shader, "vertCurrentTime");
        timeUniformF = GetShaderLocation(shader, "fragCurrentTime");

        if (makeGraph)
        {
            FILE *dataFile = fopen(DATA_PATH, "r");
            if (dataFile != NULL)
            {
                float type = -1;
                float inputs[MAX_INPUTS] = { -1, -1, -1, -1 };
                float inputsCount = -1;
                float inputsLimit = -1;
                float dataCount = -1;
                float property = -1;
                float data[MAX_VALUES] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
                float shapeX = -1;
                float shapeY = -1;

                while (fscanf(dataFile, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,\n", &type, &property,
                &inputs[0], &inputs[1], &inputs[2], &inputs[3], &inputsCount, &inputsLimit, &dataCount, &data[0], &data[1], &data[2],
                &data[3], &data[4], &data[5], &data[6], &data[7], &data[8], &data[9], &data[10], &data[11], &data[12], &data[13], &data[14],
                &data[15], &shapeX, &shapeY) > 0)
                {                
                    FNode newNode = InitializeNode(true);
                    newNode->type = type;
                    newNode->property = property;

                    if (type < FNODE_ADD) newNode->inputShape = (Rectangle){ 0, 0, 0, 0 };

                    switch ((int)type)
                    {
                        case FNODE_PI: newNode->name = "Pi"; break;
                        case FNODE_E: newNode->name = "e"; break;
                        case FNODE_TIME: newNode->name = "Current Time"; break;
                        case FNODE_VERTEXPOSITION: newNode->name = "Vertex Position"; break;
                        case FNODE_VERTEXNORMAL: newNode->name = "Normal Direction"; break;
                        case FNODE_FRESNEL: newNode->name = "Fresnel"; break;
                        case FNODE_VIEWDIRECTION: newNode->name = "View Direction"; break;
                        case FNODE_MVP: newNode->name = "MVP Matrix"; break;
                        case FNODE_MATRIX: newNode->name = "Matrix 4x4"; break;
                        case FNODE_VALUE: newNode->name = "Value"; break;
                        case FNODE_VECTOR2: newNode->name = "Vector 2"; break;
                        case FNODE_VECTOR3: newNode->name = "Vector 3"; break;
                        case FNODE_VECTOR4: newNode->name = "Vector 4"; break;
                        case FNODE_ADD: newNode->name = "Add"; break;
                        case FNODE_SUBTRACT: newNode->name = "Subtract"; break;
                        case FNODE_MULTIPLY: newNode->name = "Multiply"; break;
                        case FNODE_DIVIDE: newNode->name = "Divide"; break;
                        case FNODE_APPEND: newNode->name = "Append"; break;
                        case FNODE_ONEMINUS: newNode->name = "One Minus"; break;
                        case FNODE_ABS: newNode->name = "Abs"; break;
                        case FNODE_COS:newNode->name = "Cos"; break;
                        case FNODE_SIN: newNode->name = "Sin"; break;
                        case FNODE_TAN: newNode->name = "Tan"; break;
                        case FNODE_DEG2RAD: newNode->name = "Deg to Rad"; break;
                        case FNODE_RAD2DEG: newNode->name = "Rad to Deg"; break;
                        case FNODE_NORMALIZE: newNode->name = "Normalize"; break;
                        case FNODE_NEGATE: newNode->name = "Negate"; break;
                        case FNODE_RECIPROCAL: newNode->name = "Reciprocal"; break;
                        case FNODE_SQRT: newNode->name = "Square Root"; break;
                        case FNODE_TRUNC: newNode->name = "Trunc"; break;
                        case FNODE_ROUND: newNode->name = "Round"; break;
                        case FNODE_VERTEXCOLOR: newNode->name = "Vertex Color"; break;
                        case FNODE_CEIL: newNode->name = "Ceil"; break;
                        case FNODE_CLAMP01: newNode->name = "Clamp 0-1"; break;
                        case FNODE_EXP2: newNode->name = "Exp 2"; break;
                        case FNODE_POWER: newNode->name = "Power"; break;
                        case FNODE_STEP: newNode->name = "Step"; break;
                        case FNODE_POSTERIZE: newNode->name = "Posterize"; break;
                        case FNODE_MAX: newNode->name = "Max"; break;
                        case FNODE_MIN: newNode->name = "Min"; break;
                        case FNODE_LERP: newNode->name = "Lerp"; break;
                        case FNODE_SMOOTHSTEP: newNode->name = "Smooth Step"; break;
                        case FNODE_CROSSPRODUCT: newNode->name = "Cross Product"; break;
                        case FNODE_DESATURATE: newNode->name = "Desaturate"; break;
                        case FNODE_DISTANCE: newNode->name = "Distance"; break;
                        case FNODE_DOTPRODUCT: newNode->name = "Dot Product"; break;
                        case FNODE_LENGTH: newNode->name = "Length"; break;
                        case FNODE_MULTIPLYMATRIX: newNode->name = "Multiply Matrix"; break;
                        case FNODE_TRANSPOSE: newNode->name = "Transpose"; break;
                        case FNODE_PROJECTION: newNode->name = "Projection Vector"; break;
                        case FNODE_REJECTION: newNode->name = "Rejection Vector"; break;
                        case FNODE_HALFDIRECTION: newNode->name = "Half Direction"; break;
                        case FNODE_SAMPLER2D: newNode->name = "Sampler 2D"; break;
                        case FNODE_VERTEX: newNode->name = "[OUTPUT] Vertex Position"; break;
                        case FNODE_FRAGMENT: newNode->name = "[OUTPUT] Fragment Color"; break;
                        default: break;
                    }

                    for (int i = 0; i < MAX_INPUTS; i++) newNode->inputs[i] = inputs[i];

                    newNode->inputsCount = inputsCount;
                    newNode->inputsLimit = inputsLimit;

                    for (int i = 0; i < MAX_VALUES; i++)
                    {
                        newNode->output.data[i].value = data[i];
                        FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
                    }

                    newNode->output.dataCount = dataCount;
                    newNode->shape.x = shapeX;
                    newNode->shape.y = shapeY;

                    UpdateNodeShapes(newNode);
                }

                int from = -1;
                int to = -1;            
                while (fscanf(dataFile, "?%i?%i\n", &from, &to) > 0)
                {
                    tempLine = CreateNodeLine(from);
                    tempLine->to = to;
                }

                for (int i = 0; i < nodesCount; i++) UpdateNodeShapes(nodes[i]);
                CalculateValues();
                for (int i = 0; i < nodesCount; i++) UpdateNodeShapes(nodes[i]);

                loadedShader = true;
                fclose(dataFile);
            }
            else TraceLogFNode(false, "error when trying to open previous shader data file");
        }
    }

    if (!loadedShader)
    {
        CreateNodeMaterial(FNODE_VERTEX, "[OUTPUT] Vertex Position", 0);
        CreateNodeMaterial(FNODE_FRAGMENT, "[OUTPUT] Fragment Color", 0);
    }
}

// Updates current mouse position and delta position
void UpdateMouseData()
{
    // Update mouse position values
    lastMousePosition = mousePosition;
    mousePosition = GetMousePosition();
    mouseDelta = (Vector2){ mousePosition.x - lastMousePosition.x, mousePosition.y - lastMousePosition.y };
}

// Updates current inputs states
void UpdateInputsData()
{
    if (IsKeyPressed('H')) help = !help;
    else if (IsKeyPressed(KEY_RIGHT_ALT))
    {
        fullVisor = !fullVisor;
        UnloadRenderTexture(visorTarget);
        visorTarget = LoadRenderTexture((fullVisor ? screenSize.x : (screenSize.x/4)), (fullVisor ? screenSize.y : (screenSize.y/4)));
    }

    // Update canvas camera values
    camera.target = mousePosition;
}

// Updates mouse scrolling for menu and canvas drag
void UpdateScroll()
{
    // Check zoom input
    if (GetMouseWheelMove() != 0)
    {
        if (CheckCollisionPointRec(mousePosition, (Rectangle){ canvasSize.x - visorTarget.texture.width - UI_PADDING, screenSize.y - visorTarget.texture.height - UI_PADDING, visorTarget.texture.width, visorTarget.texture.height }))
        {
            UpdateCamera(&camera3d);
        }
        else if (CheckCollisionPointRec(mousePosition, (Rectangle){ 0, 0, canvasSize.x, canvasSize.y }))
        {
            if (IsKeyDown(KEY_LEFT_ALT)) camera.offset.x -= GetMouseWheelMove()*UI_SCROLL;
            else camera.offset.y -= GetMouseWheelMove()*UI_SCROLL;
        }
        else
        {
            menuScroll -= GetMouseWheelMove()*UI_SCROLL;
            menuScroll = FClamp(menuScroll, scrollLimits.x, scrollLimits.y);
            menuScrollRec.y = (menuScrollLimits.y - menuScrollLimits.x)*menuScroll/(scrollLimits.y - scrollLimits.x);
        }
    }
    else if (CheckCollisionPointRec(mousePosition, (Rectangle){ canvasSize.x - visorTarget.texture.width - UI_PADDING, screenSize.y - visorTarget.texture.height - UI_PADDING, visorTarget.texture.width, visorTarget.texture.height }))
    {
        UpdateCamera(&camera3d);
    }
    // Check mouse drag interface scrolling input
    if (scrollState == 0)
    {
        if ((IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(mousePosition, menuScrollRec))) scrollState = 1;
    }
    else
    {
        menuScroll += mouseDelta.y*2.0f;
        menuScrollRec.y += mouseDelta.y;

        if (menuScrollRec.y >= menuScrollLimits.y)
        {
            menuScroll = scrollLimits.y;
            menuScrollRec.y = menuScrollLimits.y;
        }
        else if (menuScrollRec.y <= menuScrollLimits.x)
        {
            menuScroll = scrollLimits.x;
            menuScrollRec.y = menuScrollLimits.x;
        }

        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) scrollState = 0;
    }
}

// Check node data values edit input
void UpdateNodesEdit()
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        int index = -1;
        int data = -1;
        for (int i = 0; i < nodesCount; i++)
        {
            if ((nodes[i]->type >= FNODE_MATRIX) && (nodes[i]->type <= FNODE_VECTOR4))
            {
                for (int k = 0; k < nodes[i]->output.dataCount; k++)
                {
                    if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->output.data[k].shape, camera)))
                    {
                        index = i;
                        data = k;
                        break;
                    }
                }
            }
        }

        if (index != -1)
        {
            if ((editNode == -1) && (selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1) && (editSize == -1) && (editSizeType == -1) && (editComment == -1))
            {
                editNode = nodes[index]->id;
                editNodeType = data;
                editNodeText = (char *)FNODE_MALLOC(MAX_NODE_LENGTH);
                usedMemory += MAX_NODE_LENGTH;
                for (int i = 0; i < MAX_NODE_LENGTH; i++) editNodeText[i] = nodes[index]->output.data[data].valueText[i];
            }
            else if ((editNode != -1) && (selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1) && (editSize == -1) && (editSizeType == -1) && (editComment == -1))
            {
                if ((nodes[index]->id != editNode) || (data != editNodeType))
                {
                    for (int i = 0; i < nodesCount; i++)
                    {
                        if (nodes[i]->id == editNode)
                        {
                            for (int k = 0; k < MAX_NODE_LENGTH; k++) nodes[i]->output.data[editNodeType].valueText[k] = editNodeText[k];
                        }
                    }

                    editNode = nodes[index]->id;
                    editNodeType = data;

                    for (int i = 0; i < MAX_NODE_LENGTH; i++) editNodeText[i] = nodes[index]->output.data[data].valueText[i];
                }
            }
        }
        else if ((editNode != -1) && (editNodeType != -1))
        {
            for (int i = 0; i < nodesCount; i++)
            {
                if (nodes[i]->id == editNode)
                {
                    for (int k = 0; k < MAX_NODE_LENGTH; k++) nodes[i]->output.data[editNodeType].valueText[k] = editNodeText[k];
                }
            }

            editNode = -1;
            editNodeType = -1;
            FNODE_FREE(editNodeText);
            usedMemory -= MAX_NODE_LENGTH;
            editNodeText = NULL;
        }     
    }
}

// Check node drag input
void UpdateNodesDrag()
{
    if ((selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            for (int i = nodesCount - 1; i >= 0; i--)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->shape, camera)))
                {
                    selectedNode = nodes[i]->id;
                    currentOffset = (Vector2){ mousePosition.x - nodes[i]->shape.x, mousePosition.y - nodes[i]->shape.y };
                    break;
                }
            }

            if ((selectedNode == -1) && (scrollState == 0) && (!CheckCollisionPointRec(mousePosition, (Rectangle){ canvasSize.x, 0, (screenSize.x - canvasSize.x), screenSize.y })))
            {
                camera.offset.x += mouseDelta.x;
                camera.offset.y += mouseDelta.y;
            }
        }
        else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            for (int i = nodesCount - 1; i >= 0; i--)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->shape, camera)) && (nodes[i]->type < FNODE_VERTEX))
                {
                    DestroyNode(nodes[i]);
                    CalculateValues();
                    break;
                }
            }
        }
    }
    else if ((selectedNode != -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1))
    {
        for (int i = 0; i < nodesCount; i++)
        {
            if (nodes[i]->id == selectedNode)
            {
                nodes[i]->shape.x = mousePosition.x - currentOffset.x;
                nodes[i]->shape.y = mousePosition.y - currentOffset.y;

                // Check aligned drag movement input
                if (IsKeyDown(KEY_LEFT_ALT)) AlignNode(nodes[i]);

                UpdateNodeShapes(nodes[i]);
                break;
            }
        }

        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) selectedNode = -1;
    }
}

// Check node link input
void UpdateNodesLink()
{
    if ((selectedNode == -1) && (commentState == 0) && (selectedComment == -1)) 
    {
        switch (lineState)
        {
            case 0:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    for (int i = nodesCount - 1; i >= 0; i--)
                    {
                        if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->outputShape, camera)))
                        {
                            tempLine = CreateNodeLine(nodes[i]->id);
                            lineState = 1;
                            break;
                        }
                    }
                }
                else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !IsKeyDown(KEY_LEFT_ALT))
                {
                    for (int i = nodesCount - 1; i >= 0; i--)
                    {
                        if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->outputShape, camera)))
                        {
                            for (int k = linesCount - 1; k >= 0; k--)
                            {
                                if (nodes[i]->id == lines[k]->from) DestroyNodeLine(lines[k]);
                            }

                            CalculateValues();
                            CalculateValues();
                            break;
                        }
                        else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->inputShape, camera)))
                        {
                            for (int k = linesCount - 1; k >= 0; k--)
                            {
                                if (nodes[i]->id == lines[k]->to) DestroyNodeLine(lines[k]);
                            }

                            CalculateValues();
                            CalculateValues();
                            break;
                        }
                    }
                }
            } break;
            case 1:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    for (int i = 0; i < nodesCount; i++)
                    {
                        if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->inputShape, camera)) && (nodes[i]->id != tempLine->from) && (nodes[i]->inputsCount < nodes[i]->inputsLimit))
                        {
                            // Get which index has the first input node id from current nude                            
                            int indexFrom = GetNodeIndex(tempLine->from);

                            bool valuesCheck = true;
                            if (nodes[i]->type == FNODE_SAMPLER2D) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                            else if (nodes[i]->type == FNODE_LERP) valuesCheck = (nodes[i]->inputsCount <= nodes[i]->inputsLimit);
                            else if (nodes[i]->type == FNODE_APPEND) valuesCheck = ((nodes[i]->output.dataCount + nodes[indexFrom]->output.dataCount <= 4) && (nodes[indexFrom]->output.dataCount == 1));
                            else if (nodes[i]->type == FNODE_VERTEXCOLOR) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                            else if ((nodes[i]->type == FNODE_POWER) && (nodes[i]->inputsCount == 1)) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                            else if (nodes[i]->type == FNODE_STEP) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                            else if (nodes[i]->type == FNODE_NORMALIZE) valuesCheck = ((nodes[indexFrom]->output.dataCount > 1) && (nodes[indexFrom]->output.dataCount <= 4));
                            else if (nodes[i]->type == FNODE_CROSSPRODUCT) valuesCheck = (nodes[indexFrom]->output.dataCount == 3);
                            else if (nodes[i]->type == FNODE_DESATURATE)
                            {
                                if (nodes[i]->inputsCount == 0) valuesCheck = (nodes[indexFrom]->output.dataCount < 4);
                                else if (nodes[i]->inputsCount == 1) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                            }
                            else if ((nodes[i]->type == FNODE_DOTPRODUCT) || (nodes[i]->type == FNODE_LENGTH) || ((nodes[i]->type >= FNODE_PROJECTION) && (nodes[i]->type <= FNODE_HALFDIRECTION)))
                            {
                                valuesCheck = ((nodes[indexFrom]->output.dataCount > 1) && (nodes[indexFrom]->output.dataCount <= 4));

                                if (valuesCheck && (nodes[i]->inputsCount > 0))
                                {
                                    int index = GetNodeIndex(nodes[i]->inputs[0]);
                                    
                                    if (index != -1) valuesCheck = (nodes[indexFrom]->output.dataCount == nodes[index]->output.dataCount);
                                    else TraceLogFNode(true, "error when trying to get node inputs index");
                                }
                            }
                            else if (nodes[i]->type == FNODE_DISTANCE)
                            {
                                valuesCheck = ((nodes[indexFrom]->output.dataCount <= 4));

                                if (valuesCheck && (nodes[i]->inputsCount > 0))
                                {
                                    int index = GetNodeIndex(nodes[i]->inputs[0]);
                                    
                                    if (index != -1) valuesCheck = (nodes[indexFrom]->output.dataCount == nodes[index]->output.dataCount);
                                    else TraceLogFNode(true, "error when trying to get node inputs index");
                                }
                            }
                            else if ((nodes[i]->type == FNODE_MULTIPLYMATRIX) || (nodes[i]->type == FNODE_TRANSPOSE)) valuesCheck = (nodes[indexFrom]->output.dataCount == 16);
                            else if (nodes[i]->type >= FNODE_VERTEX) valuesCheck = (nodes[indexFrom]->output.dataCount <= nodes[i]->output.dataCount);
                            else if (nodes[i]->type > FNODE_DIVIDE) valuesCheck = (nodes[i]->output.dataCount == nodes[indexFrom]->output.dataCount);

                            if (((nodes[i]->inputsCount == 0) && (nodes[i]->type != FNODE_NORMALIZE) && (nodes[i]->type != FNODE_DOTPRODUCT) && 
                            (nodes[i]->type != FNODE_LENGTH) && (nodes[i]->type != FNODE_MULTIPLYMATRIX) && (nodes[i]->type != FNODE_TRANSPOSE) && (nodes[i]->type != FNODE_PROJECTION) &&
                            (nodes[i]->type != FNODE_DISTANCE) && (nodes[i]->type != FNODE_REJECTION) && (nodes[i]->type != FNODE_HALFDIRECTION) && (nodes[i]->type != FNODE_STEP) &&
                            (nodes[i]->type != FNODE_LERP) && (nodes[i]->type != FNODE_SAMPLER2D)) || valuesCheck)
                            {
                                // Check if there is already a line created with same linking ids
                                for (int k = 0; k < linesCount; k++)
                                {
                                    if ((lines[k]->to == nodes[i]->id) && (lines[k]->from == tempLine->from))
                                    {
                                        DestroyNodeLine(lines[k]);
                                        break;
                                    }
                                }

                                // Save temporal line values and destroy it
                                int from = tempLine->from;
                                int to = nodes[i]->id;
                                DestroyNodeLine(tempLine);

                                // Create final node line
                                FLine temp = CreateNodeLine(from);
                                temp->to = to;

                                // Reset linking state values
                                lineState = 0;
                                CalculateValues();
                                CalculateValues();
                                break;
                            }
                            else TraceLogFNode(false, "error trying to link node ID %i (length: %i) with node ID %i (length: %i)", nodes[i]->id, nodes[i]->output.dataCount, nodes[indexFrom]->id, nodes[indexFrom]->output.dataCount);
                        }
                    }
                }
                else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                {
                    DestroyNodeLine(tempLine);
                    lineState = 0;
                }
            } break;
            default: break;
        }
    }
}

// Check comment creation input
void UpdateCommentCreationEdit()
{
    if ((selectedNode == -1) && (lineState == 0) && (selectedComment == -1)) 
    {
        switch (commentState)
        {
            case 0:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    if (IsKeyDown(KEY_LEFT_ALT))
                    {
                        commentState = 1;

                        tempCommentPos.x = mousePosition.x;
                        tempCommentPos.y = mousePosition.y;

                        tempComment = CreateComment();
                        tempComment->shape.x = mousePosition.x - camera.offset.x;
                        tempComment->shape.y = mousePosition.y - camera.offset.y;

                        UpdateCommentShapes(tempComment);
                    }
                    else
                    {
                        for (int i = 0; i < commentsCount; i++)
                        {
                            if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeTShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 0;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeBShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 1;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeLShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 2;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeRShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 3;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeTlShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 4;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeTrShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 5;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeBlShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 6;
                                break;
                            }
                            else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->sizeBrShape, camera)))
                            {
                                editSize = comments[i]->id;
                                commentState = 1;
                                editSizeType = 7;
                                break;
                            }
                        }
                    }
                }
            } break;
            case 1:
            {
                if (editSize != -1)
                {
                    for (int i = 0; i < commentsCount; i++)
                    {
                        if (comments[i]->id == editSize)
                        {
                            switch (editSizeType)
                            {
                                case 0:
                                {
                                    comments[i]->shape.y += mouseDelta.y;
                                    comments[i]->shape.height -= mouseDelta.y;
                                } break;
                                case 1: comments[i]->shape.height += mouseDelta.y; break;
                                case 2:
                                {
                                    comments[i]->shape.x += mouseDelta.x;
                                    comments[i]->shape.width -= mouseDelta.x;
                                } break;
                                case 3: comments[i]->shape.width += mouseDelta.x; break;
                                case 4:
                                {
                                    comments[i]->shape.x += mouseDelta.x;
                                    comments[i]->shape.width -= mouseDelta.x;
                                    comments[i]->shape.y += mouseDelta.y;
                                    comments[i]->shape.height -= mouseDelta.y;
                                } break;
                                case 5:
                                {
                                    comments[i]->shape.width += mouseDelta.x;
                                    comments[i]->shape.y += mouseDelta.y;
                                    comments[i]->shape.height -= mouseDelta.y;
                                } break;
                                case 6:
                                {
                                    comments[i]->shape.x += mouseDelta.x;
                                    comments[i]->shape.width -= mouseDelta.x;
                                    comments[i]->shape.height += mouseDelta.y;
                                } break;
                                case 7:
                                {
                                    comments[i]->shape.width += mouseDelta.x;
                                    comments[i]->shape.height += mouseDelta.y;
                                } break;
                                default: break;
                            }

                            UpdateCommentShapes(comments[i]);
                            break;
                        }
                    }

                    if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
                    {
                        editSize = -1;
                        editSizeType = -1;
                        commentState = 0;
                    }
                }
                else
                {
                    if ((mousePosition.x - tempCommentPos.x) >= 0) tempComment->shape.width = mousePosition.x - tempComment->shape.x - camera.offset.x;
                    else
                    {
                        tempComment->shape.width = tempCommentPos.x - mousePosition.x;
                        tempComment->shape.x = tempCommentPos.x - tempComment->shape.width - camera.offset.x;
                    }

                    if ((mousePosition.y - tempCommentPos.y) >= 0) tempComment->shape.height = mousePosition.y - tempComment->shape.y - camera.offset.y;
                    else
                    {
                        tempComment->shape.height = tempCommentPos.y - mousePosition.y;
                        tempComment->shape.y = tempCommentPos.y - tempComment->shape.height - camera.offset.y;
                    }

                    UpdateCommentShapes(tempComment);

                    if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
                    {
                        // Save temporal comment values
                        Rectangle tempRec = { tempComment->shape.x, tempComment->shape.y, tempComment->shape.width, tempComment->shape.height };
                        DestroyComment(tempComment);

                        // Reset comment state
                        commentState = 0;

                        if (tempRec.width >= 0 && tempRec.height >= 0)
                        {
                            // Create final comment
                            FComment temp = CreateComment();
                            temp->shape = tempRec;
                            
                            UpdateCommentShapes(temp);
                        }
                        else TraceLogFNode(false, "comment have not been created because its width or height are has a negative value");
                    }
                }
            } break;
            default: break;
        }
    }
}

// Check comment drag input
void UpdateCommentsDrag()
{
    if ((selectedComment == -1) && (lineState == 0) && (commentState == 0) && (selectedNode == -1))
    {
        if (!IsKeyDown(KEY_LEFT_ALT))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                for (int i = commentsCount - 1; i >= 0; i--)
                {
                    if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->shape, camera)))
                    {
                        selectedComment = comments[i]->id;
                        currentOffset = (Vector2){ mousePosition.x - comments[i]->shape.x, mousePosition.y - comments[i]->shape.y };

                        for (int k = 0; k < nodesCount; k++)
                        {
                            if (CheckCollisionRecs(CameraToViewRec(comments[i]->shape, camera), CameraToViewRec(nodes[k]->shape, camera)))
                            {
                                selectedCommentNodes[selectedCommentNodesCount] = nodes[k]->id;
                                selectedCommentNodesCount++;

                                if (selectedCommentNodesCount > MAX_NODES) break;
                            }
                        }

                        break;
                    }
                }
            }
        }
        else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            for (int i = commentsCount - 1; i >= 0; i--)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->shape, camera)))
                {
                    DestroyComment(comments[i]);
                    break;
                }
            }
        }
    }
    else if ((selectedComment != -1) && (lineState == 0) && (commentState == 0) && (selectedNode == -1))
    {
        for (int i = 0; i < commentsCount; i++)
        {
            if (comments[i]->id == selectedComment)
            {
                comments[i]->shape.x = mousePosition.x - currentOffset.x;
                comments[i]->shape.y = mousePosition.y - currentOffset.y;

                UpdateCommentShapes(comments[i]);

                for (int k = 0; k < selectedCommentNodesCount; k++)
                {
                    for (int j = 0; j < nodesCount; j++)
                    {
                        if (nodes[j]->id == selectedCommentNodes[k])
                        {
                            nodes[j]->shape.x += mouseDelta.x;
                            nodes[j]->shape.y += mouseDelta.y;

                            UpdateNodeShapes(nodes[j]);
                            break;
                        }
                    }
                }
                break;
            }
        }

        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
        {
            selectedComment = -1;

            for (int i = 0; i < selectedCommentNodesCount; i++) selectedCommentNodes[i] = -1;
            selectedCommentNodesCount = 0;
        }
    }
}

// Check comment text edit input
void UpdateCommentsEdit()
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if ((editComment == -1) && (selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1) && (editSize == -1) && (editSizeType == -1) && (editNode == -1))
        {
            for (int i = 0; i < commentsCount; i++)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->valueShape, camera)))
                {
                    editComment = i;
                    break;
                }
            }
        }
        else if ((editComment != -1) && (selectedNode == -1) && (lineState == 0) && (commentState == 0) && (selectedComment == -1) && (editSize == -1) && (editSizeType == -1) && (editNode == -1))
        {
            bool isCurrentText = false;
            int currentEdit = editComment;
            for (int i = 0; i < commentsCount; i++)
            {
                if (comments[i]->id == editComment)
                {
                    if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->valueShape, camera)))
                    {
                        isCurrentText = true;
                        break;
                    }
                }

                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(comments[i]->valueShape, camera)))
                {
                    editComment = i;
                    break;
                }
            }

            // Reset current editing text any other text label is pressed
            if (!isCurrentText && (currentEdit == editComment)) editComment = -1;
        }
    }
}

// Update required values to created shader for geometry data calculations
void UpdateShaderData()
{
    currentTime += GetFrameTime();
    framesCounter++;
    
    // Update visor model current rotation
    modelRotation -= VISOR_MODEL_ROTATION;
    
    if (compileState >= 0)
    {
        if (framesCounter - compileFrame >= COMPILE_DURATION)
        {
            compileState = -1;
            compileFrame = 0;
        }
    }
    
    if (IsFileDropped())
    {
        int filesCount = 0;
        droppedFiles = GetDroppedFiles(&filesCount);
        char *path = droppedFiles[0];
        
        if (CheckTextureExtension(path) && (loadedFiles < MAX_TEXTURES))
        {
            if (textures[loadedFiles].id != 0) UnloadTexture(textures[loadedFiles]);
            textures[loadedFiles] = LoadTexture(path);
            
            if (shader.id > 0)
            {
                switch (loadedFiles)
                {
                    case 0: model.material.maps[MAP_ALBEDO].texture = textures[loadedFiles]; break;
                    case 1: model.material.maps[MAP_NORMAL].texture = textures[loadedFiles]; break;
                    case 2: model.material.maps[MAP_SPECULAR].texture = textures[loadedFiles]; break;
                    default: break;
                }
            }
            
            loadedFiles++;
            if (loadedFiles == MAX_TEXTURES) loadedFiles = 0;
        }
        else TraceLogFNode(false, "error when trying to import a non texture file or achieved maximum number of textures");
        
        
        filesCount = 0;
    }

    if (shader.id > 0)
    {
        // Check if view direction is used in shader
        if (viewUniform != -1)
        {
            // Convert view vector to float array and send it to shader
            Vector3 viewVector = { camera3d.position.x - camera3d.target.x, camera3d.position.y - camera3d.target.y, camera3d.position.z - camera3d.target.z };
            viewVector = FVector3Normalize(viewVector);
            float viewDir[3] = {  viewVector.x, viewVector.y, viewVector.z };
            SetShaderValue(shader, viewUniform, viewDir, 3);
        }

        // Check if model transform matrix is used in shader and send it if needed
        if (transformUniform != -1) SetShaderValueMatrix(shader, transformUniform, model.transform);

        // Check if current time is used in vertex shader
        if (timeUniformV != -1)
        {
            // Convert time value to float array and send it to shader
            float time[1] = { currentTime };
            SetShaderValue(shader, timeUniformV, time, 1);
        }
 
        // Check if current time is used in fragment shader
        if (timeUniformF != -1)
        {
            // Convert time value to float array and send it to shader
            float time[1] = { currentTime };
            SetShaderValue(shader, timeUniformF, time, 1);
        }

        float resolution[2] = { (fullVisor ? screenSize.x : (screenSize.x/4)), (fullVisor ? screenSize.y : (screenSize.y/4)) };
        SetShaderValue(fxaa, fxaaUniform, resolution, 2);
    }
}

// Compiles all node structure to create the GLSL fragment shader in output folder
void CompileShader()
{
    // Reset previous compiled shader data
    if (loadedShader || (shader.id > 0)) UnloadShader(shader);
    remove(DATA_PATH);
    remove(VERTEX_PATH);
    remove(FRAGMENT_PATH);
    model.material.shader = GetShaderDefault();
    for (int i = 0; i < MAX_TEXTURES; i++) usedUnits[i] = false;
    compileState = -1;
    compileFrame = 0;
    viewUniform = -1;
    transformUniform = -1;
    timeUniformV = -1;
    timeUniformF = -1;

    // Open shader data file
    FILE *dataFile = fopen(DATA_PATH, "w");
    if (dataFile != NULL)
    {
        // Nodes data reading
        int count = 0;
        for (int i = 0; i < MAX_NODES; i++)
        {
            for (int k = 0; k < nodesCount; k++)
            {
                if (nodes[k]->id == i)
                {
                    float type = (float)nodes[k]->type;
                    float property = (float)nodes[k]->property;
                    float inputs[MAX_INPUTS] = { (float)nodes[k]->inputs[0], (float)nodes[k]->inputs[1], (float)nodes[k]->inputs[2], (float)nodes[k]->inputs[3] };
                    float inputsCount = (float)nodes[k]->inputsCount;
                    float inputsLimit = (float)nodes[k]->inputsLimit;
                    float dataCount = (float)nodes[k]->output.dataCount;
                    float data[MAX_VALUES] = { nodes[k]->output.data[0].value, nodes[k]->output.data[1].value, nodes[k]->output.data[2].value, nodes[k]->output.data[3].value, nodes[k]->output.data[4].value,
                    nodes[k]->output.data[5].value, nodes[k]->output.data[6].value, nodes[k]->output.data[7].value, nodes[k]->output.data[8].value, nodes[k]->output.data[9].value, nodes[k]->output.data[10].value,
                    nodes[k]->output.data[11].value, nodes[k]->output.data[12].value, nodes[k]->output.data[13].value, nodes[k]->output.data[14].value, nodes[k]->output.data[15].value };
                    float shapeX = (float)nodes[k]->shape.x;
                    float shapeY = (float)nodes[k]->shape.y;

                    fprintf(dataFile, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,\n", type, property,
                    inputs[0], inputs[1], inputs[2], inputs[3], inputsCount, inputsLimit, dataCount, data[0], data[1], data[2],
                    data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14],
                    data[15], shapeX, shapeY);
                    
                    count++;
                    break;
                }
            }

            if (count == nodesCount) break;
        }

        // Lines data reading
        count = 0;

        for (int i = 0; i < MAX_NODES; i++)
        {
            for (int k = 0; k < nodesCount; k++)
            {
                if (lines[k]->id == i)
                {
                    fprintf(dataFile, "?%i?%i\n", lines[k]->from, lines[k]->to);
                    
                    count++;
                    break;
                }
            }

            if (count == linesCount) break;
        }

        fclose(dataFile);
    }
    else TraceLogFNode(true, "error when trying to open and write in data file");

    // Open vertex shader file to write data
    FILE *vertexFile = fopen(VERTEX_PATH, "w");
    if (vertexFile != NULL)
    {
        const char vCredits[] = "// Shader created with FNode 1.0 - Credits: Victor Fisac\n\n";
        fprintf(vertexFile, vCredits);

        // Vertex shader definition to embed, no external file required
        switch (version)
        {
            case GLSL_330:
            {
                const char vHeader[] = "#version 330\n\n";
                fprintf(vertexFile, vHeader);
            } break;
            case GLSL_100:
            {
                const char vHeader[] = "#version 100\n\n";
                fprintf(vertexFile, vHeader);
            } break;
            default: break;
        }

        switch (version)
        {
            case GLSL_330:
            {
                const char vIn[] = 
                "in vec3 vertexPosition;\n"
                "in vec3 vertexNormal;\n"
                "in vec2 vertexTexCoord;\n"
                "in vec4 vertexColor;\n\n";
                fprintf(vertexFile, vIn);

                const char vOut[] = 
                "out vec3 fragPosition;\n"
                "out vec3 fragNormal;\n"
                "out vec2 fragTexCoord;\n"
                "out vec4 fragColor;\n\n";
                fprintf(vertexFile, vOut);
            } break;
            case GLSL_100:
            {
                const char vIn[] = 
                "attribute vec3 vertexPosition;\n"
                "attribute vec3 vertexNormal;\n"
                "attribute vec2 vertexTexCoord;\n"
                "attribute vec4 vertexColor;\n\n";
                fprintf(vertexFile, vIn);

                const char vOut[] = 
                "varying vec3 fragPosition;\n"
                "varying vec3 fragNormal;\n"
                "varying vec2 fragTexCoord;\n"
                "varying vec4 fragColor;\n\n";
                fprintf(vertexFile, vOut);
            } break;
            default: break;
        }

        const char vUniforms[] = 
        "uniform mat4 mvp;\n"
        "uniform float vertCurrentTime;\n\n";
        fprintf(vertexFile, vUniforms);
        
        fprintf(vertexFile, "// Constant and uniform values\n");
        int index = GetNodeIndex(nodes[0]->inputs[0]);
        CheckConstant(nodes[index], vertexFile);

        const char vMain[] = 
        "\nvoid main()\n"
        "{\n"
        "    fragPosition = vertexPosition;\n"
        "    fragNormal = vertexNormal;\n"
        "    fragTexCoord = vertexTexCoord;\n"
        "    fragColor = vertexColor;\n\n";
        fprintf(vertexFile, vMain);

        CompileNode(nodes[index], vertexFile, false);

        switch (nodes[index]->output.dataCount)
        {
            case 1: fprintf(vertexFile, "\n    gl_Position = vec4(node_%02i, node_%02i, node_%02i, 1.0);\n}", nodes[0]->inputs[0], nodes[0]->inputs[0], nodes[0]->inputs[0]); break;
            case 2: fprintf(vertexFile, "\n    gl_Position = vec4(node_%02i.xy, 0.0, 1.0);\n}", nodes[0]->inputs[0]); break;
            case 3: fprintf(vertexFile, "\n    gl_Position = vec4(node_%02i.xyz, 1.0);\n}", nodes[0]->inputs[0]); break;
            case 4: fprintf(vertexFile, "\n    gl_Position = node_%02i;\n}", nodes[0]->inputs[0]); break;
            case 16: fprintf(vertexFile, "\n    gl_Position = node_%02i;\n}", nodes[0]->inputs[0]); break;
            default: break;
        }

        fclose(vertexFile);
    }
    else TraceLogFNode(true, "error when trying to open and write in vertex shader file");

    // Open fragment shader file to write data
    FILE *fragmentFile = fopen(FRAGMENT_PATH, "w");
    if (fragmentFile != NULL)
    {
        const char vCredits[] = "// Shader created with FNode 1.0 - Credits: Victor Fisac\n\n";
        fprintf(vertexFile, vCredits);

        // Fragment shader definition to embed, no external file required
        switch (version)
        {
            case GLSL_330:
            {
                const char fHeader[] = "#version 330\n\n";
                fprintf(vertexFile, fHeader);
            } break;
            case GLSL_100:
            {
                const char fHeader[] = "#version 100\n"
                "precision mediump float;\n\n";
                fprintf(vertexFile, fHeader);
            } break;
            default: break;
        }

        fprintf(fragmentFile, "// Input attributes\n");
        
        switch (version)
        {
            case GLSL_330:
            {
                const char fIn[] = 
                "in vec3 fragPosition;\n"
                "in vec3 fragNormal;\n"
                "in vec2 fragTexCoord;\n"
                "in vec4 fragColor;\n\n";
                fprintf(fragmentFile, fIn);
            } break;
            case GLSL_100:
            {
                const char fIn[] = 
                "varying vec3 fragPosition;\n"
                "varying vec3 fragNormal;\n"
                "varying vec2 fragTexCoord;\n"
                "varying vec4 fragColor;\n\n";
                fprintf(fragmentFile, fIn);
            } break;
            default: break;
        }

        fprintf(fragmentFile, "// Uniform attributes\n");
        const char fUniforms[] = 
        "uniform vec3 viewDirection;\n"
        "uniform mat4 modelMatrix;\n"
        "uniform float fragCurrentTime;\n\n";
        fprintf(fragmentFile, fUniforms);

        if (version == GLSL_330)
        {
            fprintf(fragmentFile, "// Output attributes\n");
            const char fOut[] = 
            "out vec4 finalColor;\n\n";
            fprintf(fragmentFile, fOut);
        }

        fprintf(fragmentFile, "// Constant and uniform values\n");
        int index = GetNodeIndex(nodes[1]->inputs[0]);
        CheckConstant(nodes[index], fragmentFile);

        const char fMain[] = 
        "\nvoid main()\n"
        "{\n";
        fprintf(fragmentFile, fMain);

        CompileNode(nodes[index], fragmentFile, true);

        switch (version)
        {
            case GLSL_330:
            {
                switch (nodes[index]->output.dataCount)
                {
                    case 1: fprintf(fragmentFile, "\n    finalColor = vec4(node_%02i, node_%02i, node_%02i, 1.0);\n}", nodes[1]->inputs[0], nodes[1]->inputs[0], nodes[1]->inputs[0]); break;
                    case 2: fprintf(fragmentFile, "\n    finalColor = vec4(node_%02i.xy, 0.0, 1.0);\n}", nodes[1]->inputs[0]); break;
                    case 3: fprintf(fragmentFile, "\n    finalColor = vec4(node_%02i.xyz, 1.0);\n}", nodes[1]->inputs[0]); break;
                    case 4: fprintf(fragmentFile, "\n    finalColor = node_%02i;\n}", nodes[1]->inputs[0]); break;
                    default: break;
                }
            } break;
            case GLSL_100:
            {
                switch (nodes[index]->output.dataCount)
                {
                    case 1: fprintf(fragmentFile, "\n    gl_FragColor = vec4(node_%02i, node_%02i, node_%02i, 1.0);\n}", nodes[1]->inputs[0], nodes[1]->inputs[0], nodes[1]->inputs[0]); break;
                    case 2: fprintf(fragmentFile, "\n    gl_FragColor = vec4(node_%02i.xy, 0.0, 1.0);\n}", nodes[1]->inputs[0]); break;
                    case 3: fprintf(fragmentFile, "\n    gl_FragColor = vec4(node_%02i.xyz, 1.0);\n}", nodes[1]->inputs[0]); break;
                    case 4: fprintf(fragmentFile, "\n    gl_FragColor = node_%02i;\n}", nodes[1]->inputs[0]); break;
                    default: break;
                }
            } break;
            default: break;
        }

        fclose(fragmentFile);
    }
    else TraceLogFNode(true, "error when trying to open and write in vertex shader file");

    compileState = 1;
    compileFrame = framesCounter;

    CheckPreviousShader(false);
}

// Check nodes searching for constant values to define them in shaders
void CheckConstant(FNode node, FILE *file)
{
    switch (node->type)
    {
        case FNODE_PI: fprintf(file, "const float node_%02i = 3.14159265358979323846;\n", node->id); break;
        case FNODE_E: fprintf(file, "const float node_%02i = 2.71828182845904523536;\n", node->id); break;
        case FNODE_VALUE:
        {
            if (node->property)
            {
                const char fConstantVector4[] = "uniform float node_%02i;\n";
                fprintf(file, fConstantVector4, node->id);
            }
            else
            {
                const char fConstantValue[] = "const float node_%02i = %.3f;\n";
                fprintf(file, fConstantValue, node->id, node->output.data[0].value);
            }
        } break;
        case FNODE_VECTOR2:
        {
            const char fConstantVector2[] = "const vec2 node_%02i = vec2(%.3f, %.3f);\n";
            fprintf(file, fConstantVector2, node->id, node->output.data[0].value, node->output.data[1].value);
        } break;
        case FNODE_VECTOR3:
        {
            const char fConstantVector3[] = "const vec3 node_%02i = vec3(%.3f, %.3f, %.3f);\n";
            fprintf(file, fConstantVector3, node->id, node->output.data[0].value, node->output.data[1].value, node->output.data[2].value);
        } break;
        case FNODE_VECTOR4:
        {
            if (node->property)
            {
                const char fConstantVector4[] = "uniform vec4 node_%02i;\n";
                fprintf(file, fConstantVector4, node->id);
            }
            else
            {
                const char fConstantVector4[] = "const vec4 node_%02i = vec4(%.3f, %.3f, %.3f, %.3f);\n";
                fprintf(file, fConstantVector4, node->id, node->output.data[0].value, node->output.data[1].value, node->output.data[2].value, node->output.data[3].value);
            }
        } break;
        case FNODE_SAMPLER2D:
        {
            char fConstantSampler[32] = { '\0' };
            int index = GetNodeIndex(node->inputs[0]);
            sprintf(fConstantSampler, "uniform sampler2D texture%i;\n", (int)nodes[index]->output.data[0].value);
            if (!usedUnits[(int)nodes[index]->output.data[0].value])
            {
                fprintf(file, fConstantSampler);
                usedUnits[(int)nodes[index]->output.data[0].value] = true;
            }
        } break;
        default:
        {
            for (int i = 0; i < node->inputsCount; i++)
            {
                int index = GetNodeIndex(node->inputs[i]);
                CheckConstant(nodes[index], file);
            }
        } break;
    }
}

// Compiles a specific node checking its inputs and writing current node operation in shader
void CompileNode(FNode node, FILE *file, bool fragment)
{
    // Check if current node is an operator
    if ((node->inputsCount > 0) || ((node->type < FNODE_MATRIX) && (node->type > FNODE_E)) || (node->type == FNODE_SAMPLER2D))
    {
        // Check for operator nodes in inputs to compile them first
        for (int i = 0; i < node->inputsCount; i++)
        {
            int index = GetNodeIndex(node->inputs[i]);
            if (nodes[index]->type > FNODE_VECTOR4 || ((nodes[index]->type < FNODE_MATRIX) && (nodes[index]->type > FNODE_E))) CompileNode(nodes[index], file, fragment);
        }

        // Store variable declaration into a string
        char check[16] = { '\0' };
        switch (node->output.dataCount)
        {
            case 1: sprintf(check, "float node_%02i", node->id); break;
            case 2: sprintf(check, "vec2 node_%02i", node->id); break;
            case 3: sprintf(check, "vec3 node_%02i", node->id); break;
            case 4: sprintf(check, "vec4 node_%02i", node->id); break;
            case 16: sprintf(check, "mat4 node_%02i", node->id); break;
            default: break;
        }

        // Check if current node is previously defined and declared
        if (!FSearch(FRAGMENT_PATH, check))
        {
            // Variable definition based on current node output data count
            char body[4096] = { '\0' };
            char definition[32] = { '\0' };
            switch (node->output.dataCount)
            {
                case 1: sprintf(definition, "    float node_%02i = ", node->id); break;
                case 2: sprintf(definition, "    vec2 node_%02i = ", node->id); break;
                case 3: sprintf(definition, "    vec3 node_%02i = ", node->id); break;
                case 4: sprintf(definition, "    vec4 node_%02i = ", node->id); break;
                case 16: sprintf(definition, "    mat4 node_%02i = ", node->id); break;
                default: break;
            }
            strcat(body, definition);

            if (((node->type < FNODE_MATRIX) && (node->type > FNODE_E)) || (node->type == FNODE_SAMPLER2D))
            {
                switch (node->type)
                {
                    case FNODE_TIME:
                    {
                        if (fragment) strcat(body, "fragCurrentTime;\n");
                        else strcat(body, "vertCurrentTime;\n");
                    } break;
                    case FNODE_VERTEXPOSITION:
                    {
                        if (fragment) strcat(body, "fragPosition;\n");
                        else strcat(body, "vertexPosition;\n");
                    } break;
                    case FNODE_VERTEXNORMAL:
                    {
                        if (fragment) strcat(body, "fragNormal;\n");
                        else strcat(body, "vertexNormal;\n");
                    } break;
                    case FNODE_FRESNEL: strcat(body, "1.0 - dot(fragNormal, viewDirection);\n"); break;
                    case FNODE_VIEWDIRECTION: strcat(body, "viewDirection;\n"); break;
                    case FNODE_MVP: strcat(body, "mvp;\n"); break;
                    case FNODE_SAMPLER2D:
                    {
                        char test[128] = { '\0' };

                        int indexA = GetNodeIndex(node->inputs[0]);
                        int indexB = GetNodeIndex(node->inputs[1]);
                        
                        switch (version)
                        {
                            case GLSL_330:
                            {
                                switch ((int)nodes[indexB]->output.data[0].value)
                                {
                                    case 0: sprintf(test, "texture(texture%i, fragTexCoord);\n    if (node_%02i.a == 0.0) discard;\n", (int)nodes[indexA]->output.data[0].value, node->id); break;
                                    case 1: sprintf(test, "texture(texture%i, fragTexCoord).rgb;\n", (int)nodes[indexA]->output.data[0].value); break;
                                    case 2: sprintf(test, "texture(texture%i, fragTexCoord).r;\n", (int)nodes[indexA]->output.data[0].value); break;
                                    case 3: sprintf(test, "texture(texture%i, fragTexCoord).g;\n", (int)nodes[indexA]->output.data[0].value); break;
                                    case 4: sprintf(test, "texture(texture%i, fragTexCoord).b;\n", (int)nodes[indexA]->output.data[0].value); break;
                                    case 5: sprintf(test, "texture(texture%i, fragTexCoord).a;\n    if (node_%02i == 0.0) discard;\n", (int)nodes[indexA]->output.data[0].value, node->id); break;
                                    default: sprintf(test, "texture(texture%i, fragTexCoord);\n", (int)nodes[indexA]->output.data[0].value); break;
                                }
                            } break;
                            case GLSL_100:
                            {
                                switch ((int)nodes[indexB]->output.data[0].value)
                                {
                                    case 0: sprintf(test, "texture2D(texture%i, fragTexCoord);\n    if (node_%02i.a == 0.0) discard;\n", (int)nodes[indexA]->output.data[0].value, node->id); break;
                                    case 1: sprintf(test, "texture2D(texture%i, fragTexCoord).rgb;\n", (int)nodes[indexA]->output.data[0].value); break;
                                    case 2: sprintf(test, "texture2D(texture%i, fragTexCoord).r;\n", (int)nodes[indexA]->output.data[0].value); break;
                                    case 3: sprintf(test, "texture2D(texture%i, fragTexCoord).g;\n", (int)nodes[indexA]->output.data[0].value); break;
                                    case 4: sprintf(test, "texture2D(texture%i, fragTexCoord).b;\n", (int)nodes[indexA]->output.data[0].value); break;
                                    case 5: sprintf(test, "texture2D(texture%i, fragTexCoord).a;\n    if (node_%02i == 0.0) discard;\n", (int)nodes[indexA]->output.data[0].value, node->id); break;
                                    default: sprintf(test, "texture2D(texture%i, fragTexCoord);\n", (int)nodes[indexA]->output.data[0].value); break;
                                }
                            } break;
                            default: break;
                        }

                        strcat(body, test);
                        break;
                    }
                    default: break;
                }
            }
            else if ((node->type >= FNODE_ADD && node->type <= FNODE_DIVIDE) || node->type == FNODE_MULTIPLYMATRIX)
            {
                // Operate with each input node
                for (int i = 0; i < node->inputsCount; i++)
                {
                    char temp[32] = { '\0' };
                    if ((i+1) == node->inputsCount) sprintf(temp, "node_%02i;\n", node->inputs[i]);
                    else
                    {
                        int nextIndex = GetNodeIndex(node->inputs[i+1]);
                        if ((nodes[nextIndex]->output.dataCount - 1) == node->output.dataCount)
                        {
                            switch (node->output.dataCount)
                            {
                                case 1: sprintf(temp, "vec2(node_%02i, 0.0);\n", node->inputs[i]);
                                case 2: sprintf(temp, "vec3(node_%02i, 0.0);\n", node->inputs[i]);
                                case 3: sprintf(temp, "vec4(node_%02i, 0.0);\n", node->inputs[i]);
                                default: break;
                            }
                        }
                        else sprintf(temp, "node_%02i", node->inputs[i]);
                        switch (node->type)
                        {
                            case FNODE_ADD: strcat(temp, " + "); break;
                            case FNODE_SUBTRACT: strcat(temp, " - "); break;
                            case FNODE_MULTIPLYMATRIX:
                            case FNODE_MULTIPLY: strcat(temp, "*"); break;
                            case FNODE_DIVIDE: strcat(temp, "/"); break;
                            default: break;
                        }
                    }

                    strcat(body, temp);
                }
            }
            else if (node->type >= FNODE_APPEND)
            {
                char temp[128] = { '\0' };
                switch (node->type)
                {
                    case FNODE_APPEND:
                    {
                        switch (node->output.dataCount)
                        {
                            case 2: strcat(temp, "vec2("); break;
                            case 3: strcat(temp, "vec3("); break;
                            case 4: strcat(temp, "vec4("); break;
                            default: break;
                        }

                        for (int i = 0; i < node->inputsCount; i++)
                        {
                            char append[32] = { '\0' };
                            if ((i + 1) == node->inputsCount) sprintf(append, "node_%02i);\n", node->inputs[i]);
                            else sprintf(append, "node_%02i, ", node->inputs[i]);
                            
                            strcat(temp, append);
                        }
                    } break;
                    case FNODE_ONEMINUS: sprintf(temp, "(1.0 - node_%02i);\n", node->inputs[0]); break;
                    case FNODE_ABS: sprintf(temp, "abs(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_COS: sprintf(temp, "cos(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_SIN: sprintf(temp, "sin(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_TAN: sprintf(temp, "tan(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_DEG2RAD: sprintf(temp, "node_%02i*(3.14159265358979323846/180.0);\n", node->inputs[0]); break;
                    case FNODE_RAD2DEG: sprintf(temp, "node_%02i*(180.0/3.14159265358979323846);\n", node->inputs[0]); break;
                    case FNODE_NORMALIZE: sprintf(temp, "normalize(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_NEGATE: sprintf(temp, "node_%02i*-1;\n", node->inputs[0]); break;
                    case FNODE_RECIPROCAL: sprintf(temp, "1.0/node_%02i;\n", node->inputs[0]); break;
                    case FNODE_SQRT: sprintf(temp, "sqrt(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_TRUNC: sprintf(temp, "trunc(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_ROUND: sprintf(temp, "round(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_VERTEXCOLOR:
                    {
                        int index = GetNodeIndex(node->inputs[0]);
                        switch ((int)nodes[index]->output.data[0].value)
                        {
                            case 0: sprintf(temp, (fragment ? "fragColor;\n" : "vertexColor;\n")); break;
                            case 1: sprintf(temp, (fragment ? "fragColor.rgb;\n" : "vertexColor.rgb;\n")); break;
                            case 2: sprintf(temp, (fragment ? "fragColor.r;\n" : "vertexColor.r;\n")); break;
                            case 3: sprintf(temp, (fragment ? "fragColor.g;\n" : "vertexColor.g;\n")); break;
                            case 4: sprintf(temp, (fragment ? "fragColor.b;\n" : "vertexColor.b;\n")); break;
                            case 5: sprintf(temp, (fragment ? "fragColor.a;\n" : "vertexColor.a;\n")); break;
                            default: break;
                        }
                    } break;
                    case FNODE_CEIL: sprintf(temp, "ceil(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_CLAMP01: sprintf(temp, "clamp(node_%02i, 0.0, 1.0);\n", node->inputs[0]); break;
                    case FNODE_EXP2: sprintf(temp, "exp2(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_POWER: sprintf(temp, "pow(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_STEP: sprintf(temp, "((node_%02i <= node_%02i) ? 1.0 : 0.0);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_POSTERIZE: sprintf(temp, "floor(node_%02i*node_%02i)/node_%02i;\n", node->inputs[0], node->inputs[1], node->inputs[1]); break;
                    case FNODE_MAX: sprintf(temp, "max(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[1]); break;
                    case FNODE_MIN: sprintf(temp, "min(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[1]); break;
                    case FNODE_LERP: sprintf(temp, "mix(node_%02i, node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[2]); break;
                    case FNODE_SMOOTHSTEP: sprintf(temp, "smoothstep(node_%02i, node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1], node->inputs[2]); break;
                    case FNODE_CROSSPRODUCT: sprintf(temp, "cross(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_DESATURATE:
                    {
                        switch (node->output.dataCount)
                        {
                            case 1: sprintf(temp, "mix(node_%02i, 0.3, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                            case 2: sprintf(temp, "vec2(mix(node_%02i.x, vec2(0.3, 0.59), node_%02i));\n", node->inputs[0], node->inputs[1]); break;
                            case 3: sprintf(temp, "vec3(mix(node_%02i.xyz, vec3(0.3, 0.59, 0.11), node_%02i));\n", node->inputs[0], node->inputs[1]); break;
                            case 4: sprintf(temp, "vec4(mix(node_%02i.xyz, vec3(0.3, 0.59, 0.11), node_%02i), 1.0);\n", node->inputs[0], node->inputs[1]); break;
                            default: break;
                        }
                    } break;
                    case FNODE_DISTANCE: sprintf(temp, "distance(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_DOTPRODUCT: sprintf(temp, "dot(node_%02i, node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    case FNODE_LENGTH: sprintf(temp, "length(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_TRANSPOSE: sprintf(temp, "transpose(node_%02i);\n", node->inputs[0]); break;
                    case FNODE_PROJECTION:
                    {
                        switch (node->output.dataCount)
                        {
                            case 2: sprintf(temp, "vec2(dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y);\n", 
                            node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            case 3: sprintf(temp, "vec3(dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.z);\n", 
                            node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            case 4: sprintf(temp, "vec4(dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.z, dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.w);\n", 
                            node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            default: break;
                        }
                    }
                    case FNODE_REJECTION:
                    {
                        switch (node->output.dataCount)
                        {
                            case 2: sprintf(temp, "vec2(node_%02i.x - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, node_%02i.y - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y);\n", 
                            node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            case 3: sprintf(temp, "vec3(node_%02i.x - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, node_%02i.y - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y, node_%02i.z - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.z);\n", 
                            node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            case 4: sprintf(temp, "vec4(node_%02i.x - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.x, node_%02i.y - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.y, node_%02i.z - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.z, node_%02i.w - dot(node_%02i, node_%02i)/dot(node_%02i, node_%02i)*node_%02i.w);\n", 
                            node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[0], node->inputs[0], node->inputs[1], node->inputs[1], node->inputs[1], node->inputs[1]); break;
                            default: break;
                        }
                    } break;
                    case FNODE_HALFDIRECTION: sprintf(temp, "normalize(node_%02i + node_%02i);\n", node->inputs[0], node->inputs[1]); break;
                    default: break;
                }

                strcat(body, temp);
            }

            // Write current node string to shader file
            fprintf(file, body);
        }
    }
}

// Aligns all created nodes
void AlignAllNodes()
{
    for (int i = 0; i < nodesCount; i++)
    {
        AlignNode(nodes[i]);
        UpdateNodeShapes(nodes[i]);
    }
}

// Destroys all unused nodes
void ClearUnusedNodes()
{
    for (int i = nodesCount - 1; i >= 0; i--)
    {
        bool used = (nodes[i]->type >= FNODE_VERTEX);

        if (!used)
        {
            for (int k = 0; k < linesCount; k++)
            {
                if ((nodes[i]->id == lines[k]->from) || (nodes[i]->id == lines[k]->to))
                {
                    used = true;
                    break;
                }
            }
        }

        if (!used) DestroyNode(nodes[i]);
    }

    TraceLogFNode(false, "all unused nodes have been deleted [USED RAM: %i bytes]", usedMemory);
}

// Destroys all created nodes and its linked lines
void ClearGraph()
{
    for (int i = nodesCount - 1; i >= 0; i--)
    {
        if (nodes[i]->type < FNODE_VERTEX) DestroyNode(nodes[i]);
    }
    for (int i = commentsCount - 1; i >= 0; i--) DestroyComment(comments[i]);

    TraceLogFNode(false, "all nodes have been deleted [USED RAM: %i bytes]", usedMemory);
}

// Draw canvas space to create nodes
void DrawCanvas()
{
    BeginShaderMode(GetShaderDefault());
    
    // Draw background title and credits
    DrawText("FNODE 1.0", (canvasSize.x - MeasureText("FNODE 1.0", 120))/2, canvasSize.y/2 - 60, 120, Fade(LIGHTGRAY, UI_GRID_ALPHA*2));
    DrawText("VICTOR FISAC", (canvasSize.x - MeasureText("VICTOR FISAC", 40))/2, canvasSize.y*0.65f - 20, 40, Fade(LIGHTGRAY, UI_GRID_ALPHA*2));

    Begin2dMode(camera);

        DrawCanvasGrid(UI_GRID_COUNT);

        // Draw all created comments, lines and nodes
        for (int i = 0; i < commentsCount; i++) DrawComment(comments[i]);
        for (int i = 0; i < nodesCount; i++) DrawNode(nodes[i]);
        for (int i = 0; i < linesCount; i++) DrawNodeLine(lines[i]);

    End2dMode();
    
    EndShaderMode();
}

// Draw canvas grid with a specific number of divisions for horizontal and vertical lines
void DrawCanvasGrid(int divisions)
{
    int spacing = 0;
    for (int i = 0; i < divisions; i++)
    {
        for (int k = 0; k < 5; k++)
        {
            DrawRectangle(-(divisions/2*UI_GRID_SPACING*5) + spacing, -100000, 1, 200000, ((k == 0) ? Fade(BLACK, UI_GRID_ALPHA*2) : Fade(GRAY, UI_GRID_ALPHA)));
            spacing += UI_GRID_SPACING;
        }
    }

    spacing = 0;
    for (int i = 0; i < divisions; i++)
    {
        for (int k = 0; k < 5; k++)
        {
            DrawRectangle(-100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 200000, 1, ((k == 0) ? Fade(BLACK, UI_GRID_ALPHA*2) : Fade(GRAY, UI_GRID_ALPHA)));
            spacing += UI_GRID_SPACING;
        }
    }
}

// Draws a visor with default model rotating and current shader
void DrawVisor()
{
    BeginTextureMode(visorTarget);

        DrawRectangle(0, 0, screenSize.x, screenSize.y, GRAY);

        // Draw background title and credits
        DrawText("FNODE 1.0", (canvasSize.x - MeasureText("FNODE 1.0", 120))/2, canvasSize.y/2 - 60, 120, Fade(LIGHTGRAY, UI_GRID_ALPHA*2));
        DrawText("VICTOR FISAC", (canvasSize.x - MeasureText("VICTOR FISAC", 40))/2, canvasSize.y*0.65f - 20, 40, Fade(LIGHTGRAY, UI_GRID_ALPHA*2));
        
        BeginShaderMode(model.material.shader);

        Begin3dMode(camera3d);

            DrawModelEx(model, (Vector3){ 0.0f, -1.0f, 0.0f }, (Vector3){ 0, 1, 0 }, modelRotation, (Vector3){ VISOR_MODEL_SCALE, VISOR_MODEL_SCALE, VISOR_MODEL_SCALE }, RED);

        End3dMode();
        
        EndShaderMode();

    EndTextureMode();

    Rectangle visor = { canvasSize.x - visorTarget.texture.width - UI_PADDING, screenSize.y - visorTarget.texture.height - UI_PADDING, visorTarget.texture.width, visorTarget.texture.height };
    
    if (fullVisor)
    {
        visor.x = 0;
        visor.y = 0;
        visor.width = screenSize.x;
        visor.height = screenSize.y;
    }

    DrawRectangle(visor.x - VISOR_BORDER, visor.y - VISOR_BORDER, visor.width + VISOR_BORDER*2, visor.height + VISOR_BORDER*2, BLACK);

    BeginShaderMode(fxaa);

        DrawTexturePro(visorTarget.texture, (Rectangle){ 0, 0, visorTarget.texture.width, -visorTarget.texture.height }, visor, (Vector2){ 0, 0 }, 0.0f, WHITE);

    EndShaderMode();
}

// Draw interface to create nodes
void DrawInterface()
{
    if (help)
    {
        DrawRectangle(5, 5, 450, 200, (Color){ 150, 150, 150, 150 });
        DrawRectangleLines(5, 5, 450, 200, BLACK);
        
        DrawText("Welcome to FNode, adventurer!", 15, 15, 10, BLACK);
        DrawText("Controls:", 15, 35, 10, BLACK);
        DrawText("- Drag Canvas/Node: LEFT MOUSE BUTTON", 35, 55, 10, BLACK);
        DrawText("- Drag Comment: LEFT ALT + LEFT MOUSE BUTTON (DRAG IN COMMENT)", 35, 75, 10, BLACK);
        DrawText("- Create Comment: LEFT ALT + LEFT MOUSE BUTTON (DRAG IN CANVAS)", 35, 95, 10, BLACK);
        DrawText("- Delete Node/Line/Comment: RIGHT MOUSE BUTTON", 35, 115, 10, BLACK);
        DrawText("- Link: LEFT MOUSE BUTTON (INPUT/OUTPUT RECTANGLES)", 35, 135, 10, BLACK);
        DrawText("- Preview: RIGHT ALT BUTTON", 35, 155, 10, BLACK);
        DrawText("Credits: Victor Fisac [www.victorfisac.com]", 15, 180, 10, BLACK);
        
        Rectangle iconRect = (Rectangle){ 450 - iconTex.width/4, 200 - iconTex.height/4, iconTex.width/4, iconTex.height/4 };
        DrawTexturePro(iconTex, (Rectangle){ 0, 0, iconTex.width, iconTex.height }, iconRect, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    }
    else DrawText("Press 'H' to display HELP menu", 10, 10, 10, BLACK);
    
    // Draw interface background
    DrawRectangleRec((Rectangle){ canvasSize.x, 0.0f, screenSize.x - canvasSize.x, screenSize.y }, DARKGRAY);

    // Draw interface main buttons
    if (FButton((Rectangle){ UI_PADDING, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Compile")) CompileShader(); menuOffset = 1;
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*menuOffset, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Clear Graph")) ClearGraph();
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*menuOffset, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Align Nodes")) AlignAllNodes();
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*menuOffset, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Clear Unused")) ClearUnusedNodes();
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*menuOffset, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Settings"))
    {
        settings = true;
        interact = false;
    }

    // Draw interface nodes buttons
    DrawText("Constant Vectors", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Constant Vectors", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 - menuScroll, 10, WHITE); menuOffset = 1;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Value")) CreateNodeValue((float)GetRandomValue(-11, 10));
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector 2")) CreateNodeVector2((Vector2){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector 3")) CreateNodeVector3((Vector3){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector 4")) CreateNodeVector4((Vector4){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Matrix 4x4")) CreateNodeMatrix(FMatrixIdentity());

    DrawText("Properties", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Properties", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Value")) CreateNodeProperty(FNODE_VALUE, "Value", 1, 0);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Color")) CreateNodeProperty(FNODE_VECTOR4, "Color", 4, 0);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Sampler 2D")) CreateNodeProperty(FNODE_SAMPLER2D, "Sampler 2D", 4, 2);
    
    DrawText("Arithmetic", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Arithmetic", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Add")) CreateNodeOperator(FNODE_ADD, "Add", MAX_INPUTS);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Subtract")) CreateNodeOperator(FNODE_SUBTRACT, "Subtract", MAX_INPUTS);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Multiply")) CreateNodeOperator(FNODE_MULTIPLY, "Multiply", MAX_INPUTS);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Multiply Matrix")) CreateNodeOperator(FNODE_MULTIPLYMATRIX, "Multiply Matrix", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Divide")) CreateNodeOperator(FNODE_DIVIDE, "Divide", MAX_INPUTS);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "One Minus")) CreateNodeOperator(FNODE_ONEMINUS, "One Minus", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Abs")) CreateNodeOperator(FNODE_ABS, "Abs", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Clamp 0-1")) CreateNodeOperator(FNODE_CLAMP01, "Clamp 0-1", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Max")) CreateNodeOperator(FNODE_MAX, "Max", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Min")) CreateNodeOperator(FNODE_MIN, "Min", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Negate")) CreateNodeOperator(FNODE_NEGATE, "Negate", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Reciprocal")) CreateNodeOperator(FNODE_RECIPROCAL, "Reciprocal", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Square Root")) CreateNodeOperator(FNODE_SQRT, "Square Root", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Power")) CreateNodeOperator(FNODE_POWER, "Power", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Exp 2")) CreateNodeOperator(FNODE_EXP2, "Exp 2", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Posterize")) CreateNodeOperator(FNODE_POSTERIZE, "Posterize", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Ceil")) CreateNodeOperator(FNODE_CEIL, "Ceil", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Round")) CreateNodeOperator(FNODE_ROUND, "Round", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Trunc")) CreateNodeOperator(FNODE_TRUNC, "Trunc", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Lerp")) CreateNodeOperator(FNODE_LERP, "Lerp", 3);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Step")) CreateNodeOperator(FNODE_STEP, "Step", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "SmoothStep")) CreateNodeOperator(FNODE_SMOOTHSTEP, "SmoothStep", 3);

    DrawText("Vector Operations", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Vector Operations", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Append")) CreateNodeOperator(FNODE_APPEND, "Append", 4);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Normalize")) CreateNodeOperator(FNODE_NORMALIZE, "Normalize", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Cross Product")) CreateNodeOperator(FNODE_CROSSPRODUCT, "Cross Product", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Desaturate")) CreateNodeOperator(FNODE_DESATURATE, "Desaturate", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Distance")) CreateNodeOperator(FNODE_DISTANCE, "Distance", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Dot Product")) CreateNodeOperator(FNODE_DOTPRODUCT, "Dot Product", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Length")) CreateNodeOperator(FNODE_LENGTH, "Length", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Transpose")) CreateNodeOperator(FNODE_TRANSPOSE, "Transpose", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector Projection")) CreateNodeOperator(FNODE_PROJECTION, "Vector Projection", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vector Rejection")) CreateNodeOperator(FNODE_REJECTION, "Vector Rejection", 2);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Half Direction")) CreateNodeOperator(FNODE_HALFDIRECTION, "Half Direction", 2);

    DrawText("Geometry Data", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Geometry Data", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vertex Position")) CreateNodeUniform(FNODE_VERTEXPOSITION, "Vertex Position", 3);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Normal Direction")) CreateNodeUniform(FNODE_VERTEXNORMAL, "Normal Direction", 3);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Vertex Color")) CreateNodeOperator(FNODE_VERTEXCOLOR, "Vertex Color", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "View Direction")) CreateNodeUniform(FNODE_VIEWDIRECTION, "View Direction", 3);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Fresnel")) CreateNodeUniform(FNODE_FRESNEL, "Fresnel", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "MVP Matrix")) CreateNodeUniform(FNODE_MVP, "MVP Matrix", 16);

    DrawText("Math Constants", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Math Constants", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "PI")) CreateNodePI();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "e")) CreateNodeE();

    DrawText("Trigonometry", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Trigonometry", 10))/2 - UI_PADDING_SCROLL/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, 10, WHITE); menuOffset++;
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Current Time")) CreateNodeUniform(FNODE_TIME, "Current Time", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Cosine")) CreateNodeOperator(FNODE_COS, "Cosine", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Sine")) CreateNodeOperator(FNODE_SIN, "Sine", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Tangent")) CreateNodeOperator(FNODE_TAN, "Tangent", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Deg to Rad")) CreateNodeOperator(FNODE_DEG2RAD, "Deg to Rad", 1);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*menuOffset - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2 - UI_PADDING_SCROLL, UI_BUTTON_HEIGHT }, "Rad to Deg")) CreateNodeOperator(FNODE_RAD2DEG, "Rad to Deg", 1);

    DrawRectangle(menuScrollRec.x - 3, 2, menuScrollRec.width + 6, screenSize.y - 4, (Color){ UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, 255 });
    DrawRectangle(menuScrollRec.x - 2, menuScrollRec.y - 2, menuScrollRec.width + 4, menuScrollRec.height + 4, DARKGRAY);
    DrawRectangleRec(menuScrollRec, ((scrollState == 1) ? LIGHTGRAY : RAYWHITE));

    if (settings)
    {
        DrawRectangle(0, 0, screenSize.x, screenSize.y, (Color){ 0, 0, 0, 100 });

        #define     SETTINGS_WIDTH      300
        #define     SETTINGS_HEIGHT     175

        DrawRectangle((canvasSize.x - SETTINGS_WIDTH)/2, (canvasSize.y - SETTINGS_HEIGHT)/2, SETTINGS_WIDTH, SETTINGS_HEIGHT, LIGHTGRAY);
        DrawRectangleLines((canvasSize.x - SETTINGS_WIDTH)/2, (canvasSize.y - SETTINGS_HEIGHT)/2, SETTINGS_WIDTH, SETTINGS_HEIGHT, BLACK);

        DrawText("Settings", (canvasSize.x - SETTINGS_WIDTH)/2 + 15, (canvasSize.y - SETTINGS_HEIGHT)/2 + 15, 20, BLACK);
        DrawText("Shader version", (canvasSize.x - SETTINGS_WIDTH)/2 + 15, (canvasSize.y - SETTINGS_HEIGHT)/2 + 50, 10, BLACK);
        DrawText(((version == 0) ? "GLSL 330" : "GLSL 100"), (canvasSize.x - SETTINGS_WIDTH)/2 + SETTINGS_WIDTH/1.9f + MeasureText(((version == 0) ? "GLSL 330" : "GLSL 100"), 10)/2, (canvasSize.y - SETTINGS_HEIGHT)/2 + 50, 10, BLACK);

        if (FButton((Rectangle){ (canvasSize.x - SETTINGS_WIDTH)/2 + SETTINGS_WIDTH/2 - 20, (canvasSize.y - SETTINGS_HEIGHT)/2 + 45, 20, 20 }, "<"))
        {
            version--; 
            if (version < 0) version = 1;
        }

        if (FButton((Rectangle){ (canvasSize.x - SETTINGS_WIDTH)/2 + SETTINGS_WIDTH - 40, (canvasSize.y - SETTINGS_HEIGHT)/2 + 45, 20, 20 }, ">"))
        {
            version++; 
            if (version > 1) version = 0;
        }

        if (FButton((Rectangle){ (canvasSize.x - SETTINGS_WIDTH)/2 + SETTINGS_WIDTH/2 - 40, (canvasSize.y - SETTINGS_HEIGHT)/2 + SETTINGS_HEIGHT/2 + 45, 80, 25 }, "Close"))
        {
            settings = false;
            interact = true;
        }
        
        DrawText("Backface Culling", (canvasSize.x - SETTINGS_WIDTH)/2 + 15, (canvasSize.y - SETTINGS_HEIGHT)/2 + 85, 10, BLACK);
        
        backfaceCulling = FToggle((Rectangle){ canvasSize.x/2 + 40, canvasSize.y/2 - 7, 20, 20 }, backfaceCulling);
    }

    if (compileState >= 0)
    {
        Rectangle compileRec = { UI_PADDING, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT };
        DrawRectangleRec(compileRec, ((compileState == 1) ? Fade(GREEN, 0.5f) : Fade(RED, 0.5f)));
    }
}

// Returns the extension of a file
char *GetFileExtension(char *filename)
{
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return (dot + 1);
}

// Check filename for specific extension
bool CheckFileExtension(char *filename, char *extension)
{
    return (strcmp(GetFileExtension(filename),extension) == 0);
}

// Check filename for compatible texture extensions
bool CheckTextureExtension(char *filename)
{
    return (CheckFileExtension(filename, "jpg") || CheckFileExtension(filename, "png") || CheckFileExtension(filename, "tga") || CheckFileExtension(filename, "tiff") || CheckFileExtension(filename, "bmp"));
}

//----------------------------------------------------------------------------------
// Program
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenSize.x, screenSize.y, "FNode - Visual scripting shader editor");
    Image icon = LoadImage(WINDOW_ICON);
    SetWindowIcon(icon);
    iconTex = LoadTexture(WINDOW_ICON);

    // Load resources
    model = LoadModel(MODEL_PATH);
    visorTarget = LoadRenderTexture(screenSize.x/4, screenSize.y/4);
    fxaa = LoadShader(FXAA_VERTEX, FXAA_FRAGMENT);
    textures[0] = LoadTexture(MODEL_TEXTURE_WINDAMOUNT);
    textures[1] = LoadTexture(MODEL_TEXTURE_DIFFUSE);
    model.material.maps[MAP_ALBEDO].texture = textures[0];
    model.material.maps[MAP_NORMAL].texture = textures[1];

    // Initialize values
    camera = (Camera2D){ (Vector2){ 0, 0 }, (Vector2){ screenSize.x/2, screenSize.y/2 }, 0.0f, 1.0f };
    canvasSize = (Vector2){ screenSize.x*0.85f, screenSize.y };
    camera3d = (Camera){{ 4.0f, 2.0f, 4.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };
    SetCameraMode(camera3d, CAMERA_FREE);
    menuScrollRec = (Rectangle){ screenSize.x - 17, 5, 9, 30 };

    // Initialize shaders values
    fxaaUniform = GetShaderLocation(fxaa, FXAA_SCREENSIZE_UNIFORM);

    InitFNode();
    CheckPreviousShader(true);
    UpdateCamera(&camera3d);

    SetTargetFPS(60);
    SetLineWidth(3);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateMouseData();

        if (!settings)
        {
            UpdateInputsData();
            UpdateScroll();

            if (!fullVisor)
            {
                UpdateNodesEdit();
                UpdateNodesDrag();
                UpdateNodesLink();
                UpdateCommentCreationEdit();
                UpdateCommentsEdit();
                UpdateCommentsDrag();
            }
        }

        UpdateShaderData();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            if (!fullVisor)
            {
                DrawCanvas();
                DrawInterface();
            }
            DrawVisor();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(iconTex);
    UnloadRenderTexture(visorTarget);
    UnloadModel(model);
    UnloadShader(fxaa);
    if (loadedShader) UnloadShader(shader);
    for (int i = 0; i < MAX_TEXTURES; i++) UnloadTexture(textures[i]);

    CloseFNode();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}