#include <list>
#include <unordered_map>
#include <variant>
#include <vector>

/**
 * A custom container that combines the mid-iteration editing of a list with
 * the fast value lookup and unique entry constraint of an unordered_map.
 * 
 * The idea is that you can iterate along the same order and elements as the
 * list, but you can also call for an element to be moved - an element which
 * you only have the value, not the position or iterator). That operation
 * (move by value) can be performed in constant time complexity.
 * 
 * Also, the arbitrary number of partitions means that you can insert into
 * a number of predefined positions in the list, not just the begin or end.
 * 
 * Each partition starts with a node in the list that has a Sentinel type.
 * This value MUST NOT be directly accessed. These nodes serve to enable
 * moving elements arbitrarily without knowing if they are on the border
 * of a partition or not.
 */
template <typename T>
class partitioned_value_reorderable_list {
	private:
		struct Sentinel {};
		using NodeData = std::variant<Sentinel, T>;
		std::list<NodeData> list;
		std::unordered_map<T, typename std::list<NodeData>::iterator> map;
		std::vector<typename std::list<NodeData>::iterator> partitions;
	
	public:
		// Standard container type aliases (required for compliance with STL standards for iterating, sorting, etc.)
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using pointer = T*; 
		using reference = value_type&;
		using const_reference = const value_type&;
		
		// Iterators
		
		struct iterator {
			using value_type = partitioned_value_reorderable_list::value_type;
			using reference = partitioned_value_reorderable_list::reference;
			using pointer = partitioned_value_reorderable_list::pointer;
			using difference_type = partitioned_value_reorderable_list::difference_type;
			using iterator_category = std::forward_iterator_tag;
			
			typename std::list<NodeData>::iterator listIterator;
			typename std::list<NodeData>::iterator listEnd;
			
			void skip_sentinels() {
				while(listIterator != listEnd && std::holds_alternative<Sentinel>(*listIterator)) {
					++listIterator;
				}
			}
			
			// Iterator constructor that initializes to the position of the first real element in the list which comes after the given list iterator. end(), if the list is empty
			iterator(	typename std::list<NodeData>::iterator it,
						typename std::list<NodeData>::iterator end
			) : listIterator(it), listEnd(end) {
				skip_sentinels();
			}
			
			reference operator*() const {
				if(listIterator == listEnd) {
					throw std::out_of_range("Cannot dereference end iterator");
				}
				
				return std::get<T>(*listIterator);
			}
			
			pointer operator->() const {
				return &(operator*());
			}
			
			iterator& operator++() {
				if(listIterator == listEnd) {
					throw std::out_of_range("Cannot increment iterator past end");
				}
				
				++listIterator;
				skip_sentinels();
				return *this;
			}
			
			iterator operator++(int) {
				iterator tmp = *this;
				++(*this);
				return tmp;
			}
			
			friend bool operator==(const iterator& a, const iterator& b) {
				return a.listIterator == b.listIterator;
			}
			friend bool operator!=(const iterator& a, const iterator& b) {
				return !(a == b);
			}
		};
		
		iterator begin() {
			return iterator(list.begin(), list.end());
		}
		
		iterator end() {
			return iterator(list.end(), list.end());
		}
		
		// Constructors
		
		/**
		 * Parameterized constructor that receives a desired number of partitions.
		 */
		partitioned_value_reorderable_list(std::size_t numPartitions)
				: list(numPartitions, NodeData(std::in_place_type<Sentinel>)) {
			partitions.reserve(numPartitions); // Optimization: prevent internal array copying
			for(typename std::list<NodeData>::iterator currentPartition = list.begin(); currentPartition != list.end(); currentPartition++) {
				partitions.push_back(currentPartition);
			}
		};
		
		/**
		 * Default constructor. Starts with a single partition.
		 */
		partitioned_value_reorderable_list()
				: partitioned_value_reorderable_list(1) {};

		// Capacity
		
		bool empty() const noexcept {
			return map.empty();
		}
		
		size_type size() const noexcept {
			return map.size();
		}
		
		// Modifiers
		
		/**
		 * Moves an existing element (identified by value) to the front of a partition
		 */
		void move_to_partition_front(const T& value, std::size_t partition) {
			typename std::list<NodeData>::iterator iteratorToPartitionSentinel = partitions.at(partition);
			typename std::list<NodeData>::iterator iteratorToSourceItem = map.at(value);
			list.splice(++iteratorToPartitionSentinel, list, iteratorToSourceItem);
		}
		
		/**
		 * Inserts a new element to the front of a partition
		 */
		void insert_to_partition_front(const T& value, std::size_t partition) {
			if(map.find(value) != map.end()) {
				throw std::invalid_argument("Key already exists in the map");
			}
			
			typename std::list<NodeData>::iterator iteratorToPartitionSentinel = partitions.at(partition);
			typename std::list<NodeData>::iterator iteratorToInsertedItem = list.insert(++iteratorToPartitionSentinel, NodeData(std::in_place_type<T>, value));
			map.insert({value, iteratorToInsertedItem});
		}
		
		/**
		 * Erases the element to which the given iterator points
		 * @return an iterator to the element following the erased one, or end() if no such element exists.
		 */
		iterator erase(const iterator pos) {
			map.erase(std::get<T>(*(pos.listIterator)));
			typename std::list<NodeData>::iterator nextItem = list.erase(pos.listIterator);
			return iterator(nextItem, list.end());
		}
};