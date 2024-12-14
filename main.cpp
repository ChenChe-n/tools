#include "tools.hpp"
#include "./append/memory_allocator.hpp"
#include "./append/random.hpp"
#include "./append/time.hpp"


int main() {
	auto a1 = tools::time::ticking_time(360,tools::time::seconds_nano(1'000'000'000), tools::time::accuracy_level::high);
	a1.start();
	//timeBeginPeriod(1);


	auto now = tools::time::clock::now();

	u64 n = 0;
	tools::time::seconds_nano t(0);
	auto time1 = std::chrono::high_resolution_clock::now();
	while (true)
	{
		a1.wait();

		if (tools::time::clock::now() - now > tools::time::seconds_nano(1'000'000'000))
		{
			now = tools::time::clock::now();
			std::cout << "s:\t" << std::to_string(1'000'000'000.0 / a1.get_average_ticks()) << "\n";

			n = 0;
		}
		n++;


		//t += (std::chrono::high_resolution_clock::now() - time1);
		//std::cout << t / n << std::endl;
		//time1 = std::chrono::high_resolution_clock::now();
	}
    return 0;
}


