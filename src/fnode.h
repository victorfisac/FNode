/**********************************************************************************************
*
*   FNode 1.1 - Node based shading library
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2016-2020 Victor Fisac
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

#if !defined(FNODE_H)
#define FNODE_H

#define FNODE_STATIC

#if defined(FNODE_STATIC)
    #define FNODEDEF static            // Functions just visible to module including this file
#else
    #if defined(__cplusplus)
        #define FNODEDEF extern "C"    // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define FNODEDEF extern        // Functions visible from other files
    #endif
#endif

//----------------------------------------------------------------------------------
// External Includes
//----------------------------------------------------------------------------------
#include "raylib.h"             // Required for window management, 2D camera drawing and inputs detection

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define     MAX_INPUTS                  4                       // Max number of inputs in every node
#define     MAX_VALUES                  16                      // Max number of values in every output

//----------------------------------------------------------------------------------
// Enums Definition
//----------------------------------------------------------------------------------
typedef enum {
    FNODE_PI = -2,
    FNODE_E,
    FNODE_TIME,
    FNODE_VERTEXPOSITION,
    FNODE_VERTEXNORMAL,
    FNODE_FRESNEL,
    FNODE_VIEWDIRECTION,
    FNODE_MVP,
    FNODE_MATRIX,
    FNODE_VALUE,
    FNODE_VECTOR2,
    FNODE_VECTOR3,
    FNODE_VECTOR4,
    FNODE_ADD,
    FNODE_SUBTRACT,
    FNODE_MULTIPLY,
    FNODE_DIVIDE,
    FNODE_APPEND,
    FNODE_ONEMINUS,
    FNODE_ABS,
    FNODE_COS,
    FNODE_SIN,
    FNODE_TAN,
    FNODE_DEG2RAD,
    FNODE_RAD2DEG,
    FNODE_NORMALIZE,
    FNODE_NEGATE,
    FNODE_RECIPROCAL,
    FNODE_SQRT,
    FNODE_TRUNC,
    FNODE_ROUND,
    FNODE_VERTEXCOLOR,
    FNODE_CEIL,
    FNODE_CLAMP01,
    FNODE_EXP2,
    FNODE_POWER,
    FNODE_STEP,
    FNODE_POSTERIZE,
    FNODE_MAX,
    FNODE_MIN,
    FNODE_LERP,
    FNODE_SMOOTHSTEP,
    FNODE_CROSSPRODUCT,
    FNODE_DESATURATE,
    FNODE_DISTANCE,
    FNODE_DOTPRODUCT,
    FNODE_LENGTH,
    FNODE_MULTIPLYMATRIX,
    FNODE_TRANSPOSE,
    FNODE_PROJECTION,
    FNODE_REJECTION,
    FNODE_HALFDIRECTION,
    FNODE_SAMPLER2D,
    FNODE_VERTEX,
    FNODE_FRAGMENT
} FNodeType;

typedef enum {
    GLSL_330,
    GLSL_100
} ShaderVersion;

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct FNodeValue {
    float value;                            // Output data value
    Rectangle shape;                        // Output data shape
    char *valueText;                        // Output data value as text
} FNodeValue;

typedef struct FNodeOutput {
    FNodeValue data[MAX_VALUES];            // Output data array (float, Vector2, Vector3 or Vector4)
    int dataCount;                          // Output data length to know which type of data represents
} FNodeOutput;

typedef struct FNodeData {
    unsigned int id;                        // Node unique identifier
    FNodeType type;                         // Node type (values, operations...)
    const char *name;                       // Displayed name of the node
    int inputs[MAX_INPUTS];                 // Inputs node ids array
    unsigned int inputsCount;               // Inputs node ids array length
    unsigned int inputsLimit;               // Inputs node ids length limit
    bool property;                          // Node is property state
    FNodeOutput output;                     // Value output (contains the value and its length)
    Rectangle shape;                        // Node rectangle data
    Rectangle inputShape;                   // Node input rectangle data (automatically calculated from shape)
    Rectangle outputShape;                  // Node input rectangle data (automatically calculated from shape)
} FNodeData, *FNode;

typedef struct FLineData {
    unsigned int id;                        // Line unique identifier
    unsigned int from;                      // Id from line start node
    unsigned int to;                        // Id from line end node
} FLineData, *FLine;

typedef struct FCommentData {
    unsigned int id;                        // Comment unique identifier
    char *value;                            // Comment text label value
    Rectangle shape;                        // Comment rectangle data
    Rectangle valueShape;                   // Comment label rectangle data 
    Rectangle sizeTShape;                   // Comment top size edit rectangle data
    Rectangle sizeBShape;                   // Comment bottom size edit rectangle data
    Rectangle sizeLShape;                   // Comment left size edit rectangle data
    Rectangle sizeRShape;                   // Comment right size edit rectangle data
    Rectangle sizeTlShape;                  // Comment top-left size edit rectangle data
    Rectangle sizeTrShape;                  // Comment top-right size edit rectangle data
    Rectangle sizeBlShape;                  // Comment bottom-right size edit rectangle data
    Rectangle sizeBrShape;                  // Comment bottom-right size edit rectangle data
} FCommentData, *FComment;

#if defined(__cplusplus)
extern "C" {                                    // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
FNODEDEF void InitFNode();                                                           // Initializes FNode global variables
FNODEDEF void SetBackfaceCulling(bool state);                                        // Set backface culling state in openGL context
FNODEDEF FNode CreateNodePI();                                                       // Creates a node which returns PI value
FNODEDEF FNode CreateNodeE();                                                        // Creates a node which returns e value
FNODEDEF FNode CreateNodeMatrix(Matrix mat);                                         // Creates a matrix 4x4 node (OpenGL style 4x4 - right handed, column major)
FNODEDEF FNode CreateNodeValue(float value);                                         // Creates a value node (1 float)
FNODEDEF FNode CreateNodeVector2(Vector2 vector);                                    // Creates a Vector2 node (2 float)
FNODEDEF FNode CreateNodeVector3(Vector3 vector);                                    // Creates a Vector3 node (3 float)
FNODEDEF FNode CreateNodeVector4(Vector4 vector);                                    // Creates a Vector4 node (4 float)
FNODEDEF FNode CreateNodeOperator(FNodeType type, const char *name, int inputs);     // Creates an operator node with type name and inputs limit as parameters
FNODEDEF FNode CreateNodeUniform(FNodeType type, const char *name, int dataCount);   // Creates an uniform node with type name and data count as parameters
FNODEDEF FNode CreateNodeProperty(FNodeType type, const char *name, int dataCount, int inputs);  // Creates a property node with type name and data count as parameters
FNODEDEF FNode CreateNodeMaterial(FNodeType type, const char *name, int dataCount);  // Creates the main node that contains final fragment color
FNODEDEF FNode InitializeNode(bool isOperator);                                      // Initializes a new node with generic parameters
FNODEDEF int GetNodeIndex(int id);                                                   // Returns the index of a node searching by its id
FNODEDEF FLine CreateNodeLine();                                                     // Creates a line between two nodes
FNODEDEF FComment CreateComment();                                                   // Creates a comment
FNODEDEF void AlignNode(FNode node);                                                 // Aligns a node to the nearest grid intersection
FNODEDEF void UpdateNodeShapes(FNode node);                                          // Updates a node shapes due to drag behaviour
FNODEDEF void UpdateCommentShapes(FComment comment);                                 // Updates a comment shapes due to drag behaviour
FNODEDEF Vector2 CameraToViewVector2(Vector2 vector, Camera2D camera);               // Converts Vector2 coordinates from world space to Camera2D space based on its offset
FNODEDEF Rectangle CameraToViewRec(Rectangle rec, Camera2D camera);                  // Converts rectangle coordinates from world space to Camera2D space based on its offset
FNODEDEF void CalculateValues();                                                     // Calculates nodes output values based on current inputs
FNODEDEF void DrawNode(FNode node);                                                  // Draws a previously created node
FNODEDEF void DrawNodeLine(FLine line);                                              // Draws a previously created node line
FNODEDEF void DrawComment(FComment comment);                                         // Draws a previously created comment
FNODEDEF void DestroyNode(FNode node);                                               // Destroys a node and its linked lines
FNODEDEF void DestroyNodeLine(FLine line);                                           // Destroys a node line
FNODEDEF void DestroyComment(FComment comment);                                      // Destroys a comment
FNODEDEF void CloseFNode();                                                          // Unitializes FNode global variables
FNODEDEF void TraceLogFNode(bool error, const char *text, ...);                      // Outputs a trace log message
FNODEDEF int FSearch(char *filename, char *string);                                  // Returns 1 if a specific string is found in a text file

#if defined(__cplusplus)
}
#endif

#endif // FNODE_H

/***********************************************************************************
*
*   FNODE IMPLEMENTATION
*
************************************************************************************/

#if defined(FNODE_IMPLEMENTATION)

// Check if custom malloc/free functions defined, if not, using standard ones
#if !defined(FNODE_MALLOC)
    #include <stdlib.h>     // Required for: malloc(), free()

    #define     FNODE_MALLOC(size)      malloc(size)            // Memory allocation function as define
    #define     FNODE_FREE(ptr)         free(ptr)               // Memory deallocation function as define
#endif

#include <stdio.h>              // Required for: FILE, fopen(), fprintf(), fclose(), fscanf(), stdout, vprintf(), sprintf(), fgets()
#include <string.h>             // Required for: strcat(), strstr()
#include <math.h>               // Required for: fabs(), sqrt(), sinf(), cosf(), cos(), sin(), tan(), pow(), floor()
#include <stdarg.h>             // Required for: va_list, va_start(), vfprintf(), va_end()

#include "external/glad.h"      // Required for GLAD extensions loading library, includes OpenGL headers

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define     MAX_NODES                       128                     // Max number of nodes
#define     MAX_NODE_LENGTH                 16                      // Max node output data value text length
#define     MAX_LINES                       512                     // Max number of lines (8 lines for each node)
#define     MAX_COMMENTS                    16                      // Max number of comments
#define     MAX_COMMENT_LENGTH              20                      // Max comment value text length
#define     MIN_COMMENT_SIZE                75                      // Min comment width and height values
#define     NODE_LINE_DIVISIONS             20                      // Node curved line divisions
#define     NODE_DATA_WIDTH                 30                      // Node data text width
#define     NODE_DATA_HEIGHT                30                      // Node data text height
#define     UI_GRID_SPACING                 25                      // Interface canvas background grid divisions length
#define     UI_GRID_COUNT                   100                     // Interface canvas background grid divisions count
#define     UI_COMMENT_WIDTH                220                     // Interface comment text box width
#define     UI_COMMENT_HEIGHT               25                      // Interface comment text box height
#define     UI_BUTTON_DEFAULT_COLOR         LIGHTGRAY               // Interface button background color
#define     UI_BORDER_DEFAULT_COLOR         125                     // Interface button border color
#define     UI_TOGGLE_TEXT_PADDING          20                      // Interface toggle text padding
#define     UI_TOGGLE_BORDER_WIDTH          2                       // Interface toogle border width

#define     COLOR_INPUT_DISABLED_SHAPE      (Color){ 255, 151, 163, 255 }
#define     COLOR_INPUT_DISABLED_BORDER     (Color){ 199, 4, 10, 255 }
#define     COLOR_INPUT_ENABLED_SHAPE       (Color){ 151, 232, 255, 255 }
#define     COLOR_INPUT_ENABLED_BORDER      (Color){ 4, 140, 199, 255 }
#define     COLOR_INPUT_ADDITIVE            (Color){ 255, 255, 255, 40 }

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//------------------------------------------------------------------------------------
// Global Variables Definition
//------------------------------------------------------------------------------------
int usedMemory = 0;                         // Total used RAM from memory allocation

int nodesCount = 0;                         // Created nodes count
FNode nodes[MAX_NODES];                     // Nodes pointers pool
int selectedNode = -1;                      // Current selected node to drag in canvas
int editNode = -1;                          // Current edited node data
int editNodeType = -1;                      // Current edited node data index
char *editNodeText = NULL;                  // Current edited node data value text before any changes

