/*

	- Generic 2D search tree
	Written by Phillip Jenks 2016

	Usage:
	Provide a predicate that implements the SearchPredicate<Value, NodeCompare> interface
	where Value is the value type of objects inserted into the tree and NodeCompare is 
	the type that defines the search space for a node. i.e. Value could be a type with
	a defined vector position and NodeCompare could be a Rect

	The search space for each node is divided into four quadrants. A value can belong to
	more than one quadrant.
*/

#ifndef __SEARCH_TREE_2D_H_
#define __SEARCH_TREE_2D_H_

#include <vector>
#include <set>
#include <map>

// Utility enum to mark each search quadrant
// The values are chosen to allow bitwise operations
// since values can belong to more than one quadrant
enum class RegionCode {
	UPPER_LEFT = 1 << 0,
	UPPER_RIGHT = 1 << 1,
	LOWER_LEFT = 1 << 2,
	LOWER_RIGHT = 1 << 3
};

const std::size_t g_minDataSize = 3;

//=======================================
// Implementation interface
//=======================================
template<class Value, class NodeCompare>
class SearchPredicate {
public:
	// Returns default value for the node comparison type
	virtual NodeCompare nilCompare() = 0;

	// Used for the root node. Builds the root search space from a set of values
	// belonging to the tree
	// inputs:
	//		values - set of values to belonging to the tree
	// outputs:
	//		Search space used as the root search space for the tree
	virtual NodeCompare buildRegionFromData(const std::set<Value>& values) = 0;

	// Subdivides the search space of a parent into quadrants given a set of values belonging
	// to the parent
	// inputs: 
	//		parentRegion - search space for the parent node
	//		values - values belonging to the parent
	//		quads - A mapping of Region code to child search spaces. The NodeCompare values will be used to build the child nodes
	virtual void buildQuadrantsFromData(const NodeCompare& parentRegion, const std::set<Value>& values, const std::map<RegionCode, NodeCompare&>& quads) = 0;

	// Returns whether or not a value belongs to a node's search space
	// inputs:
	//		nodeCompare - a 2D search space
	//		val - Value to test against the search space
	// outputs:
	//		returns true if the value belongs to the search space
	virtual bool satisfies(const NodeCompare& nodeCompare, Value val) = 0;

	// Returns whether or not two search spaces overlap
	// Used to return all values that belong to a test search space
	// i.e. given a Rect, used to find all values that belong to nodes whose search space
	// overlaps the test Rect
	// inputs:
	//		compareLeft, compareRight - Two search spaces to test against each other
	// outputs:
	//		returns true if the search spaces overlap. false otherwise
	virtual bool overlaps(const NodeCompare& compareLeft, const NodeCompare& compareRight) = 0;
};


//=======================================
// Main Tree Interface
//=======================================
template<class Value, class NodeCompare>
class SearchTree2D {

	using Predicate = SearchPredicate<Value, NodeCompare>;
	using SetValue = std::set<Value>;

public:


	// Constructors. A predicate should be provided as the tree will not operate without one
	// Memory management for the predicate is assumed to be handled by the caller
	SearchTree2D();
	SearchTree2D(Predicate*);
	~SearchTree2D();

	// Sets the predicate for the tree
	// Memory management for the predicate is assumed to be handled by the caller
	void setPredicate(Predicate*);

	// Inserts a value into the tree
	// This may leave the tree unbalanced
	void add(const Value& val);

	// Removes a value from the tree
	void remove(const Value& val);


	// Empties the tree
	void clear();

	// Returns all values belonging to nodes whose search spaces overlap (as defined by the predicate)
	// with the input search space
	SetValue getNearbyValues(const NodeCompare& compare) const;

	// Rebalances the tree, possibly removing or adding nodes as necessary.
	// This should be called if the location of values in the tree may have changed
	// as the tree will not update on value changes
	void rebalance();

private:

	// Private Node class used for nodes in the tree
	class Node {
	public:

