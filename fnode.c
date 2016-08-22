/*
---- TODO LIST ----
    - Change node debug info text drawing position
    - Add comments system
    - Add value change feature
*/

#include <stdlib.h>         // Required for: malloc(), free()
#include <math.h>           // Required for: cos(), sin(), sqrt(), pow(), floor()
#include "raylib.h"         // Required for window management, 2D camera drawing and inputs detection

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Defines
#define     MAX_INPUTS          8           // Max number of inputs in every node
#define     MAX_VALUES          4           // Max number of values in every output
#define     MAX_NODES           256         // Max number of nodes
#define     MAX_LINES           1024        // Max number of lines
#define     UI_PADDING          5           // Interface bounds padding with background
#define     UI_BUTTON_HEIGHT    30          // Interface bounds height
#define     UI_SCROLL           10          // Interface scroll sensitivity
#define     UI_GRID_SPACING     25          // Interface canvas background grid divisions length
#define     UI_GRID_ALPHA       0.25f       // Interface canvas background grid lines alpha
#define     UI_GRID_COUNT       500         // Interface canvas background grid divisions count

// Enums
typedef enum FNodeType {
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
    FNODE_TRUNC,
    FNODE_ROUND,
    FNODE_CEIL,
    FNODE_CLAMP01,
    FNODE_EXP2,
    FNODE_POWER,
    FNODE_POSTERIZE,
    FNODE_MAX,
    FNODE_MIN,
    FNODE_NOISE,
    FNODE_NOISEINT
} FNodeType;

// Structs
typedef struct FNodeValue {
    float values[MAX_VALUES];               // Output values array (float, Vector2, Vector3 or Vector4)
    int valuesCount;                        // Output values length to know which type of data represents
} FNodeValue;

typedef struct FNodeData {
    unsigned int id;                        // Node unique identifier
    FNodeType type;                         // Node type (values, operations...)
    const char *name;                       // Displayed name of the node
    int inputs[MAX_INPUTS];                 // Inputs node ids array
    unsigned int inputsCount;               // Inputs node ids array length
    unsigned int inputsLimit;               // Inputs node ids length limit
    FNodeValue output;                      // Value output (contains the value and its length)
    Rectangle shape;                        // Node rectangle data
    Rectangle inputShape;                   // Node input rectangle data (automatically calculated from shape)
    Rectangle outputShape;                  // Node input rectangle data (automatically calculated from shape)
} FNodeData, *FNode;

typedef struct FNodeLine {
    unsigned int id;                        // Line unique identifier
    unsigned int from;                      // Id from line start node
    unsigned int to;                        // Id from line end node
} FNodeLine, *FLine;

typedef struct FComment {
    unsigned int id;                        // Comment unique identifier
    char *value;                            // Comment text label value
    Rectangle shape;                        // Comment label rectangle data
} FComment;

typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} Vector4;

// Global variables
int usedMemory = 0;                         // Total USED RAM from memory allocation

int nodesCount;                             // Created nodes count
FNode nodes[MAX_NODES];                     // Nodes pointers pool
int selectedId = -1;                        // Current selected node to drag in screen

int linesCount;                             // Created lines count
FLine lines[MAX_LINES];                     // Lines pointers pool
int lineState = 0;                          // Current line linking state (0 = waiting for output to link, 1 = output selected, waiting for input to link)
FLine tempLine = NULL;                      // Temporally created line during line states

Vector2 screenSize = { 1280, 720 };         // Window screen width      
Vector2 mousePosition = { 0, 0 };           // Current mouse position 
Vector2 lastMousePosition = { 0, 0 };       // Previous frame mouse position
Vector2 mouseDelta = { 0, 0 };              // Current frame mouse position increment since previous frame
Vector2 currentOffset = { 0, 0 };           // Current selected node offset between mouse position and node shape

bool debugMode = false;                     // Drawing debug information state
Camera2D camera;                            // Node area 2d camera for panning
Vector2 canvasSize;                         // Interface screen size
float menuScroll = 0.0f;                    // Current interface scrolling amount
Vector2 scrollLimits = { 0, 335 };          // Interface scrolling limits

// Functions declarations
void UpdateMouseInputs();                   // Updates mouse inputs to drag, link nodes and canvas camera movement
void UpdateValues();                        // Updates nodes output values based on current inputs
void CalculateValues();                     // Calculates nodes output values based on current inputs
void UpdateRectangles();                    // Updates node shapes due to drag behaviour
void AlignNode(FNode node);                 // Aligns a node to the nearest grid intersection
void AlignAllNodes();                       // Aligns all created nodes
void ClearUnusedNodes();                    // Destroys all unused nodes
void ClearGraph();                          // Destroys all created nodes and its linked lines
void DrawCanvas();                          // Draw canvas space to create nodes
void DrawCanvasGrid(int divisions);         // Draw canvas grid with a specific number of divisions for horizontal and vertical lines
void DrawInterface();                       // Draw interface to create nodes

Vector2 CameraToViewVector2(Vector2 vector, Camera2D camera);   // Converts Vector2 coordinates from world space to Camera2D space based on its offset
Rectangle CameraToViewRec(Rectangle rec, Camera2D camera);      // Converts rectangle coordinates from world space to Camera2D space based on its offset

// FNODE functions declarations
void InitFNode();                               // Initializes FNode global variables

