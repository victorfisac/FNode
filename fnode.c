/*
    ----------------- TODO LIST -----------------
    - Fix mouse inputs conflicts between different input features
    - Add remaining operator nodes
    - Create main node
    
*/

#include <stdlib.h>         // Required for: malloc(), free(), atof()
#include <stdio.h>          // Required for: fprintf(), sprintf()
#include <math.h>           // Required for: cos(), sin(), sqrt(), pow(), floor()
#include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#include <raylib.h>         // Required for window management, 2D camera drawing and inputs detection

// Defines
#define     MAX_INPUTS                  8               // Max number of inputs in every node
#define     MAX_VALUES                  4               // Max number of values in every output
#define     MAX_NODES                   256             // Max number of nodes
#define     MAX_NODE_LENGTH             16              // Max node output data value text length
#define     MAX_LINES                   2048            // Max number of lines (8 lines for each node)
#define     MAX_COMMENTS                64              // Max number of comments
#define     MAX_COMMENT_LENGTH          20              // Max comment value text length
#define     MIN_COMMENT_SIZE            75              // Min comment width and height values
#define     NODE_LINE_DIVISIONS         20              // Node curved line divisions
#define     NODE_DATA_WIDTH             30              // Node data text width
#define     NODE_DATA_HEIGHT            30              // Node data text height
#define     UI_PADDING                  5               // Interface bounds padding with background
#define     UI_BUTTON_HEIGHT            30              // Interface bounds height
#define     UI_SCROLL                   20              // Interface scroll sensitivity
#define     UI_GRID_SPACING             25              // Interface canvas background grid divisions length
#define     UI_GRID_ALPHA               0.25f           // Interface canvas background grid lines alpha
#define     UI_GRID_COUNT               100             // Interface canvas background grid divisions count
#define     UI_COMMENT_WIDTH            220             // Interface comment text box width
#define     UI_COMMENT_HEIGHT           25              // Interface comment text box height
#define     UI_BUTTON_DEFAULT_COLOR     LIGHTGRAY       // Interface button background color
#define     UI_BORDER_DEFAULT_COLOR     125             // Interface button border color

// Enums
typedef enum {
    FNODE_PI = -2,
    FNODE_E,
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

typedef enum {
    BUTTON_DEFAULT,
    BUTTON_HOVER,
    BUTTON_PRESSED,
    BUTTON_CLICKED
} ButtonState;

// Structs
typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} Vector4;

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

// Global variables
int usedMemory = 0;                         // Total USED RAM from memory allocation

int nodesCount;                             // Created nodes count
FNode nodes[MAX_NODES];                     // Nodes pointers pool
int selectedNode = -1;                      // Current selected node to drag in canvas
int editNode = -1;                          // Current edited node data
int editNodeType = -1;                      // Current edited node data index
char *editNodeText = NULL;                  // Current edited node data value text before any changes

int linesCount;                             // Created lines count
FLine lines[MAX_LINES];                     // Lines pointers pool
int lineState = 0;                          // Current line linking state (0 = waiting for output to link, 1 = output selected, waiting for input to link)
FLine tempLine = NULL;                      // Temporally created line during line states

int commentsCount;                          // Created comments count
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

Vector2 screenSize = { 1280, 720 };         // Window screen width      
Vector2 mousePosition = { 0, 0 };           // Current mouse position 
Vector2 lastMousePosition = { 0, 0 };       // Previous frame mouse position
Vector2 mouseDelta = { 0, 0 };              // Current frame mouse position increment since previous frame
Vector2 currentOffset = { 0, 0 };           // Current selected node offset between mouse position and node shape

bool debugMode = false;                     // Drawing debug information state
Camera2D camera;                            // Node area 2d camera for panning
Vector2 canvasSize;                         // Interface screen size
float menuScroll = 10.0f;                   // Current interface scrolling amount
Vector2 scrollLimits = { 10, 580 };         // Interface scrolling limits

// Functions declarations
void UpdateMouseData();                     // Updates current mouse position and delta position
void UpdateCanvas();                        // Updates canvas space target and offset
void UpdateScroll();                        // Updates mouse scrolling for menu and canvas drag
void UpdateNodeLink();                      // Check node link input
void UpdateCommentCreationEdit();           // Check comment creation and shape edit input
void UpdateNodeEdit();                      // Check node data values edit input
void UpdateNodeDrag();                      // Check node drag input
void UpdateCommentDrag();                   // Check comment drag input
void UpdateCommentEdit();                   // Check comment text edit input
void CalculateValues();                     // Calculates nodes output values based on current inputs
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
void InitFNode();                                           // Initializes FNode global variables

