#pragma once

#include <clap/clap.h>

template <typename T>
auto    plugin_cast(const clap_plugin* _plugin) {
            return  static_cast<T>(_plugin->plugin_data);
        }

