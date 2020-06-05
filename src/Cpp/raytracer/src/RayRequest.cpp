#include "RayRequest.h"

HF::RayTracer::FullRayRequest::FullRayRequest(float X, float Y, float Z, float DX, float DY, float DZ, float Distance) :
	x(X), y(Y), z(Z), dx(DX), dy(DY), dz(DZ), distance(Distance) {};

bool HF::RayTracer::FullRayRequest::didHit()
{
	return mesh_id != -1;
}
