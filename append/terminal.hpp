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
		// ���� `operator<<` ����� `xy` ����
		std::ostream& operator<<(std::ostream& os, const xy& obj);

		// ���ƿ���̨����Ƿ���ʾ
		void ShowCursor(bool showFlag);

		// ö�ٶ������̨��ɫ����
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

		// ������ƶ���ָ��λ�� (x, y)
		void goto_xy(i32 x, i32 y);

		// ��տ���̨
		i32 clear();

		// ���ÿ���̨�ı�ǰ��ɫ
		void set_text_color(Color color);

		// ���ÿ���̨������ɫ
		void set_background_color(Color color);

		// ���ÿ���̨��ɫ
		void reset_color();

		// ��ȡ��ǰ�������
		xy get_xy();

		// ��ȡ�ն˴�С
		xy get_size();
	}



#ifdef tools_debug
	namespace test
	{
		// �����ն���ع���
		void test_terminal_functions();
	}
#endif
}