#include "terminal.hpp"

namespace tools {
	namespace terminal
	{
		// 重载 `operator<<` 来输出 `xy` 对象
		std::ostream& operator<<(std::ostream& os, const xy& obj)
		{
			os << "xy(" << obj.x << ", " << obj.y << ")";
			return os; // 返回流，以便链式调用
		}

		void ShowCursor(bool showFlag)
		{
#ifdef _WIN32
			// Windows 系统：使用 Windows API 控制光标
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_CURSOR_INFO cursorInfo;
			GetConsoleCursorInfo(hOut, &cursorInfo);
			cursorInfo.bVisible = showFlag;
			SetConsoleCursorInfo(hOut, &cursorInfo);
#else
			// 类 UNIX 系统：使用 ANSI 转义序列控制光标
			if (showFlag)
			{
				std::cout << "\033[?25h"; // 显示光标
			}
			else
			{
				std::cout << "\033[?25l"; // 隐藏光标
			}
			std::cout.flush(); // 确保立即刷新输出到终端
#endif
		}

		// 将光标移动到指定位置 (x, y)  坐标从1开始算
		void goto_xy(i32 x, i32 y)
		{
#ifdef _WIN32
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
			SetConsoleCursorPosition(hOut, coord);
#else
			std::cout << "\033[" << y << ";" << x << "H";
#endif
		}

		// 清空控制台
		i32 clear()
		{
#ifdef _WIN32
			return system("cls");
#else
			return system("clear");
#endif
		}

		// 设置控制台文本前景色
		void set_text_color(Color color)
		{
			std::cout << "\033[" << color + 30 << "m";
		}

		// 设置控制台背景颜色
		void set_background_color(Color color)
		{
			std::cout << "\033[" << color + 40 << "m";
		}

		// 重置控制台颜色
		void reset_color()
		{
			std::cout << "\033[0m";
		}

		// 获取当前光标坐标
		xy get_xy()
		{
#ifdef _WIN32
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
			{
				i32 x = csbi.dwCursorPosition.X + 1; // Convert to 1-based index
				i32 y = csbi.dwCursorPosition.Y + 1;
				return xy(x, y);
			}
			else
			{
				throw std::runtime_error("Failed to get cursor position.");
			}
#else
			termios old_termios, new_termios;

			// 获取并备份当前终端属性
			if (tcgetattr(STDIN_FILENO, &old_termios) == -1)
			{
				throw std::runtime_error("Failed to get terminal attributes.");
			}

			new_termios = old_termios;
			new_termios.c_lflag &= ~(ICANON | ECHO); // 关闭规范模式和回显
			if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1)
			{
				throw std::runtime_error("Failed to set terminal attributes.");
			}

			// 请求光标位置
			std::printf("\033[6n");
			std::fflush(stdout);

			// 捕获返回的光标位置数据
			char buffer[32] = { 0 };
			if (read(STDIN_FILENO, buffer, sizeof(buffer) - 1) > 0)
			{
				i32 x, y;
				if (sscanf(buffer, "\033[%d;%dR", &y, &x) == 2)
				{
					// 恢复终端属性
					tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
					return xy(x, y);
				}
			}

			// 恢复终端属性（如果失败，确保回滚属性）
			tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
			throw std::runtime_error("Failed to parse cursor position.");
#endif
		}

		// 获取终端大小
		xy get_size()
		{
#ifdef _WIN32
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
			{
				i32 width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
				i32 height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
				return xy(width, height);
			}
			else
			{
				throw std::runtime_error("Failed to get console size.");
			}
#else
			struct winsize w;
			if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
			{
				throw std::runtime_error("Failed to get terminal size.");
			}
#include <unistd.h>
			return xy(w.ws_col, w.ws_row);
#endif
		}
	}

#ifdef tools_debug
	namespace test
	{
		// 测试终端相关功能
		void test_terminal_functions()
		{
			using namespace tools::terminal;

			std::cout << "正在测试终端相关功能..." << std::endl;

			// 清空控制台
			clear();
			std::cout << "控制台已清空。" << std::endl;

			// 设置文本颜色和背景颜色
			set_text_color(Color::Red);			// 设置文本为红色
			set_background_color(Color::White); // 设置背景为白色
			std::cout << "这段文字应该显示为红色文字、白色背景。" << std::endl;
			reset_color(); // 重置颜色

			// 测试光标移动功能
			goto_xy(10, 10);
			std::cout << "光标移动到位置 (10, 10) 处。" << std::endl;

			// 获取当前光标位置
			xy pos = get_xy();
			std::cout << "当前光标位置为: (" << pos.x << ", " << pos.y << ")" << std::endl;

			// 显示/隐藏光标
			std::cout << "光标显示状态已切换。" << std::endl;
			ShowCursor(false);									  // 隐藏光标
			std::this_thread::sleep_for(std::chrono::seconds(5)); // 暂停 1 秒
			ShowCursor(true);									  // 显示光标

			// 获取终端大小
			xy size = get_size();
			std::cout << "当前终端大小为: (" << size.x << ", " << size.y << ")" << std::endl;
		}
	}
#endif
}