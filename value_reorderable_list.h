#include <forward_list>
#include <unordered_map>

/**
 * A custom container that combines the mid-iteration editing of a forward_list with the fast value lookup and unique entry constraint of an unordered_map.
 * The idea is that you can iterate along the same order and elements as the list, but you can also call for an element to be moved to the front of the list (an element which you only have the value, not the position or iterator) and that can be performed in constant time complexity.
 */
template <typename T>
class value_reorderable_list {
	private:
		std::forward_list<T> fl;
		std::unordered_map<T> um;
	
	//TODO: examine and implement the rest of this class
	
	public:
		// Standard container type aliases (named requirements)
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		// Note: A real implementation would need a custom iterator that handles traversal
		// across both containers. For simplicity, this example provides direct access methods.

		// Constructors
		TwoContainerWrapper() = default;
		TwoContainerWrapper(std::initializer_list<T> init) : vec_(init) {}

		// Capacity
		bool empty() const noexcept {
			return vec_.empty() && deq_.empty();
		}
		size_type size() const noexcept {
			return vec_.size() + deq_.size();
		}

		// Modifiers: Example of adding elements
		void push_back(const T& value) {
			vec_.push_back(value);
		}
		void push_front(const T& value) {
			// std::deque provides efficient push_front functionality
			deq_.push_front(value);
		}
		void pop_back() {
			if (!vec_.empty()) {
				vec_.pop_back();
			} else if (!deq_.empty()) {
				deq_.pop_back();
			}
		}
		void pop_front() {
			if (!deq_.empty()) {
				deq_.pop_front();
			} else if (!vec_.empty()) {
				// Need a way to pop from the front of the vector if deque is empty
				// This is inefficient for vector, which is why we have deque
			}
		}

		// Element access (simplified)
		// You would typically have operator[] and at() if random access is supported across the whole structure

		// Example function to consolidate data into the vector
		void consolidate() {
			// Move elements from deque to the end of the vector
			std::move(deq_.begin(), deq_.end(), std::back_inserter(vec_));
			deq_.clear();
		}
};