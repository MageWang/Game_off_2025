#pragma once
#include <string>
#include <vector>
namespace GameData{
    struct Unit {
        int id;
        std::string name;
        std::string desc;
        float hp;
        float atk;
        float spd;
        float range;
        float hp_lv;
        float atk_lv;
        float spd_lv;
        float range_lv;
        /* UNKNOWN_TYPE */;
    };

    const std::vector<Unit> AllUnits = {
        Unit{1, "footman", "basic", 10.0f, 2.0f, 1.0f, 1.0f, 2.5f, 1.0f, 0.0f, 0.0f, },
        Unit{2, "archer", "range", 5.0f, 1.0f, 1.0f, 5.0f, 0.0f, 0.5f, 0.0f, 0.2f, },
        Unit{3, "knight", "move faster", 15.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, },
        Unit{4, "spearman", "counter kinght", 10.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 0.0f, 0.0f, }
    };
}