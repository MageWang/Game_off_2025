#include "raylib.h"
#include "screens.h"
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <queue>

#define GRID_WIDTH 8
#define GRID_HEIGHT 16
#define CELL_SIZE 45
#define MAX_UNITS 32

typedef enum { TEAM_RED, TEAM_BLUE } Team;

typedef struct Unit {
    int x, y;
    int hp;
    int attack;
    Team team;
    bool alive;
} Unit;

typedef struct GamePlayNode {
    int x, y;
} GamePlayNode;

static Unit units[MAX_UNITS];
static int unitCount = 0;
static int grid[GRID_HEIGHT][GRID_WIDTH] =
{
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {1,1,1,0,1,1,1,1},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};
static int framesCounter = 0;
static int finishScreen = 0;
static float turnTimer = 0.0f;
static const float TURN_INTERVAL = 1.0f;
static bool gameOver = false;
static Team winner;

static int boardOffsetX = 0;
static int boardOffsetY = 0;

static const int INFO_PANEL_WIDTH = 200;

//-------------------------------------------------------------
// 輔助函數
//-------------------------------------------------------------
static int Distance(Unit* a, Unit* b)
{
    return abs(a->x - b->x) + abs(a->y - b->y);
}

static int DistanceWithBFS(Unit* a, Unit* b)
{
    struct BFSNode { int x, y, d; };

    static bool visited[GRID_HEIGHT][GRID_WIDTH];

    // 重置 visited
    for (int y = 0; y < GRID_HEIGHT; y++)
        for (int x = 0; x < GRID_WIDTH; x++)
            visited[y][x] = false;

    auto IsBlocked = [&](int x, int y)
    {
        if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
            return true;

        if (grid[y][x] == 1) return true; // 障礙物

        return false;
    };

    std::queue<BFSNode> q;
    q.push({ a->x, a->y, 0 });
    visited[a->y][a->x] = true;

    int dirs[4][2] = { {0,1},{0,-1},{1,0},{-1,0} };

    while (!q.empty())
    {
        BFSNode cur = q.front(); q.pop();

        if (cur.x == b->x && cur.y == b->y)
            return cur.d;

        for (auto& d : dirs)
        {
            int nx = cur.x + d[0];
            int ny = cur.y + d[1];

            if (!IsBlocked(nx, ny) && !visited[ny][nx])
            {
                visited[ny][nx] = true;
                q.push({ nx, ny, cur.d + 1 });
            }
        }
    }

    return -1; // 找不到路
}

static bool IsOccupied(int x, int y)
{
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return true;
    if (grid[y][x] == 1) return true;
    for (int i = 0; i < unitCount; i++)
    {
        if (units[i].alive && units[i].x == x && units[i].y == y)
            return true;
    }
    return false;
}

static void GenerateRandomGrid()
{
    Unit t1;
    t1.x = 0;
    t1.y = 0;

    Unit t2;
    t2.x = GRID_WIDTH - 1;
    t2.y = GRID_HEIGHT - 1;

    do
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            for (int x = 0; x < GRID_WIDTH; x++)
            {
                // 避免出生區域被障礙物封死
                bool isSpawnZoneTop = (y < 3);
                bool isSpawnZoneBottom = (y > GRID_HEIGHT - 4);

                if (isSpawnZoneTop || isSpawnZoneBottom)
                {
                    grid[y][x] = 0;
                    continue;
                }

                // 隨機生成障礙物 (20% 機率)
                int r = GetRandomValue(0, 100);
                grid[y][x] = (r < 20) ? 1 : 0;
            }
        }
    } 
    while (DistanceWithBFS(&t1, &t2) < 0);
}

bool UnitSort(const Unit& a, const Unit& b)
{
    // 1. 藍隊在紅隊前
    if (a.team != b.team)
        return a.team == TEAM_BLUE;

    // 2. 同是藍隊 => y 小的排前
    if (a.team == TEAM_BLUE)
        return a.y < b.y;

    // 3. 同是紅隊 => y 大的排前
    return a.y > b.y;
}

static void GenerateRandomUnits()
{
    unitCount = 0;

    for (int i = 0; i < MAX_UNITS / 2; i++)
    {
        int rx, ry;
        int bx, by;

        // ---- 產生紅隊 ----
        do {
            rx = rand() % GRID_WIDTH;
            ry = rand() % 4;  // 上方4列
        } while (grid[ry][rx] == 1 || IsOccupied(rx, ry)); // 避開障礙 & 避免重複

        units[unitCount++] = { rx, ry, 10, 3, TEAM_RED, true };

        // ---- 產生藍隊 ----
        do {
            bx = rand() % GRID_WIDTH;
            by = GRID_HEIGHT - 1 - rand() % 4; // 下方4列
        } while (grid[by][bx] == 1 || IsOccupied(bx, by)); // 避開障礙 & 避免重複

        units[unitCount++] = { bx, by, 10, 3, TEAM_BLUE, true };
    }
    std::sort(units, units + unitCount, UnitSort);
}

