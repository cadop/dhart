///
/// \file		unique_queue.cpp
/// \brief		Contains implementation for the <see cref="HF::GraphGenerator::UniqueQueue">UniqueQueue</see> class
///
///	\author		TBA
///	\date		26 Jun 2020

#include <unique_queue.h>

namespace HF::GraphGenerator{
	bool UniqueQueue::push(const HF::SpatialStructures::Node& p)
	{
		// Only insert if it's not set to 1
		if (hashmap[p])
			return false;

		// Push it to the end of the queue, then 
		// mark it in the hashmap.
		node_queue.push(p);
		hashmap[p] = 1;
		
		return true;
	}

	HF::SpatialStructures::Node UniqueQueue::pop()
	{
		auto r = node_queue.front();
		node_queue.pop();
		return r;
	}

	int UniqueQueue::size() const {return node_queue.size();}

	HF::SpatialStructures::Node UniqueQueue::popFromDict() {
		auto r = node_queue.front();
		node_queue.pop();

		// Erase r from the hashmap to "forget" about it
		hashmap.erase(r);
		return r;
	}

	bool UniqueQueue::hasNode(const HF::SpatialStructures::Node& p) const {
		return hashmap.count(p) > 0;
	}

	bool UniqueQueue::forcePush(const HF::SpatialStructures::Node& p) {
		// Forcibly set this to 1. Will have no effect
		// if we've already seen this node.
		hashmap[p] = 1;

		node_queue.push(p);
		return true;
	}
	bool UniqueQueue::empty() const {return size() == 0;}
	
	void UniqueQueue::clearQueue()
	{
		// Queue has no clear function. We can however potentially use a swap idiom
		// to speed this up. 
		while (!node_queue.empty()) node_queue.pop();
	}

	std::vector<SpatialStructures::Node> UniqueQueue::popMany(int max)
	{
		// Create a new vector of nodes
		std::vector<SpatialStructures::Node> out_nodes;
		out_nodes.reserve(size());
		
		// Pop a node until the queue is empty, we hit max,
		// or max is never set. 
		int num_popped = 0;
		while (!node_queue.empty() && num_popped < max)
		{
			out_nodes.push_back(pop());
			++num_popped;
		}
		return out_nodes;
	}
}
