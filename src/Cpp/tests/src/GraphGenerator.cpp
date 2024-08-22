#include "gtest/gtest.h"
#include <array>
#include <graph_generator.h>
#include <unique_queue.h>
#include <embree_raytracer.h>
#include <objloader.h>
#include <meshinfo.h>
#include <graph.h>
#include <Edge.h>
#include <node.h>
#include <Constants.h>
#include <graph_generator.h>
#include <objloader.h>
#include <unique_queue.h>

#include <MultiRT.h>

using HF::SpatialStructures::Graph;
using HF::GraphGenerator::GraphGenerator;
using HF::RayTracer::EmbreeRayTracer;

using HF::SpatialStructures::Node;
template<typename n1_type, typename n2_type> inline double DistanceTo(const n1_type& n1, const n2_type& n2);

EmbreeRayTracer CreateGGExmapleRT() {
	//! [EX_GraphGeneratorRayTracer]
	
	// Load an OBJ containing a simple plane
	auto mesh = HF::Geometry::LoadMeshObjects("plane.obj", HF::Geometry::ONLY_FILE, true);
	
	// Create a raytracer using this obj
	EmbreeRayTracer ray_tracer = HF::RayTracer::EmbreeRayTracer(mesh);
	
	//! [EX_GraphGeneratorRayTracer]
	return ray_tracer;
}	

const std::string obstacle_plane_path = "obstacle_plane.obj";
const std::string obstacle_layer = "Obstacle";
EmbreeRayTracer CreateObstacleExampleRT(std::string str = "") {

	// Load an OBJ containing a simple plane
	if (str.empty()) str = obstacle_plane_path;
	auto mesh = HF::Geometry::LoadMeshObjects(str, HF::Geometry::BY_GROUP, true);

	// Create a raytracer using this obj
	EmbreeRayTracer ray_tracer = HF::RayTracer::EmbreeRayTracer(mesh);

	for (const auto& m : mesh)
		std::cout << m.name << " " << m.meshid << " " << m.NumVerts() << " " << m.NumTris() << std::endl;

	return ray_tracer;
}

std::ostringstream PrintGraph(const Graph & g) {

	//![EX_PrintGraph]
	
	// Get nodes from the graph
	auto nodes = g.Nodes();

	// Construct Output String
	std::ostringstream out_str;

	out_str << "[";
	for (int i = 0; i < nodes.size(); i++)
		out_str << nodes[i] << ((i != nodes.size() - 1) ? "," : "]");

	// Print to console.
	std::cout << out_str.str() << std::endl;
	
	//![EX_PrintGraph]
	
	return out_str;
}

template <typename N, typename N2>
void ComparePoints(const N& expected_nodes, const N2& actual_nodes) {

	ASSERT_EQ(expected_nodes.size(), actual_nodes.size());

	for (int i = 0; i < actual_nodes.size(); i++) {
		auto distance = DistanceTo(expected_nodes[i], actual_nodes[i]);
		EXPECT_NEAR(0, distance, 0.000001);
	}
}

TEST(_GraphGenerator, BuildNetwork) {
	EmbreeRayTracer ray_tracer = CreateGGExmapleRT();

	//! [EX_BuildNetwork]
	
	// Create a graphgenerator using the raytracer we just created
	HF::GraphGenerator::GraphGenerator GG(ray_tracer);

	// Setup Graph Parameters
	std::array<float, 3> start_point{ 0,0,0.25 };
	std::array<float, 3> spacing{ 1,1,1 };
	int max_nodes = 5;
	int up_step = 1; int down_step = 1;
	int up_slope = 45; int down_slope = 45;
	int max_step_connections = 1;
	int min_connections = 1;

	// Generate the graph using our parameters
	HF::SpatialStructures::Graph g = GG.BuildNetwork(
		start_point,
		spacing,
		max_nodes,
		up_step, up_slope,
		down_step, down_slope,
		max_step_connections,
		min_connections
	);

	//! [EX_BuildNetwork]
	
	auto out_str = PrintGraph(g);

	// Compare output against expected output
	const std::vector<Node> expected_nodes = {
		{0, 0, -0}, {-1, -1, -0}, {-1, 0, -0},
		{-1, 1, 0},{0, -1, -0}, {0, 1, 0 },
		{1, -1, -0}, {1, 0, -0}, {1, 1, 0},
		{-2, -2, -0 }, { -2, -1, -0 },{-2, 0, -0}, Node{-1, -2, -0 },
		{0, -2, -0 }, { -2, 1, 0 }, Node(-2, 2, 0),
		{-1, 2, 0}, {0, 2, 0},{1, -2, -0}};
	const auto graph_nodes = g.Nodes();

	ASSERT_EQ(graph_nodes.size(), expected_nodes.size());

	ComparePoints(graph_nodes, expected_nodes);
}


