#pragma once

#include "event-logger-gui.hpp"
#include "imgui-clap-support/imgui-clap-editor.h"

template <typename T>
struct NoteLoggerGUI : public imgui_clap_editor
{
    NoteLoggerGUI(T& _plugin)
    : plugin{_plugin}
    {}

    void onRender() override {
        if (ImGui::Begin("Note Logger", nullptr, 
                ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration)
        ){
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Note Logger")) {

                    ImGui::SeparatorText("Preferred Note Dialect");
                    const auto preferred_dialect = plugin.getPreferredDialect();
                    const auto plugin_supported = plugin.getSupportedDialects();
                    const auto host_supported = plugin.hostSupportedDialects();

                    auto preferredMenuItem = 
                        [&](const char* name, uint32_t dialect) {
                            if (ImGui::MenuItem(name, nullptr,preferred_dialect == dialect,plugin_supported & dialect)) {
                                plugin.setPreferredDialect(dialect);
                            }
                        };

                    preferredMenuItem("CLAP",           CLAP_NOTE_DIALECT_CLAP);
                    preferredMenuItem("MIDI 1.0",       CLAP_NOTE_DIALECT_MIDI);
                    preferredMenuItem("MIDI 1.0 - MPE", CLAP_NOTE_DIALECT_MIDI_MPE);
                    preferredMenuItem("MIDI 2.0",       CLAP_NOTE_DIALECT_MIDI2);


                    ImGui::SeparatorText("Supported Note Dialects");
                    
                    auto supportedMenuItem = 
                        [&](const char* name, uint32_t dialect) {
                            bool supported = plugin_supported & dialect;
                            if (ImGui::MenuItem(name, nullptr,&supported,host_supported & dialect)) {
                                plugin.setSupportedDialects(dialect, supported);
                            }
                        };
                    
                    supportedMenuItem("CLAP",           CLAP_NOTE_DIALECT_CLAP);
                    supportedMenuItem("MIDI 1.0",       CLAP_NOTE_DIALECT_MIDI);
                    supportedMenuItem("MIDI 1.0 - MPE", CLAP_NOTE_DIALECT_MIDI_MPE);
                    supportedMenuItem("MIDI 2.0",       CLAP_NOTE_DIALECT_MIDI2);

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            
            log_gui.render("Events Received",plugin.logger);
        }
    }

private:
    T&                  plugin;
    ImGuiEventLog       log_gui;
};
