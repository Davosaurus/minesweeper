#include <forward_list>
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
 * Also, the arbitrary number of partitions means that you can insert into
 * a number of predefined positions in the list, not just the begin or end.
 * 
 * Each partition starts with a sentinel node that has a default value of type T
 * in the list. This value should not be directly accessed. These nodes serve
 * to enable moving elements arbitrarily without knowing if we are on the
 * border of a partition or not.
 */
template <typename T>
class partitioned_value_reorderable_list {
	private:
		std::forward_list<T> list;
		std::unordered_map<T, std::forward_list<T>::iterator> map;
		std::vector<std::forward_list<T>::iterator> partitions;
	
	public:
		// Standard container type aliases (required for compliance with STL standards for iterating, sorting, etc.)
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;

		// Constructors
		
		/**
		 * Default constructor. Starts with a single partition.
		 */
		partitioned_value_reorderable_list()
				: list(1, T()), partitions(1, list.begin()) {};
		
		/**
		 * Parameterized constructor that receives a desired number of partitions.
		 */
		partitioned_value_reorderable_list(const int& numPartitions)
				: list(numPartitions, T()) {
			partitions.reserve(numPartitions); // Optimization: prevent internal array copying
			for(std::forward_list<T>::iterator currentPartition = list.begin(); currentPartition != list.end(); currentPartition++) {
				partitions.push_back(currentPartition);
			}
		};
		
		// TODO: create a custom iterator that handles traversal
		// across the list while skipping over sentinel nodes.

		// Capacity
		
		bool empty() const noexcept {
			return map.empty();
		}
		
		size_type size() const noexcept {
			return map.size();
		}
		
		// Modifiers
		
		/**
		 * Moves an existing element to the front of a partition
		 */
		void move_to_partition_front(const T& value, const int& partition) {
			//TODO: implement
		}
		
		/**
		 * Inserts a new element to the front of a partition
		 */
		void insert_to_partition_front(const T& value, const int& partition) {
			//TODO: implement
		}
		
		/**
		 * Erases an element AFTER the element to which the given iterator points,
		 * so as to not invalidate the given iterator and it can be used to continue iterating.
		 * @return an iterator to the element following the erased one, or end() if no such element exists.
		 */
		iterator erase_after(const_iterator pos) {
			//TODO: implement
		}
};