int linesCount = 0;                         // Created lines count
FLine lines[MAX_LINES];                     // Lines pointers pool
int lineState = 0;                          // Current line linking state (0 = waiting for output to link, 1 = output selected, waiting for input to link)
FLine tempLine = NULL;                      // Temporally created line during line states

int commentsCount = 0;                      // Created comments count
FComment comments[MAX_COMMENTS];            // Comments pointers pool
int commentState = 0;                       // Current comment creation state (0 = waiting to create, 1 = waiting to finish creation)
int selectedComment = -1;                   // Current selected comment to drag in canvas
int editSize = -1;                          // Current edited comment
int editSizeType = -1;                      // Current edited comment size (0 = top, 1 = bottom, 2 = left, 3 = right, 4 = top-left, 5 = top-right, 6 = bottom-left, 7 = bottom-right)
int editComment = -1;                       // Current edited comment value
int selectedCommentNodes[MAX_NODES];        // Current selected comment nodes ids list to drag
int selectedCommentNodesCount;              // Current selected comment nodes ids list count
FComment tempComment = NULL;                // Temporally created comment during comment states
Vector2 tempCommentPos = { 0, 0 };          // Temporally created comment start position
Vector2 screenSize = { 1280, 800 };         // Window screen width
Camera2D camera;                            // Node area 2d camera for panning
Camera camera3d;                            // Visor camera 3d for model and shader visualization
bool debugMode = false;                     // Drawing debug information state
int menuOffset = 0;                         // Interface elements position current offset
bool interact = true;                       // Buttons and text can interact state

//------------------------------------------------------------------------------------
// Module Internal Functions Declaration
//------------------------------------------------------------------------------------
static float FVector2Length(Vector2 v);                                    // Returns length of a Vector2
static float FVector3Length(Vector3 v);                                    // Returns length of a Vector3
static float FVector4Length(Vector4 v);                                    // Returns length of a Vector4
static Vector2 FVector2Normalize(Vector2 v);                               // Returns a normalized Vector2
static Vector3 FVector3Normalize(Vector3 v);                               // Returns a normalized Vector3
static Vector4 FVector4Normalize(Vector4 v);                               // Returns a normalized Vector4
static float FVector2Dot(Vector2 a, Vector2 b);                            // Returns the dot product of two Vector2
static float FVector3Dot(Vector3 a, Vector3 b);                            // Returns the dot product of two Vector3
static float FVector4Dot(Vector4 a, Vector4 b);                            // Returns the dot product of two Vector4
static Vector2 FVector2Projection(Vector2 a, Vector2 b);                   // Returns the projection vector of two Vector2
static Vector3 FVector3Projection(Vector3 a, Vector3 b);                   // Returns the projection vector of two Vector3
static Vector4 FVector4Projection(Vector4 a, Vector4 b);                   // Returns the projection vector of two Vector4
static Vector2 FVector2Rejection(Vector2 a, Vector2 b);                    // Returns the rejection vector of two Vector2
static Vector3 FVector3Rejection(Vector3 a, Vector3 b);                    // Returns the rejection vector of two Vector3
static Vector4 FVector4Rejection(Vector4 a, Vector4 b);                    // Returns the rejection vector of two Vector4
static Vector3 FCrossProduct(Vector3 a, Vector3 b);                        // Returns the cross product of two vectors
static Matrix FMatrixIdentity();                                           // Returns identity matrix
static Matrix FMatrixMultiply(Matrix left, Matrix right);                  // Returns the result of multiply two matrices
static Matrix FMatrixTranslate(float x, float y, float z);                 // Returns translation matrix
static Matrix FMatrixRotate(Vector3 axis, float angle);                    // Create rotation matrix from axis and angle provided in radians
static Matrix FMatrixScale(float x, float y, float z);                     // Returns scaling matrix
static void FMatrixTranspose(Matrix *mat);                                 // Transposes provided matrix
static void FMultiplyMatrixVector(Vector4 *v, Matrix mat);                 // Transform a quaternion given a transformation matrix
static float FCos(float value);                                            // Returns the cosine value of a radian angle
static float FSin(float value);                                            // Returns the sine value of a radian angle
static float FTan(float value);                                            // Returns the tangent value of a radian angle
static float FPower(float value, float exp);                               // Returns a value to the power of an exponent
static float FSquareRoot(float value);                                     // Returns the square root of the input value
static float FPosterize(float value, float samples);                       // Returns a value rounded based on the samples
static float FClamp(float value, float min, float max);                    // Returns a value clamped by a min and max values
static float FTrunc(float value);                                          // Returns a truncated value of a value
static float FRound(float value);                                          // Returns a rounded value of a value
static float FCeil(float value);                                           // Returns a rounded up to the nearest integer of a value
static float FLerp(float valueA, float valueB, float time);                // Returns the interpolation between two values
static Vector2 FVector2Lerp(Vector2 valueA, Vector2 valueB, float time);   // Returns the interpolation between two Vector2 values
static Vector3 FVector3Lerp(Vector3 valueA, Vector3 valueB, float time);   // Returns the interpolation between two Vector3 values
static Vector4 FVector4Lerp(Vector4 valueA, Vector4 valueB, float time);   // Returns the interpolation between two Vector4 values
static float FSmoothStep(float min, float max, float value);               // Returns the interpolate of a value in a range
static float FEaseLinear(float t, float b, float c, float d);              // Returns an ease linear value between two parameters 
static float FEaseInOutQuad(float t, float b, float c, float d);           // Returns an ease quadratic in-out value between two parameters

static void FStringToFloat(float *pointer, const char *string);            // Sends a float conversion value of a string to an initialized float pointer
static void FFloatToString(char *buffer, float value);                     // Sends formatted output to an initialized string pointer

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
// Initializes FNode global variables
FNODEDEF void InitFNode()
{
    nodesCount = 0;
    linesCount = 0;
    commentsCount = 0;
    selectedCommentNodesCount = 0;
    for (int i = 0; i < MAX_NODES; i++) selectedCommentNodes[i] = -1;

    // Initialize OpenGL states
    SetBackfaceCulling(false);

    TraceLogFNode(false, "initialization complete");
}

// Set backface culling state in openGL context
FNODEDEF void SetBackfaceCulling(bool state)
{
    if (state) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);
}

