#include "tools.hpp"
namespace tools {
	namespace local {
		//初始化windows平台终端为utf-8
		Utf8Console& Utf8Console::getInstance() {
			static Utf8Console instance;
			return instance;
		}
		Utf8Console::Utf8Console() {
#ifdef _WIN32
			SetConsoleOutputCP(CP_UTF8); // 设置输出编码为UTF-8
			SetConsoleCP(CP_UTF8);       // 设置输入编码为UTF-8
#endif
		}
		// 创建一个全局对象，这会自动调用构造函数
		Utf8Console& utf8ConsoleInitializer = Utf8Console::getInstance();
	}
	namespace terminal {
		// 重载 `operator<<` 来输出 `xy` 对象
		std::ostream& operator<<(std::ostream& os, const xy& obj) {
			os << "xy(" << obj.x << ", " << obj.y << ")";
			return os;  // 返回流，以便链式调用
		}

		void ShowCursor(bool showFlag) {
#ifdef _WIN32
			// Windows 系统：使用 Windows API 控制光标
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_CURSOR_INFO cursorInfo;
			GetConsoleCursorInfo(hOut, &cursorInfo);
			cursorInfo.bVisible = showFlag;
			SetConsoleCursorInfo(hOut, &cursorInfo);
#else
			// 类 UNIX 系统：使用 ANSI 转义序列控制光标
			if (showFlag) {
				std::cout << "\033[?25h";  // 显示光标
			}
			else {
				std::cout << "\033[?25l";  // 隐藏光标
			}
#endif
		}

		// 将光标移动到指定位置 (x, y)  坐标从1开始算
		void goto_xy(int x, int y) {
#ifdef _WIN32
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
			SetConsoleCursorPosition(hOut, coord);
#else
			std::cout << "\033[" << y << ";" << x << "H";
#endif
		}

		// 清空控制台
		void clear() {
#ifdef _WIN32
			system("cls");
#else
			system("clear");
#endif
		}

		// 设置控制台文本前景色
		void set_text_color(Color color) {
			std::cout << "\033[" << color + 30 << "m";
		}

		// 设置控制台背景颜色
		void set_background_color(Color color) {
			std::cout << "\033[" << color + 40 << "m";
		}

		// 重置控制台颜色
		void reset_color() {
			std::cout << "\033[0m";
		}

		// 获取当前光标坐标
		xy get_xy() {
#ifdef _WIN32
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
				int x = csbi.dwCursorPosition.X + 1; // Convert to 1-based index
				int y = csbi.dwCursorPosition.Y + 1;
				return xy(x, y);
			}
			else {
				throw std::runtime_error("Failed to get cursor position.");
			}
#else
			std::cout << "\033[6n";  // Request cursor position (ANSI escape sequence)
			int x = 0, y = 0;
			if (std::cin.get() == '\033' && std::cin.get() == '[') {
				std::cin >> y;   // Read row
				if (std::cin.get() == ';') {
					std::cin >> x;   // Read column
					return xy(x, y);
				}
			}
			throw std::runtime_error("Failed to get cursor position.");
#endif
		}

		// 获取终端大小
		xy get_size() {
#ifdef _WIN32
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
				int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
				int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
				return xy(width, height);
			}
			else {
				throw std::runtime_error("Failed to get console size.");
			}
#else
			struct winsize w;
			if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
				throw std::runtime_error("Failed to get terminal size.");
			}
			return xy(w.ws_col, w.ws_row);
