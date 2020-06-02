#include <unique_queue.h>

namespace HF {
	namespace AnalysisMethods {
		bool UniqueQueue::push(const HF::SpatialStructures::Node& p)
		{
			// Only insert if it's not set to 1
			if (hashmap[p])
				return false;

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

		int UniqueQueue::size() const {
			return node_queue.size();
		}

		HF::SpatialStructures::Node UniqueQueue::popFromDict() {
			auto r = node_queue.front();
			node_queue.pop();
			hashmap.erase(r);

			return r;
		}

		bool UniqueQueue::hasNode(const HF::SpatialStructures::Node& p) const {
			return hashmap.count(p) > 0;
		}

		bool UniqueQueue::forcePush(const HF::SpatialStructures::Node& p) {
			hashmap[p] = 1;
			node_queue.push(p);
			return true;
		}
		bool UniqueQueue::empty() const {
			return size() == 0;
		}
		void UniqueQueue::clearQueue()
		{
			while(!node_queue.empty()) node_queue.pop();
		}
		std::vector<SpatialStructures::Node> UniqueQueue::popMany(int max)
		{
			std::vector<SpatialStructures::Node> out_nodes;
			out_nodes.reserve(size());
			int num_popped = 0;
			while (!node_queue.empty() && num_popped < max)
			{
				out_nodes.push_back(pop());
				++num_popped;
			}
			return out_nodes;
		}
	}
}