#pragma once
#include "../tools.hpp"


#include <stdexcept>
#include <cstdint>
#include <chrono>
#include <random>
#include <thread>
#include <functional>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <wincrypt.h>
#elif defined(__linux__) || defined(__unix__) || defined(__ANDROID__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <fcntl.h>
#include <unistd.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <Security/Security.h>
#endif


namespace tools {
	namespace random {
		// 函数声明：平台相关的实现和硬件随机数尝试
#if defined(_WIN32) || defined(_WIN64)
		u64 __get_random_windows();
#elif defined(__linux__) || defined(__unix__) || defined(__ANDROID__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
		u64 __get_random_linux();
#elif defined(__APPLE__) && defined(__MACH__)
		u64 __get_random_macos();
#else
#endif
		u64 __fallback_random();

		// 跨平台真随机数生成函数
		u64 safe_random();
	}


#ifdef tools_debug
	namespace test {
		u64 random_test();
	}
#endif
}