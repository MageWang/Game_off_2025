/**********************************************************************************************
*   Tree Choice Game Screen (Replace Logo Screen)
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"
#include <vector>
#include <string>

//---------------------------------------------------------------------------
// Tree data structure
//---------------------------------------------------------------------------

typedef struct Node {
    const char* text;
    std::vector<int> parents;   // 往上連的節點 index
    int level; // 上層越大
} Node;

// 模擬一棵決策樹 (可自由改)
static Node tree[] = {
    {"Ending A", {}, 0},             // 0: Leaf
    {"Ending B", {}, 0},             // 1: Leaf
    {"Ending C", {}, 0},             // 2: Leaf

    {"Path AB", {0,1}, 1},           // 3
    {"Path BC", {1,2}, 1},           // 4

    {"Root Start", {3,4}, 2}         // 5 Top
};
static const int NODE_COUNT = sizeof(tree) / sizeof(tree[0]);

//---------------------------------------------------------------------------
// Local state
//---------------------------------------------------------------------------
static int currentNode = 0;         // 玩家目前位於哪個節點
static int finishScreen = 0;

//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------
void InitGameMapScreen(void)
{
    finishScreen = 0;
    currentNode = NODE_COUNT - 1;  // 玩家從最底部 (0 = 最後結局) 開始
}

//---------------------------------------------------------------------------
// Update
//---------------------------------------------------------------------------
void UpdateGameMapScreen(void)
{
    // 玩家按空白鍵 => 跳過 (直接到標題)
    if (IsKeyPressed(KEY_SPACE)) {
        finishScreen = 1;
        return;
    }

    // 按 1 / 2 / 3 選擇父節點
    if (IsKeyPressed(KEY_ONE) && tree[currentNode].parents.size() >= 1) {
        currentNode = tree[currentNode].parents[0];
    }
    if (IsKeyPressed(KEY_TWO) && tree[currentNode].parents.size() >= 2) {
        currentNode = tree[currentNode].parents[1];
    }
    if (IsKeyPressed(KEY_THREE) && tree[currentNode].parents.size() >= 3) {
        currentNode = tree[currentNode].parents[2];
    }

    // 到頂端 Root
    if (tree[currentNode].parents.size() == 0 && currentNode != 0) {
        finishScreen = 1;
    }
}

//---------------------------------------------------------------------------
// Draw
//---------------------------------------------------------------------------
void DrawGameMapScreen(void)
{
    ClearBackground(RAYWHITE);

    DrawText("Tree Choice Game", 20, 20, 28, DARKGRAY);
    DrawText("Press 1/2/3 to go up | SPACE to skip", 20, 60, 20, GRAY);

    // 找出每一層節點
    std::vector<int> levels[10]; // 假設最多10層
    int maxLevel = 0;

    int baseX = GetScreenWidth() / 2;
    int baseY = 120;
    int offsetX = 160; // 左右間距
    int offsetY = 120; // 上下間距
    int radius = 32;

    for (int i = 0; i < NODE_COUNT; i++) {
        levels[tree[i].level].push_back(i);
        if (tree[i].level > maxLevel) maxLevel = tree[i].level;
    }

    // Draw connections (lines)
    for (int i = 0; i < NODE_COUNT; i++) {
        for (int p : tree[i].parents) {
            int x1 = baseX + (i - NODE_COUNT / 2) * offsetX;
            int y1 = baseY + tree[i].level * offsetY;

            int x2 = baseX + (p - NODE_COUNT / 2) * offsetX;
            int y2 = baseY + tree[p].level * offsetY;

            DrawLine(x1, y1, x2, y2, LIGHTGRAY);
        }
    }

    // Draw nodes (circles)
    for (int i = 0; i < NODE_COUNT; i++) {
        int x = baseX + (i - NODE_COUNT / 2) * offsetX;
        int y = baseY + tree[i].level * offsetY;

        Color c = (i == currentNode) ? RED : BLACK;
        Color fill = (i == currentNode) ? PINK : RAYWHITE;

        DrawCircle(x, y, radius, fill);
        DrawCircleLines(x, y, radius, c);

        int textWidth = MeasureText(tree[i].text, 20);
        DrawText(tree[i].text, x - textWidth / 2, y - 10, 20, c);
    }

    // Show current selection
    DrawText("Current:", 50, GetScreenHeight() - 120, 22, BLACK);
    DrawText(tree[currentNode].text, 140, GetScreenHeight() - 120, 24, BLUE);
}

//---------------------------------------------------------------------------
// Unload
//---------------------------------------------------------------------------
void UnloadGameMapScreen(void) {}

//---------------------------------------------------------------------------
// End?
//---------------------------------------------------------------------------
int FinishGameMapScreen(void)
{
    return finishScreen;
}