FNode CreateNodeValue(float value);                         // Creates a value node (1 float)
FNode CreateNodeVector2(Vector2 vector);                    // Creates a Vector2 node (2 float)
FNode CreateNodeVector3(Vector3 vector);                    // Creates a Vector3 node (3 float)
FNode CreateNodeVector4(Vector4 vector);                    // Creates a Vector4 node (4 float)
FNode CreateNodeAdd();                                      // Creates a node to add multiples values
FNode CreateNodeSubtract();                                 // Creates a node to subtract multiples values by the first linked
FNode CreateNodeMultiply();                                 // Creates a node to multiply different values
FNode CreateNodeDivide();                                   // Creates a node to divide multiples values by the first linked
FNode CreateNodeAppend();                                   // Creates a node to combine little data types into bigger data types (2 int = 1 Vector2)
FNode CreateNodeOneMinus();                                 // Creates a node to subtract a value to 1
FNode CreateNodeAbs();                                      // Creates a node to get absolute value of the input
FNode CreateNodeCos();                                      // Creates a node to get cos value of the input
FNode CreateNodeSin();                                      // Creates a node to get sin value of the input
FNode CreateNodeTan();                                      // Creates a node to get tan value of the input
FNode CreateNodeDeg2Rad();                                  // Creates a node to convert a degrees value to radians
FNode CreateNodeRad2Deg();                                  // Creates a node to convert a radians value to degrees
FNode CreateNodeNormalize();                                // Creates a node to normalize a vector
FNode CreateNodeNegate();                                   // Creates a node to get negative value of the input
FNode CreateNodeReciprocal();                               // Creates a node to get 1/input value
FNode CreateNodeTrunc();                                    // Creates a node to get the truncated value of the input
FNode CreateNodeRound();                                    // Creates a node to get the rounded value of the input
FNode CreateNodeCeil();                                     // Creates a node to get the rounded up to the nearest integer of the input
FNode CreateNodeClamp01();                                  // Creates a node to clamp the input value between 0-1 range
FNode CreateNodeExp2();                                     // Creates a node that returns 2 to the power of its input
FNode CreateNodePower();                                    // Creates a node that returns the first input to the power of the second input
FNode CreateNodePosterize();                                // Creates a node that posterizes the first input in a number of steps set by second input
FNode CreateNodeMax();                                      // Creates a node to calculate the higher value of all inputs values
FNode CreateNodeMin();                                      // Creates a node to calculate the lower value of all inputs values
FNode CreateNodeNoise();                                    // Creates a node to calculate random values between input range
FNode CreateNodeNoiseInt();                                 // Creates a node to calculate integer random values between input range
FNode CreateNodePI();                                       // Creates a node which returns PI value
FNode CreateNodeE();                                        // Creates a node which returns e value
FNode InitializeNode(bool isOperator);                      // Initializes a new node with generic parameters
FLine CreateNodeLine();                                     // Creates a line between two nodes
FComment CreateComment();                                   // Creates a comment

void UpdateNodeShapes(FNode node);                          // Updates a node shapes due to drag behaviour
void UpdateCommentShapes(FComment comment);                 // Updates a comment shapes due to drag behaviour

void DrawNode(FNode node);                                  // Draws a previously created node
void DrawNodeLine(FLine line);                              // Draws a previously created node line
void DrawComment(FComment comment);                         // Draws a previously created comment
bool FButton(Rectangle bounds, const char *text);           // Button element, returns true when pressed

void DestroyNode(FNode node);                               // Destroys a node and its linked lines
void DestroyNodeLine(FLine line);                           // Destroys a node line
void DestroyComment(FComment comment);                      // Destroys a comment

void CloseFNode();                                          // Unitializes FNode global variables
void TraceLogFNode(bool error, const char *text, ...);      // Outputs a trace log message

// Math functions declarations
float FVector2Length(Vector2 v);                            // Returns length of a Vector2
float FVector3Length(Vector3 v);                            // Returns length of a Vector3
float FVector4Length(Vector4 v);                            // Returns length of a Vector4
Vector2 FVector2Normalize(Vector2 v);                       // Returns a normalized Vector2
Vector3 FVector3Normalize(Vector3 v);                       // Returns a normalized Vector3
Vector4 FVector4Normalize(Vector4 v);                       // Returns a normalized Vector4
float FPower(float value, float exp);                       // Returns a value to the power of an exponent
float FPosterize(float value, float samples);               // Returns a value rounded based on the samples
float FClamp(float value, float min, float max);            // Returns a value clamped by a min and max values
float FTrunc(float value);                                  // Returns a truncated value of a value
float FRound(float value);                                  // Returns a rounded value of a value
float FCeil(float value);                                   // Returns a rounded up to the nearest integer of a value
float FEaseLinear(float t, float b, float c, float d);  // Returns an ease linear value between two parameters 
float FEaseInOutQuad(float t, float b, float c, float d);   // Returns an ease quadratic in-out value between two parameters
void FStringToFloat(float *pointer, const char *string);    // Sends a float conversion value of a string to an initialized float pointer
void FFloatToString(char *buffer, float value);             // Sends formatted output to an initialized string pointer

int main()
{       
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenSize.x, screenSize.y, "fnode 0.3");
    SetLineWidth(3);
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
        UpdateMouseData();
        UpdateCanvas();
        UpdateScroll();
        UpdateNodeLink();
        UpdateCommentCreationEdit();
        UpdateNodeEdit();
        UpdateNodeDrag();
        UpdateCommentEdit();
        UpdateCommentDrag();
        
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

// Updates current mouse position and delta position
void UpdateMouseData()
{
    // Update mouse position values
    lastMousePosition = mousePosition;
    mousePosition = GetMousePosition();
    mouseDelta = (Vector2){ mousePosition.x - lastMousePosition.x, mousePosition.y - lastMousePosition.y };
}

// Updates canvas space target and offset
void UpdateCanvas()
{
    // Update canvas camera values
    camera.target = mousePosition;
    camera.offset.x += mouseDelta.x*(camera.zoom - 1);
    camera.offset.y += mouseDelta.y*(camera.zoom - 1);
}

// Updates mouse scrolling for menu and canvas drag
void UpdateScroll()
{
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
}

