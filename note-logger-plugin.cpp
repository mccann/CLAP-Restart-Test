#include <string.h>
#include <stdlib.h>

#include "plugin.hpp"
#include "gui-ext.hpp"
#include "event-logger.hpp"
#include "note-logger-gui.hpp"
#include "configurable-notes-port.hpp"

struct NoteLogger 
: ConfigurableNotesPort<NoteLogger>
, ImGuiClapExt<NoteLogger,NoteLoggerGUI>
{
	clap_plugin_t       	plugin;
	const clap_host_t* 		host{};
    EventLogger 			logger;

	void    onInit() {
				ConfigurableNotesPort<NoteLogger>::init();
			}
    void    onActivate(double sampleRate) {
                logger.reset();
				logger.logInfo("plugin->activate()");
				{
					std::lock_guard lock{_task_mutex};
					_active = true;
				}
            }
    void    onDeactivate() {
				{
					std::lock_guard lock{_task_mutex};
					_active = false;
				}
				logger.logInfo("plugin->deactivate()");
				runTasks();
            }
    void    onStartProcessing() {  
				logger.logInfo("plugin->start_processing()");
			}
    void    onStopProcessing() {  
				logger.logInfo("plugin->stop_processing()");
			}
    void    onResetProcessing() {  
				logger.reset();
				logger.logInfo("plugin->reset()");
			}
    auto    onProcess(const clap_process_t *process) {
                return logger.processInEvents(process);
            }

	void    onMainThread() {
				logger.logInfo("plugin->on_main_thread()");
				runTasks();
			}

	auto    getExtension(const char *id) const -> const void* {
				if (0 == strcmp(id, CLAP_EXT_NOTE_PORTS )) {
					return static_cast<const clap_plugin_note_ports*>(this);
				} 
        		if (0 == strcmp(id, CLAP_EXT_GUI )) {
					return static_cast<const clap_plugin_gui*>(this);
				}
				return nullptr;
			}

	auto    getHostExtension(const char*id) const -> const void* {
				return host->get_extension(host,id);
			}

	using task_t = std::function<void(NoteLogger*)>;
	enum class Execute { asap, eventually };

	void    enqueueTask(Execute priority, task_t&& task) {
				std::unique_lock lock{_task_mutex};
				_inactive_tasks_queue.push_back(std::move(task));
				bool active = _active;
				lock.unlock(); // unlock before calling out
				if (priority == Execute::asap) {
					if (active) {
						logger.logInfo("host->request_restart()");
						host->request_restart(host);
					} else {
						logger.logInfo("host->request_callback()");
						host->request_callback(host);
					}
				}
			}
	void    executeWhenInactiveASAP(task_t&& task) {
				enqueueTask(Execute::asap,std::move(task));
			}

	void    runTasks() {
				std::deque<task_t> tasks;
				{
					std::lock_guard lock{_task_mutex};
					if (!_active) {
						std::swap(tasks,_inactive_tasks_queue);
					}
				}
				for (auto& task : tasks) {
					task(this);
				}
			}

private:
	std::mutex 				_task_mutex;
	std::deque<task_t>		_inactive_tasks_queue;
	bool 					_active{};
};

static const clap_plugin_descriptor_t pluginDescriptor = {
	.clap_version = CLAP_VERSION_INIT,
	.id = "mccann.CLAP-Restart-Test.NoteLogger",
	.name = "NoteLogger",
	.vendor = "CLAP-Restart-Test",
	.url = "https://github.com/mccann/CLAP-Restart-Test",
	.manual_url = "https://github.com/mccann/CLAP-Restart-Test",
	.support_url = "https://github.com/mccann/CLAP-Restart-Test",
	.version = "1.0.0",
	.description = "Display a text log of CLAP events received on input port, with configurable note-port",

	.features = (const char *[]) {
		CLAP_PLUGIN_FEATURE_ANALYZER,
		NULL,
	},
};

static const clap_plugin_t pluginClass                      = makePluginClass<NoteLogger>(&pluginDescriptor);
static const clap_plugin_factory_t pluginFactory            = makePluginFactory<NoteLogger,&pluginClass>();
extern "C" const clap_plugin_entry_t clap_entry             = makeEntry<&pluginFactory>();
