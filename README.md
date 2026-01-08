# ll-bstats - BStats(bukkit) for levilamina

此库通过模拟 bStats bukkit 协议，可以让 levilamina 的插件接入 bStats 统计。

This library simulates the bukkit protocol of bStats, allowing levilamina plugins to access bStats statistics.

## Usage

1. 你需要注册一个 bStats 插件，并获取插件 ID （仅支持 Bukkit 分区）

You need to register a bStats plugin and get the plugin ID (only supports Bukkit partition)

2. 在 xmake.lua 中添加 bStats 依赖

Add bStats dependency in xmake.lua

```lua
add_repositories("iceblcokmc https://github.com/IceBlcokMC/xmake-repo.git")

add_requires("ll-bstats")

target("YourPlugin")
    add_packages("ll-bstats") -- add bstats to your plugin
```

3. 在插件中添加 bStats 统计

Add bStats statistics to your plugin

```cpp
#include "ll-bstats/Telemetry.h"

class YourMod {
    std::unique_ptr<ll_bstats::Telemetry> telemetry;

    void enable() {
        telemetry = std::make_unique<ll_bstats::Telemetry>(YourPluginID, "YourPluginVersion");
        telemetry->launch(YourThreadPoolExecutor);
    }

    void disable() {
        telemetry->shutdown();
        telemetry.reset();
    }
}
```

## License

MIT License
