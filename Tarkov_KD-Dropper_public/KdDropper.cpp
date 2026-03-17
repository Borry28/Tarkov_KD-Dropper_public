#include "stdafx.h"
#include "KdDropper.h"
#include "utils.h"

#include <random>



inline const long long hrs(const int num) {
    return 1000LL * 60 * 60 * num;
}

void selectMap(Maps map, Time timeOfDay) {
    using namespace utils::IO::mouse;

    switch (map) {
    case Maps::Factory:
        move(704, 394);
        break;
    case Maps::Interchange:
        move(888, 512);
        break;
    case Maps::Shoreline:
        move(521, 560);
        break;
    case Maps::Customs:
        move(772, 432);
        break;
    case Maps::LightHouse:
        move(449, 438);
        break;
    case Maps::StreetsOfTarkov:
        move(929, 307);
        break;
    case Maps::Woods:
        move(687, 275);
        break;
    case Maps::Reserve:
        move(616, 471);
        break;
    case Maps::GroundZero:
        move(960, 217);
        break;
    case Maps::Labs:
        move(882, 163);
        break;
    case Maps::Terminal:
        move(562, 670);
        break;
    }

    leftClick();
    sleep(250);

    if (map != Maps::Factory) {
        long long tarkovTime = utils::getTarkovTime(0);
        bool isDayTime = tarkovTime >= hrs(6) && tarkovTime < hrs(18);
        sleep(250);

        if ((timeOfDay == Time::Day && !isDayTime) || (timeOfDay == Time::Night && isDayTime)) {
            move(699, 867);
        }
        else {
            move(894, 869);
        }

        sleep(100);

        leftClick();
        sleep(250);
        return;
    }

    sleep(250);

    bool currentSelectedTimeIsDay = utils::image::checkPixel(699, 867, { 100, 100, 100 });

    if (timeOfDay == Time::Day && !currentSelectedTimeIsDay) {
        sleep(250);
        move(699, 867);
        sleep(100);
        leftClick();
        sleep(250);
    }
    else if (timeOfDay == Time::Night && currentSelectedTimeIsDay) {
        sleep(250);
        move(894, 869);
        sleep(100);
        leftClick();
        sleep(250);
    }
    return;
}

KdDropper::extractionStatus KdDropper::checkExtractionStatus() {
    try {

        // FOLLOWING ARE JUST EXTRACTION STATUSES:
        cv::Mat screen = utils::image::takeScreenshot();
        cv::Vec3b pixelColor1 = screen.at<cv::Vec3b>(49, 993);
        cv::Vec3b pixelColor2 = screen.at<cv::Vec3b>(985, 986);

		using namespace utils::image;

		KdDropper::extractionStatus status = KdDropper::extractionStatus::StillInRaid;

        if (!withinThreshold(pixelColor1, { 200, 200, 200 }, { 255, 255, 255 }) && !withinThreshold(pixelColor2, { 180, 180, 180 }, { 255, 255, 255 })) {
            return status;
        }

        // Check for extraction reached - green extraction UI indicator
        // Extraction UI typically appears top-center with green color
        cv::Vec3b extractionPixel = screen.at<cv::Vec3b>(647, 975);
        bool extractionReached = withinThreshold(extractionPixel, { 0, 100, 60 }, { 40, 255, 180 });

        if (extractionReached) {
			status = KdDropper::extractionStatus::Extracted;
        }


        cv::Vec3b runthroughPixel = screen.at<cv::Vec3b>(645, 978);

        bool runthroughDetected = withinThreshold(runthroughPixel, { 139, 139, 139 }, { 190, 190, 190 });

        if (runthroughDetected) {
			status = KdDropper::extractionStatus::Runthrough;
        }

        cv::Vec3b extractionFailedPixel = screen.at<cv::Vec3b>(642, 948);
        bool missingInActionDetected = withinThreshold(extractionFailedPixel, { 13, 13, 159 }, { 17, 17, 163 });

        if (missingInActionDetected) {
			status = KdDropper::extractionStatus::MIA;
        }

        cv::Vec3b deathPixel = screen.at<cv::Vec3b>(649, 978);
        bool isDead = withinThreshold(deathPixel, { 0, 0, 100 }, { 25, 40, 255 });
        if (isDead) {
			status = KdDropper::extractionStatus::KIA;
        }

        return status;
    }
	catch (...) { 
        // no debug code, buy https://tarkov.bot/products/scav-bot for better debugging
    }
    return KdDropper::extractionStatus::Unknown;
}

