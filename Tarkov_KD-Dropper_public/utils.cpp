#include "stdafx.h"
#include "utils.h"
#include "KdDropper.h"


void sleep(long long milliseconds)
{
    KdDropper& kdDropper = KdDropper::getInstance();

    if (kdDropper.stopFlag.load() && kdDropper.isRunning.load()) {
        throw KdDropper::InterruptException();
    }

	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

cv::Mat utils::image::takeScreenshot(int x, int y, int width, int height)
{
    HDC hDC = GetDC(NULL);
    HDC hCaptureDC = CreateCompatibleDC(hDC);
    HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDC, width, height);
    SelectObject(hCaptureDC, hCaptureBitmap);
    BitBlt(hCaptureDC, 0, 0, width, height, hDC, x, y, SRCCOPY);
    cv::Mat screenshot;
    screenshot.create(height, width, CV_8UC4);
    GetBitmapBits(hCaptureBitmap, width * height * 4, screenshot.data);
    DeleteObject(hCaptureBitmap);
    DeleteDC(hCaptureDC);
    ReleaseDC(NULL, hDC);
    cv::Mat BGRImage;
    cv::cvtColor(screenshot, BGRImage, cv::COLOR_RGB2BGR);
    return BGRImage;
}

cv::Mat utils::image::takeScreenshot(cv::Rect region)
{
    return utils::image::takeScreenshot(region.x, region.y, region.width, region.height);
}

bool utils::image::checkPixel(int x, int y, cv::Scalar lowerThreshold, cv::Scalar upperThreshold) {
    cv::Mat screenshot = takeScreenshot(x, y, 1, 1);
    cv::Scalar pixelColor = screenshot.at<cv::Vec3b>(0, 0);
    int b = pixelColor[2];
    int g = pixelColor[1];
    int r = pixelColor[0];
    return (r >= lowerThreshold[2] && r <= upperThreshold[2] &&
        g >= lowerThreshold[1] && g <= upperThreshold[1] &&
        b >= lowerThreshold[0] && b <= upperThreshold[0]);
}

bool utils::image::checkPixel(cv::Mat& img, int x, int y, cv::Scalar lowerThreshold, cv::Scalar upperThreshold) {
    cv::Scalar pixelColor = img.at<cv::Vec3b>(y, x);
    int b = pixelColor[2];
    int g = pixelColor[1];
    int r = pixelColor[0];
    return (r >= lowerThreshold[2] && r <= upperThreshold[2] &&
        g >= lowerThreshold[1] && g <= upperThreshold[1] &&
        b >= lowerThreshold[0] && b <= upperThreshold[0]);
}

bool utils::image::withinThreshold(cv::Vec3b pix, cv::Scalar lowerThreshold, cv::Scalar upperThreshold) {
    int b = pix[2];
    int g = pix[1];
    int r = pix[0]; // 183
    return (r >= lowerThreshold[2] && r <= upperThreshold[2] &&
        g >= lowerThreshold[1] && g <= upperThreshold[1] &&
        b >= lowerThreshold[0] && b <= upperThreshold[0]);
}

MenuScreens utils::GameUI::checkPlace() {
	using namespace utils::image;
    if (checkPixel(50, 80, { 168, 197, 202 }, { 188, 217, 222 })) {
        return MenuScreens::FleaMarket;
    }
    else if (checkPixel(15, 27, { 196, 189, 188 }, { 216, 209, 208 })) {
        return MenuScreens::Traders;
    }
    else if (checkPixel(335, 20, { 224, 210, 208 }, { 244, 230, 228 })) {
        return MenuScreens::Character;
    }
    else if (checkPixel(32, 63, { 215, 245, 245 }, { 235, 9, 9 })) {
        return MenuScreens::Hideout;
    }
    else if (checkPixel(15, 1056, { 134, 147, 149 }, { 154, 167, 169 })) {
        return MenuScreens::MainMenu;
    }
    else if (checkPixel(566, 477, { 0, 0, 0 }, { 2, 3, 3 }) && checkPixel(555, 477, { 0, 0, 100 }, { 70, 70, 255 })) {
        return MenuScreens::StillInRaid;
    }
    return MenuScreens::Unknown;
}


bool utils::GameUI::gotoPlace(const MenuScreens& place) {
    using namespace utils::IO::mouse;
    MenuScreens currentPlace;
    for (int i = 0; i < 20; i++) {
        currentPlace = utils::GameUI::checkPlace();
        if (currentPlace == place)
            return true;
        int x = 0, y = 0;
        if (place == MenuScreens::Hideout) {
            x = 215;
            y = 1060;
        }
        else if (place == MenuScreens::Character) {
            x = 975;
            y = 1060;
        }
        else if (place == MenuScreens::Traders) {
            x = 1100;
            y = 1060;
        }
        else if (place == MenuScreens::FleaMarket) {
            x = 1250;
            y = 1060;
        }
        else if (place == MenuScreens::MainMenu) {
            x = 50;
            y = 1060;
        }
        else {
            sleep(1000);
            continue;
        }
        move(x, y);
        leftClick();
        sleep(1500);
    }
    return false;
}

inline const long long hrs(const int num) {
    return 1000LL * 60 * 60 * num;
}

long long utils::getTarkovTime(bool bLR) {
    constexpr int tarkovRatio = 7;

    const long long offset = hrs(3) + (bLR ? 0 : hrs(12));
    const long long tarkovMillis = (offset + (utils::getGMTTimeMs() * tarkovRatio)) % hrs(24);

    return tarkovMillis;
}