#endif
		}
	}
	namespace string {
		std::u16string utf8_to_utf16(const std::string& utf8_str) {
			std::u16string utf16_str;
			size_t i = 0;
			while (i < utf8_str.size()) {
				unsigned char c = utf8_str[i];

				if (c <= 0x7F) {  // 1 byte character
					utf16_str.push_back(c);
					++i;
				}
				else if (c >= 0xC2 && c <= 0xDF) {  // 2 bytes character
					if (i + 1 >= utf8_str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					utf16_str.push_back(((c & 0x1F) << 6) | (c2 & 0x3F));
					i += 2;
				}
				else if (c >= 0xE0 && c <= 0xEF) {  // 3 bytes character
					if (i + 2 >= utf8_str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					unsigned char c3 = utf8_str[i + 2];
					utf16_str.push_back(((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F));
					i += 3;
				}
				else if (c >= 0xF0 && c <= 0xF4) {  // 4 bytes character
					if (i + 3 >= utf8_str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					unsigned char c3 = utf8_str[i + 2];
					unsigned char c4 = utf8_str[i + 3];
					unsigned int codepoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
					codepoint -= 0x10000;
					utf16_str.push_back(0xD800 | (codepoint >> 10));  // High surrogate
					utf16_str.push_back(0xDC00 | (codepoint & 0x3FF));  // Low surrogate
					i += 4;
				}
				else {
					throw std::invalid_argument("Invalid UTF-8 sequence");
				}
			}
			return utf16_str;
		}
		std::string utf16_to_utf8(const std::u16string& utf16_str) {
			std::string utf8_str;
			for (size_t i = 0; i < utf16_str.size(); ++i) {
				uint16_t c = utf16_str[i];

				if (c <= 0x7F) {
					utf8_str.push_back(static_cast<char>(c));  // 1 byte character
				}
				else if (c <= 0x7FF) {
					utf8_str.push_back(static_cast<char>(0xC0 | (c >> 6)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F)));  // 2 byte character
				}
				else if (c >= 0xD800 && c <= 0xDBFF) {  // High surrogate (start of surrogate pair)
					if (i + 1 < utf16_str.size()) {
						uint16_t low = utf16_str[i + 1];
						if (low >= 0xDC00 && low <= 0xDFFF) {  // Valid low surrogate
							uint32_t codepoint = ((c - 0xD800) << 10) | (low - 0xDC00);
							codepoint += 0x10000;
							utf8_str.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
							utf8_str.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
							utf8_str.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
							utf8_str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));  // 4 byte character
							++i;
							continue;
						}
					}
					throw std::invalid_argument("Invalid UTF-16 surrogate pair");
				}
				else {
					utf8_str.push_back(static_cast<char>(0xE0 | (c >> 12)));
					utf8_str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F)));  // 3 byte character
				}
			}
			return utf8_str;
		}
		std::u32string utf8_to_utf32(const std::string& utf8_str) {
			std::u32string utf32_str;
			size_t i = 0;
			while (i < utf8_str.size()) {
				unsigned char c = utf8_str[i];

				if (c <= 0x7F) {  // 1 byte character
					utf32_str.push_back(c);
					++i;
				}
				else if (c >= 0xC2 && c <= 0xDF) {  // 2 bytes character
					if (i + 1 >= utf8_str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					utf32_str.push_back(((c & 0x1F) << 6) | (c2 & 0x3F));
					i += 2;
				}
				else if (c >= 0xE0 && c <= 0xEF) {  // 3 bytes character
					if (i + 2 >= utf8_str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					unsigned char c3 = utf8_str[i + 2];
					utf32_str.push_back(((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F));
					i += 3;
				}
				else if (c >= 0xF0 && c <= 0xF4) {  // 4 bytes character
					if (i + 3 >= utf8_str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
					unsigned char c2 = utf8_str[i + 1];
					unsigned char c3 = utf8_str[i + 2];
					unsigned char c4 = utf8_str[i + 3];
					unsigned int codepoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
					codepoint -= 0x10000;
					utf32_str.push_back(codepoint);  // UTF-32
					i += 4;
				}
				else {
					throw std::invalid_argument("Invalid UTF-8 sequence");
				}
			}
			return utf32_str;
		}
		std::string utf32_to_utf8(const std::u32string& utf32_str) {
			std::string utf8_str;
			for (size_t i = 0; i < utf32_str.size(); ++i) {
				uint32_t c = utf32_str[i];

				if (c <= 0x7F) {
					utf8_str.push_back(static_cast<char>(c));  // 1 byte character
				}
				else if (c <= 0x7FF) {
					utf8_str.push_back(static_cast<char>(0xC0 | (c >> 6)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F)));  // 2 byte character
				}
				else if (c <= 0xFFFF) {
					utf8_str.push_back(static_cast<char>(0xE0 | (c >> 12)));
					utf8_str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F)));  // 3 byte character
				}
				else if (c <= 0x10FFFF) {
					utf8_str.push_back(static_cast<char>(0xF0 | (c >> 18)));
					utf8_str.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
					utf8_str.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
					utf8_str.push_back(static_cast<char>(0x80 | (c & 0x3F)));  // 4 byte character
				}
				else {
					throw std::invalid_argument("Invalid UTF-32 codepoint");
				}
			}
			return utf8_str;
		}


		std::wstring utf8_to_wchar(const std::string& utf8_str)
		{
#ifdef _WIN32
			// Windows: Convert UTF-8 to UTF-16 (std::wstring)
			std::wstring wstr;
			int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0); // Get required buffer size
			wstr.resize(len);
			MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wstr[0], len); // Perform conversion
			return wstr;