FNode CreateNodeValue(float value);             // Creates a value node (1 float)
FNode CreateNodeVector2(Vector2 vector);        // Creates a Vector2 node (2 float)
FNode CreateNodeVector3(Vector3 vector);        // Creates a Vector3 node (3 float)
FNode CreateNodeVector4(Vector4 vector);        // Creates a Vector4 node (4 float)
FNode CreateNodeAdd();                          // Creates a node to add multiples values
FNode CreateNodeSubtract();                     // Creates a node to subtract multiples values by the first linked
FNode CreateNodeMultiply();                     // Creates a node to multiply different values
FNode CreateNodeDivide();                       // Creates a node to divide multiples values by the first linked
FNode CreateNodeAppend();                       // Creates a node to combine little data types into bigger data types (2 int = 1 Vector2)
FNode CreateNodeOneMinus();                     // Creates a node to subtract a value to 1
FNode CreateNodeAbs();                          // Creates a node to get absolute value of the input
FNode CreateNodeCos();                          // Creates a node to get cos value of the input
FNode CreateNodeSin();                          // Creates a node to get sin value of the input
FNode CreateNodeTan();                          // Creates a node to get tan value of the input
FNode CreateNodeDeg2Rad();                      // Creates a node to convert a degrees value to radians
FNode CreateNodeRad2Deg();                      // Creates a node to convert a radians value to degrees
FNode CreateNodeNormalize();                    // Creates a node to normalize a vector
FNode CreateNodeNegate();                       // Creates a node to get negative value of the input
FNode CreateNodeReciprocal();                   // Creates a node to get 1/input value
FNode CreateNodeTrunc();                        // Creates a node to get the truncated value of the input
FNode CreateNodeRound();                        // Creates a node to get the rounded value of the input
FNode CreateNodeCeil();                         // Creates a node to get the rounded up to the nearest integer of the input
FNode CreateNodeClamp01();                      // Creates a node to clamp the input value between 0-1 range
FNode CreateNodeExp2();                         // Creates a node that returns 2 to the power of its input
FNode CreateNodePower();                        // Creates a node that returns the first input to the power of the second input
FNode CreateNodePosterize();                    // Creates a node that posterizes the first input in a number of steps set by second input
FNode CreateNodeMax();                          // Creates a node to calculate the higher value of all inputs values
FNode CreateNodeMin();                          // Creates a node to calculate the lower value of all inputs values
FNode CreateNodeNoise();                        // Creates a node to calculate random values between input range
FNode CreateNodeNoiseInt();                     // Creates a node to calculate integer random values between input range
FLine CreateNodeLine();                         // Creates a line between two nodes

FNode InitializeNode();                         // Initializes a new node with generic parameters

void DrawNode(FNode node);                      // Draws a previously created node
void DrawNodeLine(FLine line);                  // Draws a previously created node line

void DestroyNode(FNode node);                   // Destroys a node and its linked lines
void DestroyNodeLine(FLine line);               // Destroys a node line
void CloseFNode();                              // Unitializes FNode global variables

// Math functions declarations
float FVector2Length(Vector2 v);                    // Returns length of a Vector2
float FVector3Length(Vector3 v);                    // Returns length of a Vector3
float FVector4Length(Vector4 v);                    // Returns length of a Vector4
Vector2 FVector2Normalize(Vector2 v);               // Returns a normalized Vector2
Vector3 FVector3Normalize(Vector3 v);               // Returns a normalized Vector3
Vector4 FVector4Normalize(Vector4 v);               // Returns a normalized Vector4
float FPower(float value, float exp);               // Returns a value to the power of an exponent
float FPosterize(float value, float samples);       // Returns a value rounded based on the samples
float FClamp(float value, float min, float max);    // Returns a value clamped by a min and max values
float FTrunc(float value);                          // Returns a truncated value of a value
float FRound(float value);                          // Returns a rounded value of a value
float FCeil(float value);                           // Returns a rounded up to the nearest integer of a value
void FLog(bool error, const char *text, ...);       // Outputs a trace log message

