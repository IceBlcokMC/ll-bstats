#pragma once
#include <memory>

#include "ll-bstats/bstats/Bukkit.h"

#include "ll/api/thread/ThreadPoolExecutor.h"

namespace ll::coro {
class InterruptableSleep;
}

namespace ll_bstats {


class Telemetry {
protected:
    bstats::bukkit::BukkitPayload payload;

private:
    std::shared_ptr<std::atomic<bool>>            quit{nullptr};
    std::shared_ptr<ll::coro::InterruptableSleep> sleep{nullptr};

public:
    explicit Telemetry(int pluginId, std::string pluginVersion);
    virtual ~Telemetry();

    static std::string const& getServiceUUID();

    virtual void initConstant();

    virtual void collect();

    // collects and submits the data
    void submit();

    // initConstant and launch coro task
    void launch(ll::thread::ThreadPoolExecutor& executor);

    void shutdown();
};


} // namespace ll_bstats