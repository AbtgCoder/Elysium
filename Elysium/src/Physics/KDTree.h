#pragma once

#include <Scene/Entity.h>

struct KDTreeNode
{
	std::vector<Entity> entities;
	KDTreeNode* left = nullptr;
	KDTreeNode* right = nullptr;
	float medianVal = 0.0f;

	KDTreeNode() {}
	~KDTreeNode()
	{
		entities.clear();
		delete left;
		delete right;
	}
};

KDTreeNode* makeKDTree(KDTreeNode* rootNode, uint16_t depth);
