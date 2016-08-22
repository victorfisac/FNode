#include <stdlib.h>         // Required for: malloc(), free()
#include <math.h>           // Required for: sqrt()
#include "raylib.h"         // Required for basic 2d shapes drawing and window management
#include "utils.h"          // Required for: TraceLog()

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Defines
#define MAX_INPUTS          8           // Max number of inputs in every node
#define MAX_VALUES          4           // Max number of values in every output
#define MAX_NODES           256         // Max number of nodes
#define MAX_LINES           1024        // Max number of lines
#define UI_PADDING          5           // Interface bounds padding with background
#define UI_BUTTON_HEIGHT    30          // Interface bounds height

// Enums
typedef enum {
    FNODE_VALUE,
    FNODE_VECTOR2,
    FNODE_VECTOR3,
    FNODE_COLOR,
    FNODE_ADD,
    FNODE_SUBTRACT,
    FNODE_MULTIPLY,
    FNODE_DIVIDE
} FNodeType;

// Structs
typedef struct FNodeValue {
    float values[MAX_VALUES];               // Output values array (float, Vector2, Vector3 or Color)
    int valuesCount;                        // Output values length to know which type of data represents
} FNodeValue;

typedef struct FNodeData {
    unsigned int id;                        // Node unique identifier
    FNodeType type;                         // Node type (values, operations...)
    const char *name;                       // Displayed name of the node
    int inputs[MAX_INPUTS];                 // Inputs node ids array
    unsigned int inputsCount;               // Inputs node ids array length
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

// Global variables
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

int usedMemory = 0;                         // Total USED RAM from memory allocation

Vector2 menuSize;                           // Interface screen size

// Functions declarations
void UpdateMouseInputs();                   // Updates mouse inputs to drag and link nodes
void UpdateValues();                        // Updates nodes output values based on current inputs
void CalculateValues();                     // Calculates nodes output values based on current inputs
void UpdateRectangles();                    // Updates node shapes due to drag behaviour

// FNODE functions declarations
void InitFNode();                           // Initializes FNode global variables

FNode CreateNodeValue(float value);         // Creates a value node in center of screen
FNode CreateNodeVector2(Vector2 vector);    // Creates a Vector2 node in center of screen
FNode CreateNodeVector3(Vector3 vector);    // Creates a Vector3 node in center of screen
FNode CreateNodeColor(Color vector);        // Creates a Color node in center of screen
FNode CreateNodeAdd();                      // Creates an add node in center of screen to add multiples values
FNode CreateNodeSubtract();                 // Creates a subtract node in center of screen to subtract multiples values by the first linked
FNode CreateNodeMultiply();                 // Creates a multiply node in center of screen to multiply different values
FNode CreateNodeDivide();                   // Creates a divide node in center of screen to divide multiples values by the first linked
FLine CreateNodeLine();                     // Creates a line between two nodes

FNode InitializeNode();                     // Initializes a new node with generic parameters

void DrawNode(FNode node);                  // Draws a previously created node
void DrawNodeLine(FLine line);              // Draws a previously created node line

void DestroyNode(FNode node);               // Destroys a node and its linked lines
void DestroyNodeLine(FLine line);           // Destroys a node line

void CloseFNode();                          // Unitializes FNode global variables

// Math functions declarations
float Vector2Length(Vector2 v);             // Returns length of a vector

int main()
{
    remove("debug.txt");
    
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenSize.x, screenSize.y, "fnode 0.0.1");
    InitFNode();
    
    // Initialize values
    menuSize = (Vector2){ screenSize.x*0.85f, 0.0f };
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateMouseInputs();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            // Draw all created nodes and lines
            for (int i = 0; i < nodesCount; i++) DrawNode(nodes[i]);
            for (int i = 0; i < linesCount; i++) DrawNodeLine(lines[i]);
            
            // Draw interface background
            DrawRectangleRec((Rectangle){ menuSize.x, menuSize.y, screenSize.x - menuSize.x, screenSize.y }, DARKGRAY);
            
            // Draw interface elements
            if (GuiButton((Rectangle){ menuSize.x + UI_PADDING, menuSize.y + UI_PADDING, screenSize.x - menuSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Value")) CreateNodeValue(2.0f);
            if (GuiButton((Rectangle){ menuSize.x + UI_PADDING, menuSize.y + UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING), screenSize.x - menuSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Vector 2")) CreateNodeVector2((Vector2){ 0.0f, 0.0f });
            if (GuiButton((Rectangle){ menuSize.x + UI_PADDING, menuSize.y + UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*2, screenSize.x - menuSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Vector 3")) CreateNodeVector3((Vector3){ 0.0f, 0.0f, 0.0f });
            if (GuiButton((Rectangle){ menuSize.x + UI_PADDING, menuSize.y + UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*3, screenSize.x - menuSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Color")) CreateNodeColor((Color){ 0.0f, 0.0f, 0.0f, 0.0f });
            if (GuiButton((Rectangle){ menuSize.x + UI_PADDING, menuSize.y + UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*4, screenSize.x - menuSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Add")) CreateNodeAdd();
            if (GuiButton((Rectangle){ menuSize.x + UI_PADDING, menuSize.y + UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*5, screenSize.x - menuSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Subtract")) CreateNodeSubtract();
            if (GuiButton((Rectangle){ menuSize.x + UI_PADDING, menuSize.y + UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*6, screenSize.x - menuSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Multiply")) CreateNodeMultiply();
            if (GuiButton((Rectangle){ menuSize.x + UI_PADDING, menuSize.y + UI_PADDING + (UI_BUTTON_HEIGHT + UI_PADDING)*7, screenSize.x - menuSize.x - UI_PADDING*2, UI_BUTTON_HEIGHT }, "Divide")) CreateNodeDivide();
            
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

// Updates mouse inputs to drag and link nodes
void UpdateMouseInputs()
{
    lastMousePosition = mousePosition;
    mousePosition = GetMousePosition();
    mouseDelta = (Vector2){ mousePosition.x - lastMousePosition.x, mousePosition.y - lastMousePosition.y };
    
    if (selectedId == -1)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            for (int i = 0; i < nodesCount; i++)
            {
                if (CheckCollisionPointRec(mousePosition, nodes[i]->shape))
                {
                    selectedId = nodes[i]->id;
                    break;
                }
            }
        }
        else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            for (int i = 0; i < nodesCount; i++)
            {
                if (CheckCollisionPointRec(mousePosition, nodes[i]->shape))
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
                nodes[i]->shape.x += mouseDelta.x;
                nodes[i]->shape.y += mouseDelta.y;
                UpdateRectangles();
                
                if (nodes[i]->shape.x - nodes[i]->inputShape.width < 0 || nodes[i]->shape.y - 15 < 0 || 
                    nodes[i]->shape.x + nodes[i]->shape.width + nodes[i]->outputShape.width > screenSize.x*0.85f || nodes[i]->shape.y + nodes[i]->shape.height > screenSize.y)
                {
                    nodes[i]->shape.x -= mouseDelta.x;
                    nodes[i]->shape.y -= mouseDelta.y;
                    UpdateRectangles();
                }
                break;
            }
        }
        
        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) selectedId = -1;
    }
    
    switch (lineState)
    {
        case 0:
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                for (int i = 0; i < nodesCount; i++)
                {
                    if (CheckCollisionPointRec(mousePosition, nodes[i]->outputShape))
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
                    if (CheckCollisionPointRec(mousePosition, nodes[i]->outputShape))
                    {
                        for (int k = linesCount - 1; k >= 0; k--)
                        {
                            if (nodes[i]->id == lines[k]->from) DestroyNodeLine(lines[k]);
                        }
                        
                        CalculateValues();
                    }
                    else if (CheckCollisionPointRec(mousePosition, nodes[i]->inputShape))
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
                    if (CheckCollisionPointRec(mousePosition, nodes[i]->inputShape) && (nodes[i]->id != tempLine->from) && (nodes[i]->inputsCount < MAX_INPUTS))
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

// Calculates nodes output values based on current inputs
void CalculateValues()
{
    for(int i = 0; i < nodesCount; i++)
    {
        if(nodes[i] != NULL)
        {            
            // Reset node inputs values and inputs count
            for(int k = 0; k < MAX_VALUES; k++) nodes[i]->inputs[k] = -1;
            nodes[i]->inputsCount = 0;
            
            // Search for node inputs and calculate inputs count
            for(int k = 0; k < linesCount; k++)
            {
                if(lines[k] != NULL)
                {
                    // Check if line output (to) is the node input id
                    if(lines[k]->to == nodes[i]->id)
                    {
                        nodes[i]->inputs[nodes[i]->inputsCount] = lines[k]->from;
                        nodes[i]->inputsCount++;
                    }
                }
            }
            
            // Check if current node is an operator
            if(nodes[i]->type > FNODE_COLOR)
            {
                // Calculate output values count based on first input node value count
                if(nodes[i]->inputsCount > 0) 
                {
                    // Get which index has the first input node id from current nude
                    int index = -1;
                    for(int k = 0; k < nodesCount; k++)
                    {
                        if(nodes[k]->id == nodes[i]->inputs[0])
                        {
                            index = k;
                            break;
                        }
                    }
                    
                    if(index != -1) nodes[i]->output = nodes[index]->output;
                    else TraceLog(2, "FNode: error trying to get inputs from node id %i due to index is out of bounds %i\n", nodes[i]->id, index);
                    
                    for(int k = 1; k < nodes[i]->inputsCount; k++)
                    {
                        int inputIndex = -1;
                        for(int j = 0; j < nodesCount; j++)
                        {
                            if(nodes[j]->id == nodes[i]->inputs[k])
                            {
                                inputIndex = j;
                                break;
                            }
                        }
                        
                        if(inputIndex != -1)
                        {
                            switch(nodes[i]->type)
                            {
                                case FNODE_ADD:
                                {
                                    for(int j = 0; j < nodes[i]->output.valuesCount; j++) nodes[i]->output.values[j] += nodes[inputIndex]->output.values[j];
                                } break;
                                default: break;
                            }
                        }
                        else TraceLog(2, "FNode: error trying to get node id %i due to index is out of bounds %i\n", nodes[i]->inputs[k], inputIndex);
                    }
                }
                else
                {
                    for(int k = 0; k < MAX_VALUES; k++) nodes[i]->output.values[k] = 0.0f;
                    nodes[i]->output.valuesCount = 0;
                }
            }
        }
        else TraceLog(1, "FNode: error trying to calculate values for a null referenced node\n");
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

// Initializes FNode global variables
void InitFNode()
{
    nodesCount = 0;
    linesCount = 0;
    
    TraceLog(2, "FNode: initialization complete\n");
}

// Creates a value node in center of screen
FNode CreateNodeValue(float value)
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_VALUE;
    newNode->name = "Value";
    newNode->output.valuesCount = 1;
    newNode->output.values[0] = value;
    newNode->shape = (Rectangle){ screenSize.x/2 - 50, screenSize.y/2 - 20 , 20 + 50*newNode->output.valuesCount, 40 };

    UpdateRectangles();
    
    return newNode;
}

// Creates a Vector2 node in center of screen
FNode CreateNodeVector2(Vector2 vector)
{
    // TODO
}

// Creates a Vector3 node in center of screen
FNode CreateNodeVector3(Vector3 vector)
{
    // TODO
}

// Creates a Color node in center of screen
FNode CreateNodeColor(Color color)
{
    // TODO
}

// Creates an add node in center of screen to add multiples values
FNode CreateNodeAdd()
{
    FNode newNode = InitializeNode();
    
    newNode->type = FNODE_ADD;
    newNode->name = "Add";
    newNode->shape = (Rectangle){ screenSize.x/2 - 50, screenSize.y/2 - 20 , 20 + 50*newNode->output.valuesCount, 40 };
    
    UpdateRectangles();
    
    return newNode;
}

// Creates a subtract node in center of screen to subtract multiples values by the first linked
FNode CreateNodeSubtract()
{
    // TODO
}

// Creates a multiply node in center of screen to multiply different values
FNode CreateNodeMultiply()
{
    // TODO
}

// Creates a divide node in center of screen to divide multiples values by the first linked
FNode CreateNodeDivide()
{
    // TODO
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
    else TraceLog(1, "FNode: node creation failed because there is any available id\n");
    
    // Initialize node inputs and inputs count
    for (int i = 0; i < MAX_INPUTS; i++) newNode->inputs[i] = -1;
    newNode->inputsCount = 0;
    
    // Initialize node output and output values count
    for (int i = 0; i < MAX_VALUES; i++) newNode->output.values[i] = 0.0f;
    newNode->output.valuesCount = 0;
    
    // Initialize shapes
    newNode->shape = (Rectangle){ 0, 0, 0, 0 };
    newNode->inputShape = (Rectangle){ 0, 0, 0, 0 };
    newNode->outputShape = (Rectangle){ 0, 0, 0, 0 };
    
    nodes[nodesCount] = newNode;
    nodesCount++;
    
    TraceLog(2, "FNode: created new node id %i (index: %i) [USED RAM: %i bytes]\n", newNode->id, (nodesCount - 1), usedMemory);
    
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
    else TraceLog(1, "FNode: line creation failed because there is any available id\n");
    
    // Initialize line input
    newLine->from = from;
    newLine->to = -1;
    
    lines[linesCount] = newLine;
    linesCount++;
    
    TraceLog(2, "FNode: created new line id %i (index: %i) [USED RAM: %i bytes]\n", newLine->id, (linesCount - 1), usedMemory);
    
    return newLine;
}

// Draws a previously created node
void DrawNode(FNode node)
{
    if (node != NULL)
    {
        DrawRectangleRec(node->shape, LIGHTGRAY);
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
        
        DrawRectangleRec(node->inputShape, GRAY);
        DrawRectangleLines(node->inputShape.x, node->inputShape.y, node->inputShape.width, node->inputShape.height, BLACK);
        
        DrawRectangleRec(node->outputShape, GRAY);
        DrawRectangleLines(node->outputShape.x, node->outputShape.y, node->outputShape.width, node->outputShape.height, BLACK);
        
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
    else TraceLog(1, "FNode: error trying to draw a null referenced node\n");
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
            else to = mousePosition;
        }
        else TraceLog(1, "FNode: error when trying to find node id %i due to index is out of bounds %i\n", line->from, indexFrom);
        
        DrawLine(from.x, from.y, to.x, to.y, BLACK);
                
        const char *string =
        "id: %i\n"
        "from: %i\n"
        "to: %i\n";
        
        DrawText(FormatText(string, line->id, line->from, line->to), screenSize.x*0.85f - 10 - 50, 10 + 75*line->id, 10, BLACK);
    }
    else TraceLog(1, "FNode: error trying to draw a null referenced line\n");
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
            TraceLog(2, "FNode: destroyed node id %i (index: %i) [USED RAM: %i bytes]\n", id, index, usedMemory);
            
            CalculateValues();
        }
        else TraceLog(1, "FNode: error when trying to destroy node id %i (index: %i)\n", id, index);
    }
    else TraceLog(1, "FNode: error trying to destroy a null referenced node\n");
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
            TraceLog(2, "FNode: destroyed line id %i (index: %i) [USED RAM: %i bytes]\n", id, index, usedMemory);
            
            CalculateValues();
        }
        else TraceLog(1, "FNode: error when trying to destroy line id %i due to index is out of bounds %i\n", id, index);
    }
    else TraceLog(1, "FNode: error trying to destroy a null referenced line\n");
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
    
    TraceLog(2, "FNode: unitialization complete [USED RAM: %i bytes]\n", usedMemory);
}

// Returns length of a vector
float Vector2Length(Vector2 v)
{
    return sqrt(v.x*v.x + v.y*v.y);
}