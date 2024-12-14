#pragma once
#include "../tools.hpp"

namespace tools {
	namespace terminal
	{
		class xy
		{
		public:
			i32 x;
			i32 y;
			xy(i32 x_, i32 y_) : x(x_), y(y_) {}
			~xy() = default;
		};
		// 重载 `operator<<` 来输出 `xy` 对象
		std::ostream& operator<<(std::ostream& os, const xy& obj);

		// 控制控制台光标是否显示
		void ShowCursor(bool showFlag);

		// 枚举定义控制台颜色代码
		enum Color
		{
			Black = 0,
			Red,
			Green,
			Yellow,
			Blue,
			Magenta,
			Cyan,
			White,
			Default
		};

		// 将光标移动到指定位置 (x, y)
		void goto_xy(i32 x, i32 y);

		// 清空控制台
		i32 clear();

		// 设置控制台文本前景色
		void set_text_color(Color color);

		// 设置控制台背景颜色
		void set_background_color(Color color);

		// 重置控制台颜色
		void reset_color();

		// 获取当前光标坐标
		xy get_xy();

		// 获取终端大小
		xy get_size();
	}



#ifdef tools_debug
	namespace test
	{
		// 测试终端相关功能
		void test_terminal_functions();
	}
#endif
}