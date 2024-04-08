#pragma once

#include <memory>

#include "plugin_cast.hpp"

template <typename T>
constexpr auto makePluginClass(const clap_plugin_descriptor_t* desc) -> clap_plugin_t {
    return {
	.desc = desc,
	.plugin_data = nullptr,
    .init = [] (const clap_plugin *_plugin) { 
		plugin_cast<T*>(_plugin)->onInit();
		return true; 
	},
	.destroy = [] (const clap_plugin * _plugin) {
        // born as unique_ptr, die as one
        std::unique_ptr<T>(plugin_cast<T*>(_plugin)) = nullptr;
    },

	.activate = [] (const clap_plugin *_plugin, double sampleRate, uint32_t, uint32_t) { 
        plugin_cast<T*>(_plugin)->onActivate(sampleRate);
        return true; 
    },
	.deactivate = [] (const clap_plugin *_plugin) {
		plugin_cast<T*>(_plugin)->onDeactivate();
	},

	.start_processing = [] (const clap_plugin *_plugin) {
        plugin_cast<T*>(_plugin)->onStartProcessing();
        return true; 
    },
	.stop_processing = [] (const clap_plugin *_plugin) {
		plugin_cast<T*>(_plugin)->onStopProcessing();
	},

	.reset = [] (const clap_plugin *_plugin) {
        plugin_cast<T*>(_plugin)->onResetProcessing();
	},

	.process = [] (const clap_plugin *_plugin, const clap_process_t *process) -> clap_process_status {
		return plugin_cast<T*>(_plugin)->onProcess(process);
	},

	.get_extension = [] (const clap_plugin *_plugin, const char *id) -> const void * {
		return plugin_cast<T*>(_plugin)->getExtension(id);
	},

	.on_main_thread = [] (const clap_plugin *_plugin) {
		return plugin_cast<T*>(_plugin)->onMainThread();
	}
    };
}

template <typename T,const clap_plugin_t* pluginClass>
constexpr auto makePluginFactory() -> clap_plugin_factory_t {
    return {
	.get_plugin_count = [] (const clap_plugin_factory *factory) -> uint32_t { 
		return 1; 
	},

	.get_plugin_descriptor = [] (const clap_plugin_factory *factory, uint32_t index) -> const clap_plugin_descriptor_t * { 
		return index == 0 ? pluginClass->desc : nullptr; 
	},

	.create_plugin = [] (const clap_plugin_factory *factory, const clap_host_t *host, const char *pluginID) -> const clap_plugin_t * {
		if (!clap_version_is_compatible(host->clap_version) || strcmp(pluginID, pluginClass->desc->id)) {
			return nullptr;
		}

		auto plugin = std::make_unique<T>();
		plugin->host = host;
		plugin->plugin = *pluginClass;
		plugin->plugin.plugin_data = plugin.get();
		return &plugin.release()->plugin;
	}
    };
}



template <const clap_plugin_factory_t* factory>
constexpr clap_plugin_entry_t makeEntry() {
    return {
	.clap_version = CLAP_VERSION_INIT,
	.init = [] (const char *path) -> bool { 
		return true; 
	},
	.deinit = [] () {},
	.get_factory = [] (const char *factoryID) -> const void * {
		return strcmp(factoryID, CLAP_PLUGIN_FACTORY_ID) ? nullptr : factory;
	}
    };
}
