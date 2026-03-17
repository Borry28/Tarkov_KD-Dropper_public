#pragma once

void sleep(long long milliseconds);

class pixel {
	public:
	int x;
	int y;
	pixel(int x, int y) : x(x), y(y) {}
};

enum class Maps {
	None,
	Interchange,
	Factory,
	Shoreline,
	Customs,
	LightHouse,
	StreetsOfTarkov,
	Woods,
	Reserve,
	GroundZero,
	Labs,
	Terminal
};

enum class MenuScreens
{
	Unknown,
	MainMenu,
	Hideout,
	Character,
	Traders,
	FleaMarket,
	Presets,
	Handbook,
	Settings,
	StillInRaid
};

enum class Time {
	Day,
	Night
};

namespace utils
{

	inline long long getTimeMs() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	inline long long getGMTTimeMs() {
		std::time_t t = std::time(nullptr);
		std::tm tm_gmt;
		gmtime_s(&tm_gmt, &t);
		std::time_t t_gmt = std::mktime(&tm_gmt);
		std::chrono::milliseconds milliseconds =
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::from_time_t(t_gmt).time_since_epoch()
			);
		return milliseconds.count();
	}

	long long getTarkovTime(bool bLR);

	namespace image {
		cv::Mat takeScreenshot(int x = 0, int y = 0, int width = 1920, int height = 1080);

		cv::Mat takeScreenshot(cv::Rect region);

			bool checkPixel(int x, int y,
			cv::Scalar lowerThreshold = cv::Scalar(0, 0, 0),
			cv::Scalar upperThreshold = cv::Scalar(255, 255, 255));

			bool checkPixel(cv::Mat& img, int x, int y,
			cv::Scalar lowerThreshold = cv::Scalar(0, 0, 0),
			cv::Scalar upperThreshold = cv::Scalar(255, 255, 255));

			bool withinThreshold(cv::Vec3b pix, cv::Scalar lowerThreshold, cv::Scalar upperThreshold);
	}

	namespace GameUI {
		MenuScreens checkPlace();

		bool gotoPlace(const MenuScreens& place);

	}

	namespace IO {
		namespace mouse {
			inline void move(int x, int y) {
				sleep(0);
				SetCursorPos(x, y);
			}

			inline void move(cv::Point coords) {
				sleep(0);
				SetCursorPos(coords.x, coords.y);
			}

			inline void moveRel(int x, int y) {
				sleep(0);
				INPUT input = { 0 };
				input.type = INPUT_MOUSE;
				input.mi.dwFlags = MOUSEEVENTF_MOVE;
				input.mi.dx = x;        input.mi.dy = y;
				SendInput(1, &input, sizeof(INPUT));
			}

			inline void leftClick() {
				sleep(0);
				INPUT input = { 0 };
				input.type = INPUT_MOUSE;
				input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
				if (SendInput(1, &input, sizeof(INPUT)) != 1) {
					std::cerr << "Failed to send left mouse click input.\n";
				}
			}
			
			inline void rightClick() {
				sleep(0);
				INPUT input = { 0 };
				input.type = INPUT_MOUSE;
				input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
				if (SendInput(1, &input, sizeof(INPUT)) != 1) {
					std::cerr << "Failed to send left mouse click input.\n";
				}
			}
		}
		namespace keyboard {
			inline void pressKey(WORD key) {
				sleep(0);
				INPUT input = { 0 };
				input.type = INPUT_KEYBOARD;
				input.ki.wVk = key;
				SendInput(1, &input, sizeof(INPUT));
			}

			inline void releaseKey(WORD key) {
				sleep(0);
				INPUT input = { 0 };
				input.type = INPUT_KEYBOARD;
				input.ki.wVk = key;
				input.ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput(1, &input, sizeof(INPUT));
			}

			inline void clickKey(WORD key, int interval = 50) {
				pressKey(key);
				sleep(interval);
				releaseKey(key);
			}
		}
	}
}