// Check node link input
void UpdateNodeLink()
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
                                if (nodes[i]->type == FNODE_APPEND) valuesCheck = (nodes[i]->output.dataCount + nodes[indexFrom]->output.dataCount <= MAX_VALUES);
                                else if ((nodes[i]->type == FNODE_POWER) && (nodes[i]->inputsCount == 1)) valuesCheck = (nodes[indexFrom]->output.dataCount == 1);
                                else if (nodes[i]->type == FNODE_NOISE) valuesCheck = (nodes[indexFrom]->output.dataCount == 2);
                                else if (nodes[i]->type == FNODE_NORMALIZE) valuesCheck = (nodes[indexFrom]->output.dataCount > 1);
                                else valuesCheck = (nodes[i]->output.dataCount == nodes[indexFrom]->output.dataCount);
                                
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
                                    CalculateValues();
                                    break;
                                }
                                else TraceLogFNode(false, "error trying to link node ID %i (length: %i) with node ID %i (length: %i)", nodes[i]->id, nodes[i]->output.dataCount, nodes[indexFrom]->id, nodes[indexFrom]->output.dataCount);
                            }
                            else TraceLogFNode(true, "(1) error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, indexFrom);
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

// Check node drag input
void UpdateNodeDrag()
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
            
            if ((selectedNode == -1) && (CheckCollisionPointRec(mousePosition, (Rectangle){ 0, 0, screenSize.x, screenSize.y })))
            {
                camera.offset.x += mouseDelta.x;
                camera.offset.y += mouseDelta.y;
            }
        }
        else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            for (int i = nodesCount - 1; i >= 0; i--)
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

// Check comment drag input
void UpdateCommentDrag()
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

// Check node data values edit input
void UpdateNodeEdit()
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        int index = -1;
        int data = -1;
        for (int i = 0; i < nodesCount; i++)
        {
            if ((nodes[i]->type >= FNODE_VALUE) && (nodes[i]->type <= FNODE_VECTOR4))
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
                editNodeText = (char *)malloc(MAX_NODE_LENGTH);
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
            free(editNodeText);
            usedMemory -= MAX_NODE_LENGTH;
            editNodeText = NULL;
        }     
    }
}

// Check comment text edit input
void UpdateCommentEdit()
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
                    
                    if (index != -1)
                    {
                        nodes[i]->output.dataCount = nodes[index]->output.dataCount;
                        for (int k = 0; k < nodes[i]->output.dataCount; k++) nodes[i]->output.data[k].value = nodes[index]->output.data[k].value;
                    }
                    else TraceLogFNode(true, "(2) error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, index);
                    
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
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value += nodes[inputIndex]->output.data[j].value;
                                } break;
                                case FNODE_SUBTRACT:
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value -= nodes[inputIndex]->output.data[j].value;
                                } break;
                                case FNODE_MULTIPLY:
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value *= nodes[inputIndex]->output.data[j].value;
                                } break;
                                case FNODE_DIVIDE:
                                {
                                    for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value /= nodes[inputIndex]->output.data[j].value;
                                } break;
                                default: break;
                            }
                        }
                        else TraceLogFNode(false, "error trying to get node id %i due to index is out of bounds %i", nodes[i]->inputs[k], inputIndex);
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
                            for (int j = 0; j < nodes[inputIndex]->output.dataCount; j++)
                            {
                                nodes[i]->output.data[valuesCount].value = nodes[inputIndex]->output.data[j].value;
                                valuesCount++;
                            }
                        }
                        else TraceLogFNode(true, "(3) error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, inputIndex);
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
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)cos(nodes[i]->output.data[j].value);
                            } break;
                            case FNODE_SIN:
                            {
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)sin(nodes[i]->output.data[j].value);
                            } break;
                            case FNODE_TAN:
                            {
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = (float)tan(nodes[i]->output.data[j].value);
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
                            case FNODE_TRUNC:
                            {
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FTrunc(nodes[i]->output.data[j].value);
                            } break;
                            case FNODE_ROUND:
                            {
                                for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FRound(nodes[i]->output.data[j].value);
                            } break;
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
                                    if (nodes[expIndex]->output.dataCount == 1)
                                    {
                                        for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FPower(nodes[i]->output.data[j].value, nodes[expIndex]->output.data[0].value);
                                    }
                                    else TraceLogFNode(false, "values count of node %i should be 1 because it is an exponent (currently count: %i)", nodes[expIndex]->id, nodes[expIndex]->output.dataCount);
                                }
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
                                    if (nodes[expIndex]->output.dataCount == 1)
                                    {
                                        for (int j = 0; j < nodes[i]->output.dataCount; j++) nodes[i]->output.data[j].value = FPosterize(nodes[i]->output.data[j].value, nodes[expIndex]->output.data[0].value);
                                    }
                                    else TraceLogFNode(false, "values count of node %i should be 1 because it is an exponent (currently count: %i)", nodes[expIndex]->id, nodes[expIndex]->output.dataCount);
                                }
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
                                        for (int k = 0; k < nodes[i]->output.dataCount; k++)
                                        {
                                            if ((nodes[inputIndex]->output.data[k].value > nodes[i]->output.data[k].value) && (nodes[i]->type == FNODE_MAX)) nodes[i]->output.data[k].value = nodes[inputIndex]->output.data[k].value;
                                            else if ((nodes[inputIndex]->output.data[k].value < nodes[i]->output.data[k].value) && (nodes[i]->type == FNODE_MIN)) nodes[i]->output.data[k].value = nodes[inputIndex]->output.data[k].value;
                                        }
                                    }
                                    else TraceLogFNode(true, "(6) error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, inputIndex);
                                }
                            }
                            case FNODE_NOISE:
                            case FNODE_NOISEINT:
                            {
                                nodes[i]->output.dataCount = 1;
                                nodes[i]->output.data[0].value = (float)(GetRandomValue(nodes[i]->output.data[0].value, nodes[i]->output.data[1].value));
                                if (nodes[i]->type == FNODE_NOISE) nodes[i]->output.data[0].value += (float)(GetRandomValue(0, 99))/100 + (float)(GetRandomValue(0, 99))/1000;
                                for (int j = 1; j < MAX_VALUES; j++) nodes[i]->output.data[j].value = 0.0f;
                            }
                            default: break;
                        }
                    }
                    else TraceLogFNode(true, "(0) error trying to get inputs from node id %i due to index is out of bounds %i", nodes[i]->id, index);
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

