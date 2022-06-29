#pragma once

#include <array>
#include <HitStruct.h>

namespace HF::RayTracer {
	class EmbreeRayTracer;
	class NanoRTRayTracer;

	struct MultiRT {
		using real_t = double;
		using real3 = std::array<real_t, 3>;

		enum RT_Type{
			NONE,
			EMBREE,
			NANO_RT
		};

		void* RayTracer;
		RT_Type type = RT_Type::NONE;

		inline MultiRT() {};
		MultiRT(HF::RayTracer::EmbreeRayTracer* ert);

		MultiRT(HF::RayTracer::NanoRTRayTracer* nrt);

		bool Occluded(const real3 & origin, const real3& direction, real_t distance);

		HitStruct<real_t> Intersect(const real3& origin, const real3& direction);
	};
}


