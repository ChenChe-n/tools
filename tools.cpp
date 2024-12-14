#include "tools.hpp"

namespace tools
{
	namespace local
	{
		// 初始化windows平台终端为utf-8
		Utf8Console &Utf8Console::getInstance()
		{
			static Utf8Console instance;
			return instance;
		}
		Utf8Console::Utf8Console()
		{
#ifdef _WIN32
			SetConsoleOutputCP(CP_UTF8); // 设置输出编码为UTF-8
			SetConsoleCP(CP_UTF8);		 // 设置输入编码为UTF-8
#endif
		}
		// 创建一个全局对象，这会自动调用构造函数
		Utf8Console &utf8ConsoleInitializer = Utf8Console::getInstance();
	}

	namespace string
	{
		std::u16string utf8_to_utf16(const std::string &utf8_str)
		{
			std::u16string utf16_str;
			size_t i = 0;
			while (i < utf8_str.size())
			{
				unsigned char c = utf8_str[i];

				if (c <= 0x7F)
				{ // 1 byte character
					utf16_str.push_back(c);
					++i;
				}
				else if (c >= 0xC2 && c <= 0xDF)
				{ // 2 bytes character
					if (i + 1 >= utf8_str.size())
						throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					utf16_str.push_back(((c & 0x1F) << 6) | (c2 & 0x3F));
					i += 2;
				}
				else if (c >= 0xE0 && c <= 0xEF)
				{ // 3 bytes character
					if (i + 2 >= utf8_str.size())
						throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					unsigned char c3 = utf8_str[i + 2];
					utf16_str.push_back(((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F));
					i += 3;
				}
				else if (c >= 0xF0 && c <= 0xF4)
				{ // 4 bytes character
					if (i + 3 >= utf8_str.size())
						throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					unsigned char c3 = utf8_str[i + 2];
					unsigned char c4 = utf8_str[i + 3];
					unsigned int codepoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
					codepoint -= 0x10000;
					utf16_str.push_back(0xD800 | (codepoint >> 10));   // High surrogate
					utf16_str.push_back(0xDC00 | (codepoint & 0x3FF)); // Low surrogate
					i += 4;
				}
				else
				{
					throw std::invalid_argument("Invalid UTF-8 sequence");
				}
			}
			return utf16_str;
		}
		std::string utf16_to_utf8(const std::u16string &utf16_str)
		{
			std::string utf8_str;
			for (size_t i = 0; i < utf16_str.size(); ++i)
			{
				char16_t c = utf16_str[i];

				if (c <= 0x7F)
				{
					utf8_str.push_back(static_cast<char>(c)); // 1 byte character
				}
				else if (c <= 0x7FF)
				{
					utf8_str.push_back(static_cast<char>(0xC0 | (c >> 6)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F))); // 2 byte character
				}
				else if (c >= 0xD800 && c <= 0xDBFF)
				{ // High surrogate (start of surrogate pair)
					if (i + 1 < utf16_str.size())
					{
						uint16_t low = utf16_str[i + 1];
						if (low >= 0xDC00 && low <= 0xDFFF)
						{ // Valid low surrogate
							uint32_t codepoint = ((c - 0xD800) << 10) | (low - 0xDC00);
							codepoint += 0x10000;
							utf8_str.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
							utf8_str.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
							utf8_str.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
							utf8_str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F))); // 4 byte character
							++i;
							continue;
						}
					}
					throw std::invalid_argument("Invalid UTF-16 surrogate pair");
				}
				else
				{
					utf8_str.push_back(static_cast<char>(0xE0 | (c >> 12)));
					utf8_str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F))); // 3 byte character
				}
			}
			return utf8_str;
		}
		std::u32string utf8_to_utf32(const std::string &utf8_str)
		{
			std::u32string utf32_str;
			size_t i = 0;
			while (i < utf8_str.size())
			{
				unsigned char c = utf8_str[i];

				if (c <= 0x7F)
				{ // 1 byte character
					utf32_str.push_back(c);
					++i;
				}
				else if (c >= 0xC2 && c <= 0xDF)
				{ // 2 bytes character
					if (i + 1 >= utf8_str.size())
						throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					utf32_str.push_back(((c & 0x1F) << 6) | (c2 & 0x3F));
					i += 2;
				}
				else if (c >= 0xE0 && c <= 0xEF)
				{ // 3 bytes character
					if (i + 2 >= utf8_str.size())
						throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					unsigned char c3 = utf8_str[i + 2];
					utf32_str.push_back(((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F));
					i += 3;
				}
				else if (c >= 0xF0 && c <= 0xF4)
				{ // 4 bytes character
					if (i + 3 >= utf8_str.size())
						throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					unsigned char c3 = utf8_str[i + 2];
					unsigned char c4 = utf8_str[i + 3];
					unsigned int codepoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
					codepoint -= 0x10000;
					utf32_str.push_back(codepoint); // UTF-32
					i += 4;
				}
				else
				{
					throw std::invalid_argument("Invalid UTF-8 sequence");
				}
			}
			return utf32_str;
		}
		std::string utf32_to_utf8(const std::u32string &utf32_str)
		{
			std::string utf8_str;
			for (size_t i = 0; i < utf32_str.size(); ++i)
			{
				uint32_t c = utf32_str[i];

				if (c <= 0x7F)
				{
					utf8_str.push_back(static_cast<char>(c)); // 1 byte character
				}
				else if (c <= 0x7FF)
				{
					utf8_str.push_back(static_cast<char>(0xC0 | (c >> 6)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F))); // 2 byte character
				}
				else if (c <= 0xFFFF)
				{
					utf8_str.push_back(static_cast<char>(0xE0 | (c >> 12)));
					utf8_str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F))); // 3 byte character
				}
				else if (c <= 0x10FFFF)
				{
					utf8_str.push_back(static_cast<char>(0xF0 | (c >> 18)));
					utf8_str.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
					utf8_str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F))); // 4 byte character
				}
				else
				{
					throw std::invalid_argument("Invalid UTF-32 codepoint");
				}
			}
			return utf8_str;
		}

		std::wstring utf8_to_wchar(const std::string &utf8_str)
		{
#ifdef _WIN32
			// Windows: Convert UTF-8 to UTF-16 (std::wstring)
			std::wstring wstr;
			i32 len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0); // Get required buffer size
			wstr.resize(len);
			MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wstr[0], len); // Perform conversion
			return wstr;