#else
			// Linux/macOS: Convert UTF-8 to UTF-32 (std::wstring)
			std::wstring wstr;
			size_t i = 0;
			while (i < utf8_str.size()) {
				uint32_t code_point = 0;
				unsigned char c = utf8_str[i];
				if (c <= 0x7F) {
					code_point = c;
					i++;
				}
				else if (c <= 0xDF) {
					code_point = (c & 0x1F) << 6;
					code_point |= (utf8_str[++i] & 0x3F);
					i++;
				}
				else if (c <= 0xEF) {
					code_point = (c & 0x0F) << 12;
					code_point |= (utf8_str[++i] & 0x3F) << 6;
					code_point |= (utf8_str[++i] & 0x3F);
					i++;
				}
				else if (c <= 0xF7) {
					code_point = (c & 0x07) << 18;
					code_point |= (utf8_str[++i] & 0x3F) << 12;
					code_point |= (utf8_str[++i] & 0x3F) << 6;
					code_point |= (utf8_str[++i] & 0x3F);
					i++;
				}
				else {
					throw std::invalid_argument("Invalid UTF-8 byte sequence");
				}

				// Directly append to wstr (UTF-32)
				wstr.push_back(static_cast<wchar_t>(code_point));
			}
			return wstr;
#endif
		}
		std::string wchar_to_utf8(const std::wstring& wchar_str)
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
			for (wchar_t wc : wchar_str) {
				if (wc <= 0x7F) {
					utf8_str.push_back(static_cast<char>(wc));
				}
				else if (wc <= 0x7FF) {
					utf8_str.push_back(static_cast<char>((wc >> 6) | 0xC0));
					utf8_str.push_back(static_cast<char>((wc & 0x3F) | 0x80));
				}
				else if (wc <= 0xFFFF) {
					utf8_str.push_back(static_cast<char>((wc >> 12) | 0xE0));
					utf8_str.push_back(static_cast<char>(((wc >> 6) & 0x3F) | 0x80));
					utf8_str.push_back(static_cast<char>((wc & 0x3F) | 0x80));
				}
				else {
					utf8_str.push_back(static_cast<char>((wc >> 18) | 0xF0));
					utf8_str.push_back(static_cast<char>(((wc >> 12) & 0x3F) | 0x80));
					utf8_str.push_back(static_cast<char>(((wc >> 6) & 0x3F) | 0x80));
					utf8_str.push_back(static_cast<char>((wc & 0x3F) | 0x80));
				}
			}
			return utf8_str;
