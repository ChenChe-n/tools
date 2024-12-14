#include "time.hpp"

namespace tools {
	namespace time {
		inline constexpr auto time_point__ = time_point();

#ifdef _WIN32
		// 初始化windows平台终端为utf-8
		windows_high_precision_clock& windows_high_precision_clock::getInstance()
		{
			static windows_high_precision_clock instance;
			return instance;
		}
		windows_high_precision_clock::windows_high_precision_clock()
		{
			timeBeginPeriod(1);
		}
		// 创建一个全局对象，这会自动调用构造函数
		windows_high_precision_clock& utf8ConsoleInitializer = windows_high_precision_clock::getInstance();
#endif




		timepiece::timepiece()
			: start_time(), pause_time(), pause_duration(seconds_nano::zero())
		{
		}

		timepiece::~timepiece()
		{

		}

		time_point timepiece::start()
		{
			auto now_time = clock::now();
			//未被初始化
			if (start_time == time_point__)
			{
				start_time = now_time;
				return now_time;
			}
			//以被暂停，继续计时
			if (pause_time != time_point__)
			{
				pause_duration += now_time - pause_time;
				pause_time = time_point__;
				return now_time;
			}
			//错误调用返回基点时间
			return time_point__;
		}

		time_point timepiece::pause()
		{
			auto now_time = clock::now();
			//未开始计时
			if (start_time == time_point__)
			{
				return time_point__;
			}
			//未被暂停，开始暂停
			if (pause_time == time_point__)
			{
				pause_time = now_time;
				return now_time;
			}

			return time_point__;
		}

		time_point timepiece::reset()
		{
			auto now_time = clock::now();

			start_time = time_point__;
			pause_time = time_point__;
			pause_duration = seconds_nano::zero();

			return now_time;
		}

		time_point timepiece::get_start_time()
		{
			return start_time;
		}

		time_point timepiece::get_pause_time()
		{
			return pause_time;
		}

		seconds_nano timepiece::get_duration_nanoseconds()
		{
			auto now_time = clock::now();
			seconds_nano out_duration;
			//是否正在暂停
			if (pause_time != time_point__)
			{
				// 如果处于暂停状态，则时间差不包括从暂停开始到当前的时间
				out_duration = (pause_time - start_time) - pause_duration;
			}
			else {
				// 否则，直接计算当前时间到开始时间的差减去总暂停时间
				out_duration = (now_time - start_time) - pause_duration;
			}
			return out_duration;
		}

		seconds_micro timepiece::get_duration_microseconds()
		{
			return duration_cast<seconds_micro>(get_duration_nanoseconds());
		}

		seconds_milli timepiece::get_duration_milliseconds()
		{
			return duration_cast<seconds_milli>(get_duration_nanoseconds());
		}

		seconds timepiece::get_duration_seconds()
		{
			return duration_cast<seconds>(get_duration_nanoseconds());
		}



		// 构造函数
		ticking_time::ticking_time(u32 ticks, seconds_nano duration, accuracy_level accuracy_level)
		{
			if (duration.count() <= 0 || ticks == 0) {
				std::cerr << "ticking_time: Invalid duration or ticks" << std::endl;
				throw std::invalid_argument("ticking_time: Invalid duration or ticks");
			}
			start_time_ = time_point__;
			duration_ = duration;
			now_ticks_ = 0;
			ticks_ = ticks;
			accuracy_level_ = accuracy_level;
		}

		ticking_time::~ticking_time()
		{
		}

		// 启动计时
		time_point ticking_time::start() {
			if (start_time_ == time_point()) {
				start_time_ = clock::now();
				return start_time_;
			}
			return time_point__;
		}

		// 停止计时
		time_point ticking_time::stop() {
			if (start_time_ != time_point()) {
				auto stop_time = clock::now();
				start_time_ = time_point__; // 重置起始时间
				return stop_time;
			}
			return time_point__;
		}

