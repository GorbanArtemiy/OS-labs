#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <list>
#include <unordered_map>
#include <algorithm>

struct MemoryBlock {
    size_t size;
    bool is_free;
    MemoryBlock* next;

    MemoryBlock(size_t size, bool is_free, MemoryBlock* next = nullptr)
        : size(size), is_free(is_free), next(next) {}
};

class Allocator {
protected:
    MemoryBlock* head;
    size_t total_size;
    size_t used_size;

public:
    Allocator(size_t size) : total_size(size), used_size(0) {
        head = new MemoryBlock(size, true);
    }

    virtual ~Allocator() {
        MemoryBlock* current = head;
        while (current) {
            MemoryBlock* next = current->next;
            delete current;
            current = next;
        }
    }

    virtual void* allocate(size_t size) = 0;
    virtual void deallocate(void* ptr) = 0;

    double memory_utilization() const {
        return static_cast<double>(used_size) / total_size;
    }
};

class FirstFitAllocator : public Allocator {
public:
    FirstFitAllocator(size_t size) : Allocator(size) {}

    void* allocate(size_t size) override {
        MemoryBlock* current = head;
        while (current) {
            if (current->is_free && current->size >= size) {
                if (current->size > size) {
                    MemoryBlock* new_block = new MemoryBlock(current->size - size, true, current->next);
                    current->size = size;
                    current->next = new_block;
                }
                current->is_free = false;
                used_size += size;
                return reinterpret_cast<void*>(current + 1);
            }
            current = current->next;
        }
        return nullptr;
    }

    void deallocate(void* ptr) override {
        if (!ptr) return;
        MemoryBlock* block = reinterpret_cast<MemoryBlock*>(ptr) - 1;
        block->is_free = true;
        used_size -= block->size;

        MemoryBlock* current = head;
        while (current) {
            if (current->is_free && current->next && current->next->is_free) {
                current->size += current->next->size;
                MemoryBlock* next_next = current->next->next;
                delete current->next;
                current->next = next_next;
            }
            current = current->next;
        }
    }
};

class BestFitAllocator : public Allocator {
public:
    BestFitAllocator(size_t size) : Allocator(size) {}

    void* allocate(size_t size) override {
        MemoryBlock* best_block = nullptr;
        MemoryBlock* current = head;

        while (current) {
            if (current->is_free && current->size >= size) {
                if (!best_block || current->size < best_block->size) {
                    best_block = current;
                }
            }
            current = current->next;
        }

        if (best_block) {
            if (best_block->size > size) {
                MemoryBlock* new_block = new MemoryBlock(best_block->size - size, true, best_block->next);
                best_block->size = size;
                best_block->next = new_block;
            }
            best_block->is_free = false;
            used_size += size;
            return reinterpret_cast<void*>(best_block + 1);
        }
        return nullptr;
    }

    void deallocate(void* ptr) override {
        if (!ptr) return;
        MemoryBlock* block = reinterpret_cast<MemoryBlock*>(ptr) - 1;
        block->is_free = true;
        used_size -= block->size;

        MemoryBlock* current = head;
        while (current) {
            if (current->is_free && current->next && current->next->is_free) {
                current->size += current->next->size;
                MemoryBlock* next_next = current->next->next;
                delete current->next;
                current->next = next_next;
            }
            current = current->next;
        }
    }
};

class VirtualMemory {
private:
    size_t page_size;
    size_t total_pages;
    std::list<size_t> lru_list;
    std::unordered_map<size_t, void*> page_table;
    Allocator* allocator;

public:
    VirtualMemory(size_t page_size, size_t total_pages, Allocator* allocator)
        : page_size(page_size), total_pages(total_pages), allocator(allocator) {}

    void* malloc(size_t size) {
        size_t pages_needed = (size + page_size - 1) / page_size;
        if (pages_needed > total_pages) return nullptr;

        while (lru_list.size() + pages_needed > total_pages) {
            size_t page_to_remove = lru_list.back();
            lru_list.pop_back();
            allocator->deallocate(page_table[page_to_remove]);
            page_table.erase(page_to_remove);
        }

        void* ptr = allocator->allocate(size);
        if (!ptr) return nullptr;

        for (size_t i = 0; i < pages_needed; ++i) {
            lru_list.push_front(reinterpret_cast<size_t>(ptr) + i * page_size);
            page_table[reinterpret_cast<size_t>(ptr) + i * page_size] = ptr;
        }

        return ptr;
    }

    void free(void* ptr) {
        if (!ptr) return;
        allocator->deallocate(ptr);
        for (auto it = lru_list.begin(); it != lru_list.end(); ) {
            if (page_table[*it] == ptr) {
                page_table.erase(*it);
                it = lru_list.erase(it);
            } else {
                ++it;
            }
        }
    }
};

void testAllocator(Allocator& allocator, const std::string& name) {
    auto start = std::chrono::high_resolution_clock::now();

    void* ptr1 = allocator.allocate(100);
    void* ptr2 = allocator.allocate(200);
    void* ptr3 = allocator.allocate(50);

    allocator.deallocate(ptr2);
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr3);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << name << " execution time: " << elapsed.count() << " seconds\n";
    std::cout << name << " memory utilization: " << allocator.memory_utilization() << "\n";
}

void threadedTest(Allocator& allocator, const std::string& name) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&allocator]() {
            void* ptr1 = allocator.allocate(100);
            void* ptr2 = allocator.allocate(200);
            void* ptr3 = allocator.allocate(50);

            allocator.deallocate(ptr2);
            allocator.deallocate(ptr1);
            allocator.deallocate(ptr3);
        });
    }

    for (auto& t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << name << " multithreaded execution time: " << elapsed.count() << " seconds\n";
}

int main() {
    const size_t total_size = 1024 * 1024;

    FirstFitAllocator firstFitAllocator(total_size);
    BestFitAllocator bestFitAllocator(total_size);

    testAllocator(firstFitAllocator, "First Fit");
    testAllocator(bestFitAllocator, "Best Fit");

    threadedTest(firstFitAllocator, "First Fit");
    threadedTest(bestFitAllocator, "Best Fit");

    VirtualMemory virtualMemory(4096, 256, &firstFitAllocator);
    void* vptr = virtualMemory.malloc(8192);
    virtualMemory.free(vptr);

    return 0;
}