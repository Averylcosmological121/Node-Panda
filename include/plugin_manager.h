#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <filesystem>

struct lua_State;

namespace nodepanda {

class App;

// ─── Metadatos de un plugin (leídos de manifest.json) ────────────────────────
struct PluginManifest {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> hooks;  // on_load, on_frame, on_note_open, etc.
    bool enabled = true;
};

// ─── Estado de runtime de un plugin ──────────────────────────────────────────
struct PluginInstance {
    PluginManifest manifest;
    std::filesystem::path directory;    // carpeta del plugin
    lua_State* luaState = nullptr;      // estado Lua aislado por plugin
    bool loaded  = false;               // on_load ya fue invocado
    bool hasError = false;              // último hook falló
    std::string lastError;              // mensaje del último error
    int consecutiveErrors = 0;          // errores consecutivos en on_frame
    static constexpr int kMaxFrameErrors = 3; // auto-desactivar después de N errores
};

// ─── Ítem de menú registrado por un plugin ───────────────────────────────────
struct PluginMenuItem {
    std::string pluginId;
    std::string label;
    int callbackRef = -1;   // referencia Lua al callback
    lua_State* luaState = nullptr;
};

// ─── Panel registrado por un plugin ──────────────────────────────────────────
struct PluginPanelData {
    std::string pluginId;
    std::string name;
    int renderRef = -1;     // referencia Lua a la función de render
    lua_State* luaState = nullptr;
    bool open = true;
};

// ─── Notificación emitida por un plugin ──────────────────────────────────────
struct PluginNotification {
    std::string text;
    std::string type;       // "info", "warn", "error", "success"
    float timer = 3.0f;     // segundos restantes
};

// ═════════════════════════════════════════════════════════════════════════════
//  PluginManager — descubre, carga y orquesta plugins Lua
// ═════════════════════════════════════════════════════════════════════════════
class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    /// Inicializar: escanea data/plugins/ y carga todos los habilitados
    void init(App* app);

    /// Apagar: llama on_unload en cada plugin y cierra estados Lua
    void shutdown();

    // ── Lifecycle hooks ──────────────────────────────────────────────────
    void callOnLoad();
    void callOnFrame();
    void callOnNoteOpen(const std::string& noteId);
    void callOnNoteSave(const std::string& noteId);
    void callOnNoteCreate(const std::string& noteId);

    // ── Gestión de plugins ───────────────────────────────────────────────
    void enablePlugin(const std::string& pluginId);
    void disablePlugin(const std::string& pluginId);
    void reloadPlugin(const std::string& pluginId);
    void rescanPlugins();

    // ── Acceso a datos ───────────────────────────────────────────────────
    const std::vector<PluginInstance>& getPlugins() const { return m_plugins; }
    std::vector<PluginInstance>& getPlugins() { return m_plugins; }

    const std::vector<PluginMenuItem>& getMenuItems() const { return m_menuItems; }
    const std::vector<PluginPanelData>& getPanels() const { return m_panels; }
    std::vector<PluginPanelData>& getPanels() { return m_panels; }
    std::vector<PluginNotification>& getNotifications() { return m_notifications; }

    // ── API para que los plugins registren cosas ─────────────────────────
    void registerMenuItem(const std::string& pluginId, const std::string& label,
                          int callbackRef, lua_State* L);
    void registerPanel(const std::string& pluginId, const std::string& name,
                       int renderRef, lua_State* L);
    void addNotification(const std::string& text, const std::string& type);

private:
    void discoverPlugins();
    bool parseManifest(const std::filesystem::path& manifestPath, PluginManifest& out);
    void initPluginLua(PluginInstance& plugin);
    void registerPluginBindings(PluginInstance& plugin);
    void loadPluginScript(PluginInstance& plugin);
    void callHook(PluginInstance& plugin, const std::string& hookName);
    void callHookWithArg(PluginInstance& plugin, const std::string& hookName,
                         const std::string& arg);
    void destroyPlugin(PluginInstance& plugin);
    void savePluginStates();   // guardar qué plugins están enabled/disabled
    void loadPluginStates();

    App* m_app = nullptr;
    std::filesystem::path m_pluginsDir;
    std::vector<PluginInstance> m_plugins;
    std::vector<PluginMenuItem> m_menuItems;
    std::vector<PluginPanelData> m_panels;
    std::vector<PluginNotification> m_notifications;
};

} // namespace nodepanda
