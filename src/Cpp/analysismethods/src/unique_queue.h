///
/// \file		unique_queue.h
/// \brief		Contains definitions for the <see cref="HF::GraphGenerator::UniqueQueue">UniqueQueue</see> class
///
///	\author		TBA
///	\date		26 Jun 2020

#include <robin_hood.h>
#include <queue>
#include <node.h>

#ifndef UNIQUE_QUEUE_INCLUDE_GUARD
#define UNIQUE_QUEUE_INCLUDE_GUARD

namespace HF::GraphGenerator {
	/// <summary> A queue that remembers every object inserted, and prevents the addition of nodes that have
	/// already been inserted even if they've been popped.</summary>

	/*!
		\invariant All unique nodes can only enter the queue once. After they've been popped, future attempts
		to push them will fail.

		\remarks
		This class was designed specificlaly to manage the GraphGenerator's todo list with as few hashmap
		calls as possible.

		\todo This could be a useful standalone templated class.
	*/
	class UniqueQueue {
	private:
		std::queue<HF::SpatialStructures::Node> node_queue;						///< The underlying queue
		/*! \brief Hashmap to keep track of nodes that have entered the queue.

			\details
			Values of 0 indicate that a node has never been pushed before.
			Values of 1 indicate that a node has already been in the hashmap previously.
		*/
		robin_hood::unordered_map<HF::SpatialStructures::Node, int> hashmap;

	public:

		/// <summary> Add a node to the queue if it has never previously been in the queue. </summary>
		/// <param name="p"> Node to add to the queue. </param>
		/// <returns> False if the node wasn't added, true if it was </returns>
		/*!
			\details
			If the node has a value of 1 in hashmap, then the node is ignored. If the node doesn't
			already exist in the hashmap, the call to hashmap[p] will automatically create an entry
			with a value of 0, indicating that the node hasn't been seen before.

		*/
		bool push(const HF::SpatialStructures::Node& p);

		/// <summary> Remove the topmost node from the queue and return it. </summary>
		/// <returns> The node at the top of the queue. </returns>
		/*!
			\see popFromDict for removing the topmost node and allowing it to enter again
			in the future.
		*/
		HF::SpatialStructures::Node pop();

		/// <summary> Determine how many nodes are currently in the queue. </summary>
		/// <returns> The number of nodes in the queue </returns>
		int size() const;

		/// <summary>
		/// Retrieve the topmost node from the queue, but "forget" about it to 
		/// allow it to enter again in the future.
		/// </summary>
		/// <returns> Topmost node in the queue. </returns>
		/// \details This sets the node's value to 0 in the hashmap, making it behave
		/// exactly like a node that has never been seen by the queue. 
		HF::SpatialStructures::Node popFromDict();

		/// <summary> Check to see if the node has ever been in the queue. </summary>
		/// <param name="p"> Node to check. </param>
		/// <returns> True if the node has existed in the queue. </returns>
		bool hasNode(const HF::SpatialStructures::Node& p) const;

		/// <summary>
		/// Forcibly push a node onto the queue without checking it. Saves a hash function, but
		/// risks breaking the invariant.
		/// </summary>
		/// <param name="p"> Node to add to the end of the queue. </param>
		/// <returns> Always true since it cannot fail. </returns>
		bool forcePush(const HF::SpatialStructures::Node& p);

		/// <summary> Tell if the queue is empty. </summary>
		/// <returns> False if there is atleast one node in the queue. True otherwise. </returns>
		bool empty() const;

		/// <summary> Clear every node from the queue without forgetting them. </summary>
		void clearQueue();

		/// <summary> Pop a set amount of nodes from the queue, and return them as a vector. </summary>
		/// <param name="max"> Stop after popping this amount of nodes.</param>
		/*!

			Use this to quickly pop several nodes without having to make a function call for each.
			\todo Opetion to pop every node by setting max to -1.

		*/
		std::vector<SpatialStructures::Node> popMany(int max = -1);
	};
}
#endif
