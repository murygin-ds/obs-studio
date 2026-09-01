#pragma once

/* Events of the recording and replay buffer outputs that can trigger a
 * sound and/or an on-screen popup */
enum class OutputNotification {
	ReplayBufferStarted,
	ReplayBufferStopped,
	ReplaySaved,
	RecordingStarted,
	RecordingStopped,
};
