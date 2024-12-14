#pragma once
// tools.hpp
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma execution_character_set("utf-8")
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#endif


#include <cstdio>
#include <iostream>

#include <cstring>
#include <string>

#include <sstream>
#include <fstream>
#include <filesystem>

#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

#include <future>
#include <thread>
#include <atomic>
#include <limits>

#include<exception>

#undef max
#undef min



// 是否新启用测试模块
#define tools_debug 1

// Visual Studio(MSVC)
// 在 Visual Studio 中，需要在项目属性中设置字符集为 UTF - 8：
//
// 右键点击项目，选择 属性。
// 进入 配置属性->常规。
// 将 字符集 选项设置为 使用 Unicode 字符集。
// 在 配置属性->C/C++->命令行中，添加编译选项 /utf-8，确保 MSVC 以 UTF-8 编码解释源代码文件。
// 注意在导入此头文件和修改编译选项后，控制台输入输出获取的都为UTF-8字符串
typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef uint8_t		u8;
typedef	uint8_t		byte;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef float		f32;
typedef double		f64;


namespace tools
{
	namespace local
	{
		class Utf8Console
		{
		public:
			static Utf8Console &getInstance();

			Utf8Console(const Utf8Console &) = delete;
			Utf8Console &operator=(const Utf8Console &) = delete;

		private:
			Utf8Console();

			~Utf8Console() = default;
		};

		// 在头文件中声明一个全局变量
		extern Utf8Console &utf8ConsoleInitializer;
	}
	
	namespace string
	{
		std::u16string utf8_to_utf16(const std::string &utf8_str);
		std::string utf16_to_utf8(const std::u16string &utf16_str);
		std::u32string utf8_to_utf32(const std::string &utf8_str);
		std::string utf32_to_utf8(const std::u32string &utf32_str);

		// 工具函数：从UTF-8转换为本地编码（宽字符版本）
		std::wstring utf8_to_wchar(const std::string &utf8_str);

		// 工具函数：从本地编码转换为UTF-8（宽字符版本）
		std::string wchar_to_utf8(const std::wstring &local_str);

		// 连续特殊字符过滤器  input:目标字符串 special_string:过滤字符字符串 replacement:替换目标字符串
		std::u32string filter_consecutive_special_string(const std::u32string &input, const std::vector<std::u32string> &special_strings, const std::u32string &replacement);

		// 连续特殊字符过滤器  input:目标字符串 special_chars:过滤字符 replacement:替换目标字符串
		std::u32string filter_consecutive_special_chars(const std::u32string &input, const std::u32string &special_chars, const std::u32string &replacement);

		// 按特殊字符串分割  input:目标字符串 delimiter:分隔字符
		std::vector<std::u32string> split_by_special_string(const std::u32string &input, const std::u32string &delimiter);
	}
	namespace data_container
	{
		class atomic_apin_lock {
		private:
			std::atomic<std::thread::id> thread_id = std::thread::id();  // 标记锁的持有者

		public:
			inline void lock(bool is_yield = true) {
				auto this_thread_id = std::this_thread::get_id();

				while (true) {
					std::thread::id expected = std::thread::id(); // 空闲状态

					// 尝试获取锁：如果 thread_id 为 std::thread::id()，则可以成功获取锁
					if (thread_id.compare_exchange_weak(expected, this_thread_id, std::memory_order_acquire)) {
						return; // 获取锁成功
					}

					// 如果当前线程已经持有锁，抛出异常
					if (expected == this_thread_id) {
						throw std::runtime_error("Attempt to repeatedly acquire lock by the same thread");
					}

					// 防止无意义的CPU竞争，让出控制权
					if (is_yield)
					{
						//std::this_thread::yield();
					}
				}
			}
			inline void unlock() {
				// 检查当前线程是否是锁的持有者
				if (thread_id.load(std::memory_order_acquire) != std::this_thread::get_id()) {
					throw std::runtime_error("Thread does not own the lock");
				}

				// 释放锁
				thread_id.store(std::thread::id(), std::memory_order_release);
			}
			inline bool locked_by_current_thread() {
				if (thread_id.load(std::memory_order_acquire) == std::this_thread::get_id())
				{
					return true;
				}
				return false;
			}

