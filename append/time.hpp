#pragma once
#include "../tools.hpp"
#include <chrono>


namespace tools {
	namespace time {
		using clock = std::chrono::steady_clock;
		using time_point = std::chrono::steady_clock::time_point;
		using seconds_nano = std::chrono::duration<u64, std::nano>;
		using seconds_micro = std::chrono::duration<double, std::micro>;
		using seconds_milli = std::chrono::duration<double, std::milli>;
		using seconds = std::chrono::duration<double>;


#ifdef _WIN32
		// Windows 平台的函数声明
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
		class windows_high_precision_clock
		{
		public:
			static windows_high_precision_clock& getInstance();

			windows_high_precision_clock(const windows_high_precision_clock&) = delete;
			windows_high_precision_clock& operator=(const windows_high_precision_clock&) = delete;

		private:
			windows_high_precision_clock();

			~windows_high_precision_clock() = default;
		};

		// 在头文件中声明一个全局变量
		extern windows_high_precision_clock& utf8ConsoleInitializer;
#endif




		//如果时间返回 std::chrono::high_resolution_clock::time_point() 的默认值表示错误情况，
		//reset后需要手动调用start启动，pause后需要手动调用start启动
		class timepiece
		{
		public:
			timepiece();
			~timepiece();
			time_point		start();
			time_point		pause();
			time_point		reset();

			time_point		get_start_time();
			time_point		get_pause_time();
			seconds_nano	get_duration_nanoseconds();
			seconds_micro	get_duration_microseconds();
			seconds_milli	get_duration_milliseconds();
			seconds			get_duration_seconds();
		private:
			time_point		start_time;
			time_point		pause_time;
			seconds_nano	pause_duration;
		};

		enum class accuracy_level :u8 {
			lower, // 低精度：`std::this_thread` 提供的默认机制
			high,  // 高精度：短暂主动等待实现
			extre  // 极高精度：CPU一直自旋等待
		};

		// 基于 `time_point` 的通用睡眠函数
		void sleep_until(time_point time_point, accuracy_level accuracy_level_ = accuracy_level::lower);

		// 基于 `duration` 的睡眠函数（内部转换为 `time_point`）
		void sleep_for(seconds_nano duration, accuracy_level accuracy_level_ = accuracy_level::lower);

		class ticking_time
		{
		public:
			ticking_time(u32 ticks, seconds_nano duration = seconds_nano(1'000'000'000), accuracy_level accuracy_level = accuracy_level::lower);
			~ticking_time();
			time_point	start();
			time_point	stop();
			time_point	reset();
			time_point	reset(u32 ticks, seconds_nano duration = seconds_nano(1'000'000'000), accuracy_level accuracy_level = accuracy_level::lower);
			time_point  wait();
			u64			get_average_ticks();
		private:
			time_point		start_time_;
			//seconds_nano	duration_tick;
			seconds_nano	duration_;
			u64				now_ticks_ = 0;
			u32				ticks_;
			accuracy_level	accuracy_level_;

		};



	}




#ifdef tools_debug
	namespace test {

	}
#endif
}