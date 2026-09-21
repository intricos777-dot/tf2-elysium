#include "combat.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>

namespace tf2 {

CombatEngine::CombatEngine(const DataRegistry& data, uint32_t seed)
    : m_data(data) { std::srand(seed); }

void CombatEngine::engage(const RobotState& robot, PlayerState& player) {
    m_robot = robot;
    m_player = &player;
    m_enemy_hp = robot.hp;
    m_enemy_max_hp = robot.max_hp;
    m_ammo_in_mag = 6;
    m_ammo_reserve = player.max_ammo;
    m_ubercharge = 0;
    m_state = CombatState::player_turn;
    std::printf("[Combat] Engaged %s (hp=%u)\n", robot.name.c_str(), m_enemy_hp);
}

std::vector<std::string> CombatEngine::attack() {
    std::vector<std::string> log;
    if (m_state != CombatState::player_turn || !m_player) return log;

    const WeaponDef* wep = m_data.find_weapon(m_player->weapon_name);
    if (!wep && !m_data.weapons().empty()) wep = &m_data.weapons()[0];
    if (!wep) { log.push_back("No weapon!"); return log; }

    if (m_ammo_in_mag == 0) { log.push_back("Click! Reload needed."); return log; }

    uint32_t dmg = wep->damage;
    if (wep->class_name == "Heavy") dmg = 10 + std::rand() % 10;
    bool crit = (std::rand() % 100) < 10;
    if (crit) dmg = dmg * 3;

    m_enemy_hp = std::max(0u, m_enemy_hp - dmg);
    m_ammo_in_mag--;

    log.push_back("You fire the " + wep->name + " at " + m_robot.name + " for " + std::to_string(dmg) + " damage.");
    if (crit) log.push_back("CRITICAL HIT!");
    log.push_back("Ammo: " + std::to_string(m_ammo_in_mag) + "/" + std::to_string(m_mag_size));

    check_victory();
    if (m_state != CombatState::victory) {
        m_state = CombatState::enemy_turn;
        enemy_attack();
    }
    return log;
}

std::vector<std::string> CombatEngine::use_ability() {
    std::vector<std::string> log;
    if (m_state != CombatState::player_turn) return log;
    if (m_ubercharge >= 100 && m_player) {
        m_ubercharge = 0;
        log.push_back("\x1b[38;5;200mUBERCHARGE ACTIVATED!\x1b[0m You are invulnerable for 8 seconds!");
        // Skip enemy turn for a bit
        m_state = CombatState::player_turn;
    } else {
        m_ubercharge = std::min(100u, m_ubercharge + 25);
        log.push_back("Taunt! Ubercharge: " + std::to_string(m_ubercharge) + "%");
    }
    return log;
}

std::vector<std::string> CombatEngine::reload() {
    uint32_t need = m_mag_size - m_ammo_in_mag;
    uint32_t take = std::min(need, m_ammo_reserve);
    m_ammo_in_mag += take;
    m_ammo_reserve -= take;
    return {"Reloaded. " + std::to_string(m_ammo_in_mag) + "/" + std::to_string(m_mag_size)};
}

std::vector<std::string> CombatEngine::flee() {
    if ((std::rand() % 100) < 30) {
        m_state = CombatState::idle;
        return {"You slip away into cover."};
    }
    enemy_attack();
    return {"Can't flee!"};
}

void CombatEngine::enemy_attack() {
    if (m_enemy_hp == 0 || m_robot.has_uber) return;
    uint32_t dmg = m_robot.damage;
    if ((std::rand() % 100) / 100.0f > m_robot.accuracy) {
        m_state = CombatState::player_turn;
        return;
    }
    dmg = std::max(1u, dmg);
    m_player->hp = std::max(0.0f, m_player->hp - dmg);
    std::printf("[Combat] %s hits you for %u\n", m_robot.name.c_str(), dmg);
    if (m_player->hp <= 0) {
        m_state = CombatState::defeat;
    } else {
        m_state = CombatState::player_turn;
    }
}

void CombatEngine::check_victory() {
    if (m_enemy_hp == 0) {
        m_state = CombatState::victory;
        std::printf("[Combat] %s destroyed.\n", m_robot.name.c_str());
    }
}

// ---- WaveDirector -------------------------------------------------------

WaveDirector::WaveDirector(const DataRegistry& data) : m_data(data) {}

void WaveDirector::start_wave(uint32_t wave) {
    m_wave = wave;
}

std::vector<RobotState> WaveDirector::spawn_wave(uint32_t wave, uint32_t seed) {
    std::vector<RobotState> out;
    std::srand(seed);

    auto robots = m_data.robots();
    if (robots.empty()) return out;

    m_total = 5 + wave * 3;
    m_alive = m_total;

    for (uint32_t i = 0; i < m_total; ++i) {
        const RobotDef* rd = &robots[std::rand() % robots.size()];
        RobotState r;
        r.name = rd->name;
        r.max_hp = rd->health;
        r.hp = rd->health;
        r.damage = 10 + std::rand() % 20;
        r.accuracy = 0.6f + (std::rand() % 40) / 100.0f;
        r.role = rd->role;
        r.weapon = "bot_weapon";
        out.push_back(std::move(r));
    }
    return out;
}

// ---- NPCPopup -----------------------------------------------------------

NPCPopup::NPCPopup(const std::vector<NPCDef>& npcs, uint32_t seed) : m_npcs(npcs), m_rng(seed) {
    m_next = 20.0f + (std::rand() % 60);
}

std::string NPCPopup::try_popup() {
    if (m_npcs.empty()) return "";
    m_timer += 1.0f;
    if (m_timer < m_next) return "";
    m_timer = 0;
    m_next = 20.0f + (std::rand() % 90);
    auto& npc = m_npcs[std::rand() % m_npcs.size()];
    std::string line = "[" + npc.name + "] \"" + random_quote(npc) + "\"";
    if (npc.voice_energy == "booming") line = "\x1b[1m" + line + "\x1b[0m";
    else if (npc.voice_energy == "cold") line = "\x1b[38;5;39m" + line + "\x1b[0m";
    else if (npc.voice_energy == "theatrical") line = "\x1b[38;5;213m" + line + "\x1b[0m";
    return line;
}

std::string NPCPopup::random_quote(const NPCDef& npc) const {
    if (npc.quotes.empty()) return "...";
    return npc.quotes[std::rand() % npc.quotes.size()];
}

// ---- CoconutTracker -----------------------------------------------------

bool CoconutTracker::found_on(const std::string& map) {
    return m_found.find(map) == m_found.end();
}

void CoconutTracker::throw_coconut() {
    if (!m_carrying) return;
    m_carrying = false;
    std::printf("  \x1b[38;5;130mYou hurl the coconut. It disappears.\x1b[0m\n");
}

} // namespace tf2