			inline bool is_lock() {
				if (thread_id.load(std::memory_order_acquire) == std::thread::id())
				{
					return false;
				}
				return true;
			}

			// 用于 RAII 风格的自动加锁/解锁
			class auto_lock {
			private:
				atomic_apin_lock& spin_lock;
			public:
				inline auto_lock(atomic_apin_lock& lock) : spin_lock(lock) {
					spin_lock.lock();  // 加锁
				}

				inline ~auto_lock() {
					spin_lock.unlock();  // 解锁
				}
			};
		};

		template <typename T>
		class atomic_data {
		private:
			T data;                // 存储的数据
			atomic_apin_lock spin_lock;  // 自旋锁

		public:
			inline atomic_data(T init_data) : data(init_data) {}
			inline ~atomic_data() {
				if (spin_lock.is_lock()) {
					// Log the issue or attempt to unlock, but do not throw an exception
					std::cerr << "Warning: Lock is held when atomic_data is destroyed!" << std::endl;
					spin_lock.unlock();  // If safe, attempt to unlock the spinlock.
				}
			}
			// 获取锁
			inline void lock() {
				spin_lock.lock();
			}

			// 释放锁
			inline void unlock() {
				spin_lock.unlock();
			}

			// 获取数据（只有当前线程持有锁时才能访问）
			inline T& get_data() {
				// 检查当前线程是否持有锁
				if (spin_lock.locked_by_current_thread()) {
					return data;
				}
				else {
					throw std::runtime_error("Access denied: lock is not held by the current thread");
				}
			}

			// 修改数据（确保锁定）
			inline void set_data(T& new_data) {
				// 检查当前线程是否持有锁
				if (spin_lock.locked_by_current_thread()) {
					data = new_data;
				}
				else {
					throw std::runtime_error("Access denied: lock is not held by the current thread");
				}
			}

			// 修改数据（确保锁定）
			inline void set_data(T&& new_data) {
				// 检查当前线程是否持有锁
				if (spin_lock.locked_by_current_thread()) {
					data = new_data;
				}
				else {
					throw std::runtime_error("Access denied: lock is not held by the current thread");
				}
			}
		};
		template <typename T>
		class atomic_ptr {
		private:
			T* data;					// 存储的数据
			atomic_apin_lock spin_lock; // 自旋锁

		public:
			inline atomic_ptr(T init_data) : data(init_data) {}
			inline ~atomic_ptr() {
				if (spin_lock.is_lock()) {
					// Log the issue or attempt to unlock, but do not throw an exception
					std::cerr << "Warning: Lock is held when atomic_ptr is destroyed!" << std::endl;
					spin_lock.unlock();  // If safe, attempt to unlock the spinlock.
				}
			}

			// 获取锁
			inline void lock() {
				spin_lock.lock();
			}

			// 释放锁
			inline void unlock() {
				spin_lock.unlock();
			}

			// 获取数据（只有当前线程持有锁时才能访问）
			inline T* get_data() {
				// 检查当前线程是否持有锁
				if (spin_lock.locked_by_current_thread()) {
					return data;
				}
				else {
					throw std::runtime_error("Access denied: lock is not held by the current thread");
				}
			}

			// 修改数据（确保锁定）
			inline void set_data(T* new_data) {
				// 检查当前线程是否持有锁
				if (spin_lock.locked_by_current_thread()) {
					data = new_data;
				}
				else {
					throw std::runtime_error("Access denied: lock is not held by the current thread");
				}
			}
		};
	}

#ifdef tools_debug
	namespace test
	{
		enum 错误码
		{
			未知错误 = 1<<0,
			结果错误 = 1<<1,
		};

		// 开启所有测试
		void all_test();

		// 测试终端相关功能
		void test_terminal_functions();

		// 测试字符串相关功能
		void test_string_functions();

		// 测试文件操作相关功能
		///void test_file_functions();

	}
#endif
}


#ifdef _WIN32
#define utf8_to_local(utf8_str) tools::string::utf8_to_wchar(utf8_str) // Windows 平台，调用 utf8_to_wchar 函数
#else
#define utf8_to_local(utf8_str) utf8_str // 非 Windows 平台，直接返回原始 UTF-8 字符串
#endif