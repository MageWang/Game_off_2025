#include "raylib.h"
#include "screens.h"
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <queue>
#include <string>

#define GRID_WIDTH 8
#define GRID_HEIGHT 16
#define CELL_SIZE 45
#define MAX_UNITS 64

typedef enum { TEAM_RED, TEAM_BLUE } Team;
typedef enum { STATE_PLACING, STATE_BATTLE } GameState;

typedef struct Unit {
    int x, y;
    int hp;
    int attack;
    Team team;
    bool alive;
} Unit;

typedef struct UnitType {
    const char* name;
    int hp;
    int attack;
    int count;
} UnitType;

static Unit units[MAX_UNITS];
static int unitCount = 0;
static int grid[GRID_HEIGHT][GRID_WIDTH] = { 0 };

static int framesCounter = 0;
static int finishScreen = 0;
static float turnTimer = 0.0f;
static const float TURN_INTERVAL = 1.0f;
static bool gameOver = false;
static Team winner;

static int boardOffsetX = 0;
static int boardOffsetY = 0;
static const int INFO_PANEL_WIDTH = 220;

static GameState state = STATE_PLACING;
static int selectedTypeIndex = -1;

// 玩家可放的單位種類
static UnitType playerTypes[] = {
    {"Warrior", 12, 3, 3},
    {"Archer",  8,  4, 2},
    {"Knight", 16,  2, 2}
};
static const int playerTypeCount = sizeof(playerTypes) / sizeof(playerTypes[0]);

//-------------------------------------------------------------
static bool IsOccupied(int x, int y)
{
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return true;
    if (grid[y][x] == 1) return true;
    for (int i = 0; i < unitCount; i++)
        if (units[i].alive && units[i].x == x && units[i].y == y)
            return true;
    return false;
}

static int Distance(Unit* a, Unit* b)
{
    return abs(a->x - b->x) + abs(a->y - b->y);
}

//-------------------------------------------------------------
//bool UnitSort(const Unit& a, const Unit& b)
//{
//    if (a.team != b.team) return a.team == TEAM_BLUE;
//    if (a.team == TEAM_BLUE) return a.y < b.y;
//    return a.y > b.y;
//}

static Unit* FindNearestEnemy(Unit* u)
{
    Unit* target = NULL;
    int minDist = 999;
    for (int i = 0; i < unitCount; i++) {
        Unit* e = &units[i];
        if (!e->alive || e->team == u->team) continue;
        int d = Distance(u, e);
        if (d < minDist) { minDist = d; target = e; }
    }
    return target;
}

static void MoveTowards(Unit* u, Unit* target)
{
    int dx = target->x - u->x;
    int dy = target->y - u->y;
    int stepX = (dx != 0) ? dx / abs(dx) : 0;
    int stepY = (dy != 0) ? dy / abs(dy) : 0;
    int nx = u->x + stepX;
    int ny = u->y + stepY;
    if (!IsOccupied(nx, ny)) { u->x = nx; u->y = ny; }
}

//-------------------------------------------------------------
void InitSetupScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    gameOver = false;
    unitCount = 0;
    state = STATE_PLACING;
    selectedTypeIndex = -1;

    boardOffsetX = (GetScreenWidth() - INFO_PANEL_WIDTH * 2 - GRID_WIDTH * CELL_SIZE) / 2 + INFO_PANEL_WIDTH;
    boardOffsetY = (GetScreenHeight() - GRID_HEIGHT * CELL_SIZE) / 2;

    // 生成敵方（紅隊）
    for (int i = 0; i < 5; i++) {
        int rx = GetRandomValue(0, GRID_WIDTH - 1);
        int ry = GetRandomValue(0, 3);
        if (!IsOccupied(rx, ry))
            units[unitCount++] = { rx, ry, 10, 3, TEAM_RED, true };
    }
}