int main()
{    
    // Initialization
    //--------------------------------------------------------------------------------------
    // SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenSize.x, screenSize.y, "fnode 0.2");
    SetTargetFPS(60);
    
    // Initialize values
    camera = (Camera2D){ (Vector2){ 0, 0 }, (Vector2){ screenSize.x/2, screenSize.y/2 }, 0.0f, 1.0f };
    canvasSize = (Vector2){ screenSize.x*0.85f, screenSize.y };
    
    InitFNode();
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateMouseInputs();
        
        if (IsKeyPressed('P')) debugMode = !debugMode;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            DrawCanvas();
            
            DrawInterface();
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseFNode();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Updates mouse inputs to drag, link nodes and canvas camera movement
void UpdateMouseInputs()
{
    // Update mouse position values
    lastMousePosition = mousePosition;
    mousePosition = GetMousePosition();
    mouseDelta = (Vector2){ mousePosition.x - lastMousePosition.x, mousePosition.y - lastMousePosition.y };
    
    // Update canvas camera values
    camera.target = mousePosition;
    camera.offset.x += mouseDelta.x*(camera.zoom - 1);
    camera.offset.y += mouseDelta.y*(camera.zoom - 1);
    
    // Check zoom input
    if (GetMouseWheelMove() != 0)
    {
        if (CheckCollisionPointRec(mousePosition, (Rectangle){ 0, 0, canvasSize.x, canvasSize.y }))
        {
            if (IsKeyDown(KEY_LEFT_ALT)) camera.offset.x -= GetMouseWheelMove()*UI_SCROLL;
            else camera.offset.y -= GetMouseWheelMove()*UI_SCROLL;
        }
        else
        {
            menuScroll -= GetMouseWheelMove()*UI_SCROLL;
            menuScroll = FClamp(menuScroll, scrollLimits.x, scrollLimits.y);
        }
    }
    
    // Check node drag input
    if ((selectedId == -1) && (lineState == 0))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            for (int i = 0; i < nodesCount; i++)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->shape, camera)))
                {
                    selectedId = nodes[i]->id;
                    currentOffset = (Vector2){ mousePosition.x - nodes[i]->shape.x, mousePosition.y - nodes[i]->shape.y };
                    break;
                }
            }
            
            if ((selectedId == -1) && (CheckCollisionPointRec(mousePosition, (Rectangle){ 0, 0, screenSize.x, screenSize.y })))
            {
                camera.offset.x += mouseDelta.x;
                camera.offset.y += mouseDelta.y;
            }
        }
        else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            for (int i = 0; i < nodesCount; i++)
            {
                if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->shape, camera)))
                {
                    DestroyNode(nodes[i]);
                    CalculateValues();
                    break;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < nodesCount; i++)
        {
            if (nodes[i]->id == selectedId)
            {
                nodes[i]->shape.x = mousePosition.x - currentOffset.x;
                nodes[i]->shape.y = mousePosition.y - currentOffset.y;
                
                // Check aligned drag movement input
                if (IsKeyDown(KEY_LEFT_ALT)) AlignNode(nodes[i]);
                
                UpdateRectangles();
                break;
            }
        }
        
        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) selectedId = -1;
    }
    
    // Check node link input
    if (selectedId == -1)
    {
        switch (lineState)
        {
            case 0:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    for (int i = 0; i < nodesCount; i++)
                    {
                        if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->outputShape, camera)))
                        {
                            tempLine = CreateNodeLine(nodes[i]->id);
                            lineState = 1;
                            break;
                        }
                    }
                }
                else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                {
                    for (int i = 0; i < nodesCount; i++)
                    {
                        if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->outputShape, camera)))
                        {
                            for (int k = linesCount - 1; k >= 0; k--)
                            {
                                if (nodes[i]->id == lines[k]->from) DestroyNodeLine(lines[k]);
                            }
                            
                            CalculateValues();
                        }
                        else if (CheckCollisionPointRec(mousePosition, CameraToViewRec(nodes[i]->inputShape, camera)))
                        {
                            for (int k = linesCount - 1; k >= 0; k--)
                            {
                                if (nodes[i]->id == lines[k]->to) DestroyNodeLine(lines[k]);
                            }
                            
                            CalculateValues();
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
                            int indexFrom = -1;
                            for (int k = 0; k < nodesCount; k++)
                            {
                                if (nodes[k]->id == tempLine->from)
                                {
                                    indexFrom = k;
                                    break;
                                }
                            }
                            
                            if (indexFrom != -1)
                            {
                                bool valuesCheck = true;
                                if (nodes[i]->type == FNODE_APPEND) valuesCheck = (nodes[i]->output.valuesCount + nodes[indexFrom]->output.valuesCount <= MAX_VALUES);
                                else if ((nodes[i]->type == FNODE_POWER) && (nodes[i]->inputsCount == 1)) valuesCheck = (nodes[indexFrom]->output.valuesCount == 1);
                                else if (nodes[i]->type == FNODE_NOISE) valuesCheck = (nodes[indexFrom]->output.valuesCount == 2);
                                else if (nodes[i]->type == FNODE_NORMALIZE) valuesCheck = (nodes[indexFrom]->output.valuesCount > 1);
                                else valuesCheck = (nodes[i]->output.valuesCount == nodes[indexFrom]->output.valuesCount);
                                
                                if (((nodes[i]->inputsCount == 0) && (nodes[i]->type != FNODE_NOISE) && (nodes[i]->type != FNODE_NORMALIZE)) || valuesCheck)
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
                                    break;
                                }
                                else FLog(false, "error trying to link node ID %i (length: %i) with node ID %i (length: %i)", nodes[i]->id, nodes[i]->output.valuesCount, nodes[indexFrom]->id, nodes[indexFrom]->output.valuesCount);
                            }
                            else FLog(false, "error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, indexFrom);
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

// Calculates nodes output values based on current inputs
void CalculateValues()
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
                    int index = -1;
                    for (int k = 0; k < nodesCount; k++)
                    {
                        if (nodes[k]->id == nodes[i]->inputs[0])
                        {
                            index = k;
                            break;
                        }
                    }
                    
                    if (index != -1) nodes[i]->output = nodes[index]->output;
                    else FLog(false, "error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, index);
                    
                    for (int k = 1; k < nodes[i]->inputsCount; k++)
                    {
                        int inputIndex = -1;
                        for (int j = 0; j < nodesCount; j++)
                        {
                            if (nodes[j]->id == nodes[i]->inputs[k])
                            {
                                inputIndex = j;
                                break;
                            }
                        }
                        
                        if (inputIndex != -1)
                        {
                            // Apply inputs values to output based on node operation type
                            switch (nodes[i]->type)
                            {
                                case FNODE_ADD:
                                {
                                    for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] += nodes[inputIndex]->output.values[j];
                                } break;
                                case FNODE_SUBTRACT:
                                {
                                    for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] -= nodes[inputIndex]->output.values[j];
                                } break;
                                case FNODE_MULTIPLY:
                                {
                                    for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] *= nodes[inputIndex]->output.values[j];
                                } break;
                                case FNODE_DIVIDE:
                                {
                                    for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] /= nodes[inputIndex]->output.values[j];
                                } break;
                                default: break;
                            }
                        }
                        else FLog(false, "error trying to get node id %i due to index is out of bounds %i", nodes[i]->inputs[k], inputIndex);
                    }
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.values[k] = 0.0f;
                    nodes[i]->output.valuesCount = 0;
                }
            }
            else if (nodes[i]->type == FNODE_APPEND)
            {
                if (nodes[i]->inputsCount > 0)
                {
                    int valuesCount = 0;
                    for (int k = 0; k < nodes[i]->inputsCount; k++)
                    {
                        int inputIndex = -1;
                        for (int j = 0; j < nodesCount; j++)
                        {
                            if (nodes[j]->id == nodes[i]->inputs[k])
                            {
                                inputIndex = j;
                                break;
                            }
                        }
                        
                        if (inputIndex != -1)
                        {
                            for (int j = 0; j < nodes[inputIndex]->output.valuesCount; j++)
                            {
                                nodes[i]->output.values[valuesCount] = nodes[inputIndex]->output.values[j];
                                valuesCount++;
                            }
                        }
                        else FLog(false, "error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, inputIndex);
                    }
                    
                    nodes[i]->output.valuesCount = valuesCount;
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.values[k] = 0.0f;
                    nodes[i]->output.valuesCount = 0;
                }
            }
            else if (nodes[i]->type > FNODE_APPEND)
            {
                if (nodes[i]->inputsCount > 0)
                {
                    int index = -1;
                    for (int j = 0; j < nodesCount; j++)
                    {
                        if (nodes[j]->id == nodes[i]->inputs[0])
                        {
                            index = j;
                            break;
                        }
                    }
                    
                    if (index != -1)
                    {
                        nodes[i]->output = nodes[index]->output;
                        
                        switch (nodes[i]->type)
                        {
                            case FNODE_ONEMINUS:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = 1 - nodes[i]->output.values[j];
                            } break;
                            case FNODE_ABS:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++)
                                {
                                    if (nodes[i]->output.values[j] < 0) nodes[i]->output.values[j] *= -1;
                                }
                            } break;
                            case FNODE_COS:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = (float)cos(nodes[i]->output.values[j]);
                            } break;
                            case FNODE_SIN:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = (float)sin(nodes[i]->output.values[j]);
                            } break;
                            case FNODE_TAN:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = (float)tan(nodes[i]->output.values[j]);
                            } break;
                            case FNODE_DEG2RAD:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] *= DEG2RAD;
                            } break;
                            case FNODE_RAD2DEG:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] *= RAD2DEG;
                            } break;
                            case FNODE_NORMALIZE:
                            {
                                switch (nodes[i]->output.valuesCount)
                                {
                                    case 2:
                                    {
                                        Vector2 temp = { nodes[i]->output.values[0], nodes[i]->output.values[1] };
                                        temp = FVector2Normalize(temp);
                                        nodes[i]->output.values[0] = temp.x;
                                        nodes[i]->output.values[1] = temp.y;
                                    } break;
                                    case 3:
                                    {
                                        Vector3 temp = { nodes[i]->output.values[0], nodes[i]->output.values[1], nodes[i]->output.values[2] };
                                        temp = FVector3Normalize(temp);
                                        nodes[i]->output.values[0] = temp.x;
                                        nodes[i]->output.values[1] = temp.y;
                                        nodes[i]->output.values[2] = temp.z;
                                    } break;
                                    case 4:
                                    {
                                        Vector4 temp = { nodes[i]->output.values[0], nodes[i]->output.values[1], nodes[i]->output.values[2], nodes[i]->output.values[3] };
                                        temp = FVector4Normalize(temp);
                                        nodes[i]->output.values[0] = temp.x;
                                        nodes[i]->output.values[1] = temp.y;
                                        nodes[i]->output.values[2] = temp.z;
                                        nodes[i]->output.values[3] = temp.w;
                                    } break;
                                    default: break;
                                }
                            } break;
                            case FNODE_NEGATE:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] *= -1;
                            } break;
                            case FNODE_RECIPROCAL:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = 1/nodes[i]->output.values[j];
                            } break;
                            case FNODE_TRUNC:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = FTrunc(nodes[i]->output.values[j]);
                            } break;
                            case FNODE_ROUND:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = FRound(nodes[i]->output.values[j]);
                            } break;
                            case FNODE_CEIL:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = FCeil(nodes[i]->output.values[j]);
                            } break;
                            case FNODE_CLAMP01:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = FClamp(nodes[i]->output.values[j], 0.0f, 1.0f);
                            } break;
                            case FNODE_EXP2:
                            {
                                for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = FPower(2.0f, nodes[i]->output.values[j]);
                            } break;
                            case FNODE_POWER:
                            {
                                int expIndex = -1;
                                for (int j = 0; j < nodesCount; j++)
                                {
                                    if (nodes[j]->id == nodes[i]->inputs[1])
                                    {
                                        expIndex = j;
                                        break;
                                    }
                                }
                                
                                if (expIndex != -1)
                                {
                                    if (nodes[expIndex]->output.valuesCount == 1)
                                    {
                                        for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = FPower(nodes[i]->output.values[j], nodes[expIndex]->output.values[0]);
                                    }
                                    else FLog(false, "values count of node %i should be 1 because it is an exponent (currently count: %i)", nodes[expIndex]->id, nodes[expIndex]->output.valuesCount);
                                }
                                else FLog(false, "error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, expIndex);
                            } break;
                            case FNODE_POSTERIZE:
                            {
                                int expIndex = -1;
                                for (int j = 0; j < nodesCount; j++)
                                {
                                    if (nodes[j]->id == nodes[i]->inputs[1])
                                    {
                                        expIndex = j;
                                        break;
                                    }
                                }
                                
                                if (expIndex != -1)
                                {
                                    if (nodes[expIndex]->output.valuesCount == 1)
                                    {
                                        for (int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] = FPosterize(nodes[i]->output.values[j], nodes[expIndex]->output.values[0]);
                                    }
                                    else FLog(false, "values count of node %i should be 1 because it is an exponent (currently count: %i)", nodes[expIndex]->id, nodes[expIndex]->output.valuesCount);
                                }
                                else FLog(false, "error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, expIndex);
                            } break;
                            case FNODE_MAX:
                            case FNODE_MIN:
                            {
                                for (int j = 1; j < nodes[i]->inputsCount; j++)
                                {
                                    int inputIndex = -1;
                                    for (int k = 0; k < nodesCount; k++)
                                    {
                                        if (nodes[i]->inputs[j] == nodes[k]->id)
                                        {
                                            inputIndex = k;
                                            break;
                                        }
                                    }
                                    
                                    if (inputIndex != -1)
                                    {
                                        for (int k = 0; k < nodes[i]->output.valuesCount; k++)
                                        {
                                            if ((nodes[inputIndex]->output.values[k] > nodes[i]->output.values[k]) && (nodes[i]->type == FNODE_MAX)) nodes[i]->output.values[k] = nodes[inputIndex]->output.values[k];
                                            else if ((nodes[inputIndex]->output.values[k] < nodes[i]->output.values[k]) && (nodes[i]->type == FNODE_MIN)) nodes[i]->output.values[k] = nodes[inputIndex]->output.values[k];
                                        }
                                    }
                                    else FLog(false, "error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, inputIndex);
                                }
                            }
                            case FNODE_NOISE:
                            case FNODE_NOISEINT:
                            {
                                nodes[i]->output.valuesCount = 1;
                                nodes[i]->output.values[0] = (float)(GetRandomValue(nodes[i]->output.values[0], nodes[i]->output.values[1]));
                                if (nodes[i]->type == FNODE_NOISE) nodes[i]->output.values[0] += (float)(GetRandomValue(0, 99))/100 + (float)(GetRandomValue(0, 99))/1000;
                                for (int j = 1; j < MAX_VALUES; j++) nodes[i]->output.values[j] = 0.0f;
                            }
                            default: break;
                        }
                    }
                    else FLog(false, "error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, index);
                }
                else
                {
                    for (int k = 0; k < MAX_VALUES; k++) nodes[i]->output.values[k] = 0.0f;
                    nodes[i]->output.valuesCount = 0;
                }
            }
        }
        else FLog(true, "error trying to calculate values for a null referenced node");
    }
    
    UpdateRectangles();
}

