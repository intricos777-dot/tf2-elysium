#include "data_registries.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdio>
#include <algorithm>

namespace tf2 {

bool DataRegistry::load_all(const std::string& content_dir) {
    // Classes
    {
        std::ifstream f(content_dir + "/Data/TF2_Classes.json");
        if (f) {
            nlohmann::json j; try { f >> j; } catch (...) {}
            for (const auto& c : j) {
                ClassDef d;
                d.name = c.value("name", "");
                d.role = c.value("role", "");
                d.hp = c.value("hp", 125);
                d.speed = c.value("speed", 100);
                m_classes.push_back(std::move(d));
            }
        }
    }
    // Weapons
    {
        std::ifstream f(content_dir + "/Data/TF2_Weapons.json");
        if (f) {
            nlohmann::json j; try { f >> j; } catch (...) {}
            for (const auto& w : j) {
                WeaponDef d;
                d.name = w.value("name", "");
                d.class_name = w.value("class", "");
                d.damage = w.value("damage", 60);
                d.fire_rate = w.value("fireRate", 1.0);
                d.splash = w.value("splash", false);
                d.heal = w.value("healPerSec", 0) > 0;
                d.heal_per_sec = w.value("healPerSec", 0);
                d.charge = w.value("charge", false);
                d.backstab = w.value("backstab", false);
                d.ubercharge = w.value("ubercharge", false);
                m_weapons.push_back(std::move(d));
            }
        }
    }
    // Game Modes
    {
        std::ifstream f(content_dir + "/Data/TF2_GameModes.json");
        if (f) {
            nlohmann::json j; try { f >> j; } catch (...) {}
            for (const auto& m : j) {
                GameModeDef d;
                d.name = m.value("name", "");
                d.teams = m.value("teams", 2);
                d.players_per_team = m.value("playersPerTeam", 12);
                m_modes.push_back(std::move(d));
            }
        }
    }
    // Maps
    {
        std::ifstream f(content_dir + "/Data/TF2_Maps.json");
        if (f) {
            nlohmann::json j; try { f >> j; } catch (...) {}
            for (const auto& m : j) {
                MapDef d;
                d.name = m.value("name", "");
                d.mode = m.value("mode", "");
                d.layout = m.value("layout", "medium");
                m_maps.push_back(std::move(d));
            }
        }
    }
    // NPCs
    {
        std::ifstream f(content_dir + "/Data/TF2_NPCs.json");
        if (f) {
            nlohmann::json j; try { f >> j; } catch (...) {}
            for (const auto& n : j) {
                NPCDef d;
                d.name = n.value("name", "");
                d.type = n.value("type", "popup-npc");
                for (const auto& q : n.value("quotes", nlohmann::json::array()))
                    d.quotes.push_back(q.get<std::string>());
                d.voice_energy = n.value("voice", nlohmann::json::object()).value("energy", "calm");
                d.voice_pitch = n.value("voice", nlohmann::json::object()).value("pitch", 1.0);
                d.spawns_randomly = n.value("behavior", nlohmann::json::object()).value("spawnsRandomly", true);
                d.wanders = n.value("behavior", nlohmann::json::object()).value("wanders", false);
                d.reacts_to_coconut = n.value("behavior", nlohmann::json::object()).value("reactsToCoconut", "");
                m_npcs.push_back(std::move(d));
            }
        }
    }
    // Robots
    {
        std::ifstream f(content_dir + "/Data/TF2_Robots.json");
        if (f) {
            nlohmann::json j; try { f >> j; } catch (...) {}
            for (const auto& rb : j.value("robots", nlohmann::json::array())) {
                RobotDef d;
                d.name = rb.value("name", "");
                d.role = rb.value("role", "");
                d.health = rb.value("health", 100);
                d.speed = rb.value("speed", 1.0);
                d.note = rb.value("note", "");
                m_robots.push_back(std::move(d));
            }
        }
    }
    // Easter Eggs
    {
        std::ifstream f(content_dir + "/Data/EasterEggs.json");
        if (f) {
            nlohmann::json j; try { f >> j; } catch (...) {}
            for (const auto& e : j) {
                EasterEgg d;
                d.name = e.value("name", "");
                d.type = e.value("type", "pickup");
                d.hint = e.value("hint", "");
                for (const auto& i : e.value("interactions", nlohmann::json::array()))
                    d.interactions.push_back(i.get<std::string>());
                m_eggs.push_back(std::move(d));
            }
        }
    }
    // Difficulty
    {
        std::ifstream f(content_dir + "/Data/Difficulties.json");
        if (f) {
            nlohmann::json j; try { f >> j; } catch (...) {}
            auto prof = j.value("profiles", nlohmann::json::object()).value("casual", nlohmann::json::object());
            m_diff.profile = "casual";
            m_diff.player_health_mult = prof.value("player", nlohmann::json::object()).value("health", 100) / 100.0f;
            m_diff.enemy_health_mult = prof.value("combat", nlohmann::json::object()).value("enemyHealthMultiplier", 1.0);
            m_diff.enemy_damage_mult = prof.value("combat", nlohmann::json::object()).value("enemyDamageMultiplier", 1.0);
            m_diff.enemy_accuracy = prof.value("combat", nlohmann::json::object()).value("enemyAccuracy", 0.7);
            m_diff.ammo_mult = prof.value("economy", nlohmann::json::object()).value("ammoMultiplier", 1.0);
        }
    }

    std::printf("[TF2] Loaded: %zu classes, %zu weapons, %zu modes, %zu maps, %zu NPCs, %zu robots\n",
                m_classes.size(), m_weapons.size(), m_modes.size(), m_maps.size(), m_npcs.size(), m_robots.size());
    return !m_classes.empty() || !m_weapons.empty();
}

const ClassDef* DataRegistry::find_class(const std::string& name) const {
    for (const auto& c : m_classes) if (c.name == name) return &c;
    return nullptr;
}

const WeaponDef* DataRegistry::find_weapon(const std::string& name) const {
    for (const auto& w : m_weapons) if (w.name == name) return &w;
    return nullptr;
}

void DataRegistry::set_difficulty(const std::string& name) {
    m_diff.profile = name;
    if (name == "legend") {
        m_diff.enemy_health_mult = 1.8f;
        m_diff.enemy_damage_mult = 1.35f;
        m_diff.enemy_accuracy = 0.95f;
        m_diff.ammo_mult = 0.8f;
    }
}

} // namespace tf2
