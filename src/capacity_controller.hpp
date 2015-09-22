#ifndef CAPACITY_CONTROLLER_HPP
#define CAPACITY_CONTROLLER_HPP

#include <mutex>				// for std::mutex and locks.
#include <condition_variable>	// for std::condition_variable.


template <typename intT>
class capacity_controller {

private:
	std::condition_variable dissipated_cond;
	std::mutex capacity_mutex;
	intT max_capacity;
	intT accumulated;

public:
	capacity_controller() = delete;
	capacity_controller(intT const max, intT const init):
		max_capacity(max), accumulated(init)
	{}

	// Adds to the existing capacity in a thread-safe manner.
	void accumulate( intT thisMuch ){
		std::unique_lock<std::mutex> accumulate_mutex(capacity_mutex);
		dissipated_cond.wait( accumulate_mutex, [&]{
			auto sum = accumulated + thisMuch;
			auto ret = ( sum < max_capacity );
			if( ret )
				accumulated = sum;
			return ret;
		} );
	}

	// Deducts from the existing capacity in a thread-safe manner.
	void dissipate( intT thisMuch ){
		{
			std::lock_guard<std::mutex> dissipate_mutex( capacity_mutex );
			accumulated -= thisMuch;
		}
		dissipated_cond.notify_all();
	}

};

#endif	// CAPACITY_CONTROLLER_HPP