void KdDropper::startBotLoop() {
	using namespace utils::IO;
	using namespace utils::GameUI;
	using namespace utils::image;

    /*
     * *****************************************************************
     * VERY IMPORTANT NOTE READ NOW OR GAY:
     * 
     * 
     * BUY NOW https://tarkov.bot/products/scav-bot BUY NOW 
     * 
     * 
	 * *****************************************************************
     */
    gotoPlace(MenuScreens::MainMenu);

    sleep(1000);

    // click "ESCAPE FROM TARKOV" button

    mouse::move({ 950, 580 });
    mouse::leftClick();

    sleep(500);

    // checks if pmc is not already selected else select it
    if (!checkPixel(1229, 674, { 185, 213, 219 }, { 205, 233, 239 })) {
        mouse::move(1186, 687);
        mouse::leftClick();
        sleep(500);
    }

    // clicks next

    setStatus("Joining raid");
    mouse::move(960, 940);
    mouse::leftClick();

    sleep(500);

    for (int i = 0; i < 12; i++) {
        // selects map and time of day
        currentMap = evaluateCurrentMap(); // if it's random yk
        currentTime = evaluateCurrentTimeOfDay();
        selectMap(currentMap, currentTime);

        sleep(100);
        mouse::move(1256, 996); // hovers on ready to see if it's playable
        sleep(500);

        if (checkPixel(1191, 993, { 150, 150, 150 }, { 255, 255, 255 })) {
            break;
        }
        // mark map as bad
        workingMaps[currentMap] = false;
        sleep(250);
    }

	sleep(100);
    mouse::leftClick();

    // HERE THE PLAYER IS LOADING IN RAID
    // 
    // detects green box when raid starts
    setStatus("Waiting for raid to start");

    // if bot is stuck here, the raid didn't start properly
    // happens especially with scavs for some reason. AFAIK only PC restart fixes this issue temporarily.
    // (buy https://tarkov.bot/products/scav-bot immediately)

    long long startTime = utils::getTimeMs();
    while (!checkPixel(1576, 10, { 0, 120, 80 }, { 50, 255, 160 })) {
        // error checking missing in this version as I'm not giving you this freebie
        sleep(100);
    }

    // now we are in raid!

    sleep(1000);
    for (int i = 0; i < 50; i++) {
		utils::IO::mouse::moveRel(0, -100); // look up for extra goofy factor
        sleep(10);
	}

	utils::IO::keyboard::clickKey(equipGrenadeKeybind);
    sleep(2000);
	// wait for grenade to equip

	utils::IO::mouse::rightClick();

	// grenade is thrown; now we wait for death
	sleep(5000);


    for (int i = 0; i < 60 * 60 * 10; ++i) { // 60 * 60 * 10 * 100ms = 3600 seconds / 1 hour
        extractionStatus status = checkExtractionStatus();
        if (status == KdDropper::extractionStatus::KIA or status == KdDropper::extractionStatus::MIA) {
            stats.deathCount++;
            break;
        }
        sleep(100);
    }

	// hoping that the bot doesn't fucking spawn in the extraction directly (happened to me once, no idea how) and if it does, well, bot stuck, too bad not my problem (buy https://tarkov.bot/products/scav-bot to avoid this issue and for better support)
    mouse::move(955, 941);
    sleep(50);
    mouse::leftClick();
    sleep(500);
    mouse::leftClick();
    sleep(500);
    mouse::leftClick();
    sleep(500);
    mouse::leftClick();
    sleep(500);
    mouse::leftClick();
    sleep(500);
    mouse::leftClick();
    sleep(500);
    mouse::leftClick();
    sleep(100);

}


Maps KdDropper::evaluateCurrentMap() {

    if (cfg.useRandomMap) {
        std::vector<Maps> availableMaps;
        availableMaps.reserve(workingMaps.size());

        for (const auto& [map, isWorking] : workingMaps) {
            if (isWorking) {
                availableMaps.push_back(map);
            }
        }

        if (availableMaps.empty()) {
            return cfg.map;
        }

        static thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<size_t> distribution(0, availableMaps.size() - 1);
        return availableMaps[distribution(generator)];
	}
	return cfg.map;
}

Time KdDropper::evaluateCurrentTimeOfDay() {
    if (cfg.useRandomTimeOfDay) {
		static thread_local std::mt19937 generator(std::random_device{}());
		std::uniform_int_distribution<int> distribution(0, 1);
		return static_cast<Time>(distribution(generator));
    }
    return cfg.timeOfDay;
}

void KdDropper::start() {
	if (isRunning) {
		std::cout << "Already running!" << std::endl;
		return;
	}

    HWND hwnd = FindWindowA(NULL, "EscapeFromTarkov");
    if (hwnd) {
        SetForegroundWindow(hwnd);
    }
    else {
		setStatus("Open Tarkov (or buy scav bot to automatically open Tarkov)");
        sleep(3000);
        setStatus("Idle");
        return;
    }

	isRunning.store(true);
	stopFlag.store(false);
	stats.loopTimes.clear();

	startTime = utils::getTimeMs();
    startInterruptListener();
	std::thread([this]() {
        loopData* currentLoop = nullptr;
		try {
			while (!stopFlag.load()) {
                currentLoop = new loopData();
                currentLoop->loopStartTime = utils::getTimeMs();
                stats.loopTimes.push_back(currentLoop);
				startBotLoop();
                currentLoop->finished = true;
                currentLoop->loopEndTime = utils::getTimeMs();
                stats.loopCount++;
                currentLoop = nullptr;
			}
		}
		catch (InterruptException&) {
            if (currentLoop != nullptr) {
                currentLoop->finished = false;
            }
		}

        setStatus("Idle");

		isRunning.store(false);
		}).detach();
}

void KdDropper::startInterruptListener() {
	std::thread([this]() {
		while (true) {
			if (stopFlag.load() || (GetAsyncKeyState(stopKeybind) & 0x8000)) {
				
				stopFlag.store(true);
				return;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		}).detach();
}

float KdDropper::stats::getAverageLoopTime() {
    if (loopTimes.empty()) return 0.0f;
    long long totalTime = 0;
    size_t completedLoopCount = 0;
    for (size_t index = 0; index < loopTimes.size(); ++index) {
        const auto* data = loopTimes[index];
        if (data == nullptr || data->loopStartTime <= 0) {
            continue;
        }

        long long loopEndTime = data->loopEndTime;
        if (loopEndTime <= data->loopStartTime) {
            if (index + 1 >= loopTimes.size() || loopTimes[index + 1] == nullptr) {
                continue;
            }

            loopEndTime = loopTimes[index + 1]->loopStartTime;
            if (loopEndTime <= data->loopStartTime) {
                continue;
            }
        }

        totalTime += (loopEndTime - data->loopStartTime);
        completedLoopCount++;
    }
    if (completedLoopCount == 0) return 0.0f;
    return static_cast<float>(totalTime) / completedLoopCount;
}