TEST(_GraphGenerator, OutDegree) {
	// Load an OBJ containing a simple plane
	auto mesh = HF::Geometry::LoadMeshObjects("energy_blob_zup.obj", HF::Geometry::ONLY_FILE, false);

	// Create a raytracer using this obj
	EmbreeRayTracer ray_tracer = HF::RayTracer::EmbreeRayTracer(mesh);

	//! [EX_OutDegree]

	// Create a graphgenerator using the raytracer we just created
	HF::GraphGenerator::GraphGenerator GG(ray_tracer);

	// Setup Graph Parameters
	std::array<float, 3> start_point{ 0,0,20 };
	std::array<float, 3> spacing{ 1,1,1 };
	int max_nodes = 5000;
	float up_step = 0.5; float down_step = 0.5;
	int up_slope = 20; int down_slope = 20;
	int max_step_connections = 1;
	int min_connections = 4;

	// Generate the graph using our parameters
	HF::SpatialStructures::Graph g = GG.BuildNetwork(
		start_point,
		spacing,
		max_nodes,
		up_step, up_slope,
		down_step, down_slope,
		max_step_connections,
		min_connections
	);

	//! [EX_OutDegree]

	auto out_str = PrintGraph(g);

	const auto graph_nodes = g.Nodes();

	ASSERT_EQ(graph_nodes.size(), 3412);

}

TEST(_GraphGenerator, OBS_VisTestCase) {
	EmbreeRayTracer ray_tracer = CreateObstacleExampleRT("obstacle_vistestcase.obj");

	// Create a graphgenerator using the raytracer we just created
	HF::GraphGenerator::GraphGenerator GG(ray_tracer);

	// Setup Graph Parameters
	std::array<float, 3> start_point{ 3,0,0.25 };
	std::array<float, 3> spacing{ 1,1,20};
	int max_nodes = 10000;
	int up_step = 20; int down_step = 20;
	int up_slope = 45; int down_slope = 45;
	int max_step_connections = 1;
	int min_connections = 1;

	// Generate a graph without specifying obstacles, this will result on the graph not going on the
	// boxes due to the low upstep
	HF::SpatialStructures::Graph non_obstacle_graph = GG.BuildNetwork(
		start_point,
		spacing,
		max_nodes,
		1, 1,
		up_slope, down_slope,
		max_step_connections, min_connections
	);

	HF::GraphGenerator::GraphGenerator GG_Obstacle(ray_tracer, std::vector<int>{2});

	// Generating a graph that has a high enough upstep to get onto the boxes, but
	// doesn't because they're marked as obstacles
	HF::SpatialStructures::Graph obstacle_graph = GG_Obstacle.BuildNetwork(
		start_point,
		spacing,
		max_nodes,
		up_step, down_step,
		up_slope, down_slope,
		max_step_connections, min_connections
	);
	
	obstacle_graph.DumpToJson("Visgraph.json");
	ASSERT_EQ(obstacle_graph.size(), non_obstacle_graph.size());
}

TEST(_GraphGenerator, OBS_BuildNetwork) {
	EmbreeRayTracer ray_tracer = CreateObstacleExampleRT();

	// Create a graphgenerator using the raytracer we just created
	HF::GraphGenerator::GraphGenerator GG(ray_tracer);

	// Setup Graph Parameters
	std::array<float, 3> start_point{ 0,0,0.25 };
	std::array<float, 3> spacing{ 0.5,0.5,1 };
	int max_nodes = 1000;
	int up_step = 1; int down_step = 1;
	int up_slope = 45; int down_slope = 45;
	int max_step_connections = 1;
	int min_connections = 1;

	// Generate the graph using our parameters
	HF::SpatialStructures::Graph non_obstacle_graph = GG.BuildNetwork(
		start_point,
		spacing,
		max_nodes,
		up_step, down_step,
		up_slope, down_slope,
		max_step_connections, min_connections
	);

	HF::GraphGenerator::GraphGenerator GG_Obstacle(ray_tracer, std::vector<int>{2});

	// Generate the graph using our parameters
	HF::SpatialStructures::Graph obstacle_graph = GG_Obstacle.BuildNetwork(
		start_point,
		spacing,
		max_nodes,
		up_step, down_step,
		up_slope, down_slope,
		max_step_connections, min_connections
	);

	ASSERT_LT(obstacle_graph.size(), non_obstacle_graph.size());
}

