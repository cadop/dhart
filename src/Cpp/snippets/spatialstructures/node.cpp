/// std::array<float, 3> directionTo(const Node& n2) const;
/// [Node__directionTo]

/// [Node__directionTo]

/// std::array<float, 3> getArray() const;
/// [Node__getArray]
Node node(12.0, 23.1, 34.2, 456);

// An array is created from within the getArray() member function
std::array<float, 3> arr = node.getArray();

// ref_arr and ref_node have the same value, but
// refer to different locations in memory -- 
// arr does not consist of the same memory locations as
// that of the coordinate fields within node.
float& ref_arr = arr[0];
float& ref_node = node.x;
/// [Node__getArray]

/// float& operator[](int i);
/// [Node__operator_bracket_ref]
Node node(12.0, 23.1, 34.2, 456);	// (x, y, z), ID

float& position = node[1];			// access by reference
position = 93.5						// node.y is now 93.5
/// [Node__operator_bracket_ref]

/// float operator[](int i);
/// [Node__operator_bracket_val]
Node node(12.0, 23.1, 34.2, 456);	// (x, y, z), ID

float position = node[1];			// access by reference
position = 93.5						// node.y is still 23.1
/// [Node__operator_bracket_val]

/// bool operator==(const Node& n2) const;
/// [Node__operator_equality]
Node node_0(12.0, 23.1, 34.2, 456);
Node node_1(45.3, 56.4, 67.5, 789);

bool same_position = node_0 == node_1;

if (same_position) {
	std::cout << "Occupies the same space" << std::endl;
} else {
	std::cout << "Different positions" << std::endl;
}

// same_position evaluates to true
/// [Node__operator_equality]

/// void operator=(const std::array<float, 3>& n2);
/// [Node__operator_assignment]
Node node(12.0, 23.1, 34.2);
std::array<float, 3> position = { 45.3, 56.4, 67.5 };

node = position;			// assigns node's x, y, z fields to that of position's values
/// [Node__operator_assignment]

/// bool operator!=(const Node& n2) const;
/// [Node__operator_nequality]
Node node_0(12.0, 23.1, 34.2, 456);
Node node_1(45.3, 56.4, 67.5, 789);

// Does the inverse of operator==.
bool different_positions = node_0 != node_1;

if (different_positions) {
	std::cout << "Different positions" << std::endl;
} else {
	std::cout << "Occupies the same space" << std::endl;
}

// different_positions evaluates to true
/// [Node__operator_nequality]

/// Node operator-(const Node& n2) const;
/// [Node__operator_minus]
Node node_0(12.0, 23.1, 34.2, 456);
Node node_1(45.3, 56.4, 67.5, 789);

Node node_2 = node_1 - node_0;

// node_2 has values (x = 33.3, y = 33.3, z = 33.3, id = -1, type = NODE_TYPE::GRAPH)
// id and type are given default values as per Node::Node(const std::array<float, 3>& position)
/// [Node__operator_minus]

/// Node operator+(const Node& n2) const;
/// [Node__operator_plus]
Node node_0(12.0, 23.1, 34.2, 456);
Node node_1(45.3, 56.4, 67.5, 789);

Node node_2 = node_1 + node_0;

// node_2 has values (x = 57.3, y = 79.5, z = 101.7, id = -1, type = NODE_TYPE::GRAPH)
// id and type are given default values as per Node::Node(const std::array<float, 3>& position)
/// [Node__operator_plus]

/// Node operator*(const Node& n2) const;
/// [Node__operator_multiply]
Node node_0(12.0, 23.1, 34.2, 456);
Node node_1(45.3, 56.4, 67.5, 789);

Node node_2 = node_1 * node_0;

// node_2 has values (x = 543.6, y = 1302.84, z = 2308.5, id = -1, type = NODE_TYPE::GRAPH)
// id and type are given default values as per Node::Node(const std::array<float, 3>& position)
/// [Node__operator_multiply]

/// bool operator<(const Node& n2) const;
/// [Node__operator_lessthan]
Node node_0(12.0, 23.1, 34.2, 456);
Node node_1(45.3, 56.4, 67.5, 789);

// operator< compares ID fields of node_0 and node_1
bool compare = node_0 < node_1;		// evaluates to true, since 456 < 789
/// [Node__operator_lessthan]

/// bool operator<(const Node& n2);
/// [Node__operator_lessthan_sort]
// For this example, we are not concerned about the node coordinates.
Node node_0(0.0, 0.0, 0.0, 3);
Node node_1(0.0, 0.0, 0.0, 1);
Node node_2(0.0, 0.0, 0.0, 2);
Node node_3(0.0, 0.0, 0.0, 0);

std::vector<Node> vec{ node_0, node_1, node_2, node_3 };

// operator< sorts Node by ID, in non-decreasing order
std::sort(vec.begin(), vec.end());	// uses natural ordering through operator<, non-const

std::vector<Node>::iterator it = vec.begin();

while (it != vec.end()) {
	std::cout << "Node ID: " << it->id << std::endl;
	++it;
} // Node ID will print in order by ID, from smallest to largest
/// [Node__operator_lessthan_sort]

// bool operator>(const Node& n2) const;
/// [Node__operator_greaterthan]
Node node_0(12.0, 23.1, 34.2, 456);
Node node_1(45.3, 56.4, 67.5, 789);

// operator< compares ID fields of node_0 and node_1
bool compare = node_0 > node_1;		// evaluates to false, since 456 < 789
/// [Node__operator_greaterthan]