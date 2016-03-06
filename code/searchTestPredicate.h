/*

	- Interface for a predicate used for testing the 2D search tree
	Written by: Phillip Jenks 2016

*/

#ifndef __SEARCH_TEST_PREDICATE_H_
#define __SEARCH_TEST_PREDICATE_H_

#include "searchTree2D.h"

// Forward declarations

// A general collider class that is defined at it's most basic level
// with an axis aligned bounding box
namespace orc {
	class Collider;
}

// 2D sprite with a method getCollider to return a collider
// defining this sprite's Rect
class SearchTestSprite;


//============================================
// Interface For Our Search Test Predicate
//============================================
class TestPredicate : public SearchPredicate<SearchTestSprite*, orc::Collider> {
public:

	// Default collider value defines a Rect at pos (0, 0) with a scale of (1, 1)
	virtual orc::Collider nilCompare() override;

	// Builds our root search space as a collider
	virtual orc::Collider buildRegionFromData(const std::set<SearchTestSprite*>& data) override;

	// Subdivides our root Collider into four quadrants
	virtual void buildQuadrantsFromData(const orc::Collider& parentRegion,
						const std::set<SearchTestSprite*>& vecData,
						const std::map<RegionCode, orc::Collider&>& quads) override;

	// Returns true if the sprite's Collider overlaps with the node's collider
	virtual bool satisfies(const orc::Collider& nodeCompare, SearchTestSprite* valCompare) override;

	// Returns true if the two colliders overlap
	virtual bool overlaps(const orc::Collider& compareLeft, const orc::Collider& compareRight) override;
};

#endif