TEST(_GraphGenerator, CrawlGeom) {
	EmbreeRayTracer ray_tracer = CreateGGExmapleRT();

	//! [EX_CrawlGeom]

	// Create a graphgenerator using the raytracer we just created
	HF::GraphGenerator::GraphGenerator GG(ray_tracer);

	// Set parameters for graph generation
	std::array<float, 3> start_point{ 0,1,0 };
	std::array<float, 3> spacing{ 1,1,1 };
	int max_nodes = 5;
	int up_step = 1; int down_step = 1;
	int up_slope = 45; int down_slope = 45;
	int max_step_connections = 1;
	int min_connections = 1;

	// Since we're not calling BuildNetwork, we will need to set some parameters
	// in the GraphGenerator in order to use this function standalone
	
	// These parameters need to be set on the graph generator
	GG.core_count = -1;
	GG.max_nodes = 5;
	GG.max_step_connection = 1;
	GG.min_connections = 1;
	
	// Setup its params struct
	GG.params.up_step = up_step; GG.params.down_step = down_step;
	GG.params.up_slope = up_slope; GG.params.down_slope = down_slope;
	GG.params.precision.ground_offset = 0.01; 
	GG.params.precision.node_z = 0.001f;
	GG.params.precision.node_spacing = 0.001;

	// Assign it's spacing member, making sure to convert it to real3, the graph generator's
	// internal type for vectors of 3 coordinates.
	GG.spacing = HF::GraphGenerator::CastToReal3(spacing);

	// Construct a unique queue, and push back the start point
	HF::GraphGenerator::UniqueQueue queue;
	queue.push(start_point);
	
	//! [EX_CrawlGeom]
	
	// Define Expected Output
	const std::vector<Node> expected_output = {
		{0, 1, 0},{-1, 0, -0},{-1, 1, 0},
		{-1, 2, 0},{0, 0, -0},{0, 2, 0},
		{1, 0, -0},{1, 1, 0},{1, 2, 0},
		{-2, -1, -0},{-2, 0, -0},{-2, 1, 0},
		{-1, -1, -0},{0, -1, -0},{-2, 2, 0},
		{-2, 3, 0},{-1, 3, 0},{0, 3, 0},
		{1, -1, -0} };
	
	//! [EX_CrawlGeom_Serial]
	
	// Call Crawl Geom with the queue
	auto g = GG.CrawlGeom(queue);

	//! [EX_CrawlGeom_Serial]
	
	// Check Serial
	ComparePoints(g.Nodes(), expected_output);

	//! [EX_CrawlGeom_Parallel]
	
	// Call crawl geom parallel
	g = GG.CrawlGeomParallel(queue);
	
	//! [EX_CrawlGeom_Parallel]

	// CheckParallel
	const std::vector<Node> expected_parallel = {
		{0, 2, 0},{-1, 1, 0},{-1, 2, 0},
		{-1, 3, 0},{0, 1, 0},{0, 3, 0},
		{1, 1, 0},{1, 2, 0},{1, 3, 0},
		{1, 0, -0},{0, -1, -0},{0, 0, -0},
		{1, -1, -0},{2, -1, -0},{2, 0, -0},
		{2, 1, 0},{2, 2, 0},{2, 3, 0},
		{-2, -1, -0},{-3, -2, -0},{-3, -1, -0},
		{-3, 0, -0},{-2, -2, -0},{-2, 0, -0},
		{-1, -2, -0},{-1, -1, -0},{-1, 0, -0} };

	ComparePoints(expected_parallel, g.Nodes());
}

