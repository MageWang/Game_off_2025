/**********************************************************************************************
*   Tree Choice Game Screen (Replace Logo Screen)
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"
#include <vector>
#include <string>
#include <algorithm>

namespace Screen_GameMap
{
    //---------------------------------------------------------------------------
    // Tree data structure
    //---------------------------------------------------------------------------
    typedef struct Node {
        std::string text;
        std::vector<int> parents;   // 往上連的節點 index
        int level; // 上層越大
        Node(const char* t, std::vector<int> p, int l)
            : text(t), parents(std::move(p)), level(l) {}
        Node() : text(), parents(), level() {}
    } Node;

    // 模擬一棵決策樹 (可自由改)
    static std::vector<Node> tree = {
        {"Ending A", {}, 0},             // 0: Leaf
        {"Ending B", {}, 0},             // 1: Leaf
        {"Ending C", {}, 0},             // 2: Leaf
        {"Ending D", {}, 0},
        {"Ending E", {}, 0},

        {"Path AB", {0,1}, 1},           // 3
        {"Path BC", {1,2}, 1},           // 4

        {"Root Start", {6, 7}, 2}         // 5 Top
    };

    static int NODE_COUNT = tree.size();
    static Vector2 nodePos[100]; // 暫存每個 node 的畫面座標
    static int RADIOUS = 32;
    //---------------------------------------------------------------------------
    // Local state
    //---------------------------------------------------------------------------
    static int currentNode = 0;         // 玩家目前位於哪個節點
    static int finishScreen = 0;
    void RandGameMap(int levels, int maxNodesPerLevel, int maxParents)
    {
        tree.clear();
        std::vector<std::vector<int>> levelIndex(levels + 1); // 各層節點索引 list

        int idCounter = 0;

        // 從底層到頂層建立節點
        for (int lvl = 0; lvl <= levels; lvl++) {
            int nodeCount = (rand() % maxNodesPerLevel) + 1;

            for (int i = 0; i < nodeCount; i++) {
                Node n;
                n.level = lvl;

                // 產生名字
                if (lvl == 0) {
                    n.text = (std::string("Ending ") + std::string(1, 'A' + idCounter)).c_str();
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

                tree.push_back(n);
                levelIndex[lvl].push_back(idCounter);
                idCounter++;
            }
        }
        NODE_COUNT = tree.size();
    }
}
//---------------------------------------------------------------------------
    // Init
    //---------------------------------------------------------------------------
void InitGameMapScreen(void)
{
    Screen_GameMap::finishScreen = 0;
    Screen_GameMap::currentNode = Screen_GameMap::NODE_COUNT - 1;  // 玩家從最底部 (0 = 最後結局) 開始
    Screen_GameMap::RandGameMap(5, 4, 3);
}

//---------------------------------------------------------------------------
// Update
//---------------------------------------------------------------------------
void UpdateGameMapScreen(void)
{
    // 玩家按空白鍵 => 跳過 (直接到標題)
    if (IsKeyPressed(KEY_SPACE)) {
        Screen_GameMap::finishScreen = 1;
        return;
    }

    // 按 1 / 2 / 3 選擇父節點
    if (IsKeyPressed(KEY_ONE) && Screen_GameMap::tree[Screen_GameMap::currentNode].parents.size() >= 1) {
        Screen_GameMap::currentNode = Screen_GameMap::tree[Screen_GameMap::currentNode].parents[0];
    }
    if (IsKeyPressed(KEY_TWO) && Screen_GameMap::tree[Screen_GameMap::currentNode].parents.size() >= 2) {
        Screen_GameMap::currentNode = Screen_GameMap::tree[Screen_GameMap::currentNode].parents[1];
    }
    if (IsKeyPressed(KEY_THREE) && Screen_GameMap::tree[Screen_GameMap::currentNode].parents.size() >= 3) {
        Screen_GameMap::currentNode = Screen_GameMap::tree[Screen_GameMap::currentNode].parents[2];
    }

    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // 遍歷目前節點的父節點，看有沒有被點到
        for (int i = 0; i < Screen_GameMap::tree[Screen_GameMap::currentNode].parents.size(); i++) {
            int parentId = Screen_GameMap::tree[Screen_GameMap::currentNode].parents[i];
            Vector2 pos = Screen_GameMap::nodePos[parentId];

            float dx = mouse.x - pos.x;
            float dy = mouse.y - pos.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < Screen_GameMap::RADIOUS) {  // 圓半徑 = 32
                Screen_GameMap::currentNode = parentId;
                return;
            }
        }
    }


    // 到頂端 Root
    if (Screen_GameMap::tree[Screen_GameMap::currentNode].parents.size() == 0 && Screen_GameMap::currentNode != 0) {
        Screen_GameMap::finishScreen = 1;
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

    for (int i = 0; i < Screen_GameMap::NODE_COUNT; i++) {
        levels[Screen_GameMap::tree[i].level].push_back(i);
        if (Screen_GameMap::tree[i].level > maxLevel) maxLevel = Screen_GameMap::tree[i].level;
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
            Screen_GameMap::nodePos[node] = { (float)x, (float)y };  // ✅ 記住節點座標

            // --- Draw edges (parent lines)
            for (int p : Screen_GameMap::tree[node].parents) {
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
            Color c = (node == Screen_GameMap::currentNode) ? RED : BLACK;
            Color fill = (node == Screen_GameMap::currentNode) ? PINK : RAYWHITE;

            DrawCircle(x, y, Screen_GameMap::RADIOUS, fill);
            DrawCircleLines(x, y, Screen_GameMap::RADIOUS, c);

            bool clickable = false;
            for (int p : Screen_GameMap::tree[Screen_GameMap::currentNode].parents)
                if (p == node) clickable = true;

            if (clickable)
                DrawCircleLines(x, y, Screen_GameMap::RADIOUS + 4, ORANGE); // highlight clickable nodes

            int textWidth = MeasureText(Screen_GameMap::tree[node].text.c_str(), 20);
            DrawText(Screen_GameMap::tree[node].text.c_str(), x - textWidth / 2, y - 10, 20, c);
        }
    }

    // Show current selection
    DrawText("Current:", 50, GetScreenHeight() - 120, 22, BLACK);
    DrawText(Screen_GameMap::tree[Screen_GameMap::currentNode].text.c_str(), 140, GetScreenHeight() - 120, 24, BLUE);
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
    return Screen_GameMap::finishScreen;
}