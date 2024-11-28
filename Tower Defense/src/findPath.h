#pragma once
#include "app.h"
#include "level.h"
#include "Vector2D.h"

#include <array>
#include <queue>
#include <algorithm>
//#include <unordered_map>
#include <map>



std::vector<Vector2D> findPath(const Vector2D &start, const Vector2D &goal)
{
	const Level &currentLevel = *App::s_CurrentLevel;
	std::vector<Vector2D> result;

	// Visited positions onto the origins from which they have been visited
	//std::unordered_map<Vector2D, Vector2D> visited;
	std::map<Vector2D, Vector2D> visited;

	struct Node
	{
		uint32_t totalDistance;
		Vector2D pos;

		// Defines a three-way comparison to let std::priority_queue comparing nodes
		auto operator<=>(const Node &other) const
		{
			return totalDistance <=> other.totalDistance;
		}

		std::array<Node, 4> GetNeighbours() const
		{
			// TODO: it's pretty pointless to use Dijkstra if the distance/cost of traversal
			// is always 1 everywhere, so maybe some tiles should be more expensive
			uint32_t dx = totalDistance + 2; // tileSize (=24) * scale (=2) * 2
			uint32_t dy = totalDistance + 1; // tileSize (=24) * scale (=2)
			return { {
				{ dx, Vector2D{ pos.x - 1, pos.y } },
				{ dy, Vector2D{ pos.x, pos.y - 1 } },
				{ dx, Vector2D{ pos.x + 1, pos.y } },
				{ dy, Vector2D{ pos.x, pos.y + 1 } },
			} };
		}
	};


	// we use std::greater because by default, queue.top() returns the greatest element,
	// and by using std::greater instead of std::less (the default) we flip this,
	// and make the queue give us the lowest element
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> queue;

	queue.push(Node{ 0, start });
	// putting (start, start) there is clever for debugging because the start nodes is the only node
	// which is "visited from itself"
	visited.emplace(start, start); // dummy value for the origin from which start has been visited

	while (!queue.empty())
	{
		Node next = queue.top();
		queue.pop();

		if (next.pos == goal)
		{
			Vector2D origin = goal;
			for (; origin != start; origin = visited.at(origin))
			{
				result.emplace_back(origin);
			}
			result.emplace_back(start);
			// because we push_back'd starting from the end node and continuing to the start node,
			// the result would be in reverse order, and this feels really weird,
			// so we reverse it before returning
			std::ranges::reverse(result);
			return result;
		}

		// A - B - C
		for (const Node &neighbor : next.GetNeighbours())
		{
			if (!currentLevel.IsTileWalkable(neighbor.pos))
				continue;

			// std::unordered_map::emplace will not insert a new element into the set if one already exists
			// in that case, success is false, but we always get an iterator to the
			// new/already existing element in the set
			auto [iterator, success] = visited.emplace(neighbor.pos, next.pos);

			if (success)
				queue.push(neighbor);
		}
	}

	// :/ no path found
	return result;
}