static Unit* FindNearestEnemy(Unit* u)
{
    Unit* target = NULL;
    int minDist = 999;
    for (int i = 0; i < unitCount; i++)
    {
        Unit* e = &units[i];
        if (!e->alive || e->team == u->team) continue;
        int d = DistanceWithBFS(u, e);
        if (d < minDist)
        {
            minDist = d;
            target = e;
        }
    }
    return target;
}

static void MoveTowards(Unit* u, Unit* target)
{
    if (u->x == target->x && u->y == target->y) return;

    static std::vector<std::vector<bool>> visited(
        GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));
    static std::vector<std::vector<GamePlayNode>> parent(
        GRID_HEIGHT, std::vector<GamePlayNode>(GRID_WIDTH));

    // reset
    for (auto& row : visited) std::fill(row.begin(), row.end(), false);

    for (auto& row : parent)
        for (auto& p : row)
            p = { -1, -1 };

    auto IsBlocked = [](int x, int y, Unit* self, Unit* dest)
    {
        if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return true;
        if (grid[y][x] == 1) return true;
        for (int i = 0; i < unitCount; i++)
            if (&units[i] != dest && &units[i] != self && units[i].alive && units[i].x == x && units[i].y == y)
                return true;
        return false;
    };

    std::queue<GamePlayNode> q;
    q.push({ u->x, u->y });
    visited[u->y][u->x] = true;

    int dirs[4][2] = { {0,1},{0,-1},{1,0},{-1,0} };
    bool found = false;

    int visitOrder[GRID_HEIGHT][GRID_WIDTH];
    for (int y = 0; y < GRID_HEIGHT; y++)
        for (int x = 0; x < GRID_WIDTH; x++)
            visitOrder[y][x] = -1;

    int step = 0;
    while (!q.empty())
    {
        GamePlayNode cur = q.front(); q.pop();
        if (cur.x == target->x && cur.y == target->y) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++)
        {
            int nx = cur.x + dirs[i][0];
            int ny = cur.y + dirs[i][1];

            if (!IsBlocked(nx, ny, u, target) && !visited[ny][nx])
            {
                visited[ny][nx] = true;
                parent[ny][nx] = cur;
                q.push({ nx, ny });
                visitOrder[ny][nx] = step++;
            }
        }
    }

    // debug map
    /*printf("=== BFS Visit Map ===\n");
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            if (x == u->x && y == u->y) printf("  S ");
            else if (x == target->x && y == target->y) printf("  E ");
            else if (visitOrder[y][x] == -1) printf("  - ");
            else printf("%3d ", visitOrder[y][x]);
        }
        printf("\n");
    }
    printf("====================\n");*/

    // ✅ 找到路：回溯移動
    if (found)
    {
        std::vector<GamePlayNode> path;
        GamePlayNode cur = { target->x, target->y };
        while (!(cur.x == u->x && cur.y == u->y))
        {
            path.push_back(cur);
            cur = parent[cur.y][cur.x];
        }

        if (path.size() >= 1) {
            u->x = path[path.size() - 1].x;
            u->y = path[path.size() - 1].y;
        }
        return;
    }

    // ❌ 找不到路：走向更接近敵人的一步
    int bestX = u->x, bestY = u->y;
    int bestDist = DistanceWithBFS(u, target);

    for (int i = 0; i < 4; i++)
    {
        int nx = u->x + dirs[i][0];
        int ny = u->y + dirs[i][1];
        Unit t;
        if (!IsBlocked(nx, ny, u, target))
        {
            t.x = nx;
            t.y = ny;
            int d = DistanceWithBFS(&t, target);
            if (d < bestDist)
            {
                bestDist = d;
                bestX = nx;
                bestY = ny;
            }
        }
    }

    // move if found better spot
    if (bestX != u->x || bestY != u->y)
    {
        u->x = bestX;
        u->y = bestY;
    }
}

//-------------------------------------------------------------
// 初始化
//-------------------------------------------------------------
void InitGameplayScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    gameOver = false;
    unitCount = 0;

    // 棋盤置中
    boardOffsetX = (GetScreenWidth() - INFO_PANEL_WIDTH * 2 - GRID_WIDTH * CELL_SIZE) / 2 + INFO_PANEL_WIDTH;
    boardOffsetY = (GetScreenHeight() - GRID_HEIGHT * CELL_SIZE) / 2;
    GenerateRandomGrid();
    GenerateRandomUnits();
    /*units[unitCount++] = { 0, GRID_HEIGHT - 1, 10, 3, TEAM_BLUE, true };
    units[unitCount++] = { 0, GRID_HEIGHT - 2, 10, 3, TEAM_BLUE, true };
    units[unitCount++] = { 0, 0, 10, 3, TEAM_RED, true };*/
}

