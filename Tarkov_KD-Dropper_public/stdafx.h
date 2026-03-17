#pragma once

#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <memory>

#include <commdlg.h>
#include <commctrl.h>
#include <gdiplus.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Gdiplus.lib")