#else
			// Linux/macOS: Convert UTF-8 to UTF-32 (std::wstring)
			std::wstring wstr;
			size_t i = 0;
			while (i < utf8_str.size())
			{
				uint32_t code_point = 0;
				unsigned char c = utf8_str[i];
				if (c <= 0x7F)
				{
					code_point = c;
					i++;
				}
				else if (c <= 0xDF)
				{
					code_point = (c & 0x1F) << 6;
					code_point |= (utf8_str[++i] & 0x3F);
					i++;
				}
				else if (c <= 0xEF)
				{
					code_point = (c & 0x0F) << 12;
					code_point |= (utf8_str[++i] & 0x3F) << 6;
					code_point |= (utf8_str[++i] & 0x3F);
					i++;
				}
				else if (c <= 0xF7)
				{
					code_point = (c & 0x07) << 18;
					code_point |= (utf8_str[++i] & 0x3F) << 12;
					code_point |= (utf8_str[++i] & 0x3F) << 6;
					code_point |= (utf8_str[++i] & 0x3F);
					i++;
				}
				else
				{
					throw std::invalid_argument("Invalid UTF-8 byte sequence");
				}

				// Directly append to wstr (UTF-32)
				wstr.push_back(static_cast<wchar_t>(code_point));
			}
			return wstr;
