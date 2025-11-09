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
    {"Ending D", {}, 0},
    {"Ending E", {}, 0},

    {"Path AB", {0,1}, 1},           // 3
    {"Path BC", {1,2}, 1},           // 4

    {"Root Start", {6, 7}, 2}         // 5 Top
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

    for (int i = 0; i < NODE_COUNT; i++) {
        levels[tree[i].level].push_back(i);
        if (tree[i].level > maxLevel) maxLevel = tree[i].level;
    }

    int baseY = 150;
    int offsetY = 120;
    int offsetX = 160;
    int radius = 32;

    // 對每一層
    for (int lvl = 0; lvl <= maxLevel; lvl++) {
        int count = levels[lvl].size();
        if (count == 0) continue;

        // 這層起始 X，讓整層置中
        int totalWidth = (count - 1) * offsetX;
        int startX = GetScreenWidth() / 2 - totalWidth / 2;

        for (int idx = 0; idx < count; idx++) {
            int node = levels[lvl][idx];

            int x = startX + idx * offsetX;
            int y = baseY + lvl * offsetY;

            // --- Draw edges (parent lines)
            for (int p : tree[node].parents) {
                // 找父節點位置 (需要搜尋)
                for (int lvl2 = 0; lvl2 <= maxLevel; lvl2++) {
                    for (int idx2 = 0; idx2 < levels[lvl2].size(); idx2++) {
                        int candidate = levels[lvl2][idx2];
                        if (candidate == p) {
                            int px = GetScreenWidth() / 2 - ((levels[lvl2].size() - 1) * offsetX) / 2 + idx2 * offsetX;
                            int py = baseY + lvl2 * offsetY;
                            DrawLine(x, y, px, py, LIGHTGRAY);
                        }
                    }
                }
            }

            // --- Draw nodes
            Color c = (node == currentNode) ? RED : BLACK;
            Color fill = (node == currentNode) ? PINK : RAYWHITE;

            DrawCircle(x, y, radius, fill);
            DrawCircleLines(x, y, radius, c);

            int textWidth = MeasureText(tree[node].text, 20);
            DrawText(tree[node].text, x - textWidth / 2, y - 10, 20, c);
        }
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
