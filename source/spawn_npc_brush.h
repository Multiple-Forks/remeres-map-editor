//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_SPAWN_NPC_BRUSH_H
#define RME_SPAWN_NPC_BRUSH_H

#include "brush.h"

//=============================================================================
// SpawnNpcBrush, place spawns npc

class SpawnNpcBrush : public Brush {
public:
	SpawnNpcBrush(); // Create a RAWBrush of the specified type
	virtual ~SpawnNpcBrush();

	bool isSpawnNpc() const {
		return true;
	}
	SpawnNpcBrush* asSpawnNpc() {
		return static_cast<SpawnNpcBrush*>(this);
	}

	virtual bool canDraw(BaseMap* map, const Position &position) const;
	virtual void draw(BaseMap* map, Tile* tile, void* parameter); // parameter is brush size
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual int getLookID() const; // We don't have a look, sorry!
	virtual std::string getName() const;
	virtual bool canDrag() const {
		return true;
	}
	virtual bool canSmear() const {
		return false;
	}
	virtual bool oneSizeFitsAll() const {
		return true;
	}
};

#endif
