
#include "KDTree.h"

KDTreeNode* makeKDTree(KDTreeNode* rootNode, uint16_t depth)
{
	// if numOfEntities in an area <= 2 dont divide further
	if (rootNode->entities.size() <= 2)
	{
		return rootNode;
	}

	uint16_t currentDim = depth % 2; // 2->num of dimensions

	size_t numEntities = rootNode->entities.size();
	float sumCoord = 0.0f;
	for (auto e : rootNode->entities)
	{
		sumCoord += e.getComponent<CTransform>().GlobalTranslation[currentDim];
	}
	rootNode->medianVal = sumCoord / numEntities;

	rootNode->left = new KDTreeNode();
	rootNode->right = new KDTreeNode();

	for (auto e : rootNode->entities)
	{
		if (e.getComponent<CTransform>().GlobalTranslation[currentDim] < rootNode->medianVal)
		{
			rootNode->left->entities.push_back(e);
		}
		else
		{
			rootNode->right->entities.push_back(e);
		}
	}

	if (rootNode->left->entities.empty() || rootNode->right->entities.empty()) {
		return rootNode;
	}

	// clear rootNode's entities vector to save memory ??
	rootNode->entities.clear();

	makeKDTree(rootNode->left, depth + 1);
	makeKDTree(rootNode->right, depth + 1);

	return rootNode;
}