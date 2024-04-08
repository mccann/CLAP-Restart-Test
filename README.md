# CLAP Restart Test

A minimal clap plugins to highlight what I believe is a bug with Bitwig's (as of 5.1.7) implementation of CLAP plugin protocol with respect to host's handling of ``host->request_restart`` 

## Expectation

When ``host->request_restart`` is called, the plugin will eventually be deactivated and reactivated. 

It is reasonable that a host may need to delay restart until an appropriate time, however I believe it is unreasonable that it doesn't appear to get called, even when in an otherwise idle state.

Relevant Spec: https://github.com/free-audio/clap/blob/main/include/clap/host.h#L27

> Request the host to deactivate and then reactivate the plugin.  
> The operation may be delayed by the host.  
> [thread-safe]  

## Reality

Bitwig appears to ignore the request when executed by the plugin after initialization. 

However, if I use the **orange power button** on the plugin device in the Bitwig GUI, ``host->request_restart`` calls begin to be honoured in an expected way.

Note: My test environment is MacOS Sonoma 14.4, M1 Max, Bitwig versions 5.1 and 5.1.7
 
## Steps to reproduce

1. Compile plugin
    - Only tested with MacOS Sonoma, XCode 15's commandline tools compiler.
2. Open Bitwig (5.1.7)
3. Use Bitwig's File Browser to navigate to the ``build/note-logger`` directory and locate the plugin
4. Create an Instrument Track with the **Note Logger** plugin in the signal chain.
5. Open the **Note Logger** GUI.
6. Click the *Note Logger* menu item in the upper-left corner and select *MIDI 1.0* from the *Supported Note Dialects* list
7. Observe that ``host->request_restart`` is now displayed in the event log. This was called because the previous menu selection requires a rescan when deactivated to complete.
8. Wait, and observe that nothing appears to be happening.
9. Press the **orange power button** on the plugin device to turn to the **off** state.
10. Observe that ``host->request_restart`` has been honoured, and the followup sequence of events have been executed.
11. Click the *Note Logger* menu item in the upper-left corner and select *MIDI 1.0* from the *Supported Note Dialects* list, again.
12. Observe that ``host->request_restart`` has been honoured when in the **off** state.
13. Press the **orange power button** on the plugin device to turn to **on** state.
14. Click the *Note Logger* menu item in the upper-left corner and select *MIDI 1.0* from the *Supported Note Dialects* list, again.
15. Observe that ``host->request_restart`` has been honoured when in the **on** state.
16. Observe that ``host->request_restart`` appears to not be honoured after initial startup, but once the **orange power button** has been interacted with, it begins to be honoured.
    

### Note about code

This plugin was built to be small and focus on a minimal functionality, not intended to be examples of a robust plugin.
It is based on the **Note Logger** plugin that I wrote here: https://github.com/mccann/CLAP-Expression-Tests