// Updates a node shapes due to drag behaviour
void UpdateNodeShapes(FNode node)
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
        
        if (node->type < FNODE_ADD)
        {
            node->outputShape.x = node->shape.x + node->shape.width;
            node->outputShape.y = node->shape.y + node->shape.height/2 - 10;
        }
        else
        {
            node->shape.height = ((node->output.dataCount == 0) ? (NODE_DATA_HEIGHT/2 + 10) : ((NODE_DATA_HEIGHT + 5)*node->output.dataCount + 5));
            
            node->inputShape.x = node->shape.x - 20;
            node->inputShape.y = node->shape.y + node->shape.height/2 - 10;
            
            node->outputShape.x = node->shape.x + node->shape.width;
            node->outputShape.y = node->shape.y + node->shape.height/2 - 10;
        }
    }
    else TraceLogFNode(true, "error trying to calculate values for a null referenced node");
}

// Updates a comment shapes due to drag behaviour
void UpdateCommentShapes(FComment comment)
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
        bool used = false;
        
        for (int k = 0; k < linesCount; k++)
        {
            if (nodes[i]->id == lines[k]->from || nodes[i]->id == lines[k]->to)
            {
                used = true;
                break;
            }
        }
        
        if (!used) DestroyNode(nodes[i]);
    }
    
    TraceLogFNode(false, "all unused nodes have been deleted [USED RAM: %i bytes]", usedMemory);
}

// Destroys all created nodes and its linked lines
void ClearGraph()
{
    for (int i = nodesCount - 1; i >= 0; i--) DestroyNode(nodes[i]);
    for (int i = commentsCount - 1; i >= 0; i--) DestroyComment(comments[i]);
    
    if (usedMemory != 0) TraceLogFNode(true, "all nodes have been deleted by there are still memory in use [USED RAM: %i bytes]", usedMemory);
    else TraceLogFNode(false, "all nodes have been deleted [USED RAM: %i bytes]", usedMemory);
}

// Draw canvas space to create nodes
void DrawCanvas()
{    
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

// Draw interface to create nodes
void DrawInterface()
{
    // Draw interface background
    DrawRectangleRec((Rectangle){ canvasSize.x, 0.0f, screenSize.x - canvasSize.x, screenSize.y }, DARKGRAY);
    
    // Draw interface main buttons
    if (FButton((Rectangle){ UI_PADDING, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Clear Graph")) ClearGraph();
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*1, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Align Nodes")) AlignAllNodes();
    if (FButton((Rectangle){ UI_PADDING + ((screenSize.x - canvasSize.x - UI_PADDING*2)/2 + UI_PADDING)*2, screenSize.y - (UI_BUTTON_HEIGHT + UI_PADDING), (screenSize.x - canvasSize.x - UI_PADDING*2)/2, UI_BUTTON_HEIGHT }, "Clear Unused Nodes")) ClearUnusedNodes();
    
    // Draw interface nodes buttons
    DrawText("Constant Vectors", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Constant Vectors", 10))/2, UI_PADDING*4 - menuScroll, 10, WHITE);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*1 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Value")) CreateNodeValue((float)GetRandomValue(-11, 10));
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*2 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Vector 2")) CreateNodeVector2((Vector2){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*3 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Vector 3")) CreateNodeVector3((Vector3){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*4 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Vector 4")) CreateNodeVector4((Vector4){ (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10), (float)GetRandomValue(0, 10) });
    
    DrawText("Arithmetic", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Arithmetic", 10))/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*5 - menuScroll, 10, WHITE);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*6 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Add")) CreateNodeAdd();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*7 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Subtract")) CreateNodeSubtract();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*8 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Multiply")) CreateNodeMultiply();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*9 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Divide")) CreateNodeDivide();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*10 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "One Minus")) CreateNodeOneMinus();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*11 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Exp 2")) CreateNodeExp2();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*12 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Power")) CreateNodePower();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*13 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Posterize")) CreateNodePosterize();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*14 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Abs")) CreateNodeAbs();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*15 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Clamp 0-1")) CreateNodeClamp01();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*16 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Max")) CreateNodeMax();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*17 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Min")) CreateNodeMin();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*18 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Negate")) CreateNodeNegate();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*19 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Noise")) CreateNodeNoise();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*20 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Noise Int")) CreateNodeNoiseInt();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*21 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Reciprocal")) CreateNodeReciprocal();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*22 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Ceil")) CreateNodeCeil();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*23 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Round")) CreateNodeRound();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*24 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Trunc")) CreateNodeTrunc();
    
    DrawText("Vector Operations", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Vector Operations", 10))/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*25 - menuScroll, 10, WHITE);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*26 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Append")) CreateNodeAppend();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*27 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Normalize")) CreateNodeNormalize();
    
    DrawText("Math Constants", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Math Constants", 10))/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*28 - menuScroll, 10, WHITE);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*29 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "PI")) CreateNodePI();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*30 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "e")) CreateNodeE();

    DrawText("Trigonometry", canvasSize.x + ((screenSize.x - canvasSize.x) - MeasureText("Trigonometry", 10))/2, UI_PADDING*4 + (UI_BUTTON_HEIGHT + UI_PADDING)*31 - menuScroll, 10, WHITE);
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*32 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Cosine")) CreateNodeCos();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*33 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Sine")) CreateNodeSin();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*34 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Tangent")) CreateNodeTan();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*35 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Deg to Rad")) CreateNodeDeg2Rad();
    if (FButton((Rectangle){ canvasSize.x + UI_PADDING, UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*36 - menuScroll, screenSize.x - canvasSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Rad to Deg")) CreateNodeRad2Deg();
    
    if (debugMode)
    {
        const char *string = 
        "selectedNode: %i\n"
        "editNode: %i\n"
        "lineState: %i\n"
        "commentState: %i\n"
        "selectedComment: %i\n"
        "editSize: %i\n"
        "editSizeType: %i\n"
        "editComment: %i\n"
        "editNodeText: %s";
        
        DrawText(FormatText(string, selectedNode, editNode, lineState, commentState, selectedComment, editSize, editSizeType, editComment, ((editNodeText != NULL) ? editNodeText : "NULL")), 10, 10, 10, BLACK);
    }
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
    commentsCount = 0;
    selectedCommentNodesCount = 0;
    for (int i = 0; i < MAX_NODES; i++) selectedCommentNodes[i] = -1;
    
    TraceLogFNode(false, "initialization complete");
}

// Creates a value node (1 float)
FNode CreateNodeValue(float value)
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
FNode CreateNodeVector2(Vector2 vector)
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
}

// Creates a Vector3 node (3 float)
FNode CreateNodeVector3(Vector3 vector)
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
}

