#include "terminal.hpp"

namespace tools {
	namespace terminal
	{
		// ���� `operator<<` ����� `xy` ����
		std::ostream& operator<<(std::ostream& os, const xy& obj)
		{
			os << "xy(" << obj.x << ", " << obj.y << ")";
			return os; // ���������Ա���ʽ����
		}

		void ShowCursor(bool showFlag)
		{
#ifdef _WIN32
			// Windows ϵͳ��ʹ�� Windows API ���ƹ��
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_CURSOR_INFO cursorInfo;
			GetConsoleCursorInfo(hOut, &cursorInfo);
			cursorInfo.bVisible = showFlag;
			SetConsoleCursorInfo(hOut, &cursorInfo);
#else
			// �� UNIX ϵͳ��ʹ�� ANSI ת�����п��ƹ��
			if (showFlag)
			{
				std::cout << "\033[?25h"; // ��ʾ���
			}
			else
			{
				std::cout << "\033[?25l"; // ���ع��
			}
			std::cout.flush(); // ȷ������ˢ��������ն�
#endif
		}

		// ������ƶ���ָ��λ�� (x, y)  �����1��ʼ��
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

		// ��տ���̨
		i32 clear()
		{
#ifdef _WIN32
			return system("cls");
#else
			return system("clear");
#endif
		}

		// ���ÿ���̨�ı�ǰ��ɫ
		void set_text_color(Color color)
		{
			std::cout << "\033[" << color + 30 << "m";
		}

		// ���ÿ���̨������ɫ
		void set_background_color(Color color)
		{
			std::cout << "\033[" << color + 40 << "m";
		}

		// ���ÿ���̨��ɫ
		void reset_color()
		{
			std::cout << "\033[0m";
		}

		// ��ȡ��ǰ�������
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

			// ��ȡ�����ݵ�ǰ�ն�����
			if (tcgetattr(STDIN_FILENO, &old_termios) == -1)
			{
				throw std::runtime_error("Failed to get terminal attributes.");
			}

			new_termios = old_termios;
			new_termios.c_lflag &= ~(ICANON | ECHO); // �رչ淶ģʽ�ͻ���
			if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1)
			{
				throw std::runtime_error("Failed to set terminal attributes.");
			}

			// ������λ��
			std::printf("\033[6n");
			std::fflush(stdout);

			// ���񷵻صĹ��λ������
			char buffer[32] = { 0 };
			if (read(STDIN_FILENO, buffer, sizeof(buffer) - 1) > 0)
			{
				i32 x, y;
				if (sscanf(buffer, "\033[%d;%dR", &y, &x) == 2)
				{
					// �ָ��ն�����
					tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
					return xy(x, y);
				}
			}

			// �ָ��ն����ԣ����ʧ�ܣ�ȷ���ع����ԣ�
			tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
			throw std::runtime_error("Failed to parse cursor position.");
#endif
		}

		// ��ȡ�ն˴�С
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
		// �����ն���ع���
		void test_terminal_functions()
		{
			using namespace tools::terminal;

			std::cout << "���ڲ����ն���ع���..." << std::endl;

			// ��տ���̨
			clear();
			std::cout << "����̨����ա�" << std::endl;

			// �����ı���ɫ�ͱ�����ɫ
			set_text_color(Color::Red);			// �����ı�Ϊ��ɫ
			set_background_color(Color::White); // ���ñ���Ϊ��ɫ
			std::cout << "�������Ӧ����ʾΪ��ɫ���֡���ɫ������" << std::endl;
			reset_color(); // ������ɫ

			// ���Թ���ƶ�����
			goto_xy(10, 10);
			std::cout << "����ƶ���λ�� (10, 10) ����" << std::endl;

			// ��ȡ��ǰ���λ��
			xy pos = get_xy();
			std::cout << "��ǰ���λ��Ϊ: (" << pos.x << ", " << pos.y << ")" << std::endl;

			// ��ʾ/���ع��
			std::cout << "�����ʾ״̬���л���" << std::endl;
			ShowCursor(false);									  // ���ع��
			std::this_thread::sleep_for(std::chrono::seconds(5)); // ��ͣ 1 ��
			ShowCursor(true);									  // ��ʾ���

			// ��ȡ�ն˴�С
			xy size = get_size();
			std::cout << "��ǰ�ն˴�СΪ: (" << size.x << ", " << size.y << ")" << std::endl;
		}
	}
#endif
}