// Creates a node which returns PI value
FNODEDEF FNode CreateNodePI()
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_PI;
    newNode->name = "PI";
    newNode->output.dataCount = 1;
    newNode->output.data[0].value = PI;
    FFloatToString(newNode->output.data[0].valueText, newNode->output.data[0].value);
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a node which returns e value
FNODEDEF FNode CreateNodeE()
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_E;
    newNode->name = "e";
    newNode->output.dataCount = 1;
    newNode->output.data[0].value = 2.71828182845904523536;
    FFloatToString(newNode->output.data[0].valueText, newNode->output.data[0].value);
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a matrix 4x4 node (OpenGL style 4x4 - right handed, column major)
FNODEDEF FNode CreateNodeMatrix(Matrix mat)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_MATRIX;
    newNode->name = "Matrix (4x4)";
    newNode->output.dataCount = 16;
    newNode->output.data[0].value = mat.m0;
    newNode->output.data[1].value = mat.m1;
    newNode->output.data[2].value = mat.m2;
    newNode->output.data[3].value = mat.m3;
    newNode->output.data[4].value = mat.m4;
    newNode->output.data[5].value = mat.m5;
    newNode->output.data[6].value = mat.m6;
    newNode->output.data[7].value = mat.m7;
    newNode->output.data[8].value = mat.m8;
    newNode->output.data[9].value = mat.m9;
    newNode->output.data[10].value = mat.m10;
    newNode->output.data[11].value = mat.m11;
    newNode->output.data[12].value = mat.m12;
    newNode->output.data[13].value = mat.m13;
    newNode->output.data[14].value = mat.m14;
    newNode->output.data[15].value = mat.m15;
    for (int i = 0; i < newNode->output.dataCount; i++) FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
    newNode->shape.height = (NODE_DATA_HEIGHT + 5)*newNode->output.dataCount + 5;
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a value node (1 float)
FNODEDEF FNode CreateNodeValue(float value)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_VALUE;
    newNode->name = "Value";
    newNode->output.dataCount = 1;
    newNode->output.data[0].value = value;
    FFloatToString(newNode->output.data[0].valueText, newNode->output.data[0].value);
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a Vector2 node (2 float)
FNODEDEF FNode CreateNodeVector2(Vector2 vector)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_VECTOR2;
    newNode->name = "Vector 2";
    newNode->output.dataCount = 2;
    newNode->output.data[0].value = vector.x;
    newNode->output.data[1].value = vector.y;
    for (int i = 0; i < newNode->output.dataCount; i++) FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
    newNode->shape.height = (NODE_DATA_HEIGHT + 5)*newNode->output.dataCount + 5;
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a Vector3 node (3 float)
FNODEDEF FNode CreateNodeVector3(Vector3 vector)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_VECTOR3;
    newNode->name = "Vector 3";
    newNode->output.dataCount = 3;
    newNode->output.data[0].value = vector.x;
    newNode->output.data[1].value = vector.y;
    newNode->output.data[2].value = vector.z;
    for (int i = 0; i < newNode->output.dataCount; i++) FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
    newNode->shape.height = (NODE_DATA_HEIGHT + 5)*newNode->output.dataCount + 5;
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a Vector4 node (4 float)
FNODEDEF FNode CreateNodeVector4(Vector4 vector)
{
    FNode newNode = InitializeNode(false);

    newNode->type = FNODE_VECTOR4;
    newNode->name = "Vector 4";
    newNode->output.dataCount = 4;
    newNode->output.data[0].value = vector.x;
    newNode->output.data[1].value = vector.y;
    newNode->output.data[2].value = vector.z;
    newNode->output.data[3].value = vector.w;
    for (int i = 0; i < newNode->output.dataCount; i++) FFloatToString(newNode->output.data[i].valueText, newNode->output.data[i].value);
    newNode->shape.height = (NODE_DATA_HEIGHT + 5)*newNode->output.dataCount + 5;
    newNode->inputsLimit = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates an operator node with type name and inputs limit as parameters
FNODEDEF FNode CreateNodeOperator(FNodeType type, const char *name, int inputs)
{
    FNode newNode = InitializeNode(true);

    newNode->type = type;
    newNode->name = name;
    newNode->inputsLimit = inputs;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates an uniform node with type name and data count as parameters
FNODEDEF FNode CreateNodeUniform(FNodeType type, const char *name, int dataCount)
{
    FNode newNode = InitializeNode(false);

    newNode->type = type;
    newNode->name = name;
    newNode->output.dataCount = dataCount;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates a property node with type name and data count as parameters
FNODEDEF FNode CreateNodeProperty(FNodeType type, const char *name, int dataCount, int inputs)
{
    FNode newNode = InitializeNode((inputs > 0));

    newNode->type = type;
    newNode->name = name;
    newNode->output.dataCount = dataCount;
    newNode->property = true;
    newNode->inputsLimit = inputs;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Creates the main node that contains final material attributes
FNODEDEF FNode CreateNodeMaterial(FNodeType type, const char *name, int dataCount)
{
    FNode newNode = InitializeNode(true);

    newNode->type = type;
    newNode->name = name;
    newNode->output.dataCount = dataCount;
    newNode->outputShape.width = 0;
    newNode->outputShape.height = 0;

    UpdateNodeShapes(newNode);

    return newNode;
}

// Initializes a new node with generic parameters
FNODEDEF FNode InitializeNode(bool isOperator)
{
    FNode newNode = (FNode)FNODE_MALLOC(sizeof(FNodeData));
    usedMemory += sizeof(FNodeData);

    int id = -1;
    for (int i = 0; i < MAX_NODES; i++)
    {
        int currentId = i;

        // Check if current id already exist in other node
        for (int k = 0; k < nodesCount; k++)
        {
            if (nodes[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If not exist, set it as new node id
        if (currentId == i)
        {
            id = i;
            break;
        }
    }

    // Initialize node id
    if (id != -1) newNode->id = id;
    else TraceLogFNode(true, "node creation failed because there is any available id");

    // Initialize node inputs and inputs count
    for (int i = 0; i < MAX_INPUTS; i++) newNode->inputs[i] = -1;
    newNode->inputsCount = 0;
    newNode->inputsLimit = MAX_INPUTS;
    newNode->property = false;

    // Initialize shapes
    newNode->shape = (Rectangle){ GetRandomValue(-camera.offset.x + 0, -camera.offset.x + screenSize.x*0.85f - 50*4), GetRandomValue(-camera.offset.y + screenSize.y/2 - 20 - 100, camera.offset.y + screenSize.y/2 - 20 + 100), 10 + NODE_DATA_WIDTH, 40 };
    if (isOperator) newNode->inputShape = (Rectangle){ 0, 0, 20, 20 };
    else newNode->inputShape = (Rectangle){ 0, 0, 0, 0 };
    newNode->outputShape = (Rectangle){ 0, 0, 20, 20 };

    // Initialize node output and output values count
    newNode->output.dataCount = 0;

    for (int i = 0; i < MAX_VALUES; i++)
    {
        newNode->output.data[i].value = 0.0f;
        newNode->output.data[i].shape = (Rectangle){ 0, 0, NODE_DATA_WIDTH, NODE_DATA_HEIGHT };
        newNode->output.data[i].valueText = (char *)FNODE_MALLOC(MAX_NODE_LENGTH);
        usedMemory += MAX_NODE_LENGTH;
        for (int k = 0; k < MAX_NODE_LENGTH; k++) newNode->output.data[i].valueText[k] = '\0';
    }

    nodes[nodesCount] = newNode;
    nodesCount++;

    TraceLogFNode(false, "created new node id %i (index: %i) [USED RAM: %i bytes]", newNode->id, (nodesCount - 1), usedMemory);

    return newNode;
}

// Returns the index of a node searching by its id
FNODEDEF int GetNodeIndex(int id)
{
    int output = -1;

    for (int i = 0; i < nodesCount; i++)
    {
        if (nodes[i]->id == id)
        {
            output = i;
            break;
        }
    }

    if (output == -1) TraceLogFNode(true, "error when trying to find a node index by its id");

    return output;
}

// Creates a line between two nodes
FNODEDEF FLine CreateNodeLine(int from)
{
    FLine newLine = (FLine)FNODE_MALLOC(sizeof(FLineData));
    usedMemory += sizeof(FLineData);

    int id = -1;
    for (int i = 0; i < MAX_LINES; i++)
    {
        int currentId = i;

        // Check if current id already exist in other line
        for (int k = 0; k < linesCount; k++)
        {
            if (lines[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If not exist, set it as new line id
        if (currentId == i)
        {
            id = i;
            break;
        }
    }

    // Initialize line id
    if (id != -1) newLine->id = id;
    else TraceLogFNode(true, "line creation failed because there is any available id");

    // Initialize line input
    newLine->from = from;
    newLine->to = -1;

    lines[linesCount] = newLine;
    linesCount++;

    TraceLogFNode(false, "created new line id %i (index: %i) [USED RAM: %i bytes]", newLine->id, (linesCount - 1), usedMemory);

    return newLine;
}

// Creates a comment
FNODEDEF FComment CreateComment()
{
    FComment newComment = (FComment)FNODE_MALLOC(sizeof(FCommentData));
    usedMemory += sizeof(FCommentData);

    int id = -1;
    for (int i = 0; i < MAX_COMMENTS; i++)
    {
        int currentId = i;

        // Check if current id already exist in other comment
        for (int k = 0; k < commentsCount; k++)
        {
            if (comments[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If not exist, set it as new comment id
        if (currentId == i)
        {
            id = i;
            break;
        }
    }

    // Initialize comment id
    if (id != -1) newComment->id = id;
    else TraceLogFNode(true, "comment creation failed because there is any available id");

    // Initialize comment input
    newComment->value = (char *)FNODE_MALLOC(MAX_COMMENT_LENGTH);
    usedMemory += MAX_COMMENT_LENGTH;
    for (int i = 0; i < MAX_COMMENT_LENGTH; i++)
    {
        if (i == 0) newComment->value[i] = 'C';
        else if (i == 1) newComment->value[i] = 'h';
        else if (i == 2) newComment->value[i] = 'a';
        else if (i == 3) newComment->value[i] = 'n';
        else if (i == 4) newComment->value[i] = 'g';
        else if (i == 5) newComment->value[i] = 'e';
        else if (i == 6) newComment->value[i] = ' ';
        else if (i == 7) newComment->value[i] = 'm';
        else if (i == 8) newComment->value[i] = 'e';
        else newComment->value[i] = '\0';
    }

    newComment->shape = (Rectangle){ 0, 0, 0, 0 };
    newComment->valueShape = (Rectangle){ 0, 0, UI_COMMENT_WIDTH, UI_COMMENT_HEIGHT };
    newComment->sizeTShape = (Rectangle){ 0, 0, 40, 10 };
    newComment->sizeBShape = (Rectangle){ 0, 0, 40, 10 };
    newComment->sizeLShape = (Rectangle){ 0, 0, 10, 40 };
    newComment->sizeRShape = (Rectangle){ 0, 0, 10, 40 };
    newComment->sizeTlShape = (Rectangle){ 0, 0, 10, 10 };
    newComment->sizeTrShape = (Rectangle){ 0, 0, 10, 10 };
    newComment->sizeBlShape = (Rectangle){ 0, 0, 10, 10 };
    newComment->sizeBrShape = (Rectangle){ 0, 0, 10, 10 };

    comments[commentsCount] = newComment;
    commentsCount++;

    TraceLogFNode(false, "created new comment id %i (index: %i) [USED RAM: %i bytes]", newComment->id, (commentsCount - 1), usedMemory);

    return newComment;
}

// Aligns a node to the nearest grid intersection
FNODEDEF void AlignNode(FNode node)
{
    int spacing = 0;
    float currentDistance = 999999;
    int currentSpacing = 0;

    for (int j = 0; j < UI_GRID_COUNT*5; j++)
    {
        float temp = abs(node->shape.x - (-(UI_GRID_COUNT/2*UI_GRID_SPACING*5) + spacing));

        if (currentDistance > temp)
        {
            currentDistance = temp;
            currentSpacing = spacing;
        }

        spacing += UI_GRID_SPACING;
    }

    if (node->shape.x - (-(UI_GRID_COUNT/2*UI_GRID_SPACING*5) + currentSpacing) > 0.0f) node->shape.x -= currentDistance;
    else node->shape.x += currentDistance;

    spacing = 0;
    currentDistance = 999999;
    currentSpacing = 0;
    for (int j = 0; j < UI_GRID_COUNT*5; j++)
    {
        float temp = abs(node->shape.y - (-(UI_GRID_COUNT/2*UI_GRID_SPACING*5) + spacing));

        if (currentDistance > temp)
        {
            currentDistance = temp;
            currentSpacing = spacing;
        }

        spacing += UI_GRID_SPACING;
    }

    if (node->shape.y - (-(UI_GRID_COUNT/2*UI_GRID_SPACING*5) + currentSpacing) > 0.0f) node->shape.y -= currentDistance;
    else node->shape.y += currentDistance;
}

// Updates a node shapes due to drag behaviour
FNODEDEF void UpdateNodeShapes(FNode node)
{
    if (node != NULL)
    {
        int index = -1;
        int currentLength = 0;
        for (int i = 0; i < node->output.dataCount; i++)
        {
            node->output.data[i].shape.x = node->shape.x + 5;
            node->output.data[i].shape.width = NODE_DATA_WIDTH;
            node->output.data[i].shape.width += MeasureText(node->output.data[i].valueText, 20);

            if (i == 0) node->output.data[i].shape.y = node->shape.y + 5;
            else node->output.data[i].shape.y = node->output.data[i - 1].shape.y + node->output.data[i - 1].shape.height + 5;

            int length = MeasureText(node->output.data[i].valueText, 20);
            if (length > currentLength)
            {
                index = i;
                currentLength = length;
            }
        }

        node->shape.width = 10 + NODE_DATA_WIDTH;

        if (index != -1) node->shape.width += MeasureText(node->output.data[index].valueText, 20);
        else if (node->output.dataCount > 0)
        {
            bool isError = false;

            for (int i = 0; i < node->output.dataCount; i++)
            {
                if (node->output.data[i].valueText[0] != '\0') isError = true;
            }

            if (isError) TraceLogFNode(true, "error trying to calculate node data longest value");
        }

        if (node->type >= FNODE_MATRIX) node->shape.height = ((node->output.dataCount == 0) ? (NODE_DATA_HEIGHT/2 + 10) : ((NODE_DATA_HEIGHT + 5)*node->output.dataCount + 5));

        if (node->type >= FNODE_ADD)
        {

            node->inputShape.x = node->shape.x - 20;
            node->inputShape.y = node->shape.y + node->shape.height/2 - 10;
        }

        node->outputShape.x = node->shape.x + node->shape.width;
        node->outputShape.y = node->shape.y + node->shape.height/2 - 10;
    }
    else TraceLogFNode(true, "error trying to calculate values for a null referenced node");
}

// Updates a comment shapes due to drag behaviour
FNODEDEF void UpdateCommentShapes(FComment comment)
{
    if (comment != NULL)
    {
        if (commentState == 0)
        {
            if (comment->shape.width < MIN_COMMENT_SIZE) comment->shape.width = MIN_COMMENT_SIZE;
            if (comment->shape.height < MIN_COMMENT_SIZE) comment->shape.height = MIN_COMMENT_SIZE;
        }

        comment->valueShape.x = comment->shape.x + 10;
        comment->valueShape.y = comment->shape.y - UI_COMMENT_HEIGHT - 5;
        comment->sizeTShape.x = comment->shape.x + comment->shape.width/2 - comment->sizeTShape.width/2;
        comment->sizeTShape.y = comment->shape.y - comment->sizeTShape.height/2;
        comment->sizeBShape.x = comment->shape.x + comment->shape.width/2 - comment->sizeBShape.width/2;
        comment->sizeBShape.y = comment->shape.y + comment->shape.height - comment->sizeBShape.height/2;
        comment->sizeLShape.x = comment->shape.x - comment->sizeLShape.width/2;
        comment->sizeLShape.y = comment->shape.y + comment->shape.height/2 - comment->sizeLShape.height/2;
        comment->sizeRShape.x = comment->shape.x + comment->shape.width - comment->sizeRShape.width/2;
        comment->sizeRShape.y = comment->shape.y + comment->shape.height/2 - comment->sizeRShape.height/2;
        comment->sizeTlShape.x = comment->shape.x - comment->sizeTlShape.width/2;
        comment->sizeTlShape.y = comment->shape.y - comment->sizeTlShape.height/2;
        comment->sizeTrShape.x = comment->shape.x + comment->shape.width - comment->sizeTrShape.width/2;
        comment->sizeTrShape.y = comment->shape.y - comment->sizeTrShape.height/2;
        comment->sizeBlShape.x = comment->shape.x - comment->sizeBlShape.width/2;
        comment->sizeBlShape.y = comment->shape.y + comment->shape.height - comment->sizeBlShape.height/2;
        comment->sizeBrShape.x = comment->shape.x + comment->shape.width - comment->sizeBrShape.width/2;
        comment->sizeBrShape.y = comment->shape.y + comment->shape.height - comment->sizeBrShape.height/2;
    }
    else TraceLogFNode(true, "error trying to calculate values for a null referenced comment");
}

// Converts Vector2 coordinates from world space to Camera2D space based on its offset
FNODEDEF Vector2 CameraToViewVector2(Vector2 vector, Camera2D camera)
{
    return (Vector2){ vector.x - camera.offset.x, vector.y - camera.offset.y };
}

// Converts rectangle coordinates from world space to Camera2D space based on its offset
FNODEDEF Rectangle CameraToViewRec(Rectangle rec, Camera2D camera)
{
    return (Rectangle){ rec.x + camera.offset.x, rec.y + camera.offset.y, rec.width, rec.height };
}

// Calculates nodes output values based on current inputs
FNODEDEF void CalculateValues()
{
    for (int i = 0; i < nodesCount; i++)
    {
        if (nodes[i] != NULL)
        {
            // Reset node inputs values and inputs count
            for (int k = 0; k < MAX_INPUTS; k++) nodes[i]->inputs[k] = -1;
            nodes[i]->inputsCount = 0;

            // Search for node inputs and calculate inputs count
            for (int k = 0; k < linesCount; k++)
            {
                if (lines[k] != NULL)
                {
                    // Check if line output (to) is the node input id
                    if (lines[k]->to == nodes[i]->id)
                    {
                        nodes[i]->inputs[nodes[i]->inputsCount] = lines[k]->from;
                        nodes[i]->inputsCount++;
                    }
                }
            }

            // Check if current node is an operator
            if (nodes[i]->type > FNODE_VECTOR4 && nodes[i]->type < FNODE_APPEND)
            {
                // Calculate output values count based on first input node value count
                if (nodes[i]->inputsCount > 0) 
                {
                    // Get which index has the first input node id from current nude
                    int index = GetNodeIndex(nodes[i]->inputs[0]);

                    nodes[i]->output.dataCount = nodes[index]->output.dataCount;
                    for (int k = 0; k < nodes[i]->output.dataCount; k++) nodes[i]->output.data[k].value = nodes[index]->output.data[k].value;

                    for (int k = 1; k < nodes[i]->inputsCount; k++)
                    {
                        int inputIndex = GetNodeIndex(nodes[i]->inputs[k]);

                        // Apply inputs values to output based on node operation type
                        switch (nodes[i]->type)
                        {
                            case FNODE_ADD:
                            {
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value += nodes[inputIndex]->output.data[j].value;
                            } break;
                            case FNODE_SUBTRACT:
                            {
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value -= nodes[inputIndex]->output.data[j].value;
                            } break;
                            case FNODE_MULTIPLY:
                            {
                                if (nodes[inputIndex]->output.dataCount == 1)
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= nodes[inputIndex]->output.data[0].value;
                                }
                                else
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++)
                                    {
                                        if (nodes[inputIndex]->output.dataCount == 16 && nodes[i]->output.dataCount == 4)
                                        {
                                            Vector4 vector = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value };
                                            Matrix matrix = { nodes[inputIndex]->output.data[4].value, nodes[inputIndex]->output.data[5].value, nodes[inputIndex]->output.data[6].value, nodes[inputIndex]->output.data[7].value,
                                            nodes[inputIndex]->output.data[8].value, nodes[inputIndex]->output.data[9].value, nodes[inputIndex]->output.data[10].value, nodes[inputIndex]->output.data[11].value,
                                            nodes[inputIndex]->output.data[12].value, nodes[inputIndex]->output.data[13].value, nodes[inputIndex]->output.data[14].value, nodes[inputIndex]->output.data[15].value };
                                            FMultiplyMatrixVector(&vector, matrix);

                                            for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                                            nodes[i]->output.data[0].value = vector.x;
                                            nodes[i]->output.data[1].value = vector.y;
                                            nodes[i]->output.data[2].value = vector.z;
                                            nodes[i]->output.data[3].value = vector.w;
                                            nodes[i]->output.dataCount = 4;
                                        }
                                        else if (nodes[inputIndex]->output.dataCount == 4 && nodes[i]->output.dataCount == 16)
                                        {
                                            Vector4 vector = { nodes[inputIndex]->output.data[0].value, nodes[inputIndex]->output.data[1].value, nodes[inputIndex]->output.data[2].value, nodes[inputIndex]->output.data[3].value };
                                            Matrix matrix = { nodes[i]->output.data[4].value, nodes[i]->output.data[5].value, nodes[i]->output.data[6].value, nodes[i]->output.data[7].value,
                                            nodes[i]->output.data[8].value, nodes[i]->output.data[9].value, nodes[i]->output.data[10].value, nodes[i]->output.data[11].value,
                                            nodes[i]->output.data[12].value, nodes[i]->output.data[13].value, nodes[i]->output.data[14].value, nodes[i]->output.data[15].value };
                                            FMultiplyMatrixVector(&vector, matrix);

                                            for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                                            nodes[i]->output.data[0].value = vector.x;
                                            nodes[i]->output.data[1].value = vector.y;
                                            nodes[i]->output.data[2].value = vector.z;
                                            nodes[i]->output.data[3].value = vector.w;
                                            nodes[i]->output.dataCount = 4;
                                        }
                                        else nodes[i]->output.data[j].value *= nodes[inputIndex]->output.data[j].value;
                                    }
                                }
                            } break;
                            case FNODE_DIVIDE:
                            {
                                if (nodes[inputIndex]->output.dataCount == 1)
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value /= nodes[inputIndex]->output.data[0].value;
                                }
                                else
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value /= nodes[inputIndex]->output.data[j].value;
                                }
                            } break;
                            default: break;
                        }
                    }
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                    nodes[i]->output.dataCount = 0;
                }
            }
            else if (nodes[i]->type == FNODE_APPEND)
            {
                if (nodes[i]->inputsCount > 0)
                {
                    int valuesCount = 0;
                    for (int k = 0; k < nodes[i]->inputsCount; k++)
                    {
                        int inputIndex = GetNodeIndex(nodes[i]->inputs[k]);

                        for (int j = 0; j < nodes[inputIndex]->output.dataCount; j++)
                        {
                            nodes[i]->output.data[valuesCount].value = nodes[inputIndex]->output.data[j].value;
                            valuesCount++;
                        }
                    }

                    nodes[i]->output.dataCount = valuesCount;
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                    nodes[i]->output.dataCount = 0;
                }
            }
            else if (nodes[i]->type > FNODE_APPEND)
            {
                if (nodes[i]->inputsCount > 0)
                {
                    int index = GetNodeIndex(nodes[i]->inputs[0]);

                    nodes[i]->output.dataCount = nodes[index]->output.dataCount;
                    for (int k = 0; k < nodes[i]->output.dataCount; k++) nodes[i]->output.data[k].value = nodes[index]->output.data[k].value;

                    switch (nodes[i]->type)
                    {
                        case FNODE_ONEMINUS:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = 1 - nodes[i]->output.data[j].value;
                        } break;
                        case FNODE_ABS:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++)
                            {
                                if (nodes[i]->output.data[j].value < 0) nodes[i]->output.data[j].value *= -1;
                            }
                        } break;
                        case FNODE_COS:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)FCos(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_SIN:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)FSin(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_TAN:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)FTan(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_DEG2RAD:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= DEG2RAD;
                        } break;
                        case FNODE_RAD2DEG:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= RAD2DEG;
                        } break;
                        case FNODE_NORMALIZE:
                        {
                            switch (nodes[i]->output.dataCount)
                            {
                                case 2:
                                {
                                    Vector2 temp = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value };
                                    temp = FVector2Normalize(temp);
                                    nodes[i]->output.data[0].value = temp.x;
                                    nodes[i]->output.data[1].value = temp.y;
                                } break;
                                case 3:
                                {
                                    Vector3 temp = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value };
                                    temp = FVector3Normalize(temp);
                                    nodes[i]->output.data[0].value = temp.x;
                                    nodes[i]->output.data[1].value = temp.y;
                                    nodes[i]->output.data[2].value = temp.z;
                                } break;
                                case 4:
                                {
                                    Vector4 temp = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value };
                                    temp = FVector4Normalize(temp);
                                    nodes[i]->output.data[0].value = temp.x;
                                    nodes[i]->output.data[1].value = temp.y;
                                    nodes[i]->output.data[2].value = temp.z;
                                    nodes[i]->output.data[3].value = temp.w;
                                } break;
                                default: break;
                            }
                        } break;
                        case FNODE_NEGATE:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= -1;
                        } break;
                        case FNODE_RECIPROCAL:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = 1/nodes[i]->output.data[j].value;
                        } break;
                        case FNODE_SQRT:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FSquareRoot(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_TRUNC:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FTrunc(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_ROUND:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FRound(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_VERTEXCOLOR:
                        {
                            if (nodes[i]->inputsCount == 1)
                            {
                                int index = GetNodeIndex(nodes[i]->inputs[0]);
                                switch ((int)nodes[index]->output.data[0].value)
                                {
                                    case 0: nodes[i]->output.dataCount = 4; break;
                                    case 1: nodes[i]->output.dataCount = 3; break;
                                    default: nodes[i]->output.dataCount = 1; break;
                                }
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        }
                        case FNODE_CEIL:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FCeil(nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_CLAMP01:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FClamp(nodes[i]->output.data[j].value, 0.0f, 1.0f);
                        } break;
                        case FNODE_EXP2:
                        {
                            for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FPower(2.0f, nodes[i]->output.data[j].value);
                        } break;
                        case FNODE_POWER:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int expIndex = GetNodeIndex(nodes[i]->inputs[1]);

                                if (nodes[expIndex]->output.dataCount == 1)
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FPower(nodes[i]->output.data[j].value, nodes[expIndex]->output.data[0].value);
                                }
                                else TraceLogFNode(false, "values count of node %i should be 1 because it is an exponent (currently count: %i)", nodes[expIndex]->id, nodes[expIndex]->output.dataCount);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_STEP:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                if (nodes[indexB]->output.dataCount == 1) nodes[i]->output.data[0].value = ((nodes[i]->output.data[0].value <= nodes[indexB]->output.data[0].value) ? 1.0f : 0.0f);
                                else TraceLogFNode(false, "values count of node %i should be 1 because it is an exponent (currently count: %i)", nodes[indexB]->id, nodes[indexB]->output.dataCount);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_POSTERIZE:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int expIndex = GetNodeIndex(nodes[i]->inputs[1]);

                                if (nodes[expIndex]->output.dataCount == 1)
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FPosterize(nodes[i]->output.data[j].value, nodes[expIndex]->output.data[0].value);
                                }
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_MAX:
                        case FNODE_MIN:
                        {
                            for (int j = 1; j < nodes[i]->inputsCount; j++)
                            {
                                int inputIndex = GetNodeIndex(nodes[i]->inputs[j]);

                                for (int k = 0; k < nodes[i]->output.dataCount; k++)
                                {
                                    if ((nodes[inputIndex]->output.data[k].value > nodes[i]->output.data[k].value) && (nodes[i]->type == FNODE_MAX)) nodes[i]->output.data[k].value = nodes[inputIndex]->output.data[k].value;
                                    else if ((nodes[inputIndex]->output.data[k].value < nodes[i]->output.data[k].value) && (nodes[i]->type == FNODE_MIN)) nodes[i]->output.data[k].value = nodes[inputIndex]->output.data[k].value;
                                }
                            }
                        } break;
                        case FNODE_LERP:
                        {                                
                            if (nodes[i]->inputsCount == 3)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);
                                int indexC = GetNodeIndex(nodes[i]->inputs[2]);

                                switch (nodes[i]->output.dataCount)
                                {
                                    case 1:
                                    {
                                        for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FLerp(nodes[indexA]->output.data[j].value, nodes[indexB]->output.data[j].value, nodes[indexC]->output.data[j].value);
                                    } break;
                                    case 2:
                                    {
                                        Vector2 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value };
                                        Vector2 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value };
                                        Vector2 lerp = FVector2Lerp(vectorA, vectorB, nodes[indexC]->output.data[0].value);

                                        nodes[i]->output.dataCount = 2;
                                        nodes[i]->output.data[0].value = lerp.x;
                                        nodes[i]->output.data[1].value = lerp.y;
                                    } break;
                                    case 3:
                                    {
                                        Vector3 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                        Vector3 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                        Vector3 lerp = FVector3Lerp(vectorA, vectorB, nodes[indexC]->output.data[0].value);

                                        nodes[i]->output.dataCount = 3;
                                        nodes[i]->output.data[0].value = lerp.x;
                                        nodes[i]->output.data[1].value = lerp.y;
                                        nodes[i]->output.data[2].value = lerp.z;
                                    } break;
                                    case 4:
                                    {
                                        Vector4 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value, nodes[indexA]->output.data[3].value };
                                        Vector4 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexA]->output.data[2].value, nodes[indexA]->output.data[3].value };
                                        Vector4 lerp = FVector4Lerp(vectorA, vectorB, nodes[indexC]->output.data[0].value);

                                        nodes[i]->output.dataCount = 4;
                                        nodes[i]->output.data[0].value = lerp.x;
                                        nodes[i]->output.data[1].value = lerp.y;
                                        nodes[i]->output.data[2].value = lerp.z;
                                        nodes[i]->output.data[3].value = lerp.w;
                                    } break;
                                    default: break;
                                }
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_SMOOTHSTEP:
                        {                                
                            if (nodes[i]->inputsCount == 3)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);
                                int indexC = GetNodeIndex(nodes[i]->inputs[2]);

                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FSmoothStep(nodes[indexA]->output.data[j].value, nodes[indexB]->output.data[j].value, nodes[indexC]->output.data[j].value);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_CROSSPRODUCT:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;

                                Vector3 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                Vector3 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value };
                                Vector3 cross = FCrossProduct(vectorA, vectorB);

                                nodes[i]->output.dataCount = 3;
                                nodes[i]->output.data[0].value = cross.x;
                                nodes[i]->output.data[1].value = cross.y;
                                nodes[i]->output.data[2].value = cross.z;
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_DESATURATE:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int index = GetNodeIndex(nodes[i]->inputs[0]);
                                int index1 = GetNodeIndex(nodes[i]->inputs[1]);

                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = nodes[index]->output.data[j].value;
                                nodes[i]->output.dataCount = nodes[index]->output.dataCount;

                                float amount = FClamp(nodes[index1]->output.data[0].value, 0.0f, 1.0f);
                                float luminance = 0.3f*nodes[i]->output.data[0].value + 0.6f*nodes[i]->output.data[1].value + 0.1f*nodes[i]->output.data[2].value;

                                nodes[i]->output.data[0].value = nodes[i]->output.data[0].value + amount*(luminance - nodes[i]->output.data[0].value);
                                nodes[i]->output.data[1].value = nodes[i]->output.data[1].value + amount*(luminance - nodes[i]->output.data[1].value);
                                nodes[i]->output.data[2].value = nodes[i]->output.data[2].value + amount*(luminance - nodes[i]->output.data[2].value);
                            }
                            else
                            {
                                nodes[i]->output.dataCount = 0;
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                        } break;
                        case FNODE_DISTANCE:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                switch (nodes[i]->output.dataCount)
                                {
                                    case 1: nodes[i]->output.data[0].value = nodes[indexB]->output.data[0].value - nodes[indexA]->output.data[0].value; break;
                                    case 2:
                                    {
                                        Vector2 direction = { 0, 0 };
                                        direction.x = nodes[indexB]->output.data[0].value - nodes[indexA]->output.data[0].value;
                                        direction.y = nodes[indexB]->output.data[1].value - nodes[indexA]->output.data[1].value;

                                        nodes[i]->output.data[0].value = FVector2Length(direction);
                                    } break;
                                    case 3:
                                    {
                                        Vector3 direction = { 0, 0, 0 };
                                        direction.x = nodes[indexB]->output.data[0].value - nodes[indexA]->output.data[0].value;
                                        direction.y = nodes[indexB]->output.data[1].value - nodes[indexA]->output.data[1].value;
                                        direction.z = nodes[indexB]->output.data[2].value - nodes[indexA]->output.data[2].value;

                                        nodes[i]->output.data[0].value = FVector3Length(direction);
                                    } break;
                                    case 4:
                                    {
                                        Vector4 direction = { 0, 0, 0, 0 };
                                        direction.x = nodes[indexB]->output.data[0].value - nodes[indexA]->output.data[0].value;
                                        direction.y = nodes[indexB]->output.data[1].value - nodes[indexA]->output.data[1].value;
                                        direction.z = nodes[indexB]->output.data[2].value - nodes[indexA]->output.data[2].value;
                                        direction.w = nodes[indexB]->output.data[3].value - nodes[indexA]->output.data[3].value;

                                        nodes[i]->output.data[0].value = FVector4Length(direction);
                                    } break;
                                    default: break;
                                }

                                for (int j = 1; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 1;
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        } break;
                        case FNODE_DOTPRODUCT:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                switch (nodes[i]->output.dataCount)
                                {
                                    case 2:
                                    {
                                        Vector2 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value };
                                        Vector2 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value };
                                        nodes[i]->output.data[0].value = FVector2Dot(vectorA, vectorB);
                                    } break;
                                    case 3:
                                    {
                                        Vector3 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                        Vector3 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value };
                                        nodes[i]->output.data[0].value = FVector3Dot(vectorA, vectorB);
                                    } break;
                                    case 4:
                                    {
                                        Vector4 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value, nodes[indexA]->output.data[3].value };
                                        Vector4 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value, nodes[indexB]->output.data[3].value };
                                        nodes[i]->output.data[0].value = FVector4Dot(vectorA, vectorB);
                                    } break;
                                    default: break;
                                }

                                for (int j = 1; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 1;
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        } break;
                        case FNODE_LENGTH:
                        {
                            switch (nodes[i]->output.dataCount)
                            {
                                case 2: nodes[i]->output.data[0].value = FVector2Length((Vector2){ nodes[i]->output.data[0].value, nodes[i]->output.data[1].value }); break;
                                case 3: nodes[i]->output.data[0].value = FVector3Length((Vector3){ nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value }); break;
                                case 4: nodes[i]->output.data[0].value = FVector4Length((Vector4){ nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value }); break;
                                default: break;
                            }

                            for (int j = 1; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            nodes[i]->output.dataCount = 1;
                        } break;
                        case FNODE_MULTIPLYMATRIX:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int index = GetNodeIndex(nodes[i]->inputs[1]);

                                Matrix matrixA = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value, 
                                nodes[i]->output.data[4].value, nodes[i]->output.data[5].value, nodes[i]->output.data[6].value, nodes[i]->output.data[7].value, 
                                nodes[i]->output.data[8].value, nodes[i]->output.data[9].value, nodes[i]->output.data[10].value, nodes[i]->output.data[11].value, 
                                nodes[i]->output.data[12].value, nodes[i]->output.data[13].value, nodes[i]->output.data[14].value, nodes[i]->output.data[15].value, };
                                Matrix matrixB = { nodes[index]->output.data[0].value, nodes[index]->output.data[1].value, nodes[index]->output.data[2].value, nodes[index]->output.data[3].value, 
                                nodes[index]->output.data[4].value, nodes[index]->output.data[5].value, nodes[index]->output.data[6].value, nodes[index]->output.data[7].value, 
                                nodes[index]->output.data[8].value, nodes[index]->output.data[9].value, nodes[index]->output.data[10].value, nodes[index]->output.data[11].value, 
                                nodes[index]->output.data[12].value, nodes[index]->output.data[13].value, nodes[index]->output.data[14].value, nodes[index]->output.data[15].value, };
                                Matrix matrixAB = FMatrixMultiply(matrixA, matrixB);

                                nodes[i]->output.data[0].value = matrixAB.m0;
                                nodes[i]->output.data[1].value = matrixAB.m1;
                                nodes[i]->output.data[2].value = matrixAB.m2;
                                nodes[i]->output.data[3].value = matrixAB.m3;
                                nodes[i]->output.data[4].value = matrixAB.m4;
                                nodes[i]->output.data[5].value = matrixAB.m5;
                                nodes[i]->output.data[6].value = matrixAB.m6;
                                nodes[i]->output.data[7].value = matrixAB.m7;
                                nodes[i]->output.data[8].value = matrixAB.m8;
                                nodes[i]->output.data[9].value = matrixAB.m9;
                                nodes[i]->output.data[10].value = matrixAB.m10;
                                nodes[i]->output.data[11].value = matrixAB.m11;
                                nodes[i]->output.data[12].value = matrixAB.m12;
                                nodes[i]->output.data[13].value = matrixAB.m13;
                                nodes[i]->output.data[14].value = matrixAB.m14;
                                nodes[i]->output.data[15].value = matrixAB.m15;
                            }
                        } break;
                        case FNODE_TRANSPOSE:
                        {
                            Matrix matrix = { nodes[i]->output.data[0].value, nodes[i]->output.data[1].value, nodes[i]->output.data[2].value, nodes[i]->output.data[3].value, 
                            nodes[i]->output.data[4].value, nodes[i]->output.data[5].value, nodes[i]->output.data[6].value, nodes[i]->output.data[7].value, 
                            nodes[i]->output.data[8].value, nodes[i]->output.data[9].value, nodes[i]->output.data[10].value, nodes[i]->output.data[11].value, 
                            nodes[i]->output.data[12].value, nodes[i]->output.data[13].value, nodes[i]->output.data[14].value, nodes[i]->output.data[15].value, };
                            FMatrixTranspose(&matrix);

                            nodes[i]->output.data[0].value = matrix.m0;
                            nodes[i]->output.data[1].value = matrix.m1;
                            nodes[i]->output.data[2].value = matrix.m2;
                            nodes[i]->output.data[3].value = matrix.m3;
                            nodes[i]->output.data[4].value = matrix.m4;
                            nodes[i]->output.data[5].value = matrix.m5;
                            nodes[i]->output.data[6].value = matrix.m6;
                            nodes[i]->output.data[7].value = matrix.m7;
                            nodes[i]->output.data[8].value = matrix.m8;
                            nodes[i]->output.data[9].value = matrix.m9;
                            nodes[i]->output.data[10].value = matrix.m10;
                            nodes[i]->output.data[11].value = matrix.m11;
                            nodes[i]->output.data[12].value = matrix.m12;
                            nodes[i]->output.data[13].value = matrix.m13;
                            nodes[i]->output.data[14].value = matrix.m14;
                            nodes[i]->output.data[15].value = matrix.m15;
                        } break;
                        case FNODE_PROJECTION:
                        case FNODE_REJECTION:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                switch (nodes[i]->output.dataCount)
                                {
                                    case 2:
                                    {
                                        Vector2 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value };
                                        Vector2 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value };
                                        Vector2 newVector = ((nodes[i]->type == FNODE_PROJECTION) ? FVector2Projection(vectorA, vectorB) : FVector2Rejection(vectorA, vectorB));

                                        nodes[i]->output.data[0].value = newVector.x;
                                        nodes[i]->output.data[1].value = newVector.y;

                                        for (int j = 2; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                        nodes[i]->output.dataCount = 2;
                                    } break;
                                    case 3:
                                    {
                                        Vector3 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                        Vector3 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value };
                                        Vector3 newVector = ((nodes[i]->type == FNODE_PROJECTION) ? FVector3Projection(vectorA, vectorB) : FVector3Rejection(vectorA, vectorB));

                                        nodes[i]->output.data[0].value = newVector.x;
                                        nodes[i]->output.data[1].value = newVector.y;
                                        nodes[i]->output.data[2].value = newVector.z;

                                        for (int j = 3; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                        nodes[i]->output.dataCount = 3;
                                    } break;
                                    case 4:
                                    {
                                        Vector4 vectorA = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value, nodes[indexA]->output.data[3].value };
                                        Vector4 vectorB = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value, nodes[indexB]->output.data[3].value };
                                        Vector4 newVector = ((nodes[i]->type == FNODE_PROJECTION) ? FVector4Projection(vectorA, vectorB) : FVector4Rejection(vectorA, vectorB));

                                        nodes[i]->output.data[0].value = newVector.x;
                                        nodes[i]->output.data[1].value = newVector.y;
                                        nodes[i]->output.data[2].value = newVector.z;
                                        nodes[i]->output.data[3].value = newVector.w;

                                        for (int j = 4; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                        nodes[i]->output.dataCount = 4;
                                    } break;
                                    default: break;
                                }
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        } break;
                        case FNODE_HALFDIRECTION:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[0]);
                                int indexB = GetNodeIndex(nodes[i]->inputs[1]);

                                bool check = true;
                                check = ((nodes[indexA]->type <= FNODE_E) || (nodes[indexA]->type >= FNODE_MATRIX));
                                if (check) check = ((nodes[indexB]->type <= FNODE_E) || (nodes[indexB]->type >= FNODE_MATRIX));

                                if (check)
                                {
                                    switch (nodes[i]->output.dataCount)
                                    {
                                        case 2:
                                        {
                                            Vector2 a = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value };
                                            Vector2 b = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value };
                                            Vector2 vectorA = FVector2Normalize(a);
                                            Vector2 vectorB = FVector2Normalize(b);
                                            Vector2 output = { vectorA.x + vectorB.x, vectorA.y + vectorB.y };

                                            nodes[i]->output.data[0].value = output.x;
                                            nodes[i]->output.data[1].value = output.y;

                                            for (int j = 2; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                            nodes[i]->output.dataCount = 2;
                                        } break;
                                        case 3:
                                        {
                                            Vector3 a = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value };
                                            Vector3 b = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value };
                                            Vector3 vectorA = FVector3Normalize(a);
                                            Vector3 vectorB = FVector3Normalize(b);
                                            Vector3 output = { vectorA.x + vectorB.x, vectorA.y + vectorB.y, vectorA.z + vectorB.z };

                                            nodes[i]->output.data[0].value = output.x;
                                            nodes[i]->output.data[1].value = output.y;
                                            nodes[i]->output.data[2].value = output.z;

                                            for (int j = 3; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                            nodes[i]->output.dataCount = 3;
                                        } break;
                                        case 4:
                                        {
                                            Vector4 a = { nodes[indexA]->output.data[0].value, nodes[indexA]->output.data[1].value, nodes[indexA]->output.data[2].value, nodes[indexA]->output.data[3].value };
                                            Vector4 b = { nodes[indexB]->output.data[0].value, nodes[indexB]->output.data[1].value, nodes[indexB]->output.data[2].value, nodes[indexB]->output.data[3].value };
                                            Vector4 vectorA = FVector4Normalize(a);
                                            Vector4 vectorB = FVector4Normalize(b);
                                            Vector4 output = { vectorA.x + vectorB.x, vectorA.y + vectorB.y, vectorA.z + vectorB.z, vectorA.w + vectorB.w };

                                            nodes[i]->output.data[0].value = output.x;
                                            nodes[i]->output.data[1].value = output.y;
                                            nodes[i]->output.data[2].value = output.z;
                                            nodes[i]->output.data[3].value = output.w;

                                            for (int j = 4; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                            nodes[i]->output.dataCount = 4;
                                        } break;
                                        default: break;
                                    }
                                }
                                else
                                {
                                    for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                }
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        }
                        case FNODE_SAMPLER2D:
                        {
                            if (nodes[i]->inputsCount == 2)
                            {
                                int indexA = GetNodeIndex(nodes[i]->inputs[1]);
                                switch ((int)nodes[indexA]->output.data[0].value)
                                {
                                    case 0: nodes[i]->output.dataCount = 4; break;
                                    case 1: nodes[i]->output.dataCount = 3; break;
                                    default: nodes[i]->output.dataCount = 1; break;
                                }
                            }
                            else
                            {
                                for (int j = 0; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                                nodes[i]->output.dataCount = 0;
                            }
                        } break;
                        default: break;
                    }
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.data[k].value = 0.0f;
                    nodes[i]->output.dataCount = 0;
                }
            }

            if (nodes[i]->type > FNODE_VECTOR4)
            {
                for (int k = 0; k < nodes[i]->output.dataCount; k++) FFloatToString(nodes[i]->output.data[k].valueText, nodes[i]->output.data[k].value);
            }

            UpdateNodeShapes(nodes[i]);
        }
        else TraceLogFNode(true, "error trying to calculate values for a null referenced node");
    }
}