#endif
		}

		//连续特殊字符过滤器  input:目标字符串 special_string:过滤字符字符串 replacement:替换目标字符串
		std::u32string filter_consecutive_special_string(const std::u32string& input, const std::vector<std::u32string>& special_strings, const std::u32string& replacement) {
			std::u32string output;
			std::map<size_t, std::vector<std::u32string>> length_map;

			// 将特殊字符串按长度分类存储
			for (const auto& str : special_strings) {
				length_map[str.size()].push_back(str);
			}

			size_t i = 0;
			while (i < input.size()) {
				bool found_special = false;

				// 按长度逐个查找特殊字符串
				for (auto& pair : length_map) {
					size_t len = pair.first;

					if (i + len > input.size()) continue;  // 防止越界

					std::u32string substring = input.substr(i, len);

					// 遍历当前长度的所有特殊字符串，看看是否匹配
					for (const auto& special : pair.second) {
						if (substring == special) {
							output += replacement; // 替换特殊字符串
							i += len; // 跳过已处理的特殊字符串
							found_special = true;
							break;
						}
					}

					if (found_special) break; // 如果找到了特殊字符，跳出长度循环
				}

				// 如果没有找到特殊字符串，继续处理当前字符
				if (!found_special) {
					output += input[i];
					++i;
				}
			}

			return output;
		}

		//连续特殊字符过滤器  input:目标字符串 special_chars:过滤字符 replacement:替换目标字符串
		std::u32string filter_consecutive_special_chars(const std::u32string& input, const std::u32string& special_chars, const std::u32string& replacement) {
			std::u32string output;
			std::map<char32_t, bool> special_char_map;

			// 初始化特殊字符表
			for (const auto& ch : special_chars) {
				special_char_map[ch] = true;
			}

			bool has_special_char = false;

			for (const auto& ch : input) {
				if (special_char_map.count(ch)) {
					has_special_char = true;
				}
				else {
					// 如果刚结束一段特殊字符，追加替换目标
					if (has_special_char) {
						if (!output.empty()) {
							output += replacement;
						}
						has_special_char = false;
					}
					output += ch;  // 添加当前字符
				}
			}

			return output;
		}

		//按特殊字符串分割  input:目标字符串 delimiter:分隔字符
		std::vector<std::u32string> split_by_special_string(const std::u32string& input, const std::u32string& delimiter) {
			std::vector<std::u32string> output;
			size_t left = 0;

			while (left < input.size()) {
				// 查找分隔符位置
				size_t right = input.find(delimiter, left);

				// 如果找到分隔符
				if (right != std::u32string::npos) {
					if (right > left) { // 确保非空
						output.emplace_back(input.substr(left, right - left));
					}
					left = right + delimiter.size(); // 跳过分隔符
				}
				else {
					// 剩余内容
					if (left < input.size()) {
						output.emplace_back(input.substr(left));
					}
					break; // 结束循环
				}
			}

			return output;
		}
	}
	namespace file {
		ram_byte::ram_byte(char* data_, size_t size_) {
			if (data_ != nullptr and size_ >= 1)
			{
				data = data_;
				size = size_;
			}
		}

		ram_byte& ram_byte::operator=(const ram_byte& other) {
			if (this != &other) {  // Protect against self-assignment
				data = other.data;
				size = other.size;
			}
			return *this;
		}

		char* ram_byte::get_data() {
			return data;
		}

		size_t ram_byte::get_size() {
			return size;
		}


		// 读取文件内容为字符串
		std::string read_string(const std::string& filename) {
			// 将路径名转换为本地编码（如果需要）
			auto _filename = tools::string::utf8_to_local(filename);

			// 打开文件
			std::ifstream file(_filename, std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return "";
			}

			// 获取文件大小
			file.seekg(0, std::ios::end);  // 移动到文件末尾
			size_t file_size = file.tellg();  // 获取文件大小
			file.seekg(0, std::ios::beg);    // 移动到文件开头

			if (file_size == 0) {
				return "";  // 文件为空，直接返回空字符串
			}

			// 使用一个字符数组缓存文件内容
			std::string contents(file_size, '\0');
			file.read(&contents[0], file_size);  // 读取文件内容到字符串中

			if (!file) {
				std::cerr << "Error reading file: " << filename << std::endl;
				return "";
			}

			return contents;
		}

		// 将字符串内容写入文件
		bool write_string(const std::string& filename, const std::string& content) {
			auto _filename = tools::string::utf8_to_local(filename);  // 转换为本地编码
			std::ofstream file(_filename, std::ios::binary);  // 以二进制模式打开文件
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return false;
			}

			file << content;  // 写入内容
			file.close();  // 关闭文件
			return true;
		}

		//读取文件内容为字节数组
		std::vector<char> read_vector_byte(const std::string& filename) {
			auto _filename = tools::string::utf8_to_local(filename);
			// 打开文件进行读取
			std::ifstream file(_filename, std::ios::in | std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return {};
			}

			// 获取文件大小
			file.seekg(0, std::ios::end);
			size_t size = file.tellg();
			file.seekg(0, std::ios::beg);

			// 读取文件内容到 vector<char> 中
			std::vector<char> data(size);
			file.read(data.data(), size);

			// 返回读取的二进制数据
			return data;
		}

		// 将字节数组内容写入文件
		bool write_vector_byte(const std::string& filename, const std::vector<char>& data) {
			auto _filename = tools::string::utf8_to_local(filename);
			// 以二进制模式打开文件进行写入
			std::ofstream file(_filename, std::ios::out | std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return false;
			}

			// 写入二进制数据到文件
			file.write(data.data(), data.size());

			// 关闭文件
			file.close();
			return true;
		}

		// 读取文件内容为字节数组（并行读取）
		std::vector<char> read_fast_vector_byte(const std::string& filename,double concurrency_multiplier) {
			auto _filename = tools::string::utf8_to_local(filename);
			// 打开文件进行读取
			std::ifstream file(_filename, std::ios::in | std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return {};
			}

			// 获取系统线程数
			int max_thread = std::thread::hardware_concurrency();
			//调整最大并发线程倍率
			max_thread = static_cast<int>(concurrency_multiplier * max_thread);
			if (max_thread < 1)
			{
				max_thread = 1;
			}

			// 获取文件大小
			file.seekg(0, std::ios::end);
			size_t file_size = file.tellg();
			file.seekg(0, std::ios::beg);

			// 调整线程数以匹配文件大小
			const size_t min_block_size = 1024 * 1024;  // 设置最小块大小为1MB
			while (file_size / max_thread < min_block_size && max_thread > 1) {
				max_thread /= 2;
			}

			const size_t block_size = file_size / max_thread;   // 每个线程的标准块大小
			const size_t extra_size = file_size % max_thread;   // 分配给第一个线程的剩余字节

			// 初始化数据存储空间
			std::vector<char> data(file_size);
			std::vector<std::thread> threads;

			// 启动多个线程进行分块读取
			for (size_t i = 0; i < max_thread; ++i) {
				size_t start_offset = i * block_size + (i == 0 ? 0 : extra_size); // 第一个线程包含额外数据
				size_t read_size = block_size + (i == 0 ? extra_size : 0);        // 第一个线程读额外的字节

				threads.emplace_back([&, start_offset, read_size]() {
					std::ifstream file_thread(_filename, std::ios::in | std::ios::binary);
					if (file_thread.is_open()) {
						file_thread.seekg(start_offset);
						file_thread.read(&data[start_offset], read_size);
					}
					});
			}

			// 等待所有线程完成
			for (auto& t : threads) {
				t.join();
			}

			return data;
		}

		// 将字节数组写入文件（并行写入）
		bool write_fast_vector_byte(const std::string& filename, const std::vector<char>& data, double concurrency_multiplier) {
			// 将UTF-8编码转换为系统的本地编码
			auto _filename = tools::string::utf8_to_local(filename);

			// 预先测试是否能打开文件进行写入
			std::ofstream file_test(_filename, std::ios::out | std::ios::binary | std::ios::trunc);
			if (!file_test.is_open()) {
				std::cerr << "Failed to open file for writing: " << filename << std::endl;
				return false;
			}
			file_test.close(); // 立即关闭文件测试

			// 获取系统线程数并根据并发倍率调整
			int max_thread = std::thread::hardware_concurrency();
			max_thread = static_cast<int>(concurrency_multiplier * max_thread);
			if (max_thread < 1) {
				max_thread = 1;
			}

			// 获取数据大小
			size_t data_size = data.size();

			// 调整线程数以匹配数据大小
			const size_t min_block_size = 1024 * 1024; // 最小块大小为1MB
			while (data_size / max_thread < min_block_size && max_thread > 1) {
				max_thread /= 2;
			}

			// 计算每个线程的块大小
			const size_t block_size = data_size / max_thread;
			const size_t extra_size = data_size % max_thread; // 第一个线程负责额外字节

			// 启动多个线程进行分块写入
			std::vector<std::thread> threads;
			bool write_success = true; // 用于标记写入是否成功
			for (size_t i = 0; i < max_thread; ++i) {
				size_t start_offset = i * block_size + (i == 0 ? 0 : extra_size); // 第一个线程包含额外字节
				size_t write_size = block_size + (i == 0 ? extra_size : 0);       // 第一个线程写入额外字节

				threads.emplace_back([&, start_offset, write_size]() {
					std::ofstream file_thread(_filename, std::ios::out | std::ios::binary | std::ios::in);
					if (file_thread.is_open()) {
						file_thread.seekp(start_offset);
						file_thread.write(&data[start_offset], write_size);
						if (!file_thread) {  // 检查写入是否成功
							write_success = false;
							std::cerr << "Failed to write to file at offset: " << start_offset << std::endl;
						}
					}
					else {
						write_success = false;
						std::cerr << "Failed to open file in thread for offset: " << start_offset << std::endl;
					}
					});
			}

			// 等待所有线程完成
			for (auto& t : threads) {
				t.join();
			}

			return write_success;
		}

		//读取文件内容为字节数组
		ram_byte read_byte(const std::string& filename) {
			auto _filename = tools::string::utf8_to_local(filename);

			// 打开文件进行读取
			std::ifstream file(_filename, std::ios::in | std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return ram_byte();
			}

			// 获取文件大小
			file.seekg(0, std::ios::end);
			size_t file_size = file.tellg();
			file.seekg(0, std::ios::beg);

			// 分配内存读取文件内容
			char* data = new char[file_size];
			file.read(data, file_size);

			// 返回读取的二进制数据
			return ram_byte(data, file_size);
		}

		// 将字节数组内容写入文件
		bool write_byte(const std::string& filename, const char* data, u64 length) {
			auto _filename = tools::string::utf8_to_local(filename);

			// 以二进制模式打开文件进行写入
			std::ofstream file(_filename, std::ios::out | std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return false;
			}

			// 写入二进制数据到文件
			file.write(data, length);

			// 关闭文件
			file.close();
			return true;
		}

		// 读取文件内容为字节数组（并行读取）
		ram_byte read_fast_byte(const std::string& filename, double concurrency_multiplier) {
			auto _filename = tools::string::utf8_to_local(filename);

			// 打开文件进行读取
			std::ifstream file(_filename, std::ios::in | std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return ram_byte();
			}

			// 获取文件大小
			file.seekg(0, std::ios::end);
			uint64_t file_size = file.tellg();
			auto out_size = file_size;
			file.seekg(0, std::ios::beg);

			// 获取系统线程数
			int max_thread = std::thread::hardware_concurrency();
			// 调整最大并发线程倍率
			max_thread = static_cast<int>(concurrency_multiplier * max_thread);
			if (max_thread < 1) {
				max_thread = 1;
			}

			// 调整线程数以匹配文件大小
			const size_t min_block_size = 1024 * 1024;  // 设置最小块大小为1MB
			while (file_size / max_thread < min_block_size && max_thread > 1) {
				max_thread /= 2;
			}

			const size_t block_size = file_size / max_thread;   // 每个线程的标准块大小
			const size_t extra_size = file_size % max_thread;   // 分配给第一个线程的剩余字节

			// 初始化数据存储空间
			char* data = new char[file_size];
			std::vector<std::thread> threads;

			// 启动多个线程进行分块读取
			for (size_t i = 0; i < max_thread; ++i) {
				size_t start_offset = i * block_size + (i == 0 ? 0 : extra_size); // 第一个线程包含额外数据
				size_t read_size = block_size + (i == 0 ? extra_size : 0);        // 第一个线程读额外的字节

				threads.emplace_back([&, start_offset, read_size]() {
					std::ifstream file_thread(_filename, std::ios::in | std::ios::binary);
					if (file_thread.is_open()) {
						file_thread.seekg(start_offset);
						file_thread.read(&data[start_offset], read_size);
					}
					});
			}

			// 等待所有线程完成
			for (auto& t : threads) {
				t.join();
			}

			return ram_byte(data, file_size);
		}

		// 将字节数组写入文件（并行写入）
		bool write_fast_byte(const std::string& filename, const char* data, uint64_t length, double concurrency_multiplier) {
			auto _filename = tools::string::utf8_to_local(filename);

			// 预先测试是否能打开文件进行写入
			std::ofstream file_test(_filename, std::ios::out | std::ios::binary | std::ios::trunc);
			if (!file_test.is_open()) {
				std::cerr << "Failed to open file for writing: " << filename << std::endl;
				return false;
			}
			file_test.close(); // 立即关闭文件测试

			// 获取系统线程数并根据并发倍率调整
			int max_thread = std::thread::hardware_concurrency();
			max_thread = static_cast<int>(concurrency_multiplier * max_thread);
			if (max_thread < 1) {
				max_thread = 1;
			}

			// 调整线程数以匹配数据大小
			const size_t min_block_size = 1024 * 1024; // 最小块大小为1MB
			while (length / max_thread < min_block_size && max_thread > 1) {
				max_thread /= 2;
			}

			// 计算每个线程的块大小
			const size_t block_size = length / max_thread;
			const size_t extra_size = length % max_thread; // 第一个线程负责额外的字节

			// 启动多个线程进行分块写入
			std::vector<std::thread> threads;
			bool write_success = true; // 用于标记写入是否成功
			for (size_t i = 0; i < max_thread; ++i) {
				size_t start_offset = i * block_size + (i == 0 ? 0 : extra_size); // 第一个线程包含额外数据
				size_t write_size = block_size + (i == 0 ? extra_size : 0);       // 第一个线程写入额外字节

				threads.emplace_back([&, start_offset, write_size]() {
					std::ofstream file_thread(_filename, std::ios::out | std::ios::binary | std::ios::in);
					if (file_thread.is_open()) {
						file_thread.seekp(start_offset);
						file_thread.write(&data[start_offset], write_size);
						if (!file_thread) {  // 检查写入是否成功
							write_success = false;
							std::cerr << "Failed to write to file at offset: " << start_offset << std::endl;
						}
					}
					else {
						write_success = false;
						std::cerr << "Failed to open file in thread for offset: " << start_offset << std::endl;
					}
					});
			}

			// 等待所有线程完成
			for (auto& t : threads) {
				t.join();
			}

			return write_success;
		}
	}
	namespace test {
		void all_test()
		{
			try {
				tools::test::test_terminal_functions();  // 测试终端相关功能
				tools::test::test_string_functions();  // 测试字符串相关功能
				tools::test::test_file_functions();  // 测试文件操作相关功能
			}
			catch (const std::exception& ex) {
				std::cerr << "测试失败，异常信息: " << ex.what() << std::endl;
				return;
			}

			std::cout << "所有测试通过！" << std::endl;
			return;
		}
		// 测试终端相关功能
		void test_terminal_functions() {
			using namespace tools::terminal;

			std::cout << "正在测试终端相关功能..." << std::endl;

			// 清空控制台
			clear();
			std::cout << "控制台已清空。" << std::endl;

			// 设置文本颜色和背景颜色
			set_text_color(Color::Red);  // 设置文本为红色
			set_background_color(Color::White);  // 设置背景为白色
			std::cout << "这段文字应该显示为红色文字、白色背景。" << std::endl;
			reset_color();  // 重置颜色

			// 测试光标移动功能
			goto_xy(10, 10);
			std::cout << "光标移动到位置 (10, 10) 处。" << std::endl;

			// 获取当前光标位置
			xy pos = get_xy();
			std::cout << "当前光标位置为: (" << pos.x << ", " << pos.y << ")" << std::endl;

			// 显示/隐藏光标
			ShowCursor(false);  // 隐藏光标
			std::this_thread::sleep_for(std::chrono::seconds(1));  // 暂停 1 秒
			ShowCursor(true);  // 显示光标
			std::cout << "光标显示状态已切换。" << std::endl;

			// 获取终端大小
			xy size = get_size();
			std::cout << "当前终端大小为: (" << size.x << ", " << size.y << ")" << std::endl;
		}

		// 测试字符串相关功能
		void test_string_functions() {
			using namespace tools::string;

			std::cout << "正在测试字符串相关功能..." << std::endl;

			// UTF-8 转 UTF-16，再转换回 UTF-8
			std::string utf8_str = "你好，世界!";
			std::u16string utf16_str = utf8_to_utf16(utf8_str);  // 转换为 UTF-16
			std::string utf8_converted = utf16_to_utf8(utf16_str);  // 转回 UTF-8
			assert(utf8_str == utf8_converted);  // 验证转换正确性
			std::cout << "UTF-8 与 UTF-16 转换测试通过。" << std::endl;

			// UTF-8 转 UTF-32，再转换回 UTF-8
			std::u32string utf32_str = utf8_to_utf32(utf8_str);  // 转换为 UTF-32
			utf8_converted = utf32_to_utf8(utf32_str);  // 转回 UTF-8
			assert(utf8_str == utf8_converted);  // 验证转换正确性
			std::cout << "UTF-8 与 UTF-32 转换测试通过。" << std::endl;

			// 测试连续特殊字符过滤
			std::u32string input = U"aa@@@bb##";
			std::u32string filtered = filter_consecutive_special_chars(input, U"@#", U"_");
			assert(filtered == U"aa_bb_");
			std::cout << "连续特殊字符过滤测试通过。" << std::endl;

			// 测试按特殊字符串分割
			std::u32string delimiter = U"@@";
			std::vector<std::u32string> parts = split_by_special_string(U"aa@@bb@@cc", delimiter);
			assert(parts.size() == 3 && parts[0] == U"aa" && parts[1] == U"bb" && parts[2] == U"cc");
			std::cout << "按特殊字符串分割测试通过。" << std::endl;
		}

		// 测试文件操作相关功能
		void test_file_functions() {
			using namespace tools::file;

			std::cout << "正在测试文件操作相关功能..." << std::endl;

			// 测试字符串文件读写
			std::string test_content = "你好，文件操作!";
			std::string test_filename = "test_file.txt";
			assert(write_string(test_filename, test_content));  // 写入文件
			std::string read_content = read_string(test_filename);  // 从文件读取
			assert(read_content == test_content);  // 验证内容一致性
			std::cout << "字符串文件读写测试通过。" << std::endl;

			// 测试字节文件读写
			std::vector<char> data = { 'H', 'e', 'l', 'l', 'o' };
			assert(write_vector_byte(test_filename, data));  // 写入字节数据
			std::vector<char> read_data = read_vector_byte(test_filename);  // 读取字节数据
			assert(data == read_data);  // 验证内容一致性
			std::cout << "字节文件读写测试通过。" << std::endl;

			// 测试并行文件操作
			std::vector<char> large_data(1e9, 'x');  // 创建 1000MB 大小的测试数据
			assert(write_fast_vector_byte(test_filename, large_data));  // 并行写入
			std::vector<char> large_read_data = read_fast_vector_byte(test_filename);  // 并行读取
			assert(large_data == large_read_data);  // 验证内容一致性
			std::cout << "并行文件操作测试通过。" << std::endl;
		}
	}
}