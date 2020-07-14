///
/// \file		node.cpp
///	\brief		Contains implementation for the <see cref="HF::SpatialStructures::Node">Node</see> structure
///
/// \author		TBA
/// \date		06 Jun 2020

#include <node.h>
#include <Constants.h>
#include <exception>
#include <cmath>

namespace HF {
	namespace SpatialStructures {
		Node::Node() {
			x = NAN;
			y = NAN;
			z = NAN;
			id = NAN;
		}

		Node::Node(float x, float y, float z, int id) : x(x), y(y), z(z), id(id) {};
		
		Node::Node(const std::array<float, 3>& position) {
			x = position[0];
			y = position[1];
			z = position[2];
			id = -1;
			type = NODE_TYPE::GRAPH;
		}

		Node::Node(const std::array<float, 3>& position, NODE_TYPE t, int id) {
			id = -1;
			type = t;
			x = position[0];
			y = position[1];
			z = position[2];
		}

		float Node::distanceTo(const Node& n2) const {
			return
				sqrtf(pow((x - n2.x), 2) +
					pow((y - n2.y), 2) +
					pow((z - n2.z), 2)
				);
		}

		//TODO: This needs an actual angle formula
		//float Node::angleTo(const Node& n2) const { return glm::angle(glm::normalize(getV3() - n2.getV3()), glm::vec3(0.0f, 0.0f, 1.0f)); }

		float& Node::operator[](int i) {
			switch (i) {
			case 0:
				return x;
				break;
			case 1:
				return y;
				break;
			case 2:
				return z;
				break;
			default:
				throw std::exception(); //TODO: Make this a custom human factors exception
			}
		}

		float Node::operator[](int i) const {
			switch (i) {
			case 0:
				return x;
				break;
			case 1:
				return y;
				break;
			case 2:
				return z;
				break;
			default:
				throw std::exception(); // TODO: Make this a custom human factors exception
			}
		}

		bool Node::operator==(const Node& n2) const {
			const float dist = sqrtf(powf(n2.x - x, 2) + powf(n2.y - y, 2)  + powf(n2.z - z, 2));
			return dist < ROUNDING_PRECISION;
		}

		//TODO: How will we handle this?
		//inline bool Node::operator==(const Node& n2) const { return distanceTo(n2) <= ROUNDING_PRECISION(); }
		//inline void Node::operator=(const std::array<float, 3>& n2) {
		//	x = n2[0];
		//	y = n2[1];
		//	z = n2[2];
		//}

		bool Node::operator!=(const Node& n2) const { return !operator==(n2); }

		// inline expansion of operator- does not work
		//inline Node Node::operator-(const Node& n2)const { return Node(x - n2.x, y - n2.y, z - n2.z); }
		Node Node::operator-(const Node& n2) const { return Node(x - n2.x, y - n2.y, z - n2.z); }
		
		// inline expansion of operator+ does not work

		Node Node::operator+(const Node& n2) const { return Node(x + n2.x, y + n2.y, z + n2.z); }

		// TODO: Dot product?
		// inline expansion of operator* does not work
		/*
		inline Node Node::operator*(const Node& n2) const {
			return Node(x * n2.x, y * n2.y, z * n2.z);
		}
		*/
		Node Node::operator*(const Node& n2) const {
			return Node(x * n2.x, y * n2.y, z * n2.z);
		}

		bool Node::operator<(const Node& n2) const { return id < n2.id; }
		bool Node::operator<(const Node& n2){return id < n2.id;	} // This needs to exist for std sort
		bool Node::operator>(const Node& n2) const { return id > n2.id; }

		// Normalize the vector by reference
		void Normalize(std::array<float, 3>& vector) {
			auto magnitude = sqrtf(powf(vector[0], 2) + powf(vector[1], 2) + powf(vector[2], 2));
			vector[0] = vector[0] / magnitude; 
			vector[1] = vector[1] / magnitude; 
			vector[2] = vector[2] / magnitude;
		}

		// Get the normalized direction between two points in space
		std::array<float, 3> Node::directionTo(const Node& n2) const {
			std::array<float, 3> direction_vector;
			direction_vector[0] = n2.x - x;
			direction_vector[1] = n2.y - y;
			direction_vector[2] = n2.z - z;

			Normalize(direction_vector);
			return direction_vector;
		}

		std::array<float, 3> Node::getArray() const {
			return std::array<float, 3>{x, y, z};
		}
	}
}