TEST(_GraphGenerator, ValidateStartPoint) {
	EmbreeRayTracer ray_tracer = CreateGGExmapleRT();

	//! [EX_ValidateStartPoint]

	// Define tolerances
	HF::GraphGenerator::Precision precision;
	precision.node_z = 0.01f;
	precision.node_spacing = 0.001f;
	precision.ground_offset = 0.001f;

	// Create graphParameters to hold tolerances
	HF::GraphGenerator::GraphParams params;
	params.precision = precision;

	// Setup start_point
	HF::GraphGenerator::real3 start_point{ 0,0,10 };

	// Call ValidateStartPoint
	HF::RayTracer::MultiRT multi_rt(&ray_tracer);
	HF::GraphGenerator::optional_real3 result = HF::GraphGenerator::ValidateStartPoint(
		multi_rt, start_point, params
	);

	// If the ray intersected, print the result
	if (result)
	{
		auto result_point = *result;
		printf("(%0.000f, %0.000f, %0.000f)\n", result_point[0], result_point[1], result_point[2]);
	}

	// If it didn't print a message
	else
		printf("No intersection found\n");

	//! [EX_ValidateStartPoint]

	// Assert that the ray hit and the start point was correctly updated
	// to the point of intersection
	ASSERT_TRUE(result);
	ASSERT_TRUE(result.pt[0] == 0 && result.pt[1] == 0 && result.pt[2] == 0);
}

TEST(_GraphGenerator, CheckRay) {
	EmbreeRayTracer ray_tracer = CreateGGExmapleRT();

	//! [EX_CheckRay]

	// Define z tolerance
	HF::GraphGenerator::real_t node_z = 0.01;

	// Create a start  point 10 units above the mesh, and
	// a direction vector facing straight down.
	HF::GraphGenerator::real3 start_point{ 1,1,1 };
	HF::GraphGenerator::real3 direction{0,0,-1};

	// Call CheckRay and capture the result
	HF::RayTracer::MultiRT multi_rt(&ray_tracer);
	HF::GraphGenerator::optional_real3 result = HF::GraphGenerator::CheckRay(
		multi_rt, start_point, direction, node_z);

	// If the ray intersected, print it
	if (result)
	{
		auto result_point = *result;
		printf("(%0.000f, %0.000f, %0.000f)\n", result_point[0], result_point[1], result_point[2]);
	}

	// If it didn't print a message
	else
		printf("No intersection found\n");

	//! [EX_CheckRay]

	// Assert that the ray hit and the start point was correctly updated
	// to the point of intersection
	ASSERT_TRUE(result);
	ASSERT_TRUE(result.pt[0] == 1 && result.pt[1] == 1 && result.pt[2] == 0);
}

TEST(_GraphGenerator, CreateDirecs) {
	
	//! [EX_CreateDirecs]

	// Call create direcs with a max_step_conections of 2
	auto directions = HF::GraphGenerator::CreateDirecs(2);

	// Construct Output String
	std::ostringstream out_str;

	out_str << "[";
	for (int i = 0; i < directions.size(); i++)
		out_str << "(" << directions[i].first << ", " << directions[i].second << ")" << ((i != directions.size() - 1) ? "," : "]");

	// Print to console.
	std::cout << out_str.str() << std::endl;
	
	//! [EX_CreateDirecs]

	std::string expected_output = "[(-1, -1),(-1, 0),(-1, 1),(0, -1),(0, 1),(1, -1),(1, 0),(1, 1),(-2, -1),(-2, 1),(-1, -2),(-1, 2),(1, -2),(1, 2),(2, -1),(2, 1)]";
	ASSERT_EQ(expected_output, out_str.str());
}

TEST(_GraphGenerator, GeneratePotentialChildren) {
	//! [EX_CreatePotentialChildren]

	// Create a parent node and set the spacing for these offsets
	HF::GraphGenerator::real3 parent = { 0,0,1 };
	HF::GraphGenerator::real3 spacing = { 1,2,3 };

	// Create a vector of directions to offset it
	std::vector<HF::GraphGenerator::pair> directions = { {0,1}, {0,2}, {1,0}, {2,0}, {1,1}, {2,1} };

	// Construct a GraphParams with the spacing filled out
	HF::GraphGenerator::GraphParams gp;
	gp.precision.node_spacing = 0.001f;
	gp.precision.node_z = 0.001f;

	// Call CreateDirecs
	auto children = HF::GraphGenerator::GeneratePotentialChildren(parent, directions, spacing, gp);

	// Create Output
	std::ostringstream out_str;
	
	out_str << "[";
	for (int i = 0; i < directions.size(); i++)
		out_str << "(" << children[i][0] << ", " << children[i][1] << ", " 
		<< children[i][2] << ")" << ((i != directions.size() - 1) ? "," : "]");

	// Print to console
	std::cout << out_str.str() << std::endl;

	//![EX_CreatePotentialChildren]

	std::string expected_output = "[(0, 2, 4),(0, 4, 4),(1, 0, 4),(2, 0, 4),(1, 2, 4),(2, 2, 4)]";
	ASSERT_EQ(expected_output, out_str.str());
}