// Updates node shapes due to drag behaviour
void UpdateRectangles()
{
    for (int i = 0; i < nodesCount; i++)
    {
        if (nodes[i]->type < FNODE_ADD) nodes[i]->outputShape = (Rectangle){ nodes[i]->shape.x + nodes[i]->shape.width, nodes[i]->shape.y + nodes[i]->shape.height/2 - 10, 20, 20 };
        else
        {
            nodes[i]->shape = (Rectangle){ nodes[i]->shape.x, nodes[i]->shape.y , 20 + 50*nodes[i]->output.valuesCount, 40 };
            nodes[i]->inputShape = (Rectangle){ nodes[i]->shape.x - 20, nodes[i]->shape.y + nodes[i]->shape.height/2 - 10, 20, 20 };
            nodes[i]->outputShape = (Rectangle){ nodes[i]->shape.x + nodes[i]->shape.width, nodes[i]->shape.y + nodes[i]->shape.height/2 - 10, 20, 20 };
        }
    }
}

// Draw canvas space to create nodes
void DrawCanvas()
{
    // Draw background title and credits
    DrawText("FNODE 1.0", (canvasSize.x - MeasureText("FNODE 1.0", 120))/2, canvasSize.y/2 - 60, 120, Fade(LIGHTGRAY, UI_GRID_ALPHA*2));
    DrawText("VICTOR FISAC", (canvasSize.x - MeasureText("VICTOR FISAC", 40))/2, canvasSize.y*0.65f - 20, 40, Fade(LIGHTGRAY, UI_GRID_ALPHA*2));
    
    Begin2dMode(camera);
    
        DrawCanvasGrid(UI_GRID_COUNT);
        
        // Draw all created nodes and lines
        for (int i = 0; i < nodesCount; i++) DrawNode(nodes[i]);
        for (int i = 0; i < linesCount; i++) DrawNodeLine(lines[i]);
        
    End2dMode();
}

