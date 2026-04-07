add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
if is_config("target_type", "server") then
    add_requires("levilamina 26.10.0", {configs = {target_type = "server"}})
else
    add_requires("levilamina 26.10.0", {configs = {target_type = "client"}})
end

add_requires("cpr[ssl=y] 1.12.0")

add_requires("levibuildscript")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

target("ll-bstats")
    add_rules("@levibuildscript/linkrule")
    add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
    add_defines("NOMINMAX", "UNICODE")
    add_packages("levilamina", "cpr")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("static")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("src/(ll-bstats/**.h)")
    add_files("src/**.cc")
    add_includedirs("src")
    if is_config("target_type", "server") then
        add_defines("LL_PLAT_S")
    else
        add_defines("LL_PLAT_C")
    end