TEST(_GraphGenerator, GetChildren) {
	EmbreeRayTracer ray_tracer = CreateGGExmapleRT();
	
	//! [EX_CreateChildren]

	// Create a parent node
	HF::GraphGenerator::real3 parent{ 0,0,1 };
	
	// Create a vector of possible children
	std::vector<HF::GraphGenerator::real3> possible_children{
		HF::GraphGenerator::real3{0,2,1}, HF::GraphGenerator::real3{1,0,1},
		HF::GraphGenerator::real3{0,1,1}, HF::GraphGenerator::real3{2,0,1}
	};
	
	// Create graph parameters
	HF::GraphGenerator::GraphParams params;
	params.up_step = 2; params.down_step = 2;
	params.up_slope = 45; params.down_slope = 45;
	params.precision.node_z = 0.01f;
	params.precision.ground_offset = 0.01f;

	// Call GetChildren
	HF::RayTracer::MultiRT multi_rt(&ray_tracer);
	auto edges = HF::GraphGenerator::GetChildren(parent, possible_children, multi_rt, params);

	// Print children
	std::ostringstream out_str;
	out_str << "[";
	for (int i = 0; i < edges.size(); i++)
		out_str << "(" << edges[i].child << ", " << edges[i].score << ", " << edges[i].step_type << ")" << ((i != edges.size() - 1) ? "," : "]");
	
	std::cout << out_str.str() << std::endl;
		
	//! [EX_CreateChildren]

	ASSERT_LT(0, edges.size());

	std::string expected_output = "[((0, 2, 0), 2.23607, 1),((2, 0, -0), 2.23607, 1)]";
	ASSERT_EQ(expected_output, out_str.str());
}

template<typename n1_type, typename n2_type>
inline double DistanceTo(const n1_type& n1, const n2_type& n2) {
	return sqrt(pow((n1[0] - n2[0]), 2) + pow((n1[1] - n2[1]), 2) + pow((n1[2] - n2[2]), 2));
}

TEST(_GraphGenerator, CheckChildren) {
	EmbreeRayTracer ray_tracer = CreateGGExmapleRT();

	//! [EX_CheckChildren]

	// Create a parent node
	HF::GraphGenerator::real3 parent{ 0,0,1 };

	// Create a vector of possible children
	std::vector<HF::GraphGenerator::real3> possible_children{
		HF::GraphGenerator::real3{0,2,1}, HF::GraphGenerator::real3{1,0,1},
		HF::GraphGenerator::real3{0,1,1}, HF::GraphGenerator::real3{2,0,1}
	};

	// Create graph parameters
	HF::GraphGenerator::GraphParams params;
	params.up_step = 2; params.down_step = 2;
	params.up_slope = 45; params.down_slope = 45;
	params.precision.node_z = 0.01f;
	params.precision.ground_offset = 0.01f;

	// Call CheckChildren 
	HF::RayTracer::MultiRT multi_rt(&ray_tracer);
	auto valid_children = HF::GraphGenerator::CheckChildren(parent, possible_children, multi_rt, params);

	// Print children
	std::ostringstream out_str;
	out_str << "[";
	for (int i = 0; i < valid_children.size(); i++)
		out_str << "(" << valid_children[i][0] << ", " << valid_children[i][1] << ", " << valid_children[i][2] << ")" << ((i != valid_children.size() - 1) ? "," : "]");

	std::cout << out_str.str() << std::endl;

	//! [EX_CheckChildren]

	const std::vector<HF::GraphGenerator::real3>  correct_children = {
		{0,2,0}, {1,0,-0}, {0,1,0}, {2,0,-0}
	};

	ASSERT_EQ(valid_children.size(), correct_children.size());
	for (int i = 0; i < valid_children.size(); i++) {
		const auto& actual_child = valid_children[i];
		const auto& expected_child = correct_children[i];
		EXPECT_NEAR(0, DistanceTo(actual_child, expected_child), 0.00001);
	}
}
TEST(_GraphGenerator, CheckConnection) {
	EmbreeRayTracer ray_tracer = CreateGGExmapleRT();

	//! [EX_CheckConnection]

	// Create a parent node
	HF::GraphGenerator::real3 parent{ 0,0,1 };

	// Create a vector of possible children
	std::vector<HF::GraphGenerator::real3> possible_children{
		HF::GraphGenerator::real3{0,2,0}, HF::GraphGenerator::real3{1,0,0},
		HF::GraphGenerator::real3{0,1,0}, HF::GraphGenerator::real3{2,0,0}
	};

	// Create graph parameters
	HF::GraphGenerator::GraphParams params;
	params.up_step = 2; params.down_step = 2;
	params.up_slope = 45; params.down_slope = 45;
	params.precision.node_z = 0.01f;
	params.precision.ground_offset = 0.01f;

	// Loop through potential children call each one with check connection
	std::vector<HF::SpatialStructures::STEP> connections;
	for (const auto& child : possible_children)
	{
		HF::RayTracer::MultiRT multi_rt(&ray_tracer);
		connections.push_back(HF::GraphGenerator::CheckConnection(parent, child, multi_rt, params));
	}

	// Print children
	std::ostringstream out_str;
	out_str << "[";
	for (int i = 0; i < connections.size(); i++)
		out_str << connections[i] << ((i != connections.size() - 1) ? "," : "]");

	// In the output, 0s indicate no connection, while 1s indicate that nodes are on a flat plane
	// with no step between them. 
	std::cout << out_str.str() << std::endl;

	//! [EX_CheckConnection]

	std::string expected_output = "[1,0,0,1]";
	ASSERT_EQ(expected_output, out_str.str());
}


