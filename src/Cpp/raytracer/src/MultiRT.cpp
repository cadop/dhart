#include <MultiRT.h>
#include <embree_raytracer.h>
#include <ray_data.h>
#include <cassert>

#include <stdio.h>
namespace HF::RayTracer {
	
	inline void Log(MultiRT::RT_Type type, std::string Message = "") {
		std::cout << "SHooting Ray. Type is";
		std::string name = "";
		switch (type) {
		case(MultiRT::EMBREE):
			name = "embree";
			break;
		case(MultiRT::NANO_RT):
			name = "nano_rt";
			break;
		default:
			"NoType!";
			break;
		}

		std::cout << name << "'" << Message << "'" << std::endl;
	}


	MultiRT::MultiRT(HF::RayTracer::EmbreeRayTracer* ert) {
		assert(ert != NULL);
		this->RayTracer = ert;
		this->type = EMBREE;
	}

	MultiRT::MultiRT(HF::RayTracer::NanoRTRayTracer* nrt) {
		assert(nrt != NULL);
		this->RayTracer = nrt;
		this->type = NANO_RT;
	}

	HitStruct<MultiRT::real_t> MultiRT::Intersect(const MultiRT::real3& origin, const MultiRT::real3& direction) {
		if (this->type == EMBREE)
			return reinterpret_cast<HF::RayTracer::EmbreeRayTracer*>(this->RayTracer)->Intersect(origin, direction);
		else if (this->type == NANO_RT)
			return reinterpret_cast<HF::RayTracer::NanoRTRayTracer*>(this->RayTracer)->Intersect(origin, direction);
		else
			assert(false);
	}

	bool MultiRT::Occluded(const MultiRT::real3& origin, const MultiRT::real3& direction, MultiRT::real_t distance) {
		if (this->type == EMBREE)
			return reinterpret_cast<HF::RayTracer::EmbreeRayTracer*>(this->RayTracer)->Occluded(origin, direction, distance);
		else if (this->type == NANO_RT)
			return reinterpret_cast<HF::RayTracer::NanoRTRayTracer*>(this->RayTracer)->Occluded(origin, direction, distance);
		else
			assert(false);
	}
}
