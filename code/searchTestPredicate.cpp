/*

	- Implementation for the search test predicate
	Written by: Phillip Jenks 2016

*/


#include "stdafx.h"

#include "searchTestPredicate.h"
#include "searchTestSprite.h"
#include "Collider.h"

// Returns a collider based on a base rect for a 2d plane
// pos (0, 0) size (1, 1)
orc::Collider TestPredicate::nilCompare() {
	return orc::OracleModel().getCollider("Oracle/models/2dPlane.obj");
}

orc::Collider TestPredicate::buildRegionFromData(const std::set<SearchTestSprite*>& data) {

	using std::min;
	using std::max;

	orc::Collider ret = nilCompare();

	GLfloat xmin = 0, ymin = 0, xmax = 0, ymax = 0;

	// Set the size of our search region based on the furthest extents of the tree's values
	if (data.size() > 0) {

		for (auto sprite : data) {
			const orc::Collider& coll = sprite->getCollider();
			xmin = min(xmin, coll.getPos().x);
			xmax = max(xmax, coll.getPos().x + coll.getScale().x);
			ymin = min(ymin, coll.getPos().y);
			ymax = max(ymax, coll.getPos().y + coll.getScale().y);
		}

		ret.moveTo(xmin, ymin, 0);
		ret.setScale(xmax - xmin, ymax - ymin, 1);
	}

	return ret;
}

// Subdivide the search space into four quadrants
void TestPredicate::buildQuadrantsFromData(const orc::Collider& node,
						const std::set<SearchTestSprite*>& data,
						const std::map<RegionCode, orc::Collider&>& quads) {

	GLfloat w = node.getScale().x / 2;
	GLfloat h = node.getScale().y / 2;
	GLfloat x = node.getPos().x;
	GLfloat y = node.getPos().y;

	// For testing, do a simple subdivide into four equal quadrants
	for (auto quad : quads) {
		quad.second.setScale(w, h, 1);
		switch (quad.first) {
		case RegionCode::UPPER_LEFT:
			quad.second.moveTo(x, y, 0);
			break;
		case RegionCode::UPPER_RIGHT:
			quad.second.moveTo(x + w, y, 0);
			break;
		case RegionCode::LOWER_LEFT:
			quad.second.moveTo(x, y + h, 0);
			break;
		case RegionCode::LOWER_RIGHT:
			quad.second.moveTo(x + w, y + h, 0);
			break;
		default:
			ASSERT(false);
			break;
		}
	}
}

// Test sprite collider against node search space
bool TestPredicate::satisfies(const orc::Collider& node, SearchTestSprite* test) {
	return node.collideAABB(test->getCollider());
}

// Test if the two bounding boxes overlap
bool TestPredicate::overlaps(const orc::Collider& compL, const orc::Collider& compR) {
	return compL.collideAABB(compR);
}