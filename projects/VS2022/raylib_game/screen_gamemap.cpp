/**********************************************************************************************
*   Tree Choice Game Screen (Replace Logo Screen)
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>


//---------------------------------------------------------------------------
// Tree data structure
//---------------------------------------------------------------------------
typedef struct GameMapNode {
    std::string text;
    std::vector<int> parents;   // 往上連的節點 index
    int level; // 上層越大
    //GameMapNode(const char* t, std::vector<int> p, int l)
    //    : text(t), parents(std::move(p)), level(l) {}
    //GameMapNode() : text(), parents(), level() {}
} GameMapNode;

// 模擬一棵決策樹 (可自由改)
std::vector<GameMapNode> game_map_tree = {
    {"Ending A", {}, 0},             // 0: Leaf
    {"Ending B", {}, 0},             // 1: Leaf
    {"Ending C", {}, 0},             // 2: Leaf
    {"Ending D", {}, 0},             // 3
    {"Ending E", {}, 0},             // 4

    {"Path AB", {0,1}, 1},           // 5
    {"Path BC", {1,2}, 1},           // 6

    {"Root Start", {6, 7}, 2}         // 7 Top
};

static std::vector<Vector2> nodePos; // 暫存每個 node 的畫面座標
static int RADIOUS = 32;
//---------------------------------------------------------------------------
// Local state
//---------------------------------------------------------------------------
static int currentNode = 0;         // 玩家目前位於哪個節點
static int finishScreen = 0;
void RandGameMap(int levels, int maxNodesPerLevel, int maxParents)
{
    game_map_tree.clear();
    
    std::vector<std::vector<int>> levelIndex(levels + 1); // 各層節點索引 list

    int idCounter = 0;

    // 從底層到頂層建立節點
    for (int lvl = 0; lvl <= levels; lvl++) {
        int nodeCount = (rand() % maxNodesPerLevel) + 1;

        for (int i = 0; i < nodeCount; i++) {
            GameMapNode n;
            n.level = lvl;

            // 產生名字
            if (lvl == 0) {
                n.text = (std::string("Ending ") + std::string(1, 'A' + idCounter));
            }
            else if (lvl == levels) {
                n.text = "Root " + std::to_string(idCounter);
            }
            else {
                n.text = "Path " + std::to_string(idCounter);
            }

            // 設定父節點 (往上一層連)
            if (lvl > 0) {
                int parentsCount = std::min(maxParents, (int)levelIndex[lvl - 1].size());
                parentsCount = (parentsCount > 0) ? (rand() % parentsCount + 1) : 0;

                for (int p = 0; p < parentsCount; p++) {
                    int parentIdx = rand() % levelIndex[lvl - 1].size();
                    n.parents.push_back(levelIndex[lvl - 1][parentIdx]);
                }
            }

            game_map_tree.push_back(n);
            levelIndex[lvl].push_back(idCounter);
            idCounter++;
        }
    }
    
    printf("Draw tree=%p size=%zu\n", &game_map_tree, game_map_tree.size());
}

int GetTreeNodeCount() 
{
    return game_map_tree.size();
}
//---------------------------------------------------------------------------
    // Init
    //---------------------------------------------------------------------------
void InitGameMapScreen(void)
{
    printf("RandGameMap tree=%p size=%zu\n", &game_map_tree, game_map_tree.size());
    finishScreen = 0;
    RandGameMap(5, 4, 3);
    currentNode = GetTreeNodeCount()>0? GetTreeNodeCount() - 1 : 0;  // 玩家從最底部 (0 = 最後結局) 開始
    nodePos.resize(GetTreeNodeCount());
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
    if (IsKeyPressed(KEY_ONE) && game_map_tree[currentNode].parents.size() >= 1) {
        currentNode = game_map_tree[currentNode].parents[0];
    }
    if (IsKeyPressed(KEY_TWO) && game_map_tree[currentNode].parents.size() >= 2) {
        currentNode = game_map_tree[currentNode].parents[1];
    }
    if (IsKeyPressed(KEY_THREE) && game_map_tree[currentNode].parents.size() >= 3) {
        currentNode = game_map_tree[currentNode].parents[2];
    }

    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // 遍歷目前節點的父節點，看有沒有被點到
        for (int i = 0; i < game_map_tree[currentNode].parents.size(); i++) {
            int parentId = game_map_tree[currentNode].parents[i];
            Vector2 pos = nodePos[parentId];

            float dx = mouse.x - pos.x;
            float dy = mouse.y - pos.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < RADIOUS) {  // 圓半徑 = 32
                currentNode = parentId;
                return;
            }
        }
    }


    // 到頂端 Root
    if (game_map_tree[currentNode].parents.size() == 0 && currentNode != 0) {
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
    int maxLevel = 0;
    for (int i = 0; i < GetTreeNodeCount(); i++) if (game_map_tree[i].level > maxLevel) maxLevel = game_map_tree[i].level;
    std::vector<std::vector<int>> levels(maxLevel + 1);

    for (int i = 0; i < GetTreeNodeCount(); i++) {
        levels[game_map_tree[i].level].push_back(i);
        if (game_map_tree[i].level > maxLevel) maxLevel = game_map_tree[i].level;
    }

    int baseY = 150;
    int offsetY = 120;
    int offsetX = 160;

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
            nodePos[node] = { (float)x, (float)y };  // ✅ 記住節點座標

            // --- Draw edges (parent lines)
            for (int p : game_map_tree[node].parents) {
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

            DrawCircle(x, y, RADIOUS, fill);
            DrawCircleLines(x, y, RADIOUS, c);

            bool clickable = false;
            for (int p : game_map_tree[currentNode].parents)
                if (p == node) clickable = true;

            if (clickable)
                DrawCircleLines(x, y, RADIOUS + 4, ORANGE); // highlight clickable nodes

            int textWidth = MeasureText(game_map_tree[node].text.c_str(), 20);
            DrawText(game_map_tree[node].text.c_str(), x - textWidth / 2, y - 10, 20, c);
        }
    }

    // Show current selection
    DrawText("Current:", 50, GetScreenHeight() - 120, 22, BLACK);
    DrawText(game_map_tree[currentNode].text.c_str(), 140, GetScreenHeight() - 120, 24, BLUE);
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