TEST(_GraphGenerator, CheckSlope) {
	
	//! [EX_CheckSlope]
	
	// Setup graph parameters so the slope limits are 30 degrees in both directions.
	HF::GraphGenerator::GraphParams gp;
	gp.up_slope = 30; gp.down_slope = 30;

	// Create a parent, a child that's traversble, and a child that's too steep
	// to pass the slope check
	HF::GraphGenerator::real3 parent { 0,0,0 };
	HF::GraphGenerator::real3 child_1 { 0,1,0.5 };
	HF::GraphGenerator::real3 child_2 { 0,1,1};

	// Perform slope checks
	bool slope_check_child_1 = HF::GraphGenerator::CheckSlope(parent, child_1, gp);
	bool slope_check_child_2 = HF::GraphGenerator::CheckSlope(parent, child_2, gp);
	
	std::cout << "Slope Check For Child 1 = " << (slope_check_child_1 ? "True" : "False") << std::endl;
	std::cout << "Slope Check For Child 2 = " << (slope_check_child_2 ? "True" : "False") << std::endl;

	//! [EX_CheckSlope]
	
	ASSERT_TRUE(slope_check_child_1);
	ASSERT_FALSE(slope_check_child_2);
}

TEST(_GraphGenerator, OcclusionCheck) {
	EmbreeRayTracer ray_tracer = CreateGGExmapleRT();

	//! [EX_CheckOcclusion]

	// Create a parent node, a child that has a clear line of sight, then
	// a child that is underneath the plane
	HF::GraphGenerator::real3 parent{ 0,0,1 };
	HF::GraphGenerator::real3 child_1{ 0,0,-3 };
	HF::GraphGenerator::real3 child_2{ 0,0,1 };

	// Perform slope checks
	HF::RayTracer::MultiRT multi_rt1(&ray_tracer);
	HF::RayTracer::MultiRT multi_rt2(&ray_tracer);
	bool occlusion_check_child_1 = HF::GraphGenerator::OcclusionCheck(parent, child_1, multi_rt1);
	bool occlusion_check_child_2= HF::GraphGenerator::OcclusionCheck(parent, child_2, multi_rt2);

	std::cout << "Occlusion Check For Child 1 = " << (occlusion_check_child_1 ? "True" : "False") << std::endl;
	std::cout << "Occlusion Check For Child 2 = " << (occlusion_check_child_2 ? "True" : "False") << std::endl;
	
	//! [EX_CheckOcclusion]

	ASSERT_TRUE(occlusion_check_child_1);
	ASSERT_FALSE(occlusion_check_child_2);
}



