#pragma once


namespace HF::RayTracer {
	inline bool DidIntersect(int mesh_id);
	/// <summary> A simple hit struct to carry all relevant information about hits. </summary>
	template <typename numeric_type = double>
	struct HitStruct {
		numeric_type distance;  ///< Distance from the origin point to the hit point. Set to -1 if no hit was recorded.
		int meshid; ///< The ID of the hit mesh. Set to -1 if no hit was recorded

		HitStruct() : distance(-1), meshid(-1) {}

		HitStruct(numeric_type in_distance, int in_meshid) : distance(in_distance), meshid(in_meshid) {}

		/// <summary> Determine whether or not this hitstruct contains a hit. </summary>
		/// <returns> True if the point hit, false if it did not </returns>
		inline bool DidHit() const {
			return DidIntersect(this->meshid);
		}
	};

	const int FAIL_ID = ((unsigned int)-1);
	inline bool DidIntersect(int mesh_id) {
		return mesh_id != FAIL_ID;
	}
}
