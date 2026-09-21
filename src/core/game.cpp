#include "game.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>

namespace tf2 {

Game::Game() : m_director(m_data), m_popups(m_data.npcs(), std::time(nullptr)) {}

bool Game::initialize() {
    m_data.load_all("Content");
    m_player.max_hp = 125;
    for (auto& c : m_data.classes()) {
        if (c.name == "Scout") { m_player.max_hp = c.hp; break; }
    }
    m_player.hp = m_player.max_hp;
    m_player.max_ammo = 32;
    m_player.ammo = 6;
    m_player.weapon_name = "Scattergun";

    std::printf("[TF2] Initialized.\n");
    return true;
}

void Game::shutdown() {
    std::printf("[TF2] Shutdown.\n");
}

void Game::show_menu() {
    std::printf("\x1b[38;5;208m\x1b[1m");
    std::printf("  TEAM FORTRESS 2 ELYSIUM: MANN VS MACHINE\n");
    std::printf("  ========================================\n");
    std::printf("\x1b[0m");
    std::printf("  \x1b[2mThe robots are back. And they want your money.\x1b[0m\n\n");
    std::printf("  1) choose class\n");
    std::printf("  2) choose map\n");
    std::printf("  3) start MvM wave\n");
    std::printf("  4) visit NPC popup\n");
    std::printf("  5) hunt the coconut\n");
    std::printf("  6) upgrade / shop\n");
    std::printf("  d) set difficulty (current: %s)\n", m_state.difficulty.c_str());
    std::printf("  q) quit\n");
    draw_hud();
}

void Game::draw_hud() {
    std::printf("  \x1b[2mClass: %s | HP: %.0f/%.0f | Weapon: %s | Ammo: %u/%u | Wave: %u | Credits: %.0f\x1b[0m\n",
                m_player.class_name.c_str(), m_player.hp, m_player.max_hp,
                m_player.weapon_name.c_str(), m_player.ammo, m_player.max_ammo,
                m_state.wave, m_player.credits);
}

void Game::choose_class() {
    std::printf("\n  \x1b[38;5;208mCLASSES:\x1b[0m\n");
    for (size_t i = 0; i < m_data.classes().size(); ++i) {
        const auto& c = m_data.classes()[i];
        std::printf("  %zu) %-10s [%s] hp=%u speed=%u\n", i + 1, c.name.c_str(), c.role.c_str(), c.hp, c.speed);
    }
    std::printf("  \x1b[2m[class]\x1b[0m ");
    char buf[32];
    if (!std::fgets(buf, sizeof(buf), stdin)) return;
    int n = std::atoi(buf);
    if (n >= 1 && n <= (int)m_data.classes().size()) {
        const ClassDef& c = m_data.classes()[n - 1];
        m_player.class_name = c.name;
        m_player.max_hp = c.hp;
        m_player.hp = c.hp;
        // pick default weapon for class
        for (auto& w : m_data.weapons()) {
            if (w.class_name == c.name) { m_player.weapon_name = w.name; break; }
        }
        std::printf("  \x1b[38;5;46mSelected %s.\x1b[0m\n", c.name.c_str());
    }
}

void Game::choose_map() {
    std::printf("\n  \x1b[38;5;208mMAPS:\x1b[0m\n");
    for (size_t i = 0; i < m_data.maps().size(); ++i) {
        const auto& m = m_data.maps()[i];
        std::printf("  %zu) %-20s [mode=%s layout=%s]\n", i + 1, m.name.c_str(), m.mode.c_str(), m.layout.c_str());
    }
    std::printf("  \x1b[2m[map]\x1b[0m ");
    char buf[32];
    if (!std::fgets(buf, sizeof(buf), stdin)) return;
    int n = std::atoi(buf);
    if (n >= 1 && n <= (int)m_data.maps().size()) {
        m_state.current_map = m_data.maps()[n - 1].name;
        std::printf("  \x1b[38;5;46mSelected: %s\x1b[0m\n", m_state.current_map.c_str());
    }
}

void Game::run_wave() {
    m_state.wave++;
    m_director.start_wave(m_state.wave);
    auto robots = m_director.spawn_wave(m_state.wave, std::time(nullptr));

    std::printf("\n  \x1b[38;5;196m\x1b[1mWAVE %u INCOMING\x1b[0m\n", m_state.wave);
    std::printf("  \x1b[2m%zu robots approaching. Prepare yourself.\x1b[0m\n", robots.size());

    std::vector<RobotState> alive = robots;

    while (!alive.empty() && !m_state.game_over) {
        // NPC popup chance
        auto popup = m_popups.try_popup();
        if (!popup.empty()) std::printf("  %s\n", popup.c_str());

        draw_hud();
        std::printf("  \x1b[2m[wave] [a]ttack nearest [u]ber [r]eload [t]aunt [f]lee | robots left: %zu\x1b[0m\n", alive.size());
        std::printf("  \x1b[2m[wave]\x1b[0m ");
        char buf[32];
        if (!std::fgets(buf, sizeof(buf), stdin)) break;
        std::string cmd;
        for (char* p = buf; *p; ++p)
            if (*p != '\n' && *p != '\r') cmd += (char)std::tolower(*p);

        if (cmd == "q") break;

        if (cmd == "a" || cmd == "attack") {
            // Fight first robot in queue
            CombatEngine combat(m_data, std::time(nullptr));
            combat.engage(alive[0], m_player);
            while (!combat.is_over()) {
                std::printf("    [%s HP:%u/%u] [a]ttack [r]eload [u]ber: ",
                            alive[0].name.c_str(), combat.enemy_hp(), combat.enemy_max_hp());
                char cbuf[16];
                if (!std::fgets(cbuf, sizeof(cbuf), stdin)) break;
                std::vector<std::string> log;
                if (cbuf[0] == 'a') log = combat.attack();
                else if (cbuf[0] == 'r') log = combat.reload();
                else if (cbuf[0] == 'u') log = combat.use_ability();
                else if (cbuf[0] == 'q') break;
                for (auto& l : log) std::printf("      %s\n", l.c_str());
                if (combat.is_over()) break;
            }
            if (combat.player_won()) {
                std::printf("    \x1b[38;5;46m%s destroyed!\x1b[0m\n", alive[0].name.c_str());
                alive.erase(alive.begin());
                m_state.total_kills++;
                m_director.kill_one();
            } else if (combat.state() == CombatState::defeat) {
                m_state.game_over = true;
                std::printf("    \x1b[38;5;196mYou were scrapped. Wave over.\x1b[0m\n");
                return;
            }
            m_player.hp = std::max(1.0f, m_player.hp);
        }
        else if (cmd == "r" || cmd == "reload") {
            m_player.ammo = std::min(m_player.max_ammo, m_player.ammo + 6);
            std::printf("    Reloaded.\n");
        }
        else if (cmd == "u" || cmd == "uber" || cmd == "taunt") {
            std::printf("    \x1b[38;5;200m* Nananananana! *\x1b[0m\n");
        }
        else if (cmd == "f" || cmd == "flee") {
            if ((std::rand() % 100) < 30) {
                std::printf("    You respawn at the spawn room.\n");
                break;
            }
        }
    }

    if (alive.empty()) {
        uint32_t reward = m_director.credits_reward();
        m_player.credits += reward;
        m_state.credits_earned += reward;
        std::printf("\n  \x1b[38;5;220mWAVE %u CLEARED! +%.0f credits\x1b[0m\n", m_state.wave, (float)reward);
    }
}

void Game::visit_npc() {
    if (m_data.npcs().empty()) {
        std::printf("  No NPCs around.\n");
        return;
    }
    auto& npc = m_data.npcs()[std::rand() % m_data.npcs().size()];
    std::string line = m_popups.random_quote(npc);

    std::printf("\n  \x1b[38;5;200m\x1b[1m[%s]\x1b[0m \x1b[2m(%s)\x1b[0m\n", npc.name.c_str(), npc.type.c_str());
    std::printf("  \"%s\"\n", line.c_str());
    if (!npc.reacts_to_coconut.empty() && m_coconut.carrying()) {
        std::printf("  \x1b[38;5;130m* %s *\x1b[0m\n", npc.reacts_to_coconut.c_str());
    }
    m_state.npc_encounters++;
}

void Game::hunt_coconut() {
    if (m_coconut.found_on(m_state.current_map)) {
        std::printf("\n  \x1b[38;5;130m\x1b[1mYou found the coconut on %s!\x1b[0m\n", m_state.current_map.c_str());
        std::printf("  \x1b[2mIt's warm. It hums faintly.\x1b[0m\n");
        m_coconut.pickup();
    } else if (m_coconut.carrying()) {
        std::printf("  \x1b[38;5;130mYou're carrying the coconut. Throw it? [y/N]\x1b[0m ");
        char buf[16];
        if (std::fgets(buf, sizeof(buf), stdin) && (buf[0] == 'y' || buf[0] == 'Y')) {
            m_coconut.throw_coconut();
        }
    } else {
        std::printf("  \x1b[2mYou've already found the coconut on this map.\x1b[0m\n");
    }
    std::printf("  \x1b[2mCoconuts found: %u\x1b[0m\n", m_coconut.total_found());
}

void Game::shop() {
    std::printf("\n  \x1b[38;5;220mMANN CO. UPGRADE STAND\x1b[0m\n");
    std::printf("  Credits: %.0f\n", m_player.credits);
    std::printf("  1) +10 max HP (100 credits)\n");
    std::printf("  2) +10 ammo reserve (80 credits)\n");
    std::printf("  3) Restore HP to full (50 credits)\n");
    std::printf("  q) back\n");
    std::printf("  \x1b[2m[shop]\x1b[0m ");
    char buf[32];
    if (!std::fgets(buf, sizeof(buf), stdin)) return;
    int n = std::atoi(buf);
    if (n == 1 && m_player.credits >= 100) {
        m_player.max_hp += 10; m_player.hp = m_player.max_hp; m_player.credits -= 100;
        std::printf("  Upgraded!\n");
    } else if (n == 2 && m_player.credits >= 80) {
        m_player.max_ammo += 10; m_player.credits -= 80;
        std::printf("  Upgraded!\n");
    } else if (n == 3 && m_player.credits >= 50) {
        m_player.hp = m_player.max_hp; m_player.credits -= 50;
        std::printf("  Healed!\n");
    }
}

} // namespace tf2

int main() {
    tf2::Game game;
    if (!game.initialize()) return 1;

    while (true) {
        game.show_menu();
        std::printf("  \x1b[2m[menu]\x1b[0m ");
        char buf[32];
        if (!std::fgets(buf, sizeof(buf), stdin)) break;
        std::string cmd;
        for (char* p = buf; *p; ++p)
            if (*p != '\n' && *p != '\r') cmd += (char)std::tolower(*p);
        if (cmd == "q" || cmd == "quit") break;
        if (cmd == "1") game.choose_class();
        else if (cmd == "2") game.choose_map();
        else if (cmd == "3") game.run_wave();
        else if (cmd == "4") game.visit_npc();
        else if (cmd == "5") game.hunt_coconut();
        else if (cmd == "6") game.shop();
        else if (cmd == "d") {
            std::printf("    difficulty (casual/legend): ");
            char dbuf[32];
            std::fgets(dbuf, sizeof(dbuf), stdin);
            std::string dname;
            for (char* p = dbuf; *p; ++p)
                if (*p != '\n' && *p != '\r') dname += (char)std::tolower(*p);
            game.data().set_difficulty(dname);
            game.state().difficulty = dname;
        }
    }
    game.shutdown();
    return 0;
}
