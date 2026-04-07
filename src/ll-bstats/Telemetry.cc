#include "Telemetry.h"
#include "ll-bstats/bstats/Bukkit.h"

#include "ll/api/Versions.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/coro/InterruptableSleep.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ThreadPoolExecutor.h"
#include "ll/api/utils/RandomUtils.h"
#include "ll/api/utils/SystemUtils.h"


#include "mc/common/BuildInfo.h"
#include "mc/common/Common.h"
#include "mc/platform/UUID.h"
#include "mc/server/PropertiesSettings.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

#include "cpr/api.h"
#include "cpr/body.h"
#include "cpr/cpr.h"
#include "cpr/cprtypes.h"

#include "magic_enum/magic_enum.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>


namespace ll_bstats {

Telemetry::Telemetry(int pluginId, std::string pluginVersion) {
    payload                       = bstats::bukkit::BukkitPayload{getServiceUUID(), pluginId};
    payload.service.pluginVersion = std::move(pluginVersion);
}
Telemetry::~Telemetry() { shutdown(); };


std::string const& Telemetry::getServiceUUID() {
    static std::optional<std::string> uuid;
    if (!uuid) {
        auto path = ll::mod::getModsRoot() / u8"ll_bstats_telemetry_uuid";
        if (auto storedUid = ll::file_utils::readFile(path)) {
            uuid = *storedUid;
        } else {
            auto newUUID = mce::UUID::random().asString();
            ll::file_utils::writeFile(path, newUUID);
            uuid = newUUID;
        }
    }
    return uuid.value();
}

void Telemetry::initConstant() {
    payload.osArch    = "amd64";
    payload.coreCount = std::thread::hardware_concurrency();

#ifdef LL_PLAT_S
    payload.onlineMode = ll::service::getPropertiesSettings().value().mIsOnlineMode;
#elif LL_PLAT_C
    payload.onlineMode = true;
#endif

    payload.osName    = ll::sys_utils::isWine() ? "Linux" : "Windows";
    payload.osVersion = ll::sys_utils::getSystemVersion().to_string();

    payload.bukkitVersion = Common::getBuildInfo().mBuildId;
}

void Telemetry::collect() {
    payload.playerAmount =
        ll::service::getLevel().transform([](auto& level) { return level.getActivePlayerCount(); }).value_or(0);
}

void Telemetry::submit() {
    try {
        collect();

        cpr::Post(
            cpr::Url{
                bstats::bukkit::BukkitPayload::PostUrl
        },
            cpr::Body{payload.to_json().dump()},
            cpr::Header{
                {"Accept", "application/json"},
                {"Content-Type", "application/json"},
                {"User-Agent", "Metrics-Service/1"}
            }
        );
    } catch (...) {}
}

void Telemetry::launch(ll::thread::ThreadPoolExecutor& executor) {
    initConstant();

    quit  = std::make_shared<std::atomic<bool>>(false);
    sleep = std::make_shared<ll::coro::InterruptableSleep>();
    ll::coro::keepThis([sleep = sleep, quit = quit, this]() -> ll::coro::CoroTask<> {
        co_await sleep->sleepFor(std::chrono::minutes{1} * ll::random_utils::rand(3.0, 6.0));
        if (!quit->load()) {
            submit();
        }
        co_await sleep->sleepFor(std::chrono::minutes{1} * ll::random_utils::rand(1.0, 30.0));
        if (!quit->load()) {
            submit();
        }
        while (!quit->load()) {
            co_await sleep->sleepFor(std::chrono::minutes{30});
            if (quit->load()) {
                break;
            }
            submit();
        }
        co_return;
    }).launch(executor);
}

void Telemetry::shutdown() {
    quit->store(true);
    sleep->interrupt(true);
}

} // namespace ll_bstats