// Creates a Vector4 node (4 float)
FNode CreateNodeVector4(Vector4 vector)
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
}

// Creates a node to add multiples values
FNode CreateNodeAdd()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_ADD;
    newNode->name = "Add";

    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to subtract multiples values by the first linked
FNode CreateNodeSubtract()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_SUBTRACT;
    newNode->name = "Subtract";
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to multiply different values
FNode CreateNodeMultiply()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_MULTIPLY;
    newNode->name = "Multiply";
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to divide multiples values by the first linked
FNode CreateNodeDivide()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_DIVIDE;
    newNode->name = "Divide";
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to combine little data types into bigger data types (2 int = 1 Vector2)
FNode CreateNodeAppend()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_APPEND;
    newNode->name = "Append";
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to subtract a value to 1
FNode CreateNodeOneMinus()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_ONEMINUS;
    newNode->name = "One Minus";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to get absolute value of the input
FNode CreateNodeAbs()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_ABS;
    newNode->name = "Abs";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to get sin value of the input
FNode CreateNodeCos()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_COS;
    newNode->name = "Cosine";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to get sin value of the input
FNode CreateNodeSin()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_SIN;
    newNode->name = "Sine";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to get sin value of the input
FNode CreateNodeTan()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_TAN;
    newNode->name = "Tangent";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to convert a degrees value to radians
FNode CreateNodeDeg2Rad()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_DEG2RAD;
    newNode->name = "Deg to Rad";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to convert a radians value to degrees                  
FNode CreateNodeRad2Deg()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_RAD2DEG;
    newNode->name = "Rad to Deg";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to normalize a vector
FNode CreateNodeNormalize()
{
   FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_NORMALIZE;
    newNode->name = "Normalize";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}                    

// Creates a node to get negative value of the input
FNode CreateNodeNegate()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_NEGATE;
    newNode->name = "Negate";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to get 1/input value
FNode CreateNodeReciprocal()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_RECIPROCAL;
    newNode->name = "Reciprocal";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to get the truncated value of the input
FNode CreateNodeTrunc()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_TRUNC;
    newNode->name = "Truncate";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to get the rounded value of the input
FNode CreateNodeRound()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_ROUND;
    newNode->name = "Round";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to get the rounded up to the nearest integer of the input
FNode CreateNodeCeil()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_CEIL;
    newNode->name = "Ceil";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to clamp the input value between 0-1 range
FNode CreateNodeClamp01()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_CLAMP01;
    newNode->name = "Clamp 0-1";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node that returns 2 to the power of its input
FNode CreateNodeExp2()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_EXP2;
    newNode->name = "Exp 2";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node that returns the first input to the power of the second input
FNode CreateNodePower()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_POWER;
    newNode->name = "Power";
    newNode->inputsLimit = 2;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node that posterizes the first input in a number of steps set by second input
FNode CreateNodePosterize()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_POSTERIZE;
    newNode->name = "Posterize";
    newNode->inputsLimit = 2;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to calculate the higher value of all inputs values
FNode CreateNodeMax()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_MAX;
    newNode->name = "Max";
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to calculate the lower value of all inputs values
FNode CreateNodeMin()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_MIN;
    newNode->name = "Min";
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to calculate random values between input range
FNode CreateNodeNoise()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_NOISE;
    newNode->name = "Noise";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node to calculate integer random values between input range