		// Constructor. Uses the parent's predicate
		Node(Predicate* pred);
		~Node();

		// Sets the search predicate for this node and all child nodes
		void setPredicate(Predicate* pred);

		// Adds value to the node
		void add(const Value& val);

		// Removes value from the node
		void remove(const Value& val);

		// clears the node
		void clear();

		// Returns all values belonging to nodes whose search spaces overlap (as defined by the predicate)
		// with the input search space
		SetValue getNearbyValues(const NodeCompare& compare) const;

		// Uses this node's data to build the search space as defined
		// by the predicate for the root node.
		void buildRootRegion();

		// Rebalances the tree, creating and deleting nodes as necessary
		void rebalance();

	private:

		using RegionMap = std::map<RegionCode, Node*>;
		using QuadMap = std::map<RegionCode, NodeCompare&>;
		using QuadPair = std::pair<RegionCode, NodeCompare&>;

		// search predicate
		Predicate* m_predicate;

		// node's search space
		NodeCompare m_compare;

		// map holding child nodes keyed by region
		RegionMap m_mapRegions;

		// data belonging to this node (should be empty if this node has children)
		SetValue m_data;

		// Returns true if this node has children
		bool hasChildren() const;

		// Returns all values belonging to this node and its children
		SetValue getAllChildValues() const;

		// Deletes child nodes
		void deleteChildren();

		// Returns false if this node should be a leaf in the tree
		bool shouldSubdivide(const SetValue& values, const QuadMap& quads) const;

		// Sets the search space for this node
		void setCompare(const NodeCompare& compare);
	};

	Predicate* m_predicate;
	Node* m_tree;
};


// =========================================================
// Main Tree Implementation
// =========================================================
template<class Value, class NodeCompare>
SearchTree2D<Value, NodeCompare>::SearchTree2D(Predicate* pred)
	: m_predicate(pred)
	, m_tree(nullptr)
{}

template<class Value, class NodeCompare>
SearchTree2D<Value, NodeCompare>::SearchTree2D()
	: m_predicate(nullptr)
	, m_tree(nullptr)
{}