//-------------------------------------------------------------
// 更新邏輯
//-------------------------------------------------------------
void UpdateGameplayScreen(void)
{
    if (gameOver) {
        if (IsKeyPressed(KEY_ENTER)) finishScreen = 1;
        return;
    }

    turnTimer += GetFrameTime();
    if (turnTimer >= TURN_INTERVAL) {
        turnTimer = 0.0f;
        std::sort(units, units + unitCount, UnitSort);
        for (int i = 0; i < unitCount; i++) {
            Unit* u = &units[i];
            if (!u->alive) continue;

            Unit* enemy = FindNearestEnemy(u);
            if (!enemy) continue;

            int dist = Distance(u, enemy);
            if (dist == 1) {
                enemy->hp -= u->attack;
                if (enemy->hp <= 0) enemy->alive = false;
            }
            else {
                MoveTowards(u, enemy);
            }
        }

        bool redAlive = false, blueAlive = false;
        for (int i = 0; i < unitCount; i++) {
            if (units[i].alive) {
                if (units[i].team == TEAM_RED) redAlive = true;
                else blueAlive = true;
            }
        }
        if (!redAlive || !blueAlive) {
            gameOver = true;
            winner = redAlive ? TEAM_RED : TEAM_BLUE;
        }
    }
}

//-------------------------------------------------------------
// 統計資料
//-------------------------------------------------------------
static void GetTeamStats(Team team, int* aliveCount, int* totalHP)
{
    *aliveCount = 0;
    *totalHP = 0;
    for (int i = 0; i < unitCount; i++) {
        Unit* u = &units[i];
        if (u->alive && u->team == team) {
            (*aliveCount)++;
            (*totalHP) += u->hp;
        }
    }
}

//-------------------------------------------------------------
// 繪製邏輯
//-------------------------------------------------------------
void DrawGameplayScreen(void)
{
    ClearBackground(RAYWHITE);

    // 左右資訊欄背景
    DrawRectangle(0, 0, INFO_PANEL_WIDTH, GetScreenHeight(), { 220, 100, 100, 255 }); // Red panel
    DrawRectangle(GetScreenWidth() - INFO_PANEL_WIDTH, 0, INFO_PANEL_WIDTH, GetScreenHeight(), { 100, 100, 220, 255 }); // Blue panel

    // 棋盤
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            Rectangle cell = { boardOffsetX + x * CELL_SIZE, boardOffsetY + y * CELL_SIZE, CELL_SIZE, CELL_SIZE };

            // 格線
            DrawRectangleLines(cell.x, cell.y, cell.width, cell.height, DARKGRAY);

            // 如果是障礙物，畫黑色方塊
            if (grid[y][x] == 1)
            {
                DrawRectangle(cell.x, cell.y, cell.width, cell.height, BLACK);
            }
        }
    }

    // 單位
    for (int i = 0; i < unitCount; i++) {
        Unit* u = &units[i];
        if (!u->alive) continue;

        Color color = (u->team == TEAM_RED) ? RED : BLUE;
        int cx = boardOffsetX + u->x * CELL_SIZE + CELL_SIZE / 2;
        int cy = boardOffsetY + u->y * CELL_SIZE + CELL_SIZE / 2;
        DrawCircle(cx, cy, 10, color);
        DrawText(TextFormat("%d", u->hp), cx - 8, cy - 8, 14, WHITE);
    }

    // 資訊欄
    int redAlive, redHP, blueAlive, blueHP;
    GetTeamStats(TEAM_RED, &redAlive, &redHP);
    GetTeamStats(TEAM_BLUE, &blueAlive, &blueHP);

    DrawText("RED TEAM", 20, 40, 30, WHITE);
    DrawText(TextFormat("Alive: %d", redAlive), 20, 90, 20, WHITE);
    DrawText(TextFormat("Total HP: %d", redHP), 20, 120, 20, WHITE);

    DrawText("BLUE TEAM", GetScreenWidth() - INFO_PANEL_WIDTH + 20, 40, 30, WHITE);
    DrawText(TextFormat("Alive: %d", blueAlive), GetScreenWidth() - INFO_PANEL_WIDTH + 20, 90, 20, WHITE);
    DrawText(TextFormat("Total HP: %d", blueHP), GetScreenWidth() - INFO_PANEL_WIDTH + 20, 120, 20, WHITE);

    // 遊戲結束畫面
    if (gameOver) {
        const char* text = (winner == TEAM_RED) ? "RED WINS!" : "BLUE WINS!";
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
        DrawText(text, GetScreenWidth() / 2 - MeasureText(text, 60) / 2, GetScreenHeight() / 2 - 40, 60, YELLOW);
        DrawText("Press ENTER to return", GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 + 40, 20, WHITE);
    }
}

//-------------------------------------------------------------
void UnloadGameplayScreen(void) { }
int FinishGameplayScreen(void) { return finishScreen; }