FNode CreateNodeNoiseInt()
{
    FNode newNode = InitializeNode(true);
    
    newNode->type = FNODE_NOISEINT;
    newNode->name = "Noise Int";
    newNode->inputsLimit = 1;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Creates a node which returns PI value
FNode CreateNodePI()
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
FNode CreateNodeE()
{
    FNode newNode = InitializeNode(false);
    
    newNode->type = FNODE_PI;
    newNode->name = "e";
    newNode->output.dataCount = 1;
    newNode->output.data[0].value = 2.71828182845904523536;
    FFloatToString(newNode->output.data[0].valueText, newNode->output.data[0].value);
    newNode->inputsLimit = 0;
    
    UpdateNodeShapes(newNode);
    
    return newNode;
}

// Initializes a new node with generic parameters
FNode InitializeNode(bool isOperator)
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
    else TraceLogFNode(true, "node creation failed because there is any available id");
    
    // Initialize node inputs and inputs count
    for (int i = 0; i < MAX_INPUTS; i++) newNode->inputs[i] = -1;
    newNode->inputsCount = 0;
    newNode->inputsLimit = MAX_INPUTS;
    
    // Initialize shapes
    newNode->shape = (Rectangle){ GetRandomValue(-camera.offset.x + 0, -camera.offset.x + screenSize.x*0.85f - 50*4), GetRandomValue(-camera.offset.y + screenSize.y/2 - 20 - 100, camera.offset.y + screenSize.y/2 - 20 + 100), 10 + NODE_DATA_WIDTH, 40 };
    if (isOperator) newNode->inputShape = (Rectangle){ 0, 0, 20, 20 };
    else newNode->inputShape = (Rectangle){ 0, 0, 0, 0 };
    newNode->outputShape = (Rectangle){ 0, 0, 20, 20 };
    
    AlignNode(newNode);
    
    // Initialize node output and output values count
    newNode->output.dataCount = 0;
    
    for (int i = 0; i < MAX_VALUES; i++)
    {
        newNode->output.data[i].value = 0.0f;
        newNode->output.data[i].shape = (Rectangle){ 0, 0, NODE_DATA_WIDTH, NODE_DATA_HEIGHT };
        newNode->output.data[i].valueText = (char *)malloc(MAX_NODE_LENGTH);
        usedMemory += MAX_NODE_LENGTH;
        for (int k = 0; k < MAX_NODE_LENGTH; k++) newNode->output.data[i].valueText[k] = '\0';
    }
    
    nodes[nodesCount] = newNode;
    nodesCount++;
    
    TraceLogFNode(false, "created new node id %i (index: %i) [USED RAM: %i bytes]", newNode->id, (nodesCount - 1), usedMemory);
    
    return newNode;
}

