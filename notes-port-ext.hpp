#pragma once

#include "plugin_cast.hpp"


template <typename T>
struct NotesPortExt : clap_plugin_note_ports
{
    NotesPortExt() : clap_plugin_note_ports{
        .count  =   
            [] (const clap_plugin* plugin, bool isInput) -> uint32_t {
                plugin_cast<const T*>(plugin)->logger.logInfo("plugin_note_ports->count()");
                return plugin_cast<const T*>(plugin)->notePortCount(isInput);
            },

        .get    =   
            [] (const clap_plugin* plugin, uint32_t index, bool isInput, clap_note_port_info *info) -> bool {
                plugin_cast<const T*>(plugin)->logger.logInfo("plugin_note_ports->get()");
                return plugin_cast<const T*>(plugin)->notePortInfo(*info,index,isInput);
            },
    }{}

    auto    init() {
                _host_ports_ext = 
                    static_cast<const clap_host_note_ports*>(
                        static_cast<T*>(this)->getHostExtension(CLAP_EXT_NOTE_PORTS)
                    );
            }

    auto    hostPortsExt() const {
                return _host_ports_ext;
            }

    auto    hostSupportedNotePortsDialects() const -> uint32_t {
                auto host_note_ports = hostPortsExt();
                if (host_note_ports && host_note_ports->supported_dialects) {
                    static_cast<const T*>(this)->logger.logInfo("host_note_ports->supported_dialects()");
                    return host_note_ports->supported_dialects(static_cast<const T*>(this)->host);
                }
                return {};
            }

    void    hostRescanNotePorts(uint32_t flags) const {
                auto host_note_ports = hostPortsExt();
                if (host_note_ports && host_note_ports->rescan) {
                    static_cast<const T*>(this)->logger.logInfo("host_note_ports->rescan()");
                    host_note_ports->rescan(static_cast<const T*>(this)->host,flags);
                }
            }
    void    asyncHostRescanAllNotePortsASAP() {
                static_cast<T*>(this)->executeWhenInactiveASAP([](T* plugin) {
                    plugin->logger.logInfo("executing async task: asyncHostRescanAllNotePortsASAP()");
                    plugin->hostRescanNotePorts(CLAP_NOTE_PORTS_RESCAN_ALL);
                });
            }
private:
    const clap_host_note_ports*   _host_ports_ext{};
};