// Draws a previously created node
FNODEDEF void DrawNode(FNode node)
{
    if (node != NULL)
    {
        if (node->property) DrawRectangleRec(node->shape, ((node->id == selectedNode) ? (Color){ 128, 204, 139, 255 } : (Color){ 173, 225, 181, 255 }));
        else DrawRectangleRec(node->shape, ((node->id == selectedNode) ? GRAY : LIGHTGRAY));

        DrawRectangleLines(node->shape.x, node->shape.y, node->shape.width, node->shape.height, BLACK);
        DrawText(FormatText("%s [ID: %i]", node->name, node->id), node->shape.x + node->shape.width/2 - MeasureText(node->name, 10)/2, node->shape.y - 15, 10, BLACK);

        if ((node->type >= FNODE_MATRIX) && (node->type <= FNODE_VECTOR4))
        {
            if ((node->id == editNode) && interact)
            {
                int charac = -1;
                charac = GetKeyPressed();

                if (charac != -1)
                {
                    if (charac == KEY_BACKSPACE)
                    {
                        for (int i = 0; i < MAX_NODE_LENGTH; i++)
                        {
                            if ((node->output.data[editNodeType].valueText[i] == '\0'))
                            {
                                node->output.data[editNodeType].valueText[i - 1] = '\0';
                                break;
                            }
                        }

                        node->output.data[editNodeType].valueText[MAX_NODE_LENGTH - 1] = '\0';

                        UpdateNodeShapes(node);
                    }
                    else if (charac == KEY_ENTER)
                    {
                        // Check new node value requeriments (cannot be empty, ...)
                        bool valuesCheck = (node->output.data[editNodeType].valueText[0] != '\0');
                        if (valuesCheck)
                        {
                            FStringToFloat(&node->output.data[editNodeType].value, (const char*)node->output.data[editNodeType].valueText);
                            FFloatToString(node->output.data[editNodeType].valueText, node->output.data[editNodeType].value);

                            CalculateValues();
                        }
                        else
                        {
                            TraceLogFNode(false, "error when trying to change node id %i value due to invalid characters (%s)", node->id, node->output.data[editNodeType].valueText);
                            for (int i = 0; i < MAX_NODE_LENGTH; i++) node->output.data[editNodeType].valueText[i] = editNodeText[i];
                        }

                        UpdateNodeShapes(node);
                        editNode = -1;
                        editNodeType = -1;
                        FNODE_FREE(editNodeText);
                        usedMemory -= MAX_NODE_LENGTH;
                        editNodeText = NULL;
                    }
                    else
                    {
                        // Check for numbers, dot and dash values
                        if (((charac > 47) && (charac < 58)) || (charac == 45) || (charac == 46))
                        {
                            for (int i = 0; i < MAX_NODE_LENGTH; i++)
                            {
                                if (node->output.data[editNodeType].valueText[i] == '\0')
                                {
                                    node->output.data[editNodeType].valueText[i] = (char)charac;
                                    UpdateNodeShapes(node);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        if ((node->type <= FNODE_E) || (node->type >= FNODE_MATRIX))
        {
            for (int i = 0; i < node->output.dataCount; i++)
            {
                if ((node->type >= FNODE_MATRIX) && (node->type <= FNODE_VECTOR4)) DrawRectangleLines(node->output.data[i].shape.x,node->output.data[i].shape.y, node->output.data[i].shape.width, node->output.data[i].shape.height, (((editNode == node->id) && (editNodeType == i)) ? BLACK : GRAY));
                DrawText(node->output.data[i].valueText, node->output.data[i].shape.x + (node->output.data[i].shape.width - 
                         MeasureText(node->output.data[i].valueText, 20))/2, node->output.data[i].shape.y + 
                         node->output.data[i].shape.height/2 - 9, 20, DARKGRAY);
            }
        }

        if (node->inputShape.width > 0)
        {
            if (node->inputsCount > 0)
            {
                DrawRectangleRec(node->inputShape, COLOR_INPUT_ENABLED_SHAPE);
                DrawRectangleLines(node->inputShape.x, node->inputShape.y, node->inputShape.width, node->inputShape.height, COLOR_INPUT_ENABLED_BORDER);
            }
            else
            {
                DrawRectangleRec(node->inputShape, COLOR_INPUT_DISABLED_SHAPE);
                DrawRectangleLines(node->inputShape.x, node->inputShape.y, node->inputShape.width, node->inputShape.height, COLOR_INPUT_DISABLED_BORDER);
            }
            
            if(CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(node->inputShape, camera)))
                DrawRectangleRec(node->inputShape, COLOR_INPUT_ADDITIVE);
        }

        if (node->outputShape.width > 0)
        {
            DrawRectangleRec(node->outputShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(node->outputShape, camera)) ? LIGHTGRAY : GRAY)));
            DrawRectangleLines(node->outputShape.x, node->outputShape.y, node->outputShape.width, node->outputShape.height, BLACK);
        }

        if (debugMode)
        {
            const char *string =
            "id: %i\n"
            "name: %s\n"
            "inputs(%i/%i): %i, %i, %i, %i\n"
            "output(%i): %.02f, %.02f, %.02f, %.02f";

            DrawText(FormatText(string, node->id, node->name, node->inputsCount, node->inputsLimit, node->inputs[0], node->inputs[1], node->inputs[2], 
            node->inputs[3], node->output.dataCount, node->output.data[0].value, node->output.data[1].value, node->output.data[2].value, node->output.data[3].value), node->shape.x, node->shape.y + node->shape.height + 5, 10, BLACK);
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced node");
}

// Draws a previously created node line
FNODEDEF void DrawNodeLine(FLine line)
{
    if (line != NULL)
    {
        Vector2 from = { 0, 0 };
        Vector2 to = { 0, 0 };

        int indexTo = -1;        
        if (line->to != -1)
        {
            indexTo = GetNodeIndex(line->to);
            
            to.x = nodes[indexTo]->inputShape.x + nodes[indexTo]->inputShape.width/2;
            to.y = nodes[indexTo]->inputShape.y + nodes[indexTo]->inputShape.height/2;
        }
        else to = CameraToViewVector2(GetMousePosition(), camera);

        int indexFrom = GetNodeIndex(line->from);

        from.x = nodes[indexFrom]->outputShape.x + nodes[indexFrom]->outputShape.width/2;
        from.y = nodes[indexFrom]->outputShape.y + nodes[indexFrom]->outputShape.height/2;

        DrawCircle(from.x, from.y, 5, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));
        DrawCircle(to.x, to.y, 5, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));

        if (from.x <= to.x)
        {
            int current = 0;
            while (current < NODE_LINE_DIVISIONS) 
            {
                Vector2 fromCurve = { 0, 0 };
                fromCurve.x = FEaseLinear(current, from.x, to.x - from.x, NODE_LINE_DIVISIONS);
                fromCurve.y = FEaseInOutQuad(current, from.y, to.y - from.y, NODE_LINE_DIVISIONS);
                current++;

                Vector2 toCurve = { 0, 0 };
                toCurve.x = FEaseLinear(current, from.x, to.x - from.x, NODE_LINE_DIVISIONS);
                toCurve.y = FEaseInOutQuad(current, from.y, to.y - from.y, NODE_LINE_DIVISIONS);

                DrawLine(fromCurve.x, fromCurve.y, toCurve.x, toCurve.y, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));
            }
        }
        else
        {
            float angle = -90.0f;
            float multiplier = (((to.y - from.y) > 0) ? 1 : -1);
            float radius = (fabs(to.y - from.y)/4 + 0.02f)*multiplier;
            float distance = FClamp(fabs(to.x - from.x)/100, 0.0f, 1.0f);

            DrawLine(from.x, from.y, from.x, from.y, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));

            while (angle < 90)
            {
                DrawLine(from.x + FCos(angle*DEG2RAD)*radius*multiplier*distance, from.y + radius + FSin(angle*DEG2RAD)*radius, from.x + FCos((angle + 10)*DEG2RAD)*radius*multiplier*distance, from.y + radius + FSin((angle + 10)*DEG2RAD)*radius, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));
                angle += 10;
            }

            Vector2 lastPosition = { from.x, from.y + radius*2 };
            DrawLine(lastPosition.x, lastPosition.y, to.x + FCos(270*DEG2RAD)*radius*multiplier, to.y - radius + FSin(270*DEG2RAD)*radius, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));
            lastPosition.x = to.x;

            while (angle < 270)
            {
                DrawLine(to.x + FCos(angle*DEG2RAD)*radius*multiplier*distance, to.y - radius + FSin(angle*DEG2RAD)*radius, to.x + FCos((angle + 10)*DEG2RAD)*radius*multiplier*distance, to.y - radius + FSin((angle + 10)*DEG2RAD)*radius, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));
                angle += 10;
            }

            lastPosition.y = lastPosition.y + radius*2;
            DrawLine(to.x, to.y, to.x, to.y, ((tempLine->id == line->id && tempLine->to == -1) ? DARKGRAY : BLACK));
        }

        if (indexFrom != -1 && indexTo != -1)
        {
            switch (nodes[indexTo]->type)
            {
                case FNODE_POWER:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Input", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Exponent", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_STEP:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Value A", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Value B", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_POSTERIZE:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Input", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Samples", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_LERP:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Value A", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Value B", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 2) && (nodes[indexTo]->inputs[2] == nodes[indexFrom]->id)) DrawText("Time", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_SMOOTHSTEP:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Min", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Max", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 2) && (nodes[indexTo]->inputs[2] == nodes[indexFrom]->id)) DrawText("Value", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_DESATURATE:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Value", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Amount (0-1)", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_MULTIPLYMATRIX:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Left", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Right", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                case FNODE_SAMPLER2D:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Unit", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Type", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                default: break;
            }
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced line");
}

