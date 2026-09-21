#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <set>
#include "data/data_registries.h"

namespace tf2 {

struct PlayerState {
    std::string class_name = "Scout";
    float hp = 125;
    float max_hp = 125;
    std::string weapon_name = "Scattergun";
    uint32_t ammo = 6;
    uint32_t max_ammo = 32;
    bool uber_ready = false;
    float credits = 0;
};

enum class CombatState { idle, player_turn, enemy_turn, victory, defeat };

struct RobotState {
    std::string name;
    uint32_t hp;
    uint32_t max_hp;
    uint32_t damage;
    float accuracy;
    std::string role;
    bool has_uber = false;
    std::string weapon;
};

struct CombatRound {
    std::string attacker;
    std::string target;
    uint32_t damage = 0;
    std::string action;
    bool crit = false;
};

class CombatEngine {
public:
    CombatEngine(const DataRegistry& data, uint32_t seed);

    void engage(const RobotState& robot, PlayerState& player);
    std::vector<std::string> attack();
    std::vector<std::string> use_ability();
    std::vector<std::string> reload();
    std::vector<std::string> flee();

    bool is_over() const { return m_state == CombatState::victory || m_state == CombatState::defeat; }
    bool player_won() const { return m_state == CombatState::victory; }
    uint32_t enemy_hp() const { return m_enemy_hp; }
    uint32_t enemy_max_hp() const { return m_enemy_max_hp; }
    CombatState state() const { return m_state; }

private:
    const DataRegistry& m_data;
    CombatState m_state = CombatState::idle;
    PlayerState* m_player = nullptr;
    RobotState m_robot;
    uint32_t m_enemy_hp = 0;
    uint32_t m_enemy_max_hp = 0;
    uint32_t m_ammo_in_mag = 0;
    uint32_t m_mag_size = 6;
    uint32_t m_ammo_reserve = 32;
    uint32_t m_ubercharge = 0;
    std::string m_last_action;

    void enemy_attack();
    void check_victory();
};

// Mann vs Machine wave director
class WaveDirector {
public:
    WaveDirector(const DataRegistry& data);

    void start_wave(uint32_t wave);
    std::vector<RobotState> spawn_wave(uint32_t wave, uint32_t seed);
    bool wave_cleared() const { return m_alive == 0; }
    void kill_one() { m_alive--; }
    uint32_t wave_number() const { return m_wave; }
    uint32_t credits_reward() const { return m_alive == 0 ? 400u * m_wave : 0u; }

private:
    const DataRegistry& m_data;
    uint32_t m_wave = 0;
    uint32_t m_alive = 0;
    uint32_t m_total = 0;
};

// NPC Popup manager
class NPCPopup {
public:
    NPCPopup(const std::vector<NPCDef>& npcs, uint32_t seed);

    // Returns a line if an NPC pops up this tick
    std::string try_popup();
    std::string random_quote(const NPCDef& npc) const;

private:
    const std::vector<NPCDef>& m_npcs;
    float m_timer = 0;
    float m_next = 30.0f;
    uint32_t m_rng;
};

// Easter egg coconut tracker
class CoconutTracker {
public:
    bool found_on(const std::string& map);
    bool carrying() const { return m_carrying; }
    void pickup() { m_carrying = true; m_found.insert("current"); }
    void throw_coconut();
    uint32_t total_found() const { return m_found.size(); }

private:
    bool m_carrying = false;
    std::set<std::string> m_found;
};

} // namespace tf2
