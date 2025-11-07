#pragma once
#include "raylib.h"
#include <string>

class GameObject {
protected:
    Vector2 position;
    Vector2 scale;
    float rotation;
    bool active;
    std::string name;

public:
    GameObject(std::string name = "GameObject")
        : position({ 0, 0 }), scale({ 1, 1 }), rotation(0), active(true), name(name) {
    }

    virtual ~GameObject() = default;

    // --- 主要生命周期函數 ---
    virtual void Start() {}
    virtual void Update(float deltaTime) {}
    virtual void Draw() {}
    virtual void OnDestroy() {}

    // --- Getters / Setters ---
    void SetPosition(Vector2 pos) { position = pos; }
    Vector2 GetPosition() const { return position; }

    void SetScale(Vector2 s) { scale = s; }
    Vector2 GetScale() const { return scale; }

    void SetRotation(float r) { rotation = r; }
    float GetRotation() const { return rotation; }

    void SetActive(bool a) { active = a; }
    bool IsActive() const { return active; }

    std::string GetName() const { return name; }

    // --- 常用輔助功能 ---
    //virtual void Translate(Vector2 delta) { position = Vector2Add(position, delta); }
};