#endif
		}
		std::string wchar_to_utf8(const std::wstring &wchar_str)
		{
#ifdef _WIN32
			// Windows: Convert UTF-16 (std::wstring) to UTF-8
			std::string utf8_str;
			int len = WideCharToMultiByte(CP_UTF8, 0, wchar_str.c_str(), -1, nullptr, 0, nullptr, nullptr); // Get required buffer size
			utf8_str.resize(len);
			WideCharToMultiByte(CP_UTF8, 0, wchar_str.c_str(), -1, &utf8_str[0], len, nullptr, nullptr); // Perform conversion
			return utf8_str;
#else
			// Linux/macOS: Convert UTF-32 (std::wstring) to UTF-8
			std::string utf8_str;
			for (wchar_t wc : wchar_str)
			{
				if (wc <= 0x7F)
				{
					utf8_str.push_back(static_cast<char>(wc));
				}
				else if (wc <= 0x7FF)
				{
					utf8_str.push_back(static_cast<char>((wc >> 6) | 0xC0));
					utf8_str.push_back(static_cast<char>((wc & 0x3F) | 0x80));
				}
				else if (wc <= 0xFFFF)
				{
					utf8_str.push_back(static_cast<char>((wc >> 12) | 0xE0));
					utf8_str.push_back(static_cast<char>(((wc >> 6) & 0x3F) | 0x80));
					utf8_str.push_back(static_cast<char>((wc & 0x3F) | 0x80));
				}
				else
				{
					utf8_str.push_back(static_cast<char>((wc >> 18) | 0xF0));
					utf8_str.push_back(static_cast<char>(((wc >> 12) & 0x3F) | 0x80));
					utf8_str.push_back(static_cast<char>(((wc >> 6) & 0x3F) | 0x80));
					utf8_str.push_back(static_cast<char>((wc & 0x3F) | 0x80));
				}
			}
			return utf8_str;
#endif
		}

		// 连续特殊字符过滤器  input:目标字符串 special_string:过滤字符字符串 replacement:替换目标字符串
		std::u32string filter_consecutive_special_string(const std::u32string &input, const std::vector<std::u32string> &special_strings, const std::u32string &replacement)
		{
			std::u32string output;
			std::map<size_t, std::vector<std::u32string>> length_map;

			// 将特殊字符串按长度分类存储
			for (const auto &str : special_strings)
			{
				length_map[str.size()].push_back(str);
			}

			size_t i = 0;
			while (i < input.size())
			{
				bool found_special = false;

				// 按长度逐个查找特殊字符串
				for (auto &pair : length_map)
				{
					size_t len = pair.first;

					if (i + len > input.size())
						continue; // 防止越界

					std::u32string substring = input.substr(i, len);

					// 遍历当前长度的所有特殊字符串，看看是否匹配
					for (const auto &special : pair.second)
					{
						if (substring == special)
						{
							output += replacement; // 替换特殊字符串
							i += len;			   // 跳过已处理的特殊字符串
							found_special = true;
							break;
						}
					}

					if (found_special)
						break; // 如果找到了特殊字符，跳出长度循环
				}

				// 如果没有找到特殊字符串，继续处理当前字符
				if (!found_special)
				{
					output += input[i];
					++i;
				}
			}

			return output;
		}

		// 连续特殊字符过滤器  input:目标字符串 special_chars:过滤字符 replacement:替换目标字符串
		std::u32string filter_consecutive_special_chars(const std::u32string &input, const std::u32string &special_chars, const std::u32string &replacement)
		{
			std::u32string output;
			std::map<char32_t, bool> special_char_map;

			// 初始化特殊字符表
			for (const auto &ch : special_chars)
			{
				special_char_map[ch] = true;
			}

			bool has_special_char = false;

			for (const auto &ch : input)
			{
				if (special_char_map.count(ch))
				{
					has_special_char = true;
				}
				else
				{
					// 如果刚结束一段特殊字符，追加替换目标
					if (has_special_char)
					{
						if (!output.empty())
						{
							output += replacement;
						}
						has_special_char = false;
					}
					output += ch; // 添加当前字符
				}
			}

			// 追加最后一个字符
			if (has_special_char)
			{
				if (!output.empty())
				{
					output += replacement;
				}
				has_special_char = false;
			}

			return output;
		}

		// 按特殊字符串分割  input:目标字符串 delimiter:分隔字符
		std::vector<std::u32string> split_by_special_string(const std::u32string &input, const std::u32string &delimiter)
		{
			std::vector<std::u32string> output;
			size_t left = 0;

			while (left < input.size())
			{
				// 查找分隔符位置
				size_t right = input.find(delimiter, left);

				// 如果找到分隔符
				if (right != std::u32string::npos)
				{
					if (right > left)
					{ // 确保非空
						output.emplace_back(input.substr(left, right - left));
					}
					left = right + delimiter.size(); // 跳过分隔符
				}
				else
				{
					// 剩余内容
					if (left < input.size())
					{
						output.emplace_back(input.substr(left));
					}
					break; // 结束循环
				}
			}

			return output;
		}
	}

