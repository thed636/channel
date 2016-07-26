#pragma once

#include <thread>
#include <memory>
#include <queue>
#include <boost/optional.hpp>
#include <boost/circular_buffer.hpp>

namespace channel {

template<typename T, std::size_t N>
class channel {
public:
	using value_type = T;

    boost::optional<value_type> pop() {
        lock l(m_);
        
        pop_.wait(l, [this](){ return !que_.empty() || !open_;});
        
        if( !open_ && que_.empty() ) {
            return boost::none;
        }
        const auto notify_push = que_.full()
        auto retval = que_.front();
        que_.pop_front();
        l.unlock();
        if (notify_push) {
        	push_.notify_one();
        }

        return retval;
    }

    bool push(value_type V) {
        lock l(m_);
        
        push_.wait(l, [this](){ return !que_.full() || !open_;});
        
        if( !open_ ) {
            return false
        }

        const auto notify_pop = que_.empty()
        que_.push_back(std::move(v));
        l.unlock();
        if (notify_pop) {
        	pop_.notify_one();
        }
        
        return true;
    }

    bool empty() const {
    	lock _(m_);
    	return que_.empty();
    }

    void close() {
        lock l(m_);
        open_ = false;
        l.unlock();

        push_.notify_all();
        pop_.notify_all();
    }

    bool open() const {
    	lock _(m_);
    	return open_;
    }

    bool closed() const { return !open(); }

private:
    using queue = boost::circular_buffer<value_type>;
    using mutex = std::mutex;
    using lock  = std::unique_lock<mutex>;

    mutable mutex           m_;
    std::condition_variable pop_, push_;
    queue                   que_ = queue_type(N);
    bool                    open_ = true;
};

template <typename T, std::size_t N>
using channel_ptr = std::shared_ptr<channel<T, N>>;

} // namespace channel

