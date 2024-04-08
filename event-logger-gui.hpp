#pragma once

#include "event-logger.hpp"
#include "event-string.hpp"
#include "imgui.h"
#include <fmt/format.h>
#include <deque>


static constexpr const size_t max_event_strings = 10000;

struct ImGuiEventLog
{
    void render(const char* name, EventLogger& events_logger);
private:
    void                drainEventSource(EventLogger& events_logger);
    LogEvent::Buffer    events_buffer;

    // boost::deque would probably be better, but didn't want to include another dependency
    std::deque<std::string>     string_events_deque;
    bool                        auto_scroll=true;

};

inline void ImGuiEventLog::drainEventSource(EventLogger& events_logger)
{
    // swap this buffer, reusing buffers to minimize heap allocations
    events_logger.swapEventsBuffer(events_buffer);

    // ensure size constraints
    PrepareBufferForAppend(string_events_deque,max_event_strings,events_buffer.size());

    // convert events into strings, once.. in GUI thread
    for( auto& log_event : events_buffer) {
        if (!log_event.log_info.empty()) {
            string_events_deque.push_back(std::move(log_event.log_info));
        } else {
            string_events_deque.push_back(fmt::format(
                "{:06d}:{:d}-{}",
                log_event.block_count,
                log_event.block_size,
                EventString(&log_event.event))
            );
        }
    }
}

inline void ImGuiEventLog::render(const char* name, EventLogger& events_logger) 
{
        auto copyToClipboard = [&]{
            ImGui::LogToClipboard();
            for( auto& str: string_events_deque) {
                ImGui::LogText("%s\n",str.c_str());
            }
            ImGui::LogFinish();
        };

        drainEventSource(events_logger);

        // quasi column headers
        if (string_events_deque.empty()) {
            string_events_deque.push_back("block:size-time:flags  Type & Details");
        }

        ImGui::BeginChild(name, ImVec2(0, 0));
        {

            if (ImGui::BeginPopupContextWindow("context_menu")) {

                ImGui::MenuItem("Auto-scroll", nullptr, &auto_scroll); 
                if (ImGui::Selectable("Copy to clipboard")) { copyToClipboard(); }
                ImGui::Separator();
                if (ImGui::Selectable("Clear")) { string_events_deque.clear(); }
                ImGui::EndPopup();
            }   

            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(string_events_deque.size()));
            while (clipper.Step()) {
                for (auto index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index) {
                    auto& str = string_events_deque[static_cast<size_t>(index)];
                    ImGui::TextUnformatted(str.data(), str.data() + str.size());
                }
            }
            clipper.End();
            if (auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
        }
 
}