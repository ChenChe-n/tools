#include "random.hpp"

namespace tools {
	namespace random {
#if defined(_WIN32) || defined(_WIN64)
		// Windows 平台随机数生成实现
		u64 __get_random_windows() {
			HCRYPTPROV hProvider = 0;
			u64 random_value = 0;

			// 使用 CryptAcquireContext 获取加密服务提供者
			if (!CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
				throw std::runtime_error("CryptAcquireContext failed");
			}

			// 使用 CryptGenRandom 生成随机数
			if (!CryptGenRandom(hProvider, sizeof(random_value), reinterpret_cast<BYTE*>(&random_value))) {
				CryptReleaseContext(hProvider, 0);
				throw std::runtime_error("CryptGenRandom failed");
			}

			// 释放加密服务提供者
			CryptReleaseContext(hProvider, 0);
			return random_value;
		}
#endif


#if defined(__linux__) || defined(__unix__) || defined(__ANDROID__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
		// Linux/Unix 平台随机数生成实现
		u64 __get_random_linux() {
			u64 random_value = 0;
			// 打开 /dev/urandom 文件以获取随机数
			int fd = open("/dev/urandom", O_RDONLY);
			if (fd == -1) {
				throw std::runtime_error("Failed to open /dev/urandom");
			}

			// 从 /dev/urandom 读取随机数
			ssize_t result = read(fd, &random_value, sizeof(random_value));
			close(fd); // 确保文件描述符被关闭

			if (result != sizeof(random_value)) {
				throw std::runtime_error("Failed to read from /dev/urandom");
			}
			return random_value;
		}
#endif


#if defined(__APPLE__) && defined(__MACH__)
		// macOS 平台随机数生成实现
		u64 __get_random_macos() {
			u64 random_value = 0;
			// 使用 SecRandomCopyBytes 获取随机数
			if (SecRandomCopyBytes(kSecRandomDefault, sizeof(random_value), reinterpret_cast<uint8_t*>(&random_value)) != errSecSuccess) {
				throw std::runtime_error("SecRandomCopyBytes failed");
			}
			return random_value;
		}
#endif


		// 回退随机数生成方案，适用于不支持的系统或硬件失败的情况
		u64 __fallback_random() {
			// 使用多个熵源生成种子：随机设备、时间、线程ID
			std::random_device rd;
			u64 seed = std::hash<u64>{}
				(rd() ^
				std::hash<u64>{}(std::chrono::high_resolution_clock::now().time_since_epoch().count()) ^
				(std::hash<std::thread::id>{}(std::this_thread::get_id())));

			// 使用 Mersenne Twister 随机数生成器作为回退方案
			std::mt19937_64 rng(seed);
			return rng();
		}


		// 跨平台真随机数生成函数
		u64 safe_random() {
			u64 random_value = 0;

			// 根据平台调用对应的随机数生成实现
#if defined(_WIN32) || defined(_WIN64)
			return __get_random_windows();
#elif defined(__linux__) || defined(__unix__) || defined(__ANDROID__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
			return __get_random_linux();
#elif defined(__APPLE__) && defined(__MACH__)
			return __get_random_macos();
#else
			return __fallback_random(); // 如果不支持以上平台，使用回退方案
#endif
		}
	}



#ifdef tools_debug
	namespace test {
		u64 random_test() {
			return 0;
		}
	}
#endif
}