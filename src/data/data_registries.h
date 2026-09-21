#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace tf2 {

struct ClassDef {
    std::string name;
    std::string role;
    uint32_t hp = 125;
    uint32_t speed = 100;
};

struct WeaponDef {
    std::string name;
    std::string class_name;
    uint32_t damage = 60;
    float fire_rate = 1.0f;
    bool splash = false;
    bool heal = false;
    bool charge = false;
    bool backstab = false;
    uint32_t heal_per_sec = 0;
    bool ubercharge = false;
};

struct GameModeDef {
    std::string name;
    uint32_t teams = 2;
    uint32_t players_per_team = 12;
};

struct RobotDef {
    std::string name;
    std::string role;
    uint32_t health = 100;
    float speed = 1.0f;
    std::string note;
};

struct NPCDef {
    std::string name;
    std::string type;
    std::vector<std::string> quotes;
    std::string voice_energy;
    float voice_pitch = 1.0f;
    bool spawns_randomly = true;
    bool wanders = false;
    std::string reacts_to_coconut;
};

struct EasterEgg {
    std::string name;
    std::string type;
    std::string hint;
    std::vector<std::string> interactions;
};

struct MapDef {
    std::string name;
    std::string mode;
    std::string layout;
};

struct Difficulty {
    std::string profile;
    float player_health_mult = 1.0f;
    float enemy_health_mult = 1.0f;
    float enemy_damage_mult = 1.0f;
    float enemy_accuracy = 0.7f;
    float ammo_mult = 1.0f;
    float credits_mult = 1.0f;
};

class DataRegistry {
public:
    bool load_all(const std::string& content_dir);
    const std::vector<ClassDef>& classes() const { return m_classes; }
    const std::vector<WeaponDef>& weapons() const { return m_weapons; }
    const std::vector<GameModeDef>& modes() const { return m_modes; }
    const std::vector<RobotDef>& robots() const { return m_robots; }
    const std::vector<NPCDef>& npcs() const { return m_npcs; }
    const std::vector<EasterEgg>& eggs() const { return m_eggs; }
    const std::vector<MapDef>& maps() const { return m_maps; }
    const Difficulty& diff() const { return m_diff; }
    void set_difficulty(const std::string& name);

    const ClassDef* find_class(const std::string& name) const;
    const WeaponDef* find_weapon(const std::string& name) const;

private:
    std::vector<ClassDef> m_classes;
    std::vector<WeaponDef> m_weapons;
    std::vector<GameModeDef> m_modes;
    std::vector<RobotDef> m_robots;
    std::vector<NPCDef> m_npcs;
    std::vector<EasterEgg> m_eggs;
    std::vector<MapDef> m_maps;
    Difficulty m_diff;
};

} // namespace tf2
