#include <robin_hood.h>
#include <queue>
#include <node.h>

#ifndef UNIQUE_QUEUE_INCLUDE_GUARD
#define UNIQUE_QUEUE_INCLUDE_GUARD

namespace HF {
	namespace AnalysisMethods {
		//TODO: Make this generic?
		/// <summary>
		/// A queue that ensures every unique object can only enter the queue once
		/// </summary>
		class UniqueQueue {
		private:
			std::queue<HF::SpatialStructures::Node> node_queue;						///< A std::queue of Node ///<
			robin_hood::unordered_map<HF::SpatialStructures::Node, int> hashmap;	///< a hashmap of K = Node, V = int ///<

		public:

			/// <summary>
			/// Add a node to the queue if it has never previously been in the queue.
			/// </summary>
			/// <param name="p">The node to add</param>
			/// <returns>False if the node wasn't added, true if it was</returns>
			bool push(const HF::SpatialStructures::Node& p);

			/// <summary>
			/// Remove the topmost node from the queue and return it
			/// </summary>
			/// <returns>The node at the top of the stack</returns>
			HF::SpatialStructures::Node pop();

			/// <summary>
			/// Determine how many nodes are currently in the queue
			/// </summary>
			/// <returns>The amount of nodes in the queue</returns>
			int size() const;

			/// <summary>
			/// Remove the topmost node from the queue, but allow it to enter again in the future
			/// </summary>
			/// <returns>Topmost node in the queue</returns>
			HF::SpatialStructures::Node popFromDict();

			/// <summary>
			/// Check to see if the node has ever been in the queue
			/// </summary>
			/// <param name="p">Node to check the existance of </param>
			/// <returns>True if the node has existed in the queue</returns>
			bool hasNode(const HF::SpatialStructures::Node& p) const;

			/// <summary>
			/// Forcibly push a node onto the queue without checking it. Saves a hash function, but risks breaking
			/// the invariant.
			/// </summary>
			/// <param name="p">Node to push</param>
			/// <returns></returns>
			bool forcePush(const HF::SpatialStructures::Node& p);

			/// <summary>
			/// Tell if the queue is empty
			/// </summary>
			/// <returns>False if there is atleast one node in the queue</returns>
			bool empty() const;


			/// <summary>
			/// Clear every node from the queue, but not the dictionary
			/// </summary>
			void clearQueue();

			/// <summary>
			/// Pop a set amount of nodes from the queue, and return them as a vector
			/// <param name="max">Maximum allowed amount of nodes. If set to -1, pop all</para>
			/// </summary>
			std::vector<SpatialStructures::Node> popMany(int max = -1);
		};
	}
}

#endif
