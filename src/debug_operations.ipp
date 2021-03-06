/*
 *  Filename:   debug.ipp
 *
 *  Created on:  Apr. 17, 2020
 *      Author:  orhan
 *
 *  Description:
 *		Contains debug operation implementations for both the TwoFourTree and the Node class
 */


#ifndef SRC_DEBUG_OPERATIONS_IPP_
#define SRC_DEBUG_OPERATIONS_IPP_

#include <iomanip>
#include <functional>
#include <unordered_map>

namespace tft {
/**
 * Verify all nodes have the correct child-parent relationship
 */
template <class K, class C, class A>
bool TwoFourTree<K,C,A>::validate() const {
	return root_->validateRelationships();
}

/**
 * Use a deque<parent, child> to do a breadth-first search over the entire tree to ensure
 * every child knows who its parent is
 */
template <class K, class C, class A>
bool TwoFourTree<K,C,A>::Node::validateRelationships() const {
	const Node * node = this;
	bool rc = true;
	// parent -> child
	std::deque<std::pair<const TwoFourTree::Node*, const TwoFourTree::Node*>> allNodes;
	allNodes.push_back(std::make_pair(nullptr, node));

	while (!allNodes.empty()) {
		auto check = allNodes.front();
		allNodes.pop_front();
		node = check.second;

		// Check if the parent/child relationships are consistent
		if (check.first != check.second->parent_) {
			rc = false;
			std::cout << "parent of [" << check.second->getString() << "] doesn't have correct parent.\n";
			std::cout << "parent should be: ";
			if (check.first == nullptr)
				std::cout << "nullptr\n";
			else
				std::cout << check.first->getString() << "\n";
			std::cout << "instead it is: ";
			if (check.second->parent_ == nullptr)
				std::cout << "nullptr\n";
			else
				std::cout << check.second->parent_->getString() << "\n";
		}

		// verify the internal contents are sorted
		for (int i=1; i < node->num_keys_; i++) {
			if (node->keys_[i-1] > node->keys_[i]) {
				rc = false;
				std::cout << "node keys out of order! " << node->keys_[i-1] << " is left of " << node->keys_[i] << std::endl;
				std::cout << *this << std::endl;
			}
		}

		// Count the number of children in the node
		// For N keys in a node there should be N+1 children (except for leaf nodes)
		// Also check to make sure the children contain either bigger or smaller values,
		// depending on which child it is
		// While we're traversing we also add each child to the list of nodes to be traversed
		int num_children = 0;
		for (int i = 0; i < node->num_keys_; i++) { // First N children should have values smaller than keys_[i]
			if (node->children_[i]) {
				const Node *child = node->children_[i].get();
				if (child->keys_[child->num_keys_-1] > node->keys_[i]) {
					rc = false;
					std::cout << "child to left has key greater than key ("
							<< child->keys_[child->num_keys_ - 1] << " > "
							<< node->keys_[i] << ")\n";
				}
				allNodes.push_back(std::make_pair(node,node->children_[i].get()));
				++num_children;
			}
		}
		if (node->children_[node->num_keys_]) { // Last (Nth) child should have value bigger than keys_[max]
			const Node *child = node->children_[node->num_keys_].get();
			if (child->keys_[0] < node->keys_[node->num_keys_-1]){
				rc = false;
				std::cout << "rightmost child has key less than my key ("
						<< child->keys_[0] << " < "
						<< node->keys_[node->num_keys_-1] << ")\n";
			}
			allNodes.push_back(std::make_pair(node,child));
			++num_children;
		}

		// verify correct number of children
		if (!node->isLeaf() && num_children != node->num_keys_ + 1) {
			std::cout << "number of keys and children mismatch.\n";
			std::cout << "Node contents = [" << node->getString() << "], num_keys = " << node->num_keys_ << ", #children (" << num_children <<"): \n";
			for (int i = 0; i < num_children; i++) {
				std::cout << node->children_[i]->getString() << "   ";
			}
			std::cout << "\n";
			rc = false;
		}
	} // end loop
	return rc;
}


/**
 * Prints the tree level by level
 */
template<class K, class C, class A>
std::string TwoFourTree<K,C,A>::getString() const{
	if (root_)
		return root_->getStringAll();
	else
		return "";
}

template<class K, class C, class A>
void TwoFourTree<K,C,A>::print() const{
	root_->printAll();
}

template<class K, class C, class A>
void TwoFourTree<K,C,A>::Node::printAll() const {
	std::cout << getStringAll();
}

template<class K, class C, class A>
std::string TwoFourTree<K,C,A>::Node::getStringAll() const {

	/**
	 * Struct to track the horizontal displacement of the nodes
	 * For leaves, end-begin should be equal to the string length + 1
	 * For internal nodes, begin will be where the first child begins,
	 * 		and end will be where the last child ends
	 *
	 * When printing, leaf nodes will print normally.
	 * Internal nodes will print at the average of the begin and end locations
	 */
	struct location{
		int begin;
		int end;
	};

	std::deque<const Node*> nodes_to_be_printed;
	std::unordered_map<const Node*, location> offsets;

	std::function<void(const Node*, int)> SetBeginLocation = [&offsets, &SetBeginLocation](const Node* node, int begin) {
		auto it = offsets.find(node);
		if (it == offsets.end()) {
			std::cout << "logic error\n";
			return;
		}

		it->second.begin = begin;

		if (node->parent_ != nullptr) {
			auto myidx = node->getMyChildIdx();
			if (myidx == 0) {
				SetBeginLocation(node->parent_, begin);
			} else {
				return;
			}
		}
	};

	std::function<void(const Node*, int)> SetEndLocation = [&offsets, &SetEndLocation](const Node *node, int end) {
		auto it = offsets.find(node);
		if (it == offsets.end()) {
			std::cout << "logic error\n";
			return;
		}

		it->second.end = end;

		if (node->parent_ != nullptr) {
			auto myidx = node->getMyChildIdx();
			if (myidx == node->parent_->num_keys_) {
				auto whitespace_width = it->second.end - it->second.begin;
				auto trailing_whitespace_width = whitespace_width / 2
						- ((node->getString().length() + 1) / 2);
				SetEndLocation(node->parent_, end - trailing_whitespace_width);
			} else {
				return;
			}
		}
	};

	Node end_of_level(nullptr);

	nodes_to_be_printed.push_back(this);
	nodes_to_be_printed.push_back(&end_of_level);

	int current_offset = 0;
	while (!nodes_to_be_printed.empty()) {
		const Node *node = nodes_to_be_printed.front();
		nodes_to_be_printed.pop_front();

		if (node == &end_of_level) { // when we come across this, we know we reached the end of this line
			nodes_to_be_printed.push_back(&end_of_level); // put it back in so we know for the next iteration
			current_offset = 0;
			if (nodes_to_be_printed.front() == &end_of_level)
				break; // if there isn't another iteration then we've traversed the entire node
			else
				continue;
		}

		/**
		 * If we  are a leaf node then recursively set parent horizontal displacements
		 */
		if (node->isLeaf()) {

			auto start_offset = current_offset;
			current_offset +=  node->getString().length() + 1 ; // +1 for space afterwards

			location l;
			l.begin = start_offset;
			l.end = current_offset;
			offsets.emplace(node, l);

			SetBeginLocation(node, start_offset);
			SetEndLocation(node, current_offset);
		} else {
			location l {0,0};
			offsets.emplace(node, l);
		}

		for (int i=0; i < node->kMaxNumChildren; i++) {
			if (node->children_[i]) {
				nodes_to_be_printed.push_back(node->children_[i].get());
			}
		}

	} // end loop

//	for (const auto& it : offsets) {
//		std::cout << *it.first << " b: " << it.second.begin << ", e: " << it.second.end << std::endl;
//	}
//	std::cout << "---------------------------------------------------------------------------------\n";

	// Begin the actual printing
	std::stringstream ss;
	{
		nodes_to_be_printed.clear();
		nodes_to_be_printed.push_back(this);
		nodes_to_be_printed.push_back(&end_of_level);

		current_offset = 0;
		while (!nodes_to_be_printed.empty()) {
			const Node *node = nodes_to_be_printed.front();
			nodes_to_be_printed.pop_front();

			if (node == &end_of_level) { // when we come across this, we know we reached the end of this line
				ss << "\n"; // print newline
				nodes_to_be_printed.push_back(&end_of_level); // put it back in so we know for the next iteration
				if (nodes_to_be_printed.front() == &end_of_level) // check if there will even be a next iteration
					break; // if there isn't then we've traversed the entire node
				else
					continue;
			}

			int printed_length = 0;
			if (node->isLeaf()) {
				ss << node->getString() << ' ';
				printed_length += node->getString().length() + 1;
			} else {

				auto str = node->getString() + ' ';
				auto strlen = str.length();

				location l = offsets.find(node)->second;
				unsigned int given_length = l.end - l.begin;

				assert(given_length > strlen);

				ss << std::setw(given_length/2 + strlen/2) << std::right << str << std::setw(given_length/2 - strlen/2) << ' ';

				printed_length = given_length;
			}

			current_offset += printed_length;

			for (int i = 0; i < node->kMaxNumChildren; i++) {
				if (node->children_[i])
					nodes_to_be_printed.push_back(node->children_[i].get());
			}

		} // end loop

		ss << std::endl;
	}
	return ss.str();
}

template <class K, class C, class A>
std::string TwoFourTree<K,C,A>::Node::getString() const {
	std::stringstream ss;
	if (num_keys_ == 0)
		return "[]";

	ss << "[" << keys_[0];

	for (int i=1; i < num_keys_-1; i++) {
		ss << ", " << keys_[i];
	}
	if (num_keys_ > 1)
		ss <<", " << keys_[num_keys_-1] << "]";
	else
		ss << "]";
	return ss.str();
}

// useful when debugging, not used as part of other functions
template <class K, class C, class A>
void TwoFourTree<K,C,A>::Node::print() const {
	std::cout << getString() << std::endl;
}

template <class K, class C, class A>
void TwoFourTree<K,C,A>::const_iterator::print() const {
	std::cout << getString() << std::endl;
}

template <class K, class C, class A>
std::string TwoFourTree<K,C,A>::const_iterator::getString() const {
	std::stringstream ss;
	ss << "it: ";
	if (node_ == nullptr) {
		ss << "nullptr, idx = " << idx_ ;

	} else if (idx_ == node_->num_keys_) {
		ss << "after end iterator (node:" << node_->getString() << "), idx = " << idx_ ;

	} else if (idx_ == -1) {
		ss << "before beginning iterator (node:" << node_->getString() << "), idx = " << idx_;

	} else {
		ss << "n:" << node_->getString() << " idx_:" << idx_;
	}
	return ss.str();
}

/**
 * Goes as high up as it can then prints everything below
 */
template <class K, class C, class A>
void TwoFourTree<K, C, A>::Node::tryPrintAllFromParent(int verbosity) {
	if (verbosity < 1)
		return;
	Node *node = this;
	while (node->parent_)
		node = node->parent_;
	node->printAll();
}

} /* namespace tft */

#endif /* SRC_DEBUG_OPERATIONS_IPP_ */
