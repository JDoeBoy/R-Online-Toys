#include <sstream>
#include <string>
#include <tuple>
#include <optional>
#include <functional>

#include "iterate_tuple.h"
#include "dataconsts.h"
#include "entity_system.h"
#include "chat/whisper_chat.h"

namespace {
template <typename... Args>
class Parser {
    public:
        Parser(std::stringstream&& ss) {
            size_t index = 0;
            Core::for_each(args, [&ss, &index, this](auto& data) {
                is_arg_ok[index++] = parse(ss, data);
            });
        }

        bool is_good() const {
            return std::all_of(is_arg_ok.cbegin(), is_arg_ok.cend(), [](bool i) {
                return i;
            });
        }

        std::array<bool, sizeof...(Args)> is_arg_ok;
        std::tuple<Args...> args;

    private:
        template <typename T>
        static bool parse(std::stringstream& ss, T& h) {
            return static_cast<bool>(ss >> h);
        }

        template <typename T>
        static bool parse(std::stringstream& ss, std::optional<T>& h) {
            T tmp;
            const bool has_data = ss >> tmp;
            if (has_data) {
                h = tmp;
            }
            return true;
        }
};

template <>
class Parser<> {
    public:
        Parser(std::stringstream&&) {}

        bool is_good() const {
            return true;
        }
};


void item(EntitySystem&, RoseCommon::Entity, Parser<int, int>) {
}

void teleport(EntitySystem&, RoseCommon::Entity, Parser<int, int, int, std::optional<uint16_t>>) {
}

void zuly(EntitySystem&, RoseCommon::Entity, Parser<int>) {
}
}

#define REGISTER_FUNCTION(f) [](EntitySystem& en, Entity e, std::stringstream&& ss) { f(en, e, {std::move(ss)}); }

void help(EntitySystem&, RoseCommon::Entity, Parser<std::optional<std::string>>);

static constexpr const std::unordered_map<std::string, std::tuple<uint16_t, void(*)(EntitySystem&, Entity, std::stringstream&&), std::string>> commands = {
    //{"/item", {100, REGISTER_FUNCTION(item), "Creates an item. Usage: /item <type> <id>"}},
    {"/help", {100, REGISTER_FUNCTION(help), "Prints this help. Usage: /help [command]"}},
    //{"/zuly", {100, REGISTER_FUNCTION(zuly), "Adds zulies to your inventory (you can add a negative amount). Usage: /zuly <amount>"}},
    //{"/tp", {200, REGISTER_FUNCTION(teleport), "Teleports a player or self. usage: /tp <map_id> <x> <y> [client_id]"}}
};

void help(EntitySystem& entitySystem, RoseCommon::Entity entity, Parser<std::optional<std::string>> parser) {
    if (!parser.is_good()) {
        Chat::send_whisper(entitySystem, entity, "Error while parsing the command. Usage: /help [command]");
    }
    Chat::send_whisper(entitySystem, entity, "help (<required args> [optional args]):");
    if (!std::get<0>(parser.args)) {
        for (const auto& command : commands) {
            Chat::send_whisper(entitySystem, entity, std::get<2>(command.second));
        }
    } else {
        if (const auto it = commands.find(std::get<0>(parser.args).value()); it != commands.end()) {
            Chat::send_whisper(entitySystem, entity, std::get<2>(it->second));
        } else {
            Chat::send_whisper(entitySystem, entity, fmt::format("Error, no command {} found.", std::get<0>(parser.args)));
        }
    }
}

void execute_gm(EntitySystem& entitySystem, RoseCommon::Entity entity, const std::string& message) {
    const uint16_t access_level = entitySystem.component<Component::SocketConnector>(entity).access_level;
    std::stringstream ss(message);
    std::string command;
    ss >> command;
    if (const auto it = commands.find(command); it != commands.end() && access_level >= std::get<0>(it->second)) {
        std::get<1>(it->second)(entitySystem, entity, std::move(ss));
    }
}