//-------------------------------------------------------------
void UpdateSetupScreen(void)
{
    if (gameOver) {
        if (IsKeyPressed(KEY_ENTER)) finishScreen = 1;
        return;
    }

    // === 放置階段 ===
    if (state == STATE_PLACING)
    {
        Vector2 m = GetMousePosition();

        // 點選右方單位種類
        for (int i = 0; i < playerTypeCount; i++) {
            Rectangle r = {
                GetScreenWidth() - INFO_PANEL_WIDTH + 20,
                120 + i * 80,
                INFO_PANEL_WIDTH - 40,
                60
            };
            if (CheckCollisionPointRec(m, r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                selectedTypeIndex = i;
        }

        // 點棋盤放置單位
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int gx = (m.x - boardOffsetX) / CELL_SIZE;
            int gy = (m.y - boardOffsetY) / CELL_SIZE;
            if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
                if (!IsOccupied(gx, gy) && selectedTypeIndex >= 0) {
                    UnitType* t = &playerTypes[selectedTypeIndex];
                    if (t->count > 0) {
                        units[unitCount++] = { gx, gy, t->hp, t->attack, TEAM_BLUE, true };
                        t->count--;
                    }
                }
            }
        }

        // 所有單位放完才能開始
        bool allEmpty = true;
        for (int i = 0; i < playerTypeCount; i++)
            if (playerTypes[i].count > 0) allEmpty = false;

        if (allEmpty && IsKeyPressed(KEY_SPACE))
            state = STATE_BATTLE;

        return;
    }

    // === 戰鬥階段 ===
    if (state == STATE_BATTLE)
    {
        turnTimer += GetFrameTime();
        if (turnTimer >= TURN_INTERVAL) {
            turnTimer = 0.0f;
            // std::sort(units, units + unitCount, UnitSort);
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
}

//-------------------------------------------------------------
void DrawPlacementUI(void)
{
    DrawText("Place Your Units", GetScreenWidth() / 2 - 120, 20, 24, DARKBLUE);
    DrawText("Click board to place", GetScreenWidth() / 2 - 100, 50, 20, GRAY);
    DrawText("Press SPACE to start", GetScreenWidth() / 2 - 100, 70, 20, GRAY);
}

//-------------------------------------------------------------
void DrawSetupScreen(void)
{
    ClearBackground(RAYWHITE);

    // 左紅右藍
    DrawRectangle(0, 0, INFO_PANEL_WIDTH, GetScreenHeight(), { 220, 100, 100, 255 });
    DrawRectangle(GetScreenWidth() - INFO_PANEL_WIDTH, 0, INFO_PANEL_WIDTH, GetScreenHeight(), { 100, 100, 220, 255 });

    // 棋盤
    for (int y = 0; y < GRID_HEIGHT; y++)
        for (int x = 0; x < GRID_WIDTH; x++) {
            Rectangle cell = { boardOffsetX + x * CELL_SIZE, boardOffsetY + y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            DrawRectangleLines(cell.x, cell.y, cell.width, cell.height, DARKGRAY);
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

    // === 右側面板（可選單位） ===
    DrawText("BLUE UNITS", GetScreenWidth() - INFO_PANEL_WIDTH + 20, 40, 24, WHITE);
    for (int i = 0; i < playerTypeCount; i++) {
        Rectangle r = {
            GetScreenWidth() - INFO_PANEL_WIDTH + 20,
            120 + i * 80,
            INFO_PANEL_WIDTH - 40,
            60
        };

        Color boxColor;
        if (selectedTypeIndex == i)
        {
            boxColor = { 60, 120, 255, 255 };
        }
        else
        {
            boxColor = { 80, 80, 160, 255 };
        }
        DrawRectangleRec(r, boxColor);
        DrawRectangleLinesEx(r, 2, WHITE);
        DrawText(playerTypes[i].name, r.x + 10, r.y + 10, 20, WHITE);
        DrawText(TextFormat("HP:%d ATK:%d", playerTypes[i].hp, playerTypes[i].attack), r.x + 10, r.y + 30, 16, WHITE);
        DrawText(TextFormat("x%d", playerTypes[i].count), r.x + 140, r.y + 30, 18, YELLOW);
    }

    if (state == STATE_PLACING) DrawPlacementUI();

    if (gameOver) {
        const char* text = (winner == TEAM_RED) ? "RED WINS!" : "BLUE WINS!";
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
        DrawText(text, GetScreenWidth() / 2 - MeasureText(text, 60) / 2, GetScreenHeight() / 2 - 40, 60, YELLOW);
        DrawText("Press ENTER to return", GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 + 40, 20, WHITE);
    }
}

//-------------------------------------------------------------
void UnloadSetupScreen(void) { }
int FinishSetupScreen(void) { return finishScreen; }
