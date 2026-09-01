#include "OBSBasic.hpp"

#include <components/OSDNotification.hpp>

#include <qt-wrappers.hpp>

namespace {
struct OutputNotifyInfo {
	const char *name;
	const char *configPrefix;
	const char *defaultSound;
	const char *text;
	QColor accent;
};

const OutputNotifyInfo &GetOutputNotifyInfo(OutputNotification notification)
{
	static const OutputNotifyInfo infos[] = {
		{"replay buffer started", "ReplayBufferStart", "sounds/replay-buffer-start.wav",
		 "Basic.Main.OutputNotify.ReplayBufferStarted", QColor(64, 156, 255)},
		{"replay buffer stopped", "ReplayBufferStop", "sounds/replay-buffer-stop.wav",
		 "Basic.Main.OutputNotify.ReplayBufferStopped", QColor(255, 149, 0)},
		{"replay saved", "ReplaySaved", "sounds/replay-buffer-saved.wav", "Basic.Main.OutputNotify.ReplaySaved",
		 QColor(52, 199, 89)},
		{"recording started", "RecordingStart", "sounds/recording-start.wav",
		 "Basic.Main.OutputNotify.RecordingStarted", QColor(255, 69, 58)},
		{"recording stopped", "RecordingStop", "sounds/recording-stop.wav",
		 "Basic.Main.OutputNotify.RecordingStopped", QColor(174, 174, 178)},
	};
	return infos[static_cast<size_t>(notification)];
}
} // namespace

void OBSBasic::NotifyOutput(OutputNotification notification)
{
	const OutputNotifyInfo &info = GetOutputNotifyInfo(notification);
	config_t *config = App()->GetUserConfig();
	const std::string prefix = std::string("Notify") + info.configPrefix;

	bool playSound = config_get_bool(config, "BasicWindow", (prefix + "Sound").c_str());
	bool showOverlay = config_get_bool(config, "BasicWindow", (prefix + "Overlay").c_str());
	const char *soundPath = config_get_string(config, "BasicWindow", (prefix + "SoundFile").c_str());
	int durationMs = static_cast<int>(config_get_int(config, "BasicWindow", "NotifyOverlayDuration")) * 1000;

	ShowOutputNotification(notification, playSound, showOverlay, QT_UTF8(soundPath), durationMs);
}

void OBSBasic::ShowOutputNotification(OutputNotification notification, bool playSound, bool showOverlay,
				      const QString &soundPath, int overlayDurationMs)
{
	const OutputNotifyInfo &info = GetOutputNotifyInfo(notification);

	blog(LOG_INFO, "Output notification '%s': sound=%s, overlay=%s", info.name, playSound ? "on" : "off",
	     showOverlay ? "on" : "off");

	if (playSound) {
		std::string path = QT_TO_UTF8(soundPath);

		if (path.empty()) {
			std::string bundled;
			if (GetDataFilePath(info.defaultSound, bundled)) {
				path = bundled;
			} else {
				blog(LOG_WARNING, "Notification sound '%s' not found", info.defaultSound);
			}
		}

		if (!path.empty()) {
			PlayNotificationSound(path.c_str());
		}
	}

	if (showOverlay) {
		if (!outputNotifyOverlay) {
			outputNotifyOverlay = new OSDNotification();
		}
		outputNotifyOverlay->Show(QTStr(info.text), info.accent,
					  overlayDurationMs < 1000 ? 1000 : overlayDurationMs);
	}
}
