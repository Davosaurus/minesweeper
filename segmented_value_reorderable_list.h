#include <list>
#include <unordered_map>
#include <vector>

/**
 * A custom container that combines the mid-iteration editing of a list with
 * the fast value lookup and unique entry constraint of an unordered_map.
 * 
 * The idea is that you can iterate along the same order and elements as the
 * list, but you can also call for an element to be moved to the front of the
 * list (an element which you only have the value, not the position or iterator)
 * and that can be performed in constant time complexity.
 * 
 * Also, the arbitrary number of segments means that you can insert into
 * a number of predefined positions in the list, not just the begin or end.
 * 
 * Each segment starts with a sentinel node that has a default value of type T
 * in the list. This value should not be directly accessed. These nodes serve
 * to enable moving elements arbitrarily without knowing if we are on the
 * border of a segment or not.
 */
template <typename T>
class segmented_value_reorderable_list {
	private:
		std::list<T> list;
		std::unordered_map<T, std::list<T>::iterator> map;
		std::vector<std::list<T>::iterator> segments;
	
	//TODO: examine and implement the rest of this class
	
	public:
		// Standard container type aliases (named requirements)
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;

		// Constructors
		segmented_value_reorderable_list()
				: list(1, T()), segments(1, list.begin()) {};
		
		segmented_value_reorderable_list(const int& numSegments)
				: list(numSegments, T()) {
			segments.reserve(numSegments); // Optimization: prevent internal array copying
			for(std::list<T>::iterator currentSegment = list.begin(); currentSegment != list.end(); currentSegment++) {
				segments.push_back(currentSegment);
			}
		};
		
		// TODO: create a custom iterator that handles traversal
		// across the list while skipping over sentinel nodes.

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