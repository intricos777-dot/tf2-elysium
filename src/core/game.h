#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "data/data_registries.h"
#include "combat/combat.h"

namespace tf2 {

struct GameState {
    bool in_menu = true;
    bool game_over = false;
    uint32_t wave = 0;
    uint32_t total_kills = 0;
    uint32_t credits_earned = 0;
    uint32_t npc_encounters = 0;
    std::string current_map;
    std::string difficulty = "casual";
};

class Game {
public:
    Game();
    bool initialize();
    void shutdown();

    void show_menu();
    void choose_class();
    void choose_map();
    void run_wave();
    void visit_npc();
    void hunt_coconut();
    void shop();

    GameState& state() { return m_state; }
    DataRegistry& data() { return m_data; }
    PlayerState& player() { return m_player; }
    WaveDirector& director() { return m_director; }
    NPCPopup& popups() { return m_popups; }
    CoconutTracker& coconut() { return m_coconut; }

private:
    GameState m_state;
    DataRegistry m_data;
    PlayerState m_player;
    WaveDirector m_director;
    NPCPopup m_popups;
    CoconutTracker m_coconut;

    void draw_hud();
    void handle_command(const std::string& cmd);
    void cleanup();
};

} // namespace tf2
