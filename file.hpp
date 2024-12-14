#pragma once
#include "tools.hpp"

namespace tools {
	namespace file {
		namespace fs = std::filesystem;

		/**
		* @brief 列出指定路径下的所有文件（非递归）。
		* @param dir 要扫描的目录路径。
		* @return 包含所有文件路径的向量。
		 */
		inline std::vector<fs::path> list_files(const fs::path& dir);

		/**
		 * @brief 递归列出指定路径下的所有文件。
		 * @param dir 要扫描的目录路径。
		 * @return 包含所有文件路径的向量。
		 */
		inline std::vector<fs::path> list_files_recursive(const fs::path& dir);

		/**
		 * @brief 删除指定路径的文件。
		 * @param file_path 要删除的文件路径。
		 * @return 是否成功删除文件。
		 */
		inline bool delete_file(const fs::path& file_path);

		/**
		 * @brief 判断指定路径的文件是否存在。
		 * @param file_path 要检查的文件路径。
		 * @return 如果文件存在且为普通文件，返回 true；否则返回 false。
		 */
		inline bool file_exists(const fs::path& file_path);

		/**
		 * @brief 加载文本文件的内容为字符串。
		 * @param file_path 文件路径。
		 * @return 文件内容。
		 * @throws 如果文件不存在或无法打开，则抛出异常。
		 */
		inline std::string load_text_file(const fs::path& file_path);

		/**
		 * @brief 将字符串写入到文本文件。
		 * @param file_path 文件路径。
		 * @param content 要写入的内容。
		 * @throws 如果文件无法打开，则抛出异常。
		 */
		inline void write_text_file(const fs::path& file_path, const std::string& content);

		/**
		 * @brief 释放通过 load_binary_file 获取的内存。
		 * @param buffer 需要释放的缓冲区。
		 */
		inline void free_buffer(char* buffer);

		/**
		 * @brief 将字符串写入到文本文件。
		 * @param file_path 文件路径。
		 * @param content 要写入的内容。
		 * @throws 如果文件无法打开，则抛出异常。
		 */
		inline void write_text_file(const fs::path& file_path, const std::string& content);

		/**
		 * @brief 将字节数组写入到二进制文件。
		 * @param file_path 文件路径。
		 * @param data 要写入的字节数据。
		 * @throws 如果文件无法打开，则抛出异常。
		 */
		inline void write_binary_file(const fs::path& file_path, char* buffer, size_t size);

		/**
		 * @brief 加载二进制文件内容到内存（自动分配缓冲区）。
		 * @param file_path 文件路径。
		 * @param buffer 输出缓冲区，函数将返回数据（内存由函数分配）。
		 * @return 文件的字节数据。
		 * @throws 如果文件不存在或无法打开，则抛出异常。
		 */
		inline void load_binary_file(const fs::path& file_path, char*& buffer, size_t& size);


		/**
		 * @brief 无阻塞并行加载二进制文件。
		 * @param file_path 文件路径。
		 * @param threads 存储创建线程的容器（外部传入）。
		 * @param buffer 要存储结果的缓冲区（外部传入）。
		 * @param min_chunk_size 最小分块大小（字节），低于此值时只使用 1 个线程。
		 * @throws 如果文件不存在或无法打开，则抛出异常。
		 */
		inline void async_load_binary_file(const fs::path& file_path, std::vector<std::thread>& threads,
			char*& buffer, size_t& size, std::size_t min_chunk_size = 1 << 24);

		/**
		 * @brief 无阻塞并行写入二进制文件。
		 * @param file_path 文件路径。
		 * @param threads 存储创建线程的容器（外部传入）。
		 * @param data 要写入的字节数据。
		 * @param min_chunk_size 最小分块大小（字节），低于此值时只使用 1 个线程。
		 * @throws 如果文件无法打开，则抛出异常。
		 */
		inline void async_write_binary_file(const fs::path& file_path, std::vector<std::thread>& threads,
			const char* data, size_t size, std::size_t min_chunk_size = 1 << 24);

	}

	namespace test {
		// 测试文件操作相关功能
		void test_file_functions();
	}
}