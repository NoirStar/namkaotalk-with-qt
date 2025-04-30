#pragma once

#include <thread>
#include <future>
#include <functional>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>

namespace utils {

class ThreadPool {
public:
	explicit ThreadPool(unsigned int thread = std::thread::hardware_concurrency()) {
		for (unsigned int i = 0; i < thread; ++i) {
			workers_.emplace_back([this] {
				std::function<void()> task;
				
				while (true) {
					{
						std::unique_lock<std::mutex> lock(qMutex_);
						cond_.wait(lock, [this] {
							return stop_ || !tasks_.empty();
						});

						if (stop_ && tasks_.empty())
							return;

						task = std::move(tasks_.front());
						tasks_.pop();
					}
					task();
				}
			});
		}
	}

	~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(qMutex_);
			stop_ = true;
		}
		cond_.notify_all();

		for (auto& worker : workers_) {
			if (worker.joinable()) {
				worker.join();
			}
		}
	}

	template<typename F, typename... ARGS>
	auto Enqueue(F&& f, ARGS... args)
		-> std::future<std::invoke_result_t<F, ARGS...>> {
		
		using return_type = std::invoke_result_t<F, ARGS...>;

		// for get future object
		auto task = std::make_shared<std::packaged_task<return_type()>>(
			// forward 해서 호출가능한 함수로 bind
			std::bind(std::forward<F>(f), std::forward<ARGS>(args)...)
		);

		std::future<return_type> res = task->get_future();
		
		{
			std::unique_lock<std::mutex> lock(qMutex_);
			if (stop_) {
				throw std::runtime_error("ThreadPool is stopped");
			}
			tasks_.emplace(
				[task]() { (*task)(); } 
			);
		}
		cond_.notify_one();
		return res;
	}

private:
	std::vector<std::thread> workers_;
	std::mutex qMutex_;
	std::condition_variable cond_;
	std::queue<std::function<void()>> tasks_;
	std::atomic_bool stop_{ false };
};

} // namespace utils