// Aligns a node to the nearest grid intersection
void AlignNode(FNode node)
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

// Aligns all created nodes
void AlignAllNodes()
{
    // Align all created nodes
    for (int i = 0; i < nodesCount; i++) AlignNode(nodes[i]);
    
    UpdateRectangles();
}

// Destroys all unused nodes
void ClearUnusedNodes()
{
    for (int i = nodesCount - 1; i >= 0; i--)
    {
        bool used = false;
        
        for(int k = 0; k < linesCount; k++)
        {
            if(nodes[i]->id == lines[k]->from || nodes[i]->id == lines[k]->to)
            {
                used = true;
                break;
            }
        }
        
        if(!used) DestroyNode(nodes[i]);
    }
    
    FLog(false, "all unused nodes have been deleted [USED RAM: %i bytes]", usedMemory);
}

// Destroys all created nodes and its linked lines
void ClearGraph()
{
    for (int i = nodesCount - 1; i >= 0; i--) DestroyNode(nodes[i]);
    
    if(usedMemory != 0) FLog(true, "all nodes have been deleted by there are still memory in use [USED RAM: %i bytes]", usedMemory);
    else FLog(false, "all nodes have been deleted [USED RAM: %i bytes]", usedMemory);
}

// Draw canvas grid with a specific number of divisions for horizontal and vertical lines
void DrawCanvasGrid(int divisions)
{
    int spacing = 0;
    for (int i = 0; i < divisions; i++)
    {
        DrawLine(-(divisions/2*UI_GRID_SPACING*5) + spacing, -100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, Fade(BLACK, UI_GRID_ALPHA*2)); 
        spacing += UI_GRID_SPACING;
        DrawLine(-(divisions/2*UI_GRID_SPACING*5) + spacing, -100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, Fade(GRAY, UI_GRID_ALPHA));
        spacing += UI_GRID_SPACING;
        DrawLine(-(divisions/2*UI_GRID_SPACING*5) + spacing, -100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, Fade(GRAY, UI_GRID_ALPHA));
        spacing += UI_GRID_SPACING;
        DrawLine(-(divisions/2*UI_GRID_SPACING*5) + spacing, -100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, Fade(GRAY, UI_GRID_ALPHA));
        spacing += UI_GRID_SPACING;
        DrawLine(-(divisions/2*UI_GRID_SPACING*5) + spacing, -100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, Fade(GRAY, UI_GRID_ALPHA));
        spacing += UI_GRID_SPACING;
    }
    
    spacing = 0;
    for (int i = 0; i < divisions; i++)
    {
        DrawLine(-100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, Fade(BLACK, UI_GRID_ALPHA*2));
        spacing += UI_GRID_SPACING;
        DrawLine(-100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, Fade(GRAY, UI_GRID_ALPHA));
        spacing += UI_GRID_SPACING;
        DrawLine(-100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, Fade(GRAY, UI_GRID_ALPHA));
        spacing += UI_GRID_SPACING;
        DrawLine(-100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, Fade(GRAY, UI_GRID_ALPHA));
        spacing += UI_GRID_SPACING;
        DrawLine(-100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, 100000, -(divisions/2*UI_GRID_SPACING*5) + spacing, Fade(GRAY, UI_GRID_ALPHA));
        spacing += UI_GRID_SPACING;
    }
}

// Draw interface to create nodes
void DrawInterface()
{
    // Draw interface background
    DrawRectangleRec((Rectangle){ canvasSize.x, 0.0f, screenSize.x - canvasSize.x, screenSize.y }, DARKGRAY);
    
    // Draw interface main buttons
    if (GuiButton((Rectangle){ UI_PADDING, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Clear Graph")) ClearGraph();
    if (GuiButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*1, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Align Nodes")) AlignAllNodes();
    if (GuiButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*2, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Clear Unused Nodes")) ClearUnusedNodes();
    
    // Draw interface nodes buttons
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Value")) CreateNodeValue((float)GetRandomValue(-10, 10) + (float)GetRandomValue(0, 100)/100);
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*1 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Vector 2")) CreateNodeVector2((Vector2){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*2 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Vector 3")) CreateNodeVector3((Vector3){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*3 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Vector 4")) CreateNodeVector4((Vector4){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*4 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Add")) CreateNodeAdd();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*5 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Subtract")) CreateNodeSubtract();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*6 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Multiply")) CreateNodeMultiply();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*7 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Divide")) CreateNodeDivide();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*8 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Append")) CreateNodeAppend();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*9 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "One Minus")) CreateNodeOneMinus();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*10 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Exp 2")) CreateNodeExp2();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*11 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Power")) CreateNodePower();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*12 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Posterize")) CreateNodePosterize();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*13 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Abs")) CreateNodeAbs();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*14 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Clamp 0-1")) CreateNodeClamp01();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*15 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Max")) CreateNodeMax();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*16 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Min")) CreateNodeMin();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*17 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Negate")) CreateNodeNegate();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*18 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Noise")) CreateNodeNoise();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*19 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Noise Int")) CreateNodeNoiseInt();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*20 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Reciprocal")) CreateNodeReciprocal();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*21 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Ceil")) CreateNodeCeil();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*22 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Cosine")) CreateNodeCos();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*23 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Sine")) CreateNodeSin();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*24 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Tangent")) CreateNodeTan();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*25 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Deg to Rad")) CreateNodeDeg2Rad();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*26 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Rad to Deg")) CreateNodeRad2Deg();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*27 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Normalize")) CreateNodeNormalize();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*28 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Round")) CreateNodeRound();
    if (GuiButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*29 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Trunc")) CreateNodeTrunc();
}