#ifdef tools_debug
	namespace test
	{


		// 测试字符串相关功能
		void test_string_functions()
		{
			using namespace tools::string;

			std::cout << "正在测试字符串相关功能..." << std::endl;

			// UTF-8 转 UTF-16，再转换回 UTF-8
			std::string utf8_str = "你好，世界!";
			std::u16string utf16_str = utf8_to_utf16(utf8_str);	   // 转换为 UTF-16
			std::string utf8_converted = utf16_to_utf8(utf16_str); // 转回 UTF-8
			// 验证转换正确性
			if (!(utf8_str == utf8_converted))
			{
				std::cerr << "ERR:UTF-8 转 UTF-16，再转换回 UTF-8" << std::endl;
			}
			std::cout << "UTF-8 与 UTF-16 转换测试通过。" << std::endl;

			// UTF-8 转 UTF-32，再转换回 UTF-8
			std::u32string utf32_str = utf8_to_utf32(utf8_str); // 转换为 UTF-32
			utf8_converted = utf32_to_utf8(utf32_str);			// 转回 UTF-8
			// 验证转换正确性
			if (!(utf8_str == utf8_converted))
			{
				std::cerr << "ERR:UTF-8 转 UTF-32，再转换回 UTF-8" << std::endl;
			}
			std::cout << "UTF-8 与 UTF-32 转换测试通过。" << std::endl;

			// 测试连续特殊字符过滤
			std::u32string input = U"aa@@@bb##";
			std::u32string filtered = filter_consecutive_special_chars(input, U"@#", U"_");
			// 验证转换正确性
			if (!(filtered == U"aa_bb_"))
			{
				std::cerr << "ERR:测试连续特殊字符过滤" << std::endl;
			}
			std::cout << "测试连续特殊字符过滤测试通过。" << std::endl;

			// 测试按特殊字符串分割
			std::u32string delimiter = U"@@";
			std::vector<std::u32string> parts = split_by_special_string(U"aa@@bb@@cc", delimiter);
			// 验证转换正确性
			if (!(parts.size() == 3 && parts[0] == U"aa" && parts[1] == U"bb" && parts[2] == U"cc"))
			{
				std::cerr << "ERR:测试按特殊字符串分割" << std::endl;
			}
			std::cout << "测试按特殊字符串分割测试通过。" << std::endl;
		}

		void all_test()
		{
			try
			{
				tools::test::test_terminal_functions(); // 测试终端相关功能
				tools::test::test_string_functions();	// 测试字符串相关功能
				//tools::test::test_file_functions();		// 测试文件操作相关功能
			}
			catch (const std::exception& ex)
			{
				std::cerr << "测试失败，异常信息: " << ex.what() << std::endl;
				return;
			}

			std::cout << "所有测试通过！" << std::endl;
			return;
		}
	}
#endif

}