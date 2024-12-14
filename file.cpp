#include "file.hpp"
namespace tools {
	namespace file {
		namespace fs = std::filesystem;

		/**
		* @brief 列出指定路径下的所有文件（非递归）。
		* @param dir 要扫描的目录路径。
		* @return 包含所有文件路径的向量。
		 */
		inline std::vector<fs::path> list_files(const fs::path& dir) {
			std::vector<fs::path> files;
			for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file()) {
					files.push_back(entry.path());
				}
			}
			return files;
		}

		/**
		 * @brief 递归列出指定路径下的所有文件。
		 * @param dir 要扫描的目录路径。
		 * @return 包含所有文件路径的向量。
		 */
		inline std::vector<fs::path> list_files_recursive(const fs::path& dir) {
			std::vector<fs::path> files;
			for (const auto& entry : fs::recursive_directory_iterator(dir)) {
				if (entry.is_regular_file()) {
					files.push_back(entry.path());
				}
			}
			return files;
		}

		/**
		 * @brief 删除指定路径的文件。
		 * @param file_path 要删除的文件路径。
		 * @return 是否成功删除文件。
		 */
		inline bool delete_file(const fs::path& file_path) {
			try {
				return fs::remove(file_path);
			}
			catch (const std::exception& e) {
				std::cerr << "删除文件出错: " << e.what() << '\n';
				return false;
			}
		}

		/**
		 * @brief 判断指定路径的文件是否存在。
		 * @param file_path 要检查的文件路径。
		 * @return 如果文件存在且为普通文件，返回 true；否则返回 false。
		 */
		inline bool file_exists(const fs::path& file_path) {
			return fs::exists(file_path) && fs::is_regular_file(file_path);
		}

		/**
		 * @brief 加载文本文件的内容为字符串。
		 * @param file_path 文件路径。
		 * @return 文件内容。
		 * @throws 如果文件不存在或无法打开，则抛出异常。
		 */
		inline std::string load_text_file(const fs::path& file_path) {
			if (!file_exists(file_path)) {
				throw std::runtime_error("文件不存在: " + file_path.string());
			}

			std::ifstream file(file_path, std::ios::in);
			if (!file.is_open()) {
				throw std::runtime_error("无法打开文件: " + file_path.string());
			}

			return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
		}

		/**
		 * @brief 释放通过 load_binary_file 获取的内存。
		 * @param buffer 需要释放的缓冲区。
		 */
		inline void free_buffer(char* buffer) {
			delete[] buffer;  // 释放内存
		}

		/**
		 * @brief 将字符串写入到文本文件。
		 * @param file_path 文件路径。
		 * @param content 要写入的内容。
		 * @throws 如果文件无法打开，则抛出异常。
		 */
		inline void write_text_file(const fs::path& file_path, const std::string& content) {
			std::ofstream file(file_path, std::ios::out);
			if (!file.is_open()) {
				throw std::runtime_error("无法打开文件用于写入: " + file_path.string());
			}

			file << content;
		}

		/**
		 * @brief 将字节数组写入到二进制文件。
		 * @param file_path 文件路径。
		 * @param data 要写入的字节数据。
		 * @throws 如果文件无法打开，则抛出异常。
		 */
		inline void write_binary_file(const fs::path& file_path, char* buffer, size_t size) {
			std::ofstream file(file_path, std::ios::binary);
			if (!file.is_open()) {
				throw std::runtime_error("无法打开文件用于写入: " + file_path.string());
			}

			file.write(buffer, size);
		}

		/**
		 * @brief 加载二进制文件内容到内存（自动分配缓冲区）。
		 * @param file_path 文件路径。
		 * @param buffer 输出缓冲区，函数将返回数据（内存由函数分配）。
		 * @return 文件的字节数据。
		 * @throws 如果文件不存在或无法打开，则抛出异常。
		 */
		inline void load_binary_file(const fs::path& file_path, char*& buffer, size_t& size) {
			if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
				throw std::runtime_error("文件不存在: " + file_path.string());
			}

			std::ifstream file(file_path, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				throw std::runtime_error("无法打开文件: " + file_path.string());
			}

			size = static_cast<size_t>(file.tellg());
			buffer = new char[size];  // 自动分配内存

			file.seekg(0, std::ios::beg);
			file.read(buffer, size);
		}


		/**
		 * @brief 无阻塞并行加载二进制文件。
		 * @param file_path 文件路径。
		 * @param threads 存储创建线程的容器（外部传入）。
		 * @param buffer 要存储结果的缓冲区（外部传入）。
		 * @param min_chunk_size 最小分块大小（字节），低于此值时只使用 1 个线程。
		 * @throws 如果文件不存在或无法打开，则抛出异常。
		 */
		inline void async_load_binary_file(const fs::path& file_path, std::vector<std::thread>& threads,
			char*& buffer, size_t& size, std::size_t min_chunk_size) {
			if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
				throw std::runtime_error("文件不存在: " + file_path.string());
			}

			std::ifstream file(file_path, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				throw std::runtime_error("无法打开文件: " + file_path.string());
			}

			size = static_cast<size_t>(file.tellg());
			buffer = new char[size];  // 自动分配内存

			if (size <= min_chunk_size) {
				// 如果文件太小，不使用多线程
				threads.emplace_back([&]() {
					file.seekg(0, std::ios::beg);
					file.read(buffer, size);
					});
				return;
			}

			// 多线程分块读取
			size_t thread_count = std::thread::hardware_concurrency();
			if (thread_count == 0) thread_count = 2; // 确保至少有 2 个线程

			size_t chunk_size = std::max(min_chunk_size, size / thread_count); // 确保每个线程的分块不小于 min_chunk_size
			thread_count = (size + chunk_size - 1) / chunk_size; // 重新计算线程数

			for (unsigned int i = 0; i < thread_count; ++i) {
				threads.emplace_back([&, i]() {
					auto start = i * chunk_size;
					auto end = std::min<size_t>(start + chunk_size, size);

					std::ifstream thread_file(file_path, std::ios::binary);
					if (!thread_file.is_open()) {
						throw std::runtime_error("线程无法打开文件: " + file_path.string());
					}

					thread_file.seekg(start, std::ios::beg);
					thread_file.read(buffer + start, end - start);
					});
			}
		}

		/**
		 * @brief 无阻塞并行写入二进制文件。
		 * @param file_path 文件路径。
		 * @param threads 存储创建线程的容器（外部传入）。
		 * @param data 要写入的字节数据。
		 * @param min_chunk_size 最小分块大小（字节），低于此值时只使用 1 个线程。
		 * @throws 如果文件无法打开，则抛出异常。
		 */
		inline void async_write_binary_file(const fs::path& file_path, std::vector<std::thread>& threads,
			const char* data, size_t size, std::size_t min_chunk_size) {
			if (size <= min_chunk_size) {
				// 如果数据太小，不使用多线程
				threads.emplace_back([&]() {
					std::ofstream file(file_path, std::ios::binary);
					if (!file.is_open()) {
						throw std::runtime_error("无法打开文件用于写入: " + file_path.string());
					}

					file.write(data, size);
					});
				return;
			}

			// 多线程分块写入
			size_t thread_count = std::thread::hardware_concurrency();
			if (thread_count == 0) thread_count = 2; // 确保至少有 2 个线程

			size_t chunk_size = std::max(min_chunk_size, size / thread_count); // 确保每个线程的分块不小于 min_chunk_size
			thread_count = (size + chunk_size - 1) / chunk_size; // 重新计算线程数

			for (unsigned int i = 0; i < thread_count; ++i) {
				threads.emplace_back([&, i]() {
					auto start = i * chunk_size;
					auto end = std::min<size_t>(start + chunk_size, size);

					std::ofstream thread_file(file_path, std::ios::binary | std::ios::in | std::ios::out);
					if (!thread_file.is_open()) {
						throw std::runtime_error("线程无法打开文件用于写入: " + file_path.string());
					}

					thread_file.seekp(start, std::ios::beg);
					thread_file.write(data + start, end - start);
					});
			}
		}


	}
	namespace test {
		void test_file_functions()
		{

		}
	}
}

