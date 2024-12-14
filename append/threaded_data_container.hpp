#pragma once
#include "../tools.hpp"
#include <iostream>
#include <thread>
#include <atomic>


namespace tools{
	namespace threaded_data_container {
        class spin_lock {
            std::atomic_flag flag = ATOMIC_FLAG_INIT; // 原子标志，初始化为未锁定状态

        public:
            void lock() {
                while (flag.test_and_set(std::memory_order_acquire)) {
                    // 自旋等待，直到其他线程释放锁
                }
            }

            void unlock() {
                flag.clear(std::memory_order_release); // 释放锁
            }
        };


        class ticket_lock {
            std::atomic<int> ticket{ 0 };    // 分配给线程的票号
            std::atomic<int> serving{ 0 };  // 当前正在服务的票号

        public:
            void lock() {
                int my_ticket = ticket.fetch_add(1, std::memory_order_acquire); // 获取自己的票号
                while (serving.load(std::memory_order_acquire) != my_ticket) {
                    // 自旋等待轮到自己
                }
            }

            void unlock() {
                serving.fetch_add(1, std::memory_order_release); // 下一个线程可以进入
            }
        };


        class mcs_lock {
            struct node {
                std::atomic<bool> waiting{ true };
                node* next = nullptr;
            };

            std::atomic<node*> tail{ nullptr }; // 队尾指针
            thread_local static node my_node; // 每个线程都有自己的 Node

        public:
            void lock() {
                node* prev = tail.exchange(&my_node, std::memory_order_acquire); // 原子操作设置新的尾节点
                if (prev) {
                    prev->next = &my_node;
                    while (my_node.waiting.load(std::memory_order_acquire)) {
                        // 自旋等待前驱释放锁
                    }
                }
            }

            void unlock() {
                if (!my_node.next) { // 如果没有后继线程
                    node* expected = &my_node;
                    if (tail.compare_exchange_strong(expected, nullptr, std::memory_order_release)) {
                        return; // 没有后继，直接释放锁
                    }
                    while (!my_node.next) {
                        // 等待后继节点指针被设置
                    }
                }
                my_node.next->waiting.store(false, std::memory_order_release); // 通知后继线程
                my_node.next = nullptr; // 清理自己的 next
            }
        };
	}
}