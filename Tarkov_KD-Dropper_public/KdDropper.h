#pragma once
#include "stdafx.h"
#include "utils.h"


class KdDropper {
	KdDropper() = default;
	KdDropper(const KdDropper&) = delete;
	KdDropper& operator=(const KdDropper&) = delete;

	DWORD stopKeybind = VK_ESCAPE;
	DWORD equipGrenadeKeybind = 'G';

	struct loopData {
		long long loopStartTime = 0;
		long long loopEndTime = 0;
		bool finished = false;
	};

	struct stats {
		int loopCount = 0;
		int deathCount = 0;
		std::vector<loopData*> loopTimes = {};
		float getAverageLoopTime();
	};

	struct config {
		bool allowGameRestart = true;
		Maps map = Maps::Factory;
		Time timeOfDay = Time::Day;
		bool useRandomMap = false;
		bool useRandomTimeOfDay = false;
		int maxDeaths = 0; // 0 for infinite
	};

	enum class extractionStatus {
		Unknown,
		Extracted,
		KIA,
		MIA,
		Runthrough,
		StillInRaid
	};

	std::map<Maps, bool> workingMaps = {
		{ Maps::Interchange, true },
		{ Maps::Factory, true },
		{ Maps::Shoreline, true },
		{ Maps::Customs, true },
		{ Maps::LightHouse, true },
		{ Maps::StreetsOfTarkov, true },
		{ Maps::Woods, true },
		{ Maps::Reserve, true },
		{ Maps::GroundZero, true },
		{ Maps::Labs, true },
		{ Maps::Terminal, true }
	};

	extractionStatus checkExtractionStatus();

	Maps evaluateCurrentMap();
	Time evaluateCurrentTimeOfDay();

	config cfg = {};

	stats stats = {};

	Maps currentMap = Maps::None;
	Time currentTime = Time::Day;

	std::string status = "Idle";
	std::mutex statusMutex;



public:
	static KdDropper& getInstance() {
		static KdDropper instance;
		return instance;
	}

	class InterruptException : public std::exception {};


	std::atomic<bool> stopFlag{ false };
	std::atomic<bool> isRunning{ false };

	long long startTime = 0;

	std::string getStatus() {
		std::lock_guard<std::mutex> lock(statusMutex);
		return status;
	}

	void setStatus(const std::string& newStatus) {
		std::lock_guard<std::mutex> lock(statusMutex);
		status = newStatus;
	}

	int getLoopCount() { return stats.loopCount; }
	int getDeathCount() { return stats.deathCount; }
	float getAverageLoopTime() { return stats.getAverageLoopTime(); }

	void setMap(Maps map) { cfg.map = map; }
	void setTimeOfDay(Time time) { cfg.timeOfDay = time; }
	void setUseRandomMap(bool value) { cfg.useRandomMap = value; }
	void setUseRandomTimeOfDay(bool value) { cfg.useRandomTimeOfDay = value; }
	void setMaxDeaths(int value) { cfg.maxDeaths = value; }
	void setStopKeybind(DWORD key) { stopKeybind = key; }
	void setEquipGrenadeKeybind(DWORD key) { equipGrenadeKeybind = key; }

	void init();

	void start();

	void stop() {
		stopFlag.store(true);
		// wait for running to turn false
		while (isRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	};

	void startBotLoop();

	void startInterruptListener();


};

