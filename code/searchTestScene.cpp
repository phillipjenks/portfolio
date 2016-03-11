/*
	
	- Implementation of our search test scene
	Written by: Phillip Jenks 2016

*/
#include "stdafx.h"

#include "Oracle.h"

#include "DrawableMouse.h"
#include "Collider.h"

#include "searchTestScene.h"
#include "searchTestSprite.h"
#include "searchTestPredicate.h"

// Utility function used for rebalancing the search tree
// in a separate thread
namespace test {
	int rebalanceTree(void* tr) {
		TestTree* tree = (TestTree*) tr;
		
		if(tree) {
			tree->rebalance();
		}
	
		return 0;
	}
}

SearchTestScene::SearchTestScene()
	: m_testPredicate(nullptr)
	, m_tree()
{
}

// Initialize our tree and load assets
void SearchTestScene::load() {
	
	using namespace orc;

	// Create our predicate and tell the tree to use it
	m_testPredicate = new TestPredicate();
	m_tree.setPredicate(m_testPredicate);

	// Create 200 random sprites
	for(int i = 0; i < 200; ++i) {
		// get a random sprite size
		int w = 20 + randInt(-5, 6);
		int h = 20 + randInt(-5, 6);
		
		// get a random location
		int x = randInt(0, Oracle().screenWidth() - w);
		int y = randInt(0, Oracle().screenHeight() - h);
		
		// get a random velocity
		int vx = randInt(-150, 150);
		int vy = randInt(-150, 150);

		SearchTestSprite* spr = new SearchTestSprite();
		spr->setScale((GLfloat)w, (GLfloat)h, 1.0f);
		spr->setPos((GLfloat)x, (GLfloat)y, 0.0f);
		spr->setVel((GLfloat)vx, (GLfloat)vy, 0.0f);
		spr->setTexture("data/misc/mouse.png");

		// gameObjs will manage memory for us, deleting the sprites on a scene change or quit
		gameObjs.push_back(spr);

		// hold our own copy of the sprite
		m_sprites.push_back(spr);

		// add it to our tree
		m_tree.add(spr);
	}

	// All sprites will be at the root node, so let's rebalance and force the tree to create
	// some children
	m_tree.rebalance();

	// Set up our thread to rebalance our tree behind the render phase
	m_treeThread.setFunc(test::rebalanceTree);
	m_treeThread.setInput((void*)(&(this->m_tree)));

	// Create our mouse and make it green
	m_mouse = new DrawableMouse;
	m_mouse->setImg("data/misc/mouse.png");
	m_mouse->setScale(20, 20, 1);
	m_mouse->setDrawColor(0.0f, 1.0f, 0.0f);

	gameObjs.push_back(m_mouse);
}

void SearchTestScene::unload() {
	// Wait for any rebalancing to finish
	m_treeThread.wait();

	// Clean up our thread
	m_treeThread.setFunc(nullptr);
	m_treeThread.setInput(nullptr);

	// Clear our predicate
	if (m_testPredicate) {
		delete m_testPredicate;
		m_testPredicate = nullptr;
	}

	// Clear out our tree
	m_tree.clear();
	m_tree.setPredicate(nullptr);

	// Remove now invalid sprite pointers
	m_sprites.clear();
}

void SearchTestScene::update() {
	
	using namespace orc;
	
	// Wait for any rebalancing to finish
	m_treeThread.wait();

	// Close the game if the player hits 'q'
	if(Oracle().keyPressed(SDLK_q)) {
		Oracle().quitAll(ORACLE_FINISH_NORMAL);
	}

	// Set all of our sprites to white
	for (auto sprite : m_sprites) {
		sprite->setColor(1, 1, 1);
	}

	// Build a test collider based on our mouse
	Collider coll = TestPredicate().nilCompare();
	coll.moveTo((GLfloat)(Oracle().getMouse().x() - 20), (GLfloat)(Oracle().getMouse().y() - 20), 0.0f);
	coll.setScale(40.0f, 40.0f, 1.0f);
	
	// Grab all sprites near the mouse based on a search in the tree
	std::set<SearchTestSprite*> setnear = m_tree.getNearbyValues(coll);
	
	// Make nearby sprites red
	for (auto sprite : setnear) {
		sprite->setColor(1, 0, 0);
	}
}

void SearchTestScene::postUpdate() {
	// We've finished our update, which might have invalidated our tree
	// let's rebalance while the scene renders
	m_treeThread.start();
}