// Converts Vector2 coordinates from world space to Camera2D space based on its offset
Vector2 CameraToViewVector2(Vector2 vector, Camera2D camera)
{
    return (Vector2){ vector.x - camera.offset.x, vector.y - camera.offset.y };
}

// Converts rectangle coordinates from world space to Camera2D space based on its offset
Rectangle CameraToViewRec(Rectangle rec, Camera2D camera)
{
    return (Rectangle){ rec.x + camera.offset.x, rec.y + camera.offset.y, rec.width, rec.height };
}

// Initializes FNode global variables
void InitFNode()
{
    nodesCount = 0;
    linesCount = 0;
    
    FLog(false, "initialization complete");
}

// Creates a value node (1 float)
FNode CreateNodeValue(float value)
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_VALUE;
    newNode->name = "Value";
    newNode->output.valuesCount = 1;
    newNode->output.values[0] = value;
    newNode->shape.width += 50*newNode->output.valuesCount;
    newNode->inputsLimit = 0;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a Vector2 node (2 float)
FNode CreateNodeVector2(Vector2 vector)
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_VECTOR2;
    newNode->name = "Vector 2";
    newNode->output.valuesCount = 2;
    newNode->output.values[0] = vector.x;
    newNode->output.values[1] = vector.y;
    newNode->shape.width += 50*newNode->output.valuesCount;
    newNode->inputsLimit = 0;
    
    UpdateRectangles();
}

// Creates a Vector3 node (3 float)
FNode CreateNodeVector3(Vector3 vector)
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_VECTOR3;
    newNode->name = "Vector 3";
    newNode->output.valuesCount = 3;
    newNode->output.values[0] = vector.x;
    newNode->output.values[1] = vector.y;
    newNode->output.values[2] = vector.z;
    newNode->shape.width += 50*newNode->output.valuesCount;
    newNode->inputsLimit = 0;
    
    UpdateRectangles();
}

// Creates a Vector4 node (4 float)
FNode CreateNodeVector4(Vector4 vector)
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_VECTOR4;
    newNode->name = "Vector 4";
    newNode->output.valuesCount = 4;
    newNode->output.values[0] = vector.x;
    newNode->output.values[1] = vector.y;
    newNode->output.values[2] = vector.z;
    newNode->output.values[3] = vector.w;
    newNode->shape.width += 50*newNode->output.valuesCount;
    newNode->inputsLimit = 0;
    
    UpdateRectangles();
}

// Creates a node to add multiples values
FNode CreateNodeAdd()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_ADD;
    newNode->name = "Add";

    UpdateRectangles();
    
    return newNode;
}

// Creates a node to subtract multiples values by the first linked
FNode CreateNodeSubtract()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_SUBTRACT;
    newNode->name = "Subtract";
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to multiply different values
FNode CreateNodeMultiply()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_MULTIPLY;
    newNode->name = "Multiply";
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to divide multiples values by the first linked
FNode CreateNodeDivide()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_DIVIDE;
    newNode->name = "Divide";
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to combine little data types into bigger data types (2 int = 1 Vector2)
FNode CreateNodeAppend()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_APPEND;
    newNode->name = "Append";
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to subtract a value to 1
FNode CreateNodeOneMinus()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_ONEMINUS;
    newNode->name = "One Minus";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to get absolute value of the input
FNode CreateNodeAbs()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_ABS;
    newNode->name = "Abs";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to get sin value of the input
FNode CreateNodeCos()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_COS;
    newNode->name = "Cosine";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to get sin value of the input
FNode CreateNodeSin()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_SIN;
    newNode->name = "Sine";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to get sin value of the input
FNode CreateNodeTan()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_TAN;
    newNode->name = "Tangent";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to convert a degrees value to radians
FNode CreateNodeDeg2Rad()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_DEG2RAD;
    newNode->name = "Deg to Rad";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to convert a radians value to degrees                  
FNode CreateNodeRad2Deg()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_RAD2DEG;
    newNode->name = "Rad to Deg";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to normalize a vector
FNode CreateNodeNormalize()
{
   FNode newNode = InitializeNode();
    
    newNode->type = FNODE_NORMALIZE;
    newNode->name = "Normalize";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}                    

// Creates a node to get negative value of the input
FNode CreateNodeNegate()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_NEGATE;
    newNode->name = "Negate";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to get 1/input value
FNode CreateNodeReciprocal()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_RECIPROCAL;
    newNode->name = "Reciprocal";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to get the truncated value of the input
FNode CreateNodeTrunc()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_TRUNC;
    newNode->name = "Truncate";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to get the rounded value of the input
FNode CreateNodeRound()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_ROUND;
    newNode->name = "Round";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to get the rounded up to the nearest integer of the input
FNode CreateNodeCeil()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_CEIL;
    newNode->name = "Ceil";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to clamp the input value between 0-1 range
FNode CreateNodeClamp01()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_CLAMP01;
    newNode->name = "Clamp 0-1";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node that returns 2 to the power of its input
FNode CreateNodeExp2()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_EXP2;
    newNode->name = "Exp 2";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node that returns the first input to the power of the second input
FNode CreateNodePower()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_POWER;
    newNode->name = "Power";
    newNode->inputsLimit = 2;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node that posterizes the first input in a number of steps set by second input
FNode CreateNodePosterize()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_POSTERIZE;
    newNode->name = "Posterize";
    newNode->inputsLimit = 2;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to calculate the higher value of all inputs values
