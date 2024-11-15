#pragma once
//tools.hpp
#ifdef _WIN32
	#include <windows.h>
	#pragma execution_character_set("utf-8")
#endif
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <map>
#include <set>
#include <cassert>

//Visual Studio(MSVC)
//在 Visual Studio 中，需要在项目属性中设置字符集为 UTF - 8：
//
//右键点击项目，选择 属性。
//进入 配置属性->常规。
//将 字符集 选项设置为 使用 Unicode 字符集。
//在 配置属性->C/C++->命令行中，添加编译选项 /utf-8，确保 MSVC 以 UTF-8 编码解释源代码文件。
//注意在导入此头文件和修改编译选项后，控制台输入输出获取的都为UTF-8字符串

typedef char                i8;
typedef unsigned char       u8;
typedef short               i16;
typedef unsigned short      u16;
typedef int                 i32;
typedef unsigned int        u32;
typedef long long           i64;
typedef unsigned long long  u64;

typedef float               f32;
typedef double              f64;



namespace tools {
	namespace local {
#ifdef _WIN32 
	#define utf8_to_local(utf8_str) utf8_to_wchar(utf8_str)  // Windows 平台，调用 utf8_to_wchar 函数
#else
	#define utf8_to_local(utf8_str) utf8_str  // 非 Windows 平台，直接返回原始 UTF-8 字符串
#endif


		class Utf8Console {
		public:
			static Utf8Console& getInstance();

			Utf8Console(const Utf8Console&) = delete;
			Utf8Console& operator=(const Utf8Console&) = delete;

		private:
			Utf8Console();

			~Utf8Console() = default;
		};

		// 在头文件中声明一个全局变量
		extern Utf8Console& utf8ConsoleInitializer;
	}
	namespace terminal {
		class xy
		{
		public:
			int x;
			int y;
			xy(int x_,int y_):x(x_),y(y_){}
			~xy() = default;
		};
		// 重载 `operator<<` 来输出 `xy` 对象
		std::ostream& operator<<(std::ostream& os, const xy& obj);

		//控制控制台光标是否显示
		void ShowCursor(bool showFlag);

		// 枚举定义控制台颜色代码
		enum Color : int {
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
		void goto_xy(int x, int y);

		// 清空控制台
		void clear();

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
	namespace string {
		std::u16string utf8_to_utf16(const std::string& utf8_str);
		std::string utf16_to_utf8(const std::u16string& utf16_str);
		std::u32string utf8_to_utf32(const std::string& utf8_str);
		std::string utf32_to_utf8(const std::u32string& utf32_str);


		// 工具函数：从UTF-8转换为本地编码（宽字符版本）
		std::wstring utf8_to_wchar(const std::string& utf8_str);

		// 工具函数：从本地编码转换为UTF-8（宽字符版本）
		std::string wchar_to_utf8(const std::wstring& local_str);

		//连续特殊字符过滤器  input:目标字符串 special_string:过滤字符字符串 replacement:替换目标字符串
		std::u32string filter_consecutive_special_string(const std::u32string& input, const std::vector<std::u32string>& special_strings, const std::u32string& replacement);

		//连续特殊字符过滤器  input:目标字符串 special_chars:过滤字符 replacement:替换目标字符串
		std::u32string filter_consecutive_special_chars(const std::u32string& input, const std::u32string& special_chars, const std::u32string& replacement);

		//按特殊字符串分割  input:目标字符串 delimiter:分隔字符
		std::vector<std::u32string> split_by_special_string(const std::u32string& input, const std::u32string& delimiter);
	}
	namespace file {

		class ram_byte {
		public:
			char* data = nullptr;
			size_t     size = 0;
			ram_byte() = default;
			ram_byte(char* data_, size_t size_);
			~ram_byte() = default;
			ram_byte& operator=(const ram_byte& other);
			char* get_data();
			size_t  get_size();

			// 获取从 start_byte 开始的对象
			template<typename T>
			T* get_obj(size_t start_byte) {
				// 检查是否有足够的字节来表示 T 类型的对象
				if (start_byte + sizeof(T) <= size) {
					// 使用 reinterpret_cast 将起始字节解释为类型 T 的对象
					return reinterpret_cast<T*>(data + start_byte);
				}
				return nullptr; // 如果不够字节，返回 nullptr
			}
		};

		// 读取文件内容为字符串
		std::string read_string(const std::string& filename);

		// 将字符串内容写入文件
		bool write_string(const std::string& filename, const std::string& content);

		//读取文件内容为字节数组
		std::vector<char> read_vector_byte(const std::string& filename);

		// 将字节数组内容写入文件
		bool write_vector_byte(const std::string& filename, const std::vector<char>& data);

		// 读取文件内容为字节数组（并行读取）
		std::vector<char> read_fast_vector_byte(const std::string& filename, double concurrency_multiplier = 4.0);

		// 将字节数组写入文件（并行写入）
		bool write_fast_vector_byte(const std::string& filename, const std::vector<char>& data, double concurrency_multiplier = 4.0);




		//读取文件内容为字节数组
		ram_byte read_byte(const std::string& filename);

		// 将字节数组内容写入文件
		bool write_byte(const std::string& filename, const char* data, u64 length);

		// 读取文件内容为字节数组（并行读取）
		ram_byte read_fast_byte(const std::string& filename, double concurrency_multiplier = 4.0);

		// 将字节数组写入文件（并行写入）
		bool write_fast_byte(const std::string& filename, const char* data, uint64_t length, double concurrency_multiplier = 4.0);
	}

	namespace test {
		// 开启所有测试
		void all_test();

		// 测试终端相关功能
		void test_terminal_functions();

		// 测试字符串相关功能
		void test_string_functions();

		// 测试文件操作相关功能
		void test_file_functions();

	}
}