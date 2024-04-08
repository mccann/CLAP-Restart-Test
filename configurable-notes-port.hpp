#pragma once

#include "notes-port-ext.hpp"


template <typename T>
struct ConfigurableNotesPort : NotesPortExt<T>
{
    auto    notePortCount(bool isInput) const noexcept -> uint32_t {
                bool supported = _supported_dialects & hostSupportedDialects();
                return isInput ? supported : 0;
            }

    bool    notePortInfo(clap_note_port_info& toHere, uint32_t index, bool isInput) const noexcept {
                if (!isInput || index) {
                    return false;
                }
                toHere.id = 0;
                toHere.supported_dialects = _supported_dialects;
                toHere.preferred_dialect  = _preferred_dialect;
                strncpy(toHere.name,"Notes In",sizeof(toHere.name));
                return true;
            }

    auto    getSupportedDialects() const noexcept {
                return _supported_dialects & hostSupportedDialects();
            }
    bool    setSupportedDialects(uint32_t dialects, bool enable) {
                uint32_t new_dialects   =   ( enable 
                                            ? ( _supported_dialects | dialects )
                                            : ( _supported_dialects & ~dialects )
                                            ) & hostSupportedDialects();

                if (new_dialects != _supported_dialects) {
                    _supported_dialects = new_dialects;
                    defaultPreferredDialect();
                    this->asyncHostRescanAllNotePortsASAP();
                    return true;
                }
                return false;
            }

    auto    getPreferredDialect() const noexcept {
                return _preferred_dialect;
            }
    bool    setPreferredDialect(uint32_t dialect) {
                switch (dialect) {
                case CLAP_NOTE_DIALECT_CLAP: [[fallthrough]];
                case CLAP_NOTE_DIALECT_MIDI: [[fallthrough]];
                case CLAP_NOTE_DIALECT_MIDI_MPE: [[fallthrough]];
                case CLAP_NOTE_DIALECT_MIDI2:
                    if (dialect & hostSupportedDialects()) {
                        _preferred_dialect = dialect;
                        this->asyncHostRescanAllNotePortsASAP();
                        return true;
                    }
                }
                return false;
            }
    auto    hostSupportedDialects() const -> uint32_t {
                if (!_host_supported_dialects) {
                    _host_supported_dialects = this->hostSupportedNotePortsDialects();
                }
                return _host_supported_dialects;
            }
private:
    void    defaultPreferredDialect() {
                auto supported = getSupportedDialects();
                if (!(_preferred_dialect & supported)) {
                    for (auto dialect 
                    :   { CLAP_NOTE_DIALECT_CLAP
                        , CLAP_NOTE_DIALECT_MIDI
                        , CLAP_NOTE_DIALECT_MIDI_MPE
                        , CLAP_NOTE_DIALECT_MIDI2
                        }
                    ) {
                        if (dialect & supported) {
                            _preferred_dialect = dialect;
                            return;
                        }
                    }
                }
            }
    mutable uint32_t        _host_supported_dialects = {};
    uint32_t                _supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI | CLAP_NOTE_DIALECT_MIDI_MPE | CLAP_NOTE_DIALECT_MIDI2;
    uint32_t                _preferred_dialect  = CLAP_NOTE_DIALECT_CLAP;
};