FNode CreateNodeMax()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_MAX;
    newNode->name = "Max";
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to calculate the lower value of all inputs values
FNode CreateNodeMin()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_MIN;
    newNode->name = "Min";
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to calculate random values between input range
FNode CreateNodeNoise()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_NOISE;
    newNode->name = "Noise";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a node to calculate integer random values between input range
FNode CreateNodeNoiseInt()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_NOISEINT;
    newNode->name = "Noise Int";
    newNode->inputsLimit = 1;
    
    UpdateRectangles();
    
    return newNode;
}

// Initializes a new node with generic parameters
FNode InitializeNode()
{
    FNode newNode = (FNode)malloc(sizeof(FNodeData));
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
    else FLog(true, "node creation failed because there is any available id");
    
    // Initialize node inputs and inputs count
    for (int i = 0; i < MAX_INPUTS; i++) newNode->inputs[i] = -1;
    newNode->inputsCount = 0;
    newNode->inputsLimit = MAX_INPUTS;
    
    // Initialize node output and output values count
    for (int i = 0; i < MAX_VALUES; i++) newNode->output.values[i] = 0.0f;
    newNode->output.valuesCount = 0;
    
    // Initialize shapes
    newNode->shape = (Rectangle){ GetRandomValue(-camera.offset.x + 0, -camera.offset.x + screenSize.x*0.85f - 50*4), GetRandomValue(-camera.offset.y + screenSize.y/2 - 20 - 100, camera.offset.y + screenSize.y/2 - 20 + 100), 20, 40 };
    newNode->inputShape = (Rectangle){ 0, 0, 0, 0 };
    newNode->outputShape = (Rectangle){ 0, 0, 0, 0 };
    
    AlignNode(newNode);
    
    nodes[nodesCount] = newNode;
    nodesCount++;
    
    FLog(false, "created new node id %i (index: %i) [USED RAM: %i bytes]", newNode->id, (nodesCount - 1), usedMemory);
    
    return newNode;
}

// Creates a line between two nodes
FLine CreateNodeLine(int from)
{
    FLine newLine = (FLine)malloc(sizeof(FNodeLine));
    usedMemory += sizeof(FNodeLine);
    
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
    else FLog(true, "line creation failed because there is any available id");
    
    // Initialize line input
    newLine->from = from;
    newLine->to = -1;
    
    lines[linesCount] = newLine;
    linesCount++;
    
    FLog(false, "created new line id %i (index: %i) [USED RAM: %i bytes]", newLine->id, (linesCount - 1), usedMemory);
    
    return newLine;
}

// Draws a previously created node
void DrawNode(FNode node)
{
    if (node != NULL)
    {
        DrawRectangleRec(node->shape, ((node->id == selectedId) ? GRAY : LIGHTGRAY));
        DrawRectangleLines(node->shape.x, node->shape.y, node->shape.width, node->shape.height, BLACK);
        
        // Draw name label
        DrawText(FormatText("%s [ID: %i]", node->name, node->id), node->shape.x + node->shape.width/2 - MeasureText(FormatText("%s [ID: %i]", node->name, node->id), 10)/2, node->shape.y - 15, 10, BLACK);
        
        switch (node->output.valuesCount)
        {
            case 1: DrawText(FormatText("%.02f", node->output.values[0]), node->shape.x + node->shape.width/2 - MeasureText(FormatText("%.02f", node->output.values[0]), 20)/2, node->shape.y + node->shape.height/2 - 10, 20, BLACK); break;
            case 2: DrawText(FormatText("%.02f,%.02f", node->output.values[0], node->output.values[1]), node->shape.x + node->shape.width/2 - MeasureText(FormatText("%.02f,%.02f", node->output.values[0], node->output.values[1]), 20)/2, node->shape.y + node->shape.height/2 - 10, 20, BLACK); break;
            case 3: DrawText(FormatText("%.02f,%.02f,%.02f", node->output.values[0], node->output.values[1], node->output.values[2]), node->shape.x + node->shape.width/2 - MeasureText(FormatText("%.02f,%.02f,%.02f", node->output.values[0], node->output.values[1], node->output.values[2]), 20)/2, node->shape.y + node->shape.height/2 - 10, 20, BLACK); break;
            case 4: DrawText(FormatText("%.02f,%.02f,%.02f,%.02f", node->output.values[0], node->output.values[1], node->output.values[2], node->output.values[3]), node->shape.x + node->shape.width/2 - MeasureText(FormatText("%.02f,%.02f,%.02f,%.02f", node->output.values[0], node->output.values[1], node->output.values[2], node->output.values[3]), 20)/2, node->shape.y + node->shape.height/2 - 10, 20, BLACK); break;
            default: break;
        }
        
        if (node->inputsCount > 0) DrawRectangleRec(node->inputShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(node->inputShape, camera)) ? LIGHTGRAY : GRAY)));
        else DrawRectangleRec(node->inputShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(node->inputShape, camera)) ? LIGHTGRAY : RED)));
        DrawRectangleLines(node->inputShape.x, node->inputShape.y, node->inputShape.width, node->inputShape.height, BLACK);
        
        DrawRectangleRec(node->outputShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(node->outputShape, camera)) ? LIGHTGRAY : GRAY)));
        DrawRectangleLines(node->outputShape.x, node->outputShape.y, node->outputShape.width, node->outputShape.height, BLACK);
        
        if (debugMode)
        {
            const char *string =
            "id: %i\n"
            "type: %i\n"
            "name: %s\n"
            "inputs: %i, %i, %i, %i, %i, %i, %i, %i (count: %i)\n"
            "output: %.02f, %.02f, %.02f, %.02f (count: %i)\n"
            "shape: %i %i, %i, %i";
            
            DrawText(FormatText(string, node->id, node->type, node->name, node->inputs[0], node->inputs[1], node->inputs[2], node->inputs[3], node->inputs[4], 
            node->inputs[5], node->inputs[6], node->inputs[7], node->inputsCount, node->output.values[0], node->output.values[1], node->output.values[2], node->output.values[3], node->output.valuesCount, node->shape.x, node->shape.y, node->shape.width, node->shape.height), 10, 10 + 100*node->id, 10, BLACK);
        }
    }
    else FLog(true, "error trying to draw a null referenced node");
}