// Creates a line between two nodes
FLine CreateNodeLine(int from)
{
    FLine newLine = (FLine)malloc(sizeof(FLineData));
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
FComment CreateComment()
{
    FComment newComment = (FComment)malloc(sizeof(FCommentData));
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
    newComment->value = (char *)malloc(MAX_COMMENT_LENGTH);
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

// Draws a previously created node
void DrawNode(FNode node)
{
    if (node != NULL)
    {
        DrawRectangleRec(node->shape, ((node->id == selectedNode) ? GRAY : LIGHTGRAY));
        DrawRectangleLines(node->shape.x, node->shape.y, node->shape.width, node->shape.height, BLACK);
        DrawText(FormatText("%s [ID: %i]", node->name, node->id), node->shape.x + node->shape.width/2 - MeasureText(FormatText("%s [ID: %i]", node->name, node->id), 10)/2, node->shape.y - 15, 10, BLACK);
        
        if ((node->type >= FNODE_VALUE) && (node->type <= FNODE_VECTOR4))
        {
            if (node->id == editNode)
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
                        free(editNodeText);
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
        
        for (int i = 0; i < node->output.dataCount; i++)
        {
            if ((nodes[i]->type >= FNODE_VALUE) && (nodes[i]->type <= FNODE_VECTOR4)) DrawRectangleLines(node->output.data[i].shape.x,node->output.data[i].shape.y, node->output.data[i].shape.width, node->output.data[i].shape.height, (((editNode == node->id) && (editNodeType == i)) ? BLACK : GRAY));
            DrawText(node->output.data[i].valueText, node->output.data[i].shape.x + (node->output.data[i].shape.width - 
                     MeasureText(node->output.data[i].valueText, 20))/2, node->output.data[i].shape.y + 
                     node->output.data[i].shape.height/2 - 9, 20, DARKGRAY);
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
            "inputs(%i): %i, %i, %i, %i, %i, %i, %i, %i\n"
            "output(%i): %.02f, %.02f, %.02f, %.02f\n"
            "outputText: %s, %s, %s, %s\n"
            "shape: %i %i, %i, %i";
            
            DrawText(FormatText(string, node->id, node->type, node->name, node->inputsCount, node->inputs[0], node->inputs[1], node->inputs[2], 
            node->inputs[3], node->inputs[4], node->inputs[5], node->inputs[6], node->inputs[7], node->output.dataCount, node->output.data[0].value, 
            node->output.data[1].value, node->output.data[2].value, node->output.data[3].value, node->output.data[0].valueText, node->output.data[1].valueText, 
            node->output.data[2].valueText, node->output.data[3].valueText, node->shape.x, node->shape.y, node->shape.width, node->shape.height), node->shape.x, node->shape.y + node->shape.height + 5, 10, BLACK);
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced node");
}

// Draws a previously created node line
void DrawNodeLine(FLine line)
{
    if (line != NULL)
    {
        Vector2 from = { 0, 0 };
        Vector2 to = { 0, 0 };
        
        int indexTo = -1;        
        if (line->to != -1)
        {
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
            else TraceLogFNode(true, "error when trying to find node id %i due to index is out of bounds %i", line->to, indexTo);
        }
        else to = CameraToViewVector2(mousePosition, camera);
        
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
        }
        else TraceLogFNode(true, "error when trying to find node id %i due to index is out of bounds %i", line->from, indexFrom);
        
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
            int x = 0.0f;
            int y = 0.0f;
            float angle = -90.0f;
            float multiplier = (((to.y - from.y) > 0) ? 1 : -1);
            float radius = (fabs(to.y - from.y)/4 + 0.02f)*multiplier;
            float distance = FClamp(fabs(to.x - from.x)/100, 0.0f, 1.0f);
            
            DrawLine(from.x, from.y, from.x, from.y, BLACK);
            
            while (angle < 90)
            {
                DrawLine(from.x + cos(angle*DEG2RAD)*radius*multiplier*distance, from.y + radius + sin(angle*DEG2RAD)*radius, from.x + cos((angle + 10)*DEG2RAD)*radius*multiplier*distance, from.y + radius + sin((angle + 10)*DEG2RAD)*radius, BLACK);
                angle += 10;
            }
            
            Vector2 lastPosition = { from.x, from.y + radius*2 };
            
            DrawLine(lastPosition.x, lastPosition.y, to.x + cos(270*DEG2RAD)*radius*multiplier, to.y - radius + sin(270*DEG2RAD)*radius, BLACK);
            
            lastPosition.x = to.x;
            
            while (angle < 270)
            {
                DrawLine(to.x + cos(angle*DEG2RAD)*radius*multiplier*distance, to.y - radius + sin(angle*DEG2RAD)*radius, to.x + cos((angle + 10)*DEG2RAD)*radius*multiplier*distance, to.y - radius + sin((angle + 10)*DEG2RAD)*radius, BLACK);
                angle += 10;
            }
            
            lastPosition.y = lastPosition.y + radius*2;
            
            DrawLine(to.x, to.y, to.x, to.y, BLACK);
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
                case FNODE_POSTERIZE:
                {
                    if ((nodes[indexTo]->inputsCount > 0) && (nodes[indexTo]->inputs[0] == nodes[indexFrom]->id)) DrawText("Input", from.x + 5, from.y - 30, 10, BLACK);
                    else if ((nodes[indexTo]->inputsCount > 1) && (nodes[indexTo]->inputs[1] == nodes[indexFrom]->id)) DrawText("Samples", from.x + 5, from.y - 30, 10, BLACK);
                } break;
                default: break;
            }
        }
        
        if (debugMode)
        {
            const char *string =
            "id: %i\n"
            "from: %i\n"
            "to: %i\n";
            
            DrawText(FormatText(string, line->id, line->from, line->to), screenSize.x*0.85f - 10 - 50, 10 + 75*line->id, 10, BLACK);
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced line");
}

// Draws a previously created comment
void DrawComment(FComment comment)
{
    if (comment != NULL)
    {
        if ((commentState == 0) || ((commentState == 1) && (tempComment->id != comment->id)) || ((commentState == 1) && editSize != -1)) DrawRectangleRec(comment->shape, Fade(YELLOW, 0.2f));
        DrawRectangleLines(comment->shape.x, comment->shape.y, comment->shape.width, comment->shape.height, BLACK);
        
        if ((commentState == 0) || ((commentState == 1) && (tempComment->id != comment->id)) || ((commentState == 1) && editSize != -1))
        {
            DrawRectangleRec(comment->sizeTShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeTShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTShape.x, comment->sizeTShape.y, comment->sizeTShape.width, comment->sizeTShape.height, BLACK);
            DrawRectangleRec(comment->sizeBShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeBShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBShape.x, comment->sizeBShape.y, comment->sizeBShape.width, comment->sizeBShape.height, BLACK);
            DrawRectangleRec(comment->sizeLShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeLShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeLShape.x, comment->sizeLShape.y, comment->sizeLShape.width, comment->sizeLShape.height, BLACK);
            DrawRectangleRec(comment->sizeRShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeRShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeRShape.x, comment->sizeRShape.y, comment->sizeRShape.width, comment->sizeRShape.height, BLACK);
            DrawRectangleRec(comment->sizeTlShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeTlShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTlShape.x, comment->sizeTlShape.y, comment->sizeTlShape.width, comment->sizeTlShape.height, BLACK);
            DrawRectangleRec(comment->sizeTrShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeTrShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeTrShape.x, comment->sizeTrShape.y, comment->sizeTrShape.width, comment->sizeTrShape.height, BLACK);
            DrawRectangleRec(comment->sizeBlShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeBlShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBlShape.x, comment->sizeBlShape.y, comment->sizeBlShape.width, comment->sizeBlShape.height, BLACK);
            DrawRectangleRec(comment->sizeBrShape, ((CheckCollisionPointRec(mousePosition, CameraToViewRec(comment->sizeBrShape, camera))) ? LIGHTGRAY : GRAY));
            DrawRectangleLines(comment->sizeBrShape.x, comment->sizeBrShape.y, comment->sizeBrShape.width, comment->sizeBrShape.height, BLACK);
        }
        
        if (comment->id == editComment)
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
        
        if (debugMode)
        {
            const char *string =
            "id: %i\n"
            "value: %s\n"
            "shape: %i, %i, %i, %i\n";
            
            DrawText(FormatText(string, comment->id, comment->value, comment->shape.x, comment->shape.y, comment->shape.width, comment->shape.height), comment->shape.x, comment->shape.y + comment->shape.height + 5, 10, BLACK);
        }
    }
    else TraceLogFNode(true, "error trying to draw a null referenced line");
}

// Button element, returns true when pressed
bool FButton(Rectangle bounds, const char *text)
{
    ButtonState buttonState = BUTTON_DEFAULT;

    if (bounds.width < (MeasureText(text, 10) + 20)) bounds.width = MeasureText(text, 10) + 20;
    if (bounds.height < 10) bounds.height = 10 + 40;
    
    if (CheckCollisionPointRec(mousePosition, bounds))
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) buttonState = BUTTON_PRESSED;
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) buttonState = BUTTON_CLICKED;
        else buttonState = BUTTON_HOVER;
    }

    switch (buttonState)
    {
        case BUTTON_DEFAULT:
        {
            DrawRectangleRec(bounds, (Color){ UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, 255 });
            DrawRectangle(bounds.x + 2, bounds.y + 2, bounds.width - 4,bounds.height - 4, UI_BUTTON_DEFAULT_COLOR);
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, 10)/2)), bounds.y + (bounds.height - 10)/2, 10, DARKGRAY);
        } break;
        case BUTTON_HOVER:
        {
            DrawRectangleRec(bounds, (Color){ UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, UI_BORDER_DEFAULT_COLOR, 255 });
            DrawRectangle(bounds.x + 2, bounds.y + 2, bounds.width - 4,bounds.height - 4, UI_BUTTON_DEFAULT_COLOR);
            DrawRectangleRec(bounds, Fade(WHITE, 0.4f));
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, 10)/2)), bounds.y + (bounds.height - 10)/2, 10, BLACK);
        } break;
        case BUTTON_PRESSED:
        {
            DrawRectangleRec(bounds, UI_BUTTON_DEFAULT_COLOR);
            DrawRectangle(bounds.x + 2, bounds.y + 2, bounds.width - 4,bounds.height - 4, GRAY);
            DrawText(text, bounds.x + ((bounds.width/2) - (MeasureText(text, 10)/2)), bounds.y + (bounds.height - 10)/2, 10, LIGHTGRAY);
        } break;
        default: break;
    }
    
    return (buttonState == BUTTON_CLICKED);
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
            
            for (int i = 0; i < MAX_VALUES; i++)
            {
               free(nodes[index]->output.data[i].valueText);
               usedMemory -= MAX_NODE_LENGTH;
               nodes[index]->output.data[i].valueText = NULL;
            }

            free(nodes[index]);
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
        else TraceLogFNode(true, "error when trying to destroy node id %i (index: %i)", id, index);
    }
    else TraceLogFNode(true, "error trying to destroy a null referenced node");
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
void DestroyComment(FComment comment)
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
            free(comments[index]->value);
            usedMemory -= MAX_COMMENT_LENGTH;
            comments[index]->value = NULL;
            
            free(comments[index]);
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
void CloseFNode()
{
    for (int i = 0; i < nodesCount; i++)
    {
        if (nodes[i] != NULL)
        {
            for (int k = 0; k < MAX_VALUES; k++)
            {
                if (nodes[i]->output.data[k].valueText != NULL)
                {
                    free(nodes[i]->output.data[k].valueText);
                    usedMemory -= MAX_NODE_LENGTH;
                    nodes[i]->output.data[k].valueText = NULL;
                }
            }
            
            free(nodes[i]);
            usedMemory -= sizeof(FNodeData);
            nodes[i] = NULL;
        }
    }
    
    for (int i = 0; i < linesCount; i++)
    {
        if (lines[i] != NULL)
        {
            free(lines[i]);
            usedMemory -= sizeof(FLineData);
            lines[i] = NULL;
        }
    }
    
    for (int i = 0; i < commentsCount; i++)
    {
        if ((comments[i] != NULL) && (comments[i]->value != NULL))
        {
            free(comments[i]->value);
            usedMemory -= MAX_COMMENT_LENGTH;
            comments[i]->value = NULL;
            
            free(comments[i]);
            usedMemory -= sizeof(FCommentData);
            comments[i] = NULL;
        }
    }
    
    if (editNodeText != NULL)
    {
        free(editNodeText);
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
void TraceLogFNode(bool error, const char *text, ...)
{
    va_list args;
    
    fprintf(stdout, "FNode: ");
    va_start(args, text);
    vfprintf(stdout, text, args);
    va_end(args);

    fprintf(stdout, "\n");

    if (error) exit(1);
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

// Returns an ease linear value between two parameters
float FEaseLinear(float t, float b, float c, float d)
{ 
    return (float)(c*t/d + b); 
}

// Returns an ease quadratic in-out value between two parameters
float FEaseInOutQuad(float t, float b, float c, float d)
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
void FStringToFloat(float *pointer, const char *string)
{
    *pointer = (float)atof(string);
}

// Sends formatted output to an initialized string pointer
void FFloatToString(char *buffer, float value)
{
    sprintf(buffer, "%.2f", value);
}