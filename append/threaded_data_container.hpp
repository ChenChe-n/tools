#pragma once
#include "../tools.hpp"
#include <iostream>
#include <thread>
#include <atomic>


namespace tools{
	namespace threaded_data_container {
        class spin_lock {
            std::atomic_flag flag = ATOMIC_FLAG_INIT; // ԭ�ӱ�־����ʼ��Ϊδ����״̬

        public:
            void lock() {
                while (flag.test_and_set(std::memory_order_acquire)) {
                    // �����ȴ���ֱ�������߳��ͷ���
                }
            }

            void unlock() {
                flag.clear(std::memory_order_release); // �ͷ���
            }
        };


        class ticket_lock {
            std::atomic<int> ticket{ 0 };    // ������̵߳�Ʊ��
            std::atomic<int> serving{ 0 };  // ��ǰ���ڷ����Ʊ��

        public:
            void lock() {
                int my_ticket = ticket.fetch_add(1, std::memory_order_acquire); // ��ȡ�Լ���Ʊ��
                while (serving.load(std::memory_order_acquire) != my_ticket) {
                    // �����ȴ��ֵ��Լ�
                }
            }

            void unlock() {
                serving.fetch_add(1, std::memory_order_release); // ��һ���߳̿��Խ���
            }
        };


        class mcs_lock {
            struct node {
                std::atomic<bool> waiting{ true };
                node* next = nullptr;
            };

            std::atomic<node*> tail{ nullptr }; // ��βָ��
            thread_local static node my_node; // ÿ���̶߳����Լ��� Node

        public:
            void lock() {
                node* prev = tail.exchange(&my_node, std::memory_order_acquire); // ԭ�Ӳ��������µ�β�ڵ�
                if (prev) {
                    prev->next = &my_node;
                    while (my_node.waiting.load(std::memory_order_acquire)) {
                        // �����ȴ�ǰ���ͷ���
                    }
                }
            }

            void unlock() {
                if (!my_node.next) { // ���û�к���߳�
                    node* expected = &my_node;
                    if (tail.compare_exchange_strong(expected, nullptr, std::memory_order_release)) {
                        return; // û�к�̣�ֱ���ͷ���
                    }
                    while (!my_node.next) {
                        // �ȴ���̽ڵ�ָ�뱻����
                    }
                }
                my_node.next->waiting.store(false, std::memory_order_release); // ֪ͨ����߳�
                my_node.next = nullptr; // �����Լ��� next
            }
        };
	}
}