// Draws a previously created node line
void DrawNodeLine(FLine line)
{
    if (line != NULL)
    {
        Vector2 from = { 0, 0 };
        Vector2 to = { 0, 0 };
        
        int indexFrom = -1;
        for (int i = 0; i < nodesCount; i++)
        {
            if (nodes[i]->id == line->from)
            {
                indexFrom = i;
                break;
            }
        }
        
        if (indexFrom != -1)
        {
            from.x = nodes[indexFrom]->outputShape.x + nodes[indexFrom]->outputShape.width/2;
            from.y = nodes[indexFrom]->outputShape.y + nodes[indexFrom]->outputShape.height/2;
            
            if (line->to != -1)
            {
                int indexTo = -1;
                for (int i = 0; i < nodesCount; i++)
                {
                    if (nodes[i]->id == line->to)
                    {
                        indexTo = i;
                        break;
                    }
                }
                
                if (indexTo != -1)
                {
                    to.x = nodes[indexTo]->inputShape.x + nodes[indexTo]->inputShape.width/2;
                    to.y = nodes[indexTo]->inputShape.y + nodes[indexTo]->inputShape.height/2;
                }
            }
            else to = CameraToViewVector2(mousePosition, camera);
        }
        else FLog(true, "error when trying to find node id %i due to index is out of bounds %i", line->from, indexFrom);
        
        DrawLine(from.x, from.y, to.x, to.y, ((tempLine->id == line->id && tempLine->to == -1) ? GRAY : BLACK));
        
        if (debugMode)
        {
            const char *string =
            "id: %i\n"
            "from: %i\n"
            "to: %i\n";
            
            DrawText(FormatText(string, line->id, line->from, line->to), screenSize.x*0.85f - 10 - 50, 10 + 75*line->id, 10, BLACK);
        }
    }
    else FLog(true, "error trying to draw a null referenced line");
}

// Destroys a node and its linked lines
void DestroyNode(FNode node)
{
    if (node != NULL)
    {
        int id = node->id;
        int index = -1;
        
        for (int i = 0; i < nodesCount; i++)
        {
            if (nodes[i]->id == id)
            {
                index = i;
                break;
            }
        }
        
        if (index != -1)
        {
            for (int i = linesCount - 1; i >= 0; i--)
            {
                if (lines[i]->from == node->id || lines[i]->to == node->id) DestroyNodeLine(lines[i]);
            }
                    
            free(nodes[index]);
            
            for (int i = index; i < nodesCount; i++)
            {
                if ((i + 1) < nodesCount) nodes[i] = nodes[i + 1];
            }

            nodesCount--;
            
            usedMemory -= sizeof(FNodeData);
            FLog(false, "destroyed node id %i (index: %i) [USED RAM: %i bytes]", id, index, usedMemory);
            
            CalculateValues();
        }
        else FLog(true, "error when trying to destroy node id %i (index: %i)", id, index);
    }
    else FLog(true, "error trying to destroy a null referenced node");
}

// Destroys a node line
void DestroyNodeLine(FLine line)
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
            free(lines[index]);
            
            for (int i = index; i < linesCount; i++)
            {
                if ((i + 1) < linesCount) lines[i] = lines[i + 1];
            }

            linesCount--;
            
            usedMemory -= sizeof(FNodeLine);
            FLog(false, "destroyed line id %i (index: %i) [USED RAM: %i bytes]", id, index, usedMemory);
            
            CalculateValues();
        }
        else FLog(true, "error when trying to destroy line id %i due to index is out of bounds %i", id, index);
    }
    else FLog(true, "error trying to destroy a null referenced line");
}

// Unitializes FNode global variables
void CloseFNode()
{
    for (int i = 0; i < nodesCount; i++)
    {
        free(nodes[i]);
        usedMemory -= sizeof(FNodeData);
    }
    
    for (int i = 0; i < linesCount; i++)
    {
        free(lines[i]);
        usedMemory -= sizeof(FNodeLine);
    }
    
    nodesCount = 0;
    linesCount = 0;
    
    FLog(false, "unitialization complete [USED RAM: %i bytes]", usedMemory);
}

// Returns length of a Vector2
float FVector2Length(Vector2 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y);
}

// Returns length of a Vector3
float FVector3Length(Vector3 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

// Returns length of a Vector4
float FVector4Length(Vector4 v)
{
    return (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

// Returns a normalized Vector2
Vector2 FVector2Normalize(Vector2 v)
{
    float length = FVector2Length(v);
    return (Vector2){ v.x/length, v.y/length };
}

// Returns a normalized Vector3
Vector3 FVector3Normalize(Vector3 v)
{
    float length = FVector3Length(v);
    return (Vector3){ v.x/length, v.y/length, v.z/length };
}

// Returns a normalized Vector4
Vector4 FVector4Normalize(Vector4 v)
{
    float length = FVector4Length(v);
    return (Vector4){ v.x/length, v.y/length, v.z/length, v.w/length };
}

// Returns a value to the power of an exponent
float FPower(float value, float exp)
{
    return (float)pow(value, exp);
}

// Returns a value rounded based on the samples
float FPosterize(float value, float samples)
{
    float output = value*samples;
    
    output = (float)floor(output);
    output /= samples;
    
    return output;
}

// Returns a value clamped by a min and max values
float FClamp(float value, float min, float max)
{
    float output = value;
    
    if (output < min) output = min;
    else if (output > max) output = max;
    
    return output;
}

// Returns a truncated value of a value
float FTrunc(float value)
{
    float output = value;
    
    int truncated = (int)output;
    output = (float)truncated;
    
    return output;
}

// Returns a rounded value of a value
float FRound(float value)
{
    float output = fabs(value);
    
    int truncated = (int)output;
    float decimals = output - (float)truncated;
    
    output = ((decimals > 0.5f) ? (output - decimals + 1.0f) : (output - decimals));
    if (value < 0.0f) output *= -1;
    
    return output;
}

// Returns a rounded up to the nearest integer of a value
float FCeil(float value)
{
    float output = value;

    int truncated = (int)output;
    if (output != (float)truncated) output = (float)((output >= 0) ? (truncated + 1) : truncated);

    return output;
}

// Outputs a trace log message
void FLog(bool error, const char *text, ...)
{
    va_list args;
    
    fprintf(stdout, "FNode: ");
    va_start(args, text);
    vfprintf(stdout, text, args);
    va_end(args);

    fprintf(stdout, "\n");

    if (error) exit(1);
}