		// 重置计时
		time_point ticking_time::reset() {
			start_time_ = time_point();
			now_ticks_ = 0;
			return clock::now();
		}

		// 重置计时（带参数）
		time_point ticking_time::reset(u32 ticks, seconds_nano duration, accuracy_level accuracy_level) {
			if (duration.count() <= 0 || ticks == 0) {
				std::cerr << "ticking_time::reset: Invalid duration or ticks" << std::endl;
				throw std::invalid_argument("ticking_time::reset: Invalid duration or ticks");
			}
			start_time_ = time_point();
			duration_ = duration;
			ticks_ = ticks;
			accuracy_level_ = accuracy_level;
			now_ticks_ = 0;
			return clock::now();
		}

		// 等待下一个 tick
		time_point ticking_time::wait() {
			auto now_time = clock::now();
			if (start_time_ == time_point__) {
				return time_point__; // 未启动计时
			}

			auto elapsed_time = now_time - start_time_; // 已运行时间
			auto theoretical_tick_count = (u64)(elapsed_time * ticks_ / duration_);

			if (theoretical_tick_count < now_ticks_) {
				// 如果理论 tick 还未达到当前 tick，等待下一个 tick 时间点
				sleep_until((start_time_ + ((duration_ / ticks_) * (now_ticks_ + 2))), accuracy_level_);
			}
			else {
				// 如果理论 tick 已超过或等于当前 tick，不等待
				//
				//now_ticks_ = theoretical_tick_count; // 同步到理论值
			}

			now_ticks_++;
			return clock::now();
		}

		// 获取平均每 tick 的时间
		u64 ticking_time::get_average_ticks() {
			if (start_time_ == time_point__) {
				return 0; // 未启动
			}

			auto now_time = clock::now();
			auto elapsed_time = now_time - start_time_; // 总运行时间
			return now_ticks_ > 0 ? (elapsed_time.count() / now_ticks_) : 0; // 平均时间
		}



		void sleep_until(time_point time_point, accuracy_level accuracy_level_) {
			auto now_time = clock::now();

			if (time_point <= now_time) return; // 保护机制：目标时间点已过

			switch (accuracy_level_) {
			case accuracy_level::lower:
				// 低精度：使用标准库的 sleep_until
				std::this_thread::sleep_until(time_point);
				break;

			case accuracy_level::high: {
				// 高精度：部分时间交给 sleep_for，其余部分短暂等待
				auto remaining_time = time_point - now_time;

				if (remaining_time.count() > 2'000'000) {
					// 大于 2ms 的部分交给 sleep_for
					std::this_thread::sleep_for(remaining_time - seconds_nano(2'000'000));
				}

				// 最后的 2ms 使用主动循环
				while (clock::now() < time_point) {
					std::this_thread::yield(); // 让出 CPU 时间片
				}
				break;
			}

			case accuracy_level::extre:
				// 高精度：部分时间交给 sleep_for，其余部分短暂等待
				auto remaining_time = time_point - now_time;
				if (remaining_time.count() > 2'000'000) {
					// 大于 2ms 的部分交给 sleep_for
					std::this_thread::sleep_for(remaining_time - seconds_nano(2'000'000));
				}

				// 最后的 2ms 使用主动循环
				// 极高精度：完全自旋等待，不让出 CPU
				while (clock::now() < time_point) {
					// 自旋等待：不让出 CPU
					std::atomic_signal_fence(std::memory_order_acquire); // 防止编译器优化
				}
				break;
			}
		}

		void sleep_for(seconds_nano duration, accuracy_level accuracy_level_)
		{
			if (duration.count() <= 0) return; // 保护机制：负数或 0 持续时间直接返回

			// 转换为目标时间点，然后调用 sleep_until
			auto time_point = std::chrono::high_resolution_clock::now() + duration;
			sleep_until(time_point, accuracy_level_);
		}
	}


#ifdef tools_debug
	namespace test {

	}
#endif
}