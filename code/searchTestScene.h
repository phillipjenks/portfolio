/*

	- Scene to test our search tree
	Written by: Phillip Jenks

	A scene populated by several sprites confined to the screen space with random
	positions and random velocities. We'll test our search tree with our mouse
*/

#ifndef __SEARCH_TEST_SCENE_H_
#define __SEARCH_TEST_SCENE_H_

#include "Scene.h"
#include "searchTree2D.h"
#include "SingleThread.h"

// Forward Declarations
namespace orc {
	class DrawableMouse;
	class Collider;
}

class SearchTestSprite;
class TestPredicate;

using TestTree = SearchTree2D<SearchTestSprite*, orc::Collider>;

class SearchTestScene : public orc::Scene {
 public:

	// Constructor to initialize our pointers
	SearchTestScene();
	
	// Load scene assets and build initial game objects
	virtual void load();

	// Unload scene assets
	virtual void unload();

	// Update game objects in the scene using our search tree
	virtual void update();

	// Handle any post update operations
	virtual void postUpdate();

 private:
	// A handle to our test predicate
	TestPredicate* m_testPredicate;

	// A handle to our mouse
	orc::DrawableMouse* m_mouse;

	// A vector to specifically hold onto our sprites
	std::vector<SearchTestSprite*> m_sprites;
	
	// Our search tree
	TestTree m_tree;

	// A thread we'll use to rebalance our search tree in the background
	orc::SingleThread m_treeThread;
};

#endif