template<class Value, class NodeCompare>
SearchTree2D<Value, NodeCompare>::~SearchTree2D() {

	clear();
	if (m_tree) {
		delete m_tree;
	}
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::setPredicate(Predicate* pred) {
	m_predicate = pred;
	if (m_tree) {
		m_tree->setPredicate(pred);
	}
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::add(const Value& val) {
	if (!m_predicate) {
		return;
	}

	if (!m_tree) {
		m_tree = new Node(m_predicate);
	}

	m_tree->add(val);
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::remove(const Value& val) {
	if (m_tree) {
		m_tree->remove(val);
	}
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::clear() {
	if (m_tree) {
		m_tree->clear();
	}
}

template<class Value, class NodeCompare>
auto SearchTree2D<Value, NodeCompare>::getNearbyValues(const NodeCompare& compare) const -> SetValue {

	if (m_tree) {
		return m_tree->getNearbyValues(compare);
	}
	else {
		return SetValue();
	}
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::rebalance() {

	if (m_tree) {
		// Build the root search space for our tree
		m_tree->buildRootRegion();

		// Rebalance the tree for the new search space
		m_tree->rebalance();
	}
}

// =========================================================
// Node Implementation
// =========================================================
template<class Value, class NodeCompare>
SearchTree2D<Value, NodeCompare>::Node::Node(SearchPredicate<Value, NodeCompare>* predicate)
	: m_predicate(predicate)
	, m_compare()
	, m_mapRegions()
	, m_data()
{
	if (predicate) {
		m_compare = predicate->nilCompare();
	}

	// build our child node mapping
	m_mapRegions[RegionCode::UPPER_LEFT] = nullptr;
	m_mapRegions[RegionCode::UPPER_RIGHT] = nullptr;
	m_mapRegions[RegionCode::LOWER_LEFT] = nullptr;
	m_mapRegions[RegionCode::LOWER_RIGHT] = nullptr;
}

template<class Value, class NodeCompare>
SearchTree2D<Value, NodeCompare>::Node::~Node() {
	
	clear();
	deleteChildren();
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::Node::setPredicate(SearchPredicate<Value, NodeCompare>* predicate) {
	// set our predicate
	m_predicate = predicate;

	// set child predicates
	for (auto region : m_mapRegions) {
		if (region.second) {
			region.second->setPredicate(predicate);
		}
	}
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::Node::add(const Value& val) {

	// no predicate? We shouldn't add values if we have no predicate
	if (!m_predicate) {
		return;
	}

	if (hasChildren()) {
		for (auto region : m_mapRegions) {
			// Check children of they should hold the value
			if (region.second && m_predicate->satisfies(region.second->m_compare, val)) {
				region.second->add(val);
			}
		}
	}
	else {
		m_data.insert(val);
	}
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::Node::remove(const Value& val) {

	if (hasChildren()) {
		for (auto region : m_mapRegions) {
			if (region.second) {
				region.second->remove(val);
			}
		}
	}
	else {
		m_data.erase(val);
	}
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::Node::clear() {
	if (hasChildren()) {
		for (auto region : m_mapRegions) {
			if (region.second) {
				region.second->clear();
			}
		}
		deleteChildren();
	}
	else {
		m_data.clear();
	}
}

template<class Value, class NodeCompare>
auto SearchTree2D<Value, NodeCompare>::Node::getNearbyValues(const NodeCompare& compare) const -> SetValue {

	// Our return set
	SetValue nearbyVals;
	if (hasChildren()) {
		// Check children and get their values if compare overlaps with the childs's search space
		for (auto region : m_mapRegions) {
			if (region.second) {
				SetValue childVals = region.second->getNearbyValues(compare);
				nearbyVals.insert(childVals.begin(), childVals.end());
			}
		}
	}
	else {
		// return our values if compare overlaps with our search space
		if (m_predicate && m_predicate->overlaps(m_compare, compare)) {

			// std::set guarantees uniqueness (values may belong to more than one node)
			nearbyVals.insert(m_data.begin(), m_data.end());
		}
	}

	return nearbyVals;
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::Node::buildRootRegion() {

	if (!m_predicate) {
		return;
	}

	// Build our search space based off of our data
	m_compare = m_predicate->buildRegionFromData(getAllChildValues());
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::Node::rebalance() {
	
	if (!m_predicate) {
		return;
	}

	SetValue setAllData = getAllChildValues();

	// Remove data that no longer satisfies this node's compare
	for (SetValue::iterator itSet = setAllData.begin(); itSet != setAllData.end(); ) {
		if (!m_predicate->satisfies(m_compare, *itSet)) {
			setAllData.erase(itSet++);
		}
		else {
			++itSet;
		}
	}

	if (hasChildren()) {
		// Should we keep the children?
		// first just check raw data size

		if (setAllData.size() <= g_minDataSize) {
			// Our data set is small enough that we don't need children for our search space
			deleteChildren();
			m_data = setAllData;
		}
		else {

			// Now check if subdividing will do anything

			// Let's update our child quadrants
			// First grab references to our children's search spaces
			QuadMap mapQuads;
			for (auto region : m_mapRegions) {
				mapQuads.insert(QuadPair(region.first, region.second->m_compare));
			}

			// Use our predicate to rebuild our quadrant search spaces
			m_predicate->buildQuadrantsFromData(m_compare, setAllData, mapQuads);

			// Do we still need children?
			if (shouldSubdivide(setAllData, mapQuads)) {

				// Re-add our data to our children
				for (auto thisVal : setAllData) {
					add(thisVal);
				}

				// rebalance our child nodes
				for (auto region : m_mapRegions) {
					if (region.second) {
						region.second->rebalance();
					}
				}

				// ensure we're not holding onto duplicate data. The child nodes hold all children
				m_data.clear();
			}
			else {
				// We no longer need children. Just hold onto the data ourselves
				deleteChildren();
				m_data = setAllData;
			}
		}
	} 
	// Should we subdivide and create children?
	else {
		// check raw data size
		if (setAllData.size() <= g_minDataSize) {
			// The data set is small enough that we don't need to create children
			m_data = setAllData;
		}
		else {
			
			// Let's build some test quads and see if they will subdivide

			NodeCompare ulComp = m_predicate->nilCompare();
			NodeCompare urComp = m_predicate->nilCompare();
			NodeCompare llComp = m_predicate->nilCompare();
			NodeCompare lrComp = m_predicate->nilCompare();

			QuadMap mapQuads;
			mapQuads.insert(QuadPair(RegionCode::UPPER_LEFT, ulComp));
			mapQuads.insert(QuadPair(RegionCode::UPPER_RIGHT, urComp));
			mapQuads.insert(QuadPair(RegionCode::LOWER_LEFT, llComp));
			mapQuads.insert(QuadPair(RegionCode::LOWER_RIGHT, lrComp));

			// Build our test quads from our data
			m_predicate->buildQuadrantsFromData(m_compare, setAllData, mapQuads);

			// Do we need children?
			if (shouldSubdivide(setAllData, mapQuads)) {

				// We need children, so build some child nodes and set their search spaces
				for (auto& region : m_mapRegions) {
					if (!region.second) {
						region.second = new Node(m_predicate);
					}
					region.second->setCompare(mapQuads.at(region.first));
				}

				// Add the values to our children
				for (auto thisVal : setAllData) {
					add(thisVal);
				}

				// Rebalance our newly created children so they may create
				// children of their own
				for (auto region : m_mapRegions) {
					if (region.second) {
						region.second->rebalance();
					}
				}

				// We no longer need to hold onto this data
				m_data.clear();
			}
			else {

				// We don't need children
				m_data = setAllData;
			}
		}
	}
}

template<class Value, class NodeCompare>
bool SearchTree2D<Value, NodeCompare>::Node::hasChildren() const {

	// Check if we have at least one child
	bool hasChild = false;
	for (auto region : m_mapRegions) {
		if (region.second) {
			hasChild = true;
			break;
		}
	}
	return hasChild;
}

template<class Value, class NodeCompare>
auto SearchTree2D<Value, NodeCompare>::Node::getAllChildValues() const -> SetValue {

	// Gather all data belonging to this search space
	SetValue setData;
	if (hasChildren()) {
		for (auto region : m_mapRegions) {
			if (region.second) {
				SetValue childNodes = region.second->getAllChildValues();
				setData.insert(childNodes.begin(), childNodes.end());
			}
		}
	}
	else {
		setData.insert(m_data.begin(), m_data.end());
	}

	return setData;
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::Node::deleteChildren() {
	for (auto& region : m_mapRegions) {
		if (region.second) {
			delete region.second;
			region.second = nullptr;
		}
	}
}

template<class Value, class NodeCompare>
bool SearchTree2D<Value, NodeCompare>::Node::shouldSubdivide(const std::set<Value>& vecVals, const std::map<RegionCode, NodeCompare&>& mapQuads) const 
{
	// Is there a value that doesn't satisfy all regions?
	// If not, then all children will have the same values, so there is no need to subdivide
	// This is an admittedly simple test, but it works for an initial implementation
	// We could instead test if the tree would be well-balanced or we could 
	// limit by the number of operations
	bool inAllRegions = true;
	for (auto val : vecVals) {
		for (auto quad : mapQuads) {
			if (!m_predicate->satisfies(quad.second, val)) {
				inAllRegions = false;
				break;
			}
		}
		if (!inAllRegions) {
			break;
		}
	}

	return !inAllRegions;
}

template<class Value, class NodeCompare>
void SearchTree2D<Value, NodeCompare>::Node::setCompare(const NodeCompare& compare) {
	m_compare = compare;
}

#endif