// Draws a previously created comment
FNODEDEF void DrawComment(FComment comment)
{
    if (comment != NULL)
    {
        if ((commentState == 0) || ((commentState == 1) && (tempComment->id != comment->id)) || ((commentState == 1) && editSize != -1)) DrawRectangleRec(comment->shape, Fade(YELLOW, 0.2f));

        DrawRectangleLines(comment->shape.x, comment->shape.y, comment->shape.width, comment->shape.height, BLACK);

        if ((commentState == 0) || ((commentState == 1) && (tempComment->id != comment->id)) || ((commentState == 1) && editSize != -1))
        {
            DrawRectangleRec(comment->sizeTShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(comment->sizeTShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTShape.x, comment->sizeTShape.y, comment->sizeTShape.width, comment->sizeTShape.height, BLACK);
            DrawRectangleRec(comment->sizeBShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(comment->sizeBShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBShape.x, comment->sizeBShape.y, comment->sizeBShape.width, comment->sizeBShape.height, BLACK);
            DrawRectangleRec(comment->sizeLShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(comment->sizeLShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeLShape.x, comment->sizeLShape.y, comment->sizeLShape.width, comment->sizeLShape.height, BLACK);
            DrawRectangleRec(comment->sizeRShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(comment->sizeRShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeRShape.x, comment->sizeRShape.y, comment->sizeRShape.width, comment->sizeRShape.height, BLACK);
            DrawRectangleRec(comment->sizeTlShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(comment->sizeTlShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTlShape.x, comment->sizeTlShape.y, comment->sizeTlShape.width, comment->sizeTlShape.height, BLACK);
            DrawRectangleRec(comment->sizeTrShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(comment->sizeTrShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTrShape.x, comment->sizeTrShape.y, comment->sizeTrShape.width, comment->sizeTrShape.height, BLACK);
            DrawRectangleRec(comment->sizeBlShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(comment->sizeBlShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBlShape.x, comment->sizeBlShape.y, comment->sizeBlShape.width, comment->sizeBlShape.height, BLACK);
            DrawRectangleRec(comment->sizeBrShape, ((CheckCollisionPointRec(GetMousePosition(), CameraToViewRec(comment->sizeBrShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBrShape.x, comment->sizeBrShape.y, comment->sizeBrShape.width, comment->sizeBrShape.height, BLACK);
        }

        if ((comment->id == editComment) && interact)
        {
            int letter = -1;
            letter = GetKeyPressed();

            if (letter != -1)
            {
                if (letter == KEY_BACKSPACE)
                {
                    for (int i = 0; i < MAX_COMMENT_LENGTH; i++)
                    {
                        if ((comment->value[i] == '\0'))
                        {
                            comment->value[i - 1] = '\0';
                            break;
                        }
                    }

                    comment->value[MAX_COMMENT_LENGTH - 1] = '\0';
                }
                else if (letter == KEY_ENTER) editComment = -1;
                else
                {
                    if ((letter >= 32) && (letter < 127))
                    {
                        for (int i = 0; i < MAX_COMMENT_LENGTH; i++)
                        {
                            if (comment->value[i] == '\0')
                            {
                                comment->value[i] = (char)letter;
                                break;
                            }
                        }
                    }
                }
            }
        }

        DrawRectangleLines(comment->valueShape.x, comment->valueShape.y, comment->valueShape.width, comment->valueShape.height, ((editComment == comment->id) ? BLACK : LIGHTGRAY));

        int initPos = comment->shape.x + 14;
        for (int i = 0; i < MAX_COMMENT_LENGTH; i++)
        {
            if (comment->value[i] == '\0') break;
            
            DrawText(FormatText("%c", comment->value[i]), initPos, comment->valueShape.y + 2, 20, DARKGRAY);
            initPos += MeasureText(FormatText("%c", comment->value[i]), 20) + 1;
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced line");
}

// Destroys a node and its linked lines
FNODEDEF void DestroyNode(FNode node)
{
    if (node != NULL)
    {
        int id = node->id;
        int index = GetNodeIndex(id);

        for (int i = linesCount - 1; i >= 0; i--)
        {
            if (lines[i]->from == node->id || lines[i]->to == node->id) DestroyNodeLine(lines[i]);
        }

        for (int i = 0; i < MAX_VALUES; i++)
        {
           FNODE_FREE(nodes[index]->output.data[i].valueText);
           usedMemory -= MAX_NODE_LENGTH;
           nodes[index]->output.data[i].valueText = NULL;
        }

        FNODE_FREE(nodes[index]);
        usedMemory -= sizeof(FNodeData);
        nodes[index] = NULL;

        for (int i = index; i < nodesCount; i++)
        {
            if ((i + 1) < nodesCount) nodes[i] = nodes[i + 1];
        }

        nodesCount--;
        TraceLogFNode(false, "destroyed node id %i (index: %i) [USED RAM: %i bytes]", id, index, usedMemory);
        CalculateValues();
    }
    else TraceLogFNode(true, "error trying to destroy a null referenced node");
}

// Destroys a node line
FNODEDEF void DestroyNodeLine(FLine line)
{
    if (line != NULL)
    {
        int id = line->id;
        int index = -1;

        for (int i = 0; i < linesCount; i++)
        {
            if (lines[i]->id == id)
            {
                index = i;
                break;
            }
        }

        if (index != -1)
        {        
            FNODE_FREE(lines[index]);
            usedMemory -= sizeof(FLineData);
            lines[index] = NULL;

            for (int i = index; i < linesCount; i++)
            {
                if ((i + 1) < linesCount) lines[i] = lines[i + 1];
            }

            linesCount--;
            TraceLogFNode(false, "destroyed line id %i (index: %i) [USED RAM: %i bytes]", id, index, usedMemory);
            CalculateValues();
        }
        else TraceLogFNode(true, "error when trying to destroy line id %i due to index is out of bounds %i", id, index);
    }
    else TraceLogFNode(true, "error trying to destroy a null referenced line");
}

// Destroys a comment
FNODEDEF void DestroyComment(FComment comment)
{
    if (comment != NULL)
    {
        int id = comment->id;
        int index = -1;

        for (int i = 0; i < commentsCount; i++)
        {
            if (comments[i]->id == id)
            {
                index = i;
                break;
            }
        }

        if (index != -1)
        {
            FNODE_FREE(comments[index]->value);
            usedMemory -= MAX_COMMENT_LENGTH;
            comments[index]->value = NULL;

            FNODE_FREE(comments[index]);
            usedMemory -= sizeof(FCommentData);
            comments[index] = NULL;

            for (int i = index; i < commentsCount; i++)
            {
                if ((i + 1) < commentsCount) comments[i] = comments[i + 1];
            }

            commentsCount--;

            TraceLogFNode(false, "destroyed comment id %i (index: %i) [USED RAM: %i bytes]", id, index, usedMemory);
        }
        else TraceLogFNode(true, "error when trying to destroy comment id %i due to index is out of bounds %i", id, index);
    }
    else TraceLogFNode(true, "error trying to destroy a null referenced comment");
}

// Unitializes FNode global variables
FNODEDEF void CloseFNode()
{
    for (int i = 0; i < nodesCount; i++)
    {
        if (nodes[i] != NULL)
        {
            for (int k = 0; k < MAX_VALUES; k++)
            {
                if (nodes[i]->output.data[k].valueText != NULL)
                {
                    FNODE_FREE(nodes[i]->output.data[k].valueText);
                    usedMemory -= MAX_NODE_LENGTH;
                    nodes[i]->output.data[k].valueText = NULL;
                }
            }

            FNODE_FREE(nodes[i]);
            usedMemory -= sizeof(FNodeData);
            nodes[i] = NULL;
        }
    }

    for (int i = 0; i < linesCount; i++)
    {
        if (lines[i] != NULL)
        {
            FNODE_FREE(lines[i]);
            usedMemory -= sizeof(FLineData);
            lines[i] = NULL;
        }
    }

    for (int i = 0; i < commentsCount; i++)
    {
        if ((comments[i] != NULL) && (comments[i]->value != NULL))
        {
            FNODE_FREE(comments[i]->value);
            usedMemory -= MAX_COMMENT_LENGTH;
            comments[i]->value = NULL;

            FNODE_FREE(comments[i]);
            usedMemory -= sizeof(FCommentData);
            comments[i] = NULL;
        }
    }

    if (editNodeText != NULL)
    {
        FNODE_FREE(editNodeText);
        usedMemory -= MAX_NODE_LENGTH;
        editNodeText = NULL;
    }

    nodesCount = 0;
    linesCount = 0;
    commentsCount = 0;
    selectedCommentNodesCount = 0;

    for (int i = 0; i < MAX_NODES; i++) selectedCommentNodes[i] = -1;

    TraceLogFNode(false, "unitialization complete [USED RAM: %i bytes]", usedMemory);
}

// Outputs a trace log message
FNODEDEF void TraceLogFNode(bool error, const char *text, ...)
{
    va_list args;

    fprintf(stdout, "FNode: ");
    va_start(args, text);
    vfprintf(stdout, text, args);
    va_end(args);

    fprintf(stdout, "\n");

    if (error) exit(1);
}

// Returns 1 if a specific string is found in a text file
FNODEDEF int FSearch(char *filename, char *string)
{
	FILE *file;
	int found = 0;
	char temp[512];

	file = fopen(filename, "r");

    if (file != NULL)
    {
        while (fgets(temp, 512, file) != NULL) 
        {
            if ((strstr(temp, string)) != NULL)
            {
                found = 1;
                break;
            }
        }

        fclose(file);
    }

   	return found;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
// Returns length of a Vector2
static float FVector2Length(Vector2 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y);
}

// Returns length of a Vector3
static float FVector3Length(Vector3 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

// Returns length of a Vector4
static float FVector4Length(Vector4 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

// Returns a normalized Vector2
static Vector2 FVector2Normalize(Vector2 v)
{
    float length = FVector2Length(v);
    return (Vector2){ v.x/length, v.y/length };
}

// Returns a normalized Vector3
static Vector3 FVector3Normalize(Vector3 v)
{
    float length = FVector3Length(v);
    return (Vector3){ v.x/length, v.y/length, v.z/length };
}

// Returns a normalized Vector4
static Vector4 FVector4Normalize(Vector4 v)
{
    float length = FVector4Length(v);
    return (Vector4){ v.x/length, v.y/length, v.z/length, v.w/length };
}

// Returns the projection vector of two Vector2
static Vector2 FVector2Projection(Vector2 a, Vector2 b)
{
    Vector2 output = { 0, 0 };

    float dotAB = FVector2Dot(a, b);
    float dotBB = FVector2Dot(b, b);
    output.x = dotAB/dotBB*b.x;
    output.y = dotAB/dotBB*b.y;

    return output;
}

// Returns the projection vector of two Vector3
static Vector3 FVector3Projection(Vector3 a, Vector3 b)
{
    Vector3 output = { 0, 0, 0 };

    float dotAB = FVector3Dot(a, b);
    float dotBB = FVector3Dot(b, b);
    output.x = dotAB/dotBB*b.x;
    output.y = dotAB/dotBB*b.y;
    output.z = dotAB/dotBB*b.z;

    return output;
}

// Returns the projection vector of two Vector4
static Vector4 FVector4Projection(Vector4 a, Vector4 b)
{
    Vector4 output = { 0, 0, 0, 0 };

    float dotAB = FVector4Dot(a, b);
    float dotBB = FVector4Dot(b, b);
    output.x = dotAB/dotBB*b.x;
    output.y = dotAB/dotBB*b.y;
    output.z = dotAB/dotBB*b.z;
    output.w = dotAB/dotBB*b.w;

    return output;
}

// Returns the rejection vector of two Vector2
static Vector2 FVector2Rejection(Vector2 a, Vector2 b)
{
    Vector2 output = { 0, 0 };

    Vector2 projection = FVector2Projection(a, b);
    output.x = a.x - projection.x;
    output.y = a.y - projection.y;

    return output;
}

// Returns the rejection vector of two Vector3
static Vector3 FVector3Rejection(Vector3 a, Vector3 b)
{
    Vector3 output = { 0, 0, 0 };

    Vector3 projection = FVector3Projection(a, b);
    output.x = a.x - projection.x;
    output.y = a.y - projection.y;
    output.z = a.z - projection.z;

    return output;
}

// Returns the rejection vector of two Vector4
static Vector4 FVector4Rejection(Vector4 a, Vector4 b)
{
    Vector4 output = { 0, 0, 0, 0 };

    Vector4 projection = FVector4Projection(a, b);
    output.x = a.x - projection.x;
    output.y = a.y - projection.y;
    output.z = a.z - projection.z;
    output.w = a.w - projection.w;

    return output;
}

// Returns the dot product of two Vector2
static float FVector2Dot(Vector2 a, Vector2 b)
{
    return (float)((a.x*b.x) + (a.y*b.y));
}

// Returns the dot product of two Vector3
static float FVector3Dot(Vector3 a, Vector3 b)
{
    return (float)((a.x*b.x) + (a.y*b.y) + (a.z*b.z));
}

// Returns the dot product of two Vector4
static float FVector4Dot(Vector4 a, Vector4 b)
{
    return (float)((a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w));
}

// Returns the cross product of two vectors
static Vector3 FCrossProduct(Vector3 a, Vector3 b)
{  
    return (Vector3){ a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

// Returns identity matrix
static Matrix FMatrixIdentity()
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f, 
                      0.0f, 1.0f, 0.0f, 0.0f, 
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Returns the result of multiply two matrices
static Matrix FMatrixMultiply(Matrix left, Matrix right)
{
    Matrix result;

    result.m0 = right.m0*left.m0 + right.m1*left.m4 + right.m2*left.m8 + right.m3*left.m12;
    result.m1 = right.m0*left.m1 + right.m1*left.m5 + right.m2*left.m9 + right.m3*left.m13;
    result.m2 = right.m0*left.m2 + right.m1*left.m6 + right.m2*left.m10 + right.m3*left.m14;
    result.m3 = right.m0*left.m3 + right.m1*left.m7 + right.m2*left.m11 + right.m3*left.m15;
    result.m4 = right.m4*left.m0 + right.m5*left.m4 + right.m6*left.m8 + right.m7*left.m12;
    result.m5 = right.m4*left.m1 + right.m5*left.m5 + right.m6*left.m9 + right.m7*left.m13;
    result.m6 = right.m4*left.m2 + right.m5*left.m6 + right.m6*left.m10 + right.m7*left.m14;
    result.m7 = right.m4*left.m3 + right.m5*left.m7 + right.m6*left.m11 + right.m7*left.m15;
    result.m8 = right.m8*left.m0 + right.m9*left.m4 + right.m10*left.m8 + right.m11*left.m12;
    result.m9 = right.m8*left.m1 + right.m9*left.m5 + right.m10*left.m9 + right.m11*left.m13;
    result.m10 = right.m8*left.m2 + right.m9*left.m6 + right.m10*left.m10 + right.m11*left.m14;
    result.m11 = right.m8*left.m3 + right.m9*left.m7 + right.m10*left.m11 + right.m11*left.m15;
    result.m12 = right.m12*left.m0 + right.m13*left.m4 + right.m14*left.m8 + right.m15*left.m12;
    result.m13 = right.m12*left.m1 + right.m13*left.m5 + right.m14*left.m9 + right.m15*left.m13;
    result.m14 = right.m12*left.m2 + right.m13*left.m6 + right.m14*left.m10 + right.m15*left.m14;
    result.m15 = right.m12*left.m3 + right.m13*left.m7 + right.m14*left.m11 + right.m15*left.m15;

    return result;
}

// Returns translation matrix
static Matrix FMatrixTranslate(float x, float y, float z)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f, 
                      0.0f, 1.0f, 0.0f, 0.0f, 
                      0.0f, 0.0f, 1.0f, 0.0f, 
                      x, y, z, 1.0f };

    return result;
}

// Create rotation matrix from axis and angle provided in radians
static Matrix FMatrixRotate(Vector3 axis, float angle)
{
    Matrix result;

    Matrix mat = FMatrixIdentity();
    float x = axis.x, y = axis.y, z = axis.z;
    float length = sqrt(x*x + y*y + z*z);

    if ((length != 1.0f) && (length != 0.0f))
    {
        length = 1.0f/length;
        x *= length;
        y *= length;
        z *= length;
    }

    float sinres = sinf(angle);
    float cosres = cosf(angle);
    float t = 1.0f - cosres;

    // Cache some matrix values (speed optimization)
    float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;

    // Construct the elements of the rotation matrix
    float b00 = x*x*t + cosres, b01 = y*x*t + z*sinres, b02 = z*x*t - y*sinres;
    float b10 = x*y*t - z*sinres, b11 = y*y*t + cosres, b12 = z*y*t + x*sinres;
    float b20 = x*z*t + y*sinres, b21 = y*z*t - x*sinres, b22 = z*z*t + cosres;

    // Perform rotation-specific matrix multiplication
    result.m0 = a00*b00 + a10*b01 + a20*b02;
    result.m1 = a01*b00 + a11*b01 + a21*b02;
    result.m2 = a02*b00 + a12*b01 + a22*b02;
    result.m3 = a03*b00 + a13*b01 + a23*b02;
    result.m4 = a00*b10 + a10*b11 + a20*b12;
    result.m5 = a01*b10 + a11*b11 + a21*b12;
    result.m6 = a02*b10 + a12*b11 + a22*b12;
    result.m7 = a03*b10 + a13*b11 + a23*b12;
    result.m8 = a00*b20 + a10*b21 + a20*b22;
    result.m9 = a01*b20 + a11*b21 + a21*b22;
    result.m10 = a02*b20 + a12*b21 + a22*b22;
    result.m11 = a03*b20 + a13*b21 + a23*b22;
    result.m12 = mat.m12;
    result.m13 = mat.m13;
    result.m14 = mat.m14;
    result.m15 = mat.m15;

    return result;
}

// Returns scaling matrix
static Matrix FMatrixScale(float x, float y, float z)
{
    Matrix result = { x, 0.0f, 0.0f, 0.0f, 
                      0.0f, y, 0.0f, 0.0f, 
                      0.0f, 0.0f, z, 0.0f, 
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Transposes provided matrix
static void FMatrixTranspose(Matrix *mat)
{
    Matrix temp;

    temp.m0 = mat->m0;
    temp.m1 = mat->m4;
    temp.m2 = mat->m8;
    temp.m3 = mat->m12;
    temp.m4 = mat->m1;
    temp.m5 = mat->m5;
    temp.m6 = mat->m9;
    temp.m7 = mat->m13;
    temp.m8 = mat->m2;
    temp.m9 = mat->m6;
    temp.m10 = mat->m10;
    temp.m11 = mat->m14;
    temp.m12 = mat->m3;
    temp.m13 = mat->m7;
    temp.m14 = mat->m11;
    temp.m15 = mat->m15;

    *mat = temp;
}

// Transform a quaternion given a transformation matrix
static void FMultiplyMatrixVector(Vector4 *v, Matrix mat)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;
    float w = v->w;

    v->x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12*w;
    v->y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13*w;
    v->z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14*w;
    v->w = mat.m3*x + mat.m7*y + mat.m11*z + mat.m15*w;
}

// Returns the cosine value of a radian angle
static float FCos(float value)
{    
    return (float)cos(value);
}

// Returns the sine value of a radian angle
static float FSin(float value)
{    
    return (float)sin(value);
}

// Returns the tangent value of a radian angle
static float FTan(float value)
{    
    return (float)tan(value);
}

// Returns a value to the power of an exponent
static float FPower(float value, float exp)
{    
    return (float)pow(value, exp);
}

// Returns the square root of the input value
static float FSquareRoot(float value)
{    
    return (float)sqrt(value);
}

// Returns a value rounded based on the samples
static float FPosterize(float value, float samples)
{
    float output = value*samples;

    output = (float)floor(output);
    output /= samples;

    return output;
}

// Returns a value clamped by a min and max values
static float FClamp(float value, float min, float max)
{
    float output = value;

    if (output < min) output = min;
    else if (output > max) output = max;

    return output;
}

// Returns a truncated value of a value
static float FTrunc(float value)
{
    float output = value;

    int truncated = (int)output;
    output = (float)truncated;

    return output;
}

// Returns a rounded value of a value
static float FRound(float value)
{
    float output = fabs(value);

    int truncated = (int)output;
    float decimals = output - (float)truncated;

    output = ((decimals > 0.5f) ? (output - decimals + 1.0f) : (output - decimals));
    if (value < 0.0f) output *= -1;

    return output;
}

// Returns a rounded up to the nearest integer of a value
static float FCeil(float value)
{
    float output = value;

    int truncated = (int)output;
    if (output != (float)truncated) output = (float)((output >= 0) ? (truncated + 1) : truncated);

    return output;
}

// Returns the interpolation between two values
static float FLerp(float valueA, float valueB, float time)
{
    return (float)(valueA + (valueB - valueA)*time);
}

// Returns the interpolation between two Vector2 values
static Vector2 FVector2Lerp(Vector2 valueA, Vector2 valueB, float time)
{
    return (Vector2){ (float)(valueA.x + (valueB.x - valueA.x)*time), (float)(valueA.y + (valueB.y - valueA.y)*time) };
}

// Returns the interpolation between two Vector3 values
static Vector3 FVector3Lerp(Vector3 valueA, Vector3 valueB, float time)
{
    return (Vector3){ (float)(valueA.x + (valueB.x - valueA.x)*time), (float)(valueA.y + (valueB.y - valueA.y)*time), (float)(valueA.z + (valueB.z - valueA.z)*time) };
}

// Returns the interpolation between two Vector4 values
static Vector4 FVector4Lerp(Vector4 valueA, Vector4 valueB, float time)
{
    return (Vector4){ (float)(valueA.x + (valueB.x - valueA.x)*time), (float)(valueA.y + (valueB.y - valueA.y)*time), (float)(valueA.z + (valueB.z - valueA.z)*time), (float)(valueA.w + (valueB.w - valueA.w)*time) };
}

// Returns the interpolate of a value in a range
static float FSmoothStep(float min, float max, float value)
{
    float output = 0.0f;

    if (min <= max) output = FClamp((value/(max - min) - min), 0.0f, 1.0f);
    else output = FClamp((value/(min - max) - max), 0.0f, 1.0f);

    return output;
}

// Returns an ease linear value between two parameters
static float FEaseLinear(float t, float b, float c, float d)
{ 
    return (float)(c*t/d + b); 
}

// Returns an ease quadratic in-out value between two parameters
static float FEaseInOutQuad(float t, float b, float c, float d)
{
    float output = 0.0f;

	t /= d/2;
	if (t < 1) output = (float)(c/2*t*t + b);
    else
    {
        t--;
        output = (float)(-c/2*(t*(t-2) - 1) + b);
    }

	return output;
}

// Sends a float conversion value of a string to an initialized float pointer
static void FStringToFloat(float *pointer, const char *string)
{
    *pointer = (float)atof(string);
}

// Sends formatted output to an initialized string pointer
static void FFloatToString(char *buffer, float value)
{
    sprintf(buffer, "%.3f", value);
}

#endif  // FNODE_IMPLEMENTATION
