#pragma once
#include "../tools.hpp"
namespace tools {
	namespace memory_allocator {
		// 用于管理内存块的块类
		template<typename T>
		class block {
		public:
			block(T* start, u64 size) : start(start), size(size) {}

			// 访问块中特定索引处的对象
			T& at(u64 index) {
				if (index >= size) {
					throw std::out_of_range("Index out of bounds");
				}
				return start[index];  // 使用 start 来访问数组元素
			}

			// 使用 operator[] 访问块中的对象
			T& operator[](u64 index) {
				return start[index];  // 不进行边界检查，可能会发生越界错误
			}

			T* start;  // 该内存块的起始指针
			u64 size;  // 块的大小
		};

		class system_allocator {
		public:
			// 分配指定大小的原始内存
			block<byte> malloc_(u64 size) {
				auto block_ = (byte*)std::malloc(size);
				if (block_) {
					memory_pool.insert({ block_, MemoryType::Malloc });
				}
				return block<byte>(block_, size);
			}

			// 释放通过 `malloc_` 分配的内存
			bool free_(block<byte>* block) {
				auto it = memory_pool.find(block->start);  // 使用 start 而不是 obj
				if (it != memory_pool.end() && it->second == MemoryType::Malloc) {
					memory_pool.erase(it);
					std::free(block->start);  // 使用 std::free 释放内存
					return true;
				}
				return false;
			}

			// 分配一个类型为 T 的内存块，并返回该内存块的信息
			template<typename T>
			block<T> new_(u64 size) {
				T* block_ = new T[size];  // 使用 new[] 分配内存
				if (block_) {
					memory_pool.insert({ (byte*)block_, MemoryType::New });
				}
				return block<T>(block_, size);  // 返回 block 对象，包含分配的内存块和大小
			}

			// 释放通过 `new[]` 分配的内存
			template<typename T>
			bool delete_(block<T>& b) {
				if (b.start) {
					// 确保该内存块在 memory_pool 中并且是通过 `new[]` 分配的
					auto it = memory_pool.find((byte*)b.start);
					if (it != memory_pool.end() && it->second == MemoryType::New) {
						delete[] b.start;  // 使用 delete[] 释放内存
						memory_pool.erase(it);  // 从 memory_pool 中移除
						b.start = nullptr;  // 避免重复释放
						b.size = 0;  // 重置大小
						return true;
					}
				}
				return false;
			}

			// 构造函数
			system_allocator() {
				memory_pool.reserve(256);  // 为 unordered_map 预留 256 个空间（提高效率）
			}

			// 析构函数，释放池中所有已分配的内存
			~system_allocator() {
				for (auto& entry : memory_pool) {
					if (entry.second == MemoryType::Malloc) {
						std::free(entry.first);  // 使用 free 释放 malloc 分配的内存
					}
					else if (entry.second == MemoryType::New) {
						delete[] entry.first;  // 使用 delete[] 释放 new[] 分配的内存
					}
				}
			}

		private:
			enum class MemoryType {
				Malloc,
				New
			};

			// 用于存储所有已分配内存块的池，记录每个内存块的类型
			std::unordered_map<byte*, MemoryType> memory_pool;
		};
	}
	
	namespace test {
#ifdef tools_debug
		u64 memory_allocator_test();
#endif tools_debug
	}
}