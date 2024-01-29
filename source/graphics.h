//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_GRAPHICS_H_
#define RME_GRAPHICS_H_

#include "outfit.h"
#include "common.h"
#include "enums.h"

#include "client_assets.h"
#include "sprite_appearances.h"

#include <appearances.pb.h>

enum SpriteSize {
	SPRITE_SIZE_16x16,
	//SPRITE_SIZE_24x24,
	SPRITE_SIZE_32x32,
	SPRITE_SIZE_64x64,
	SPRITE_SIZE_COUNT
};

enum AnimationDirection {
	ANIMATION_FORWARD = 0,
	ANIMATION_BACKWARD = 1
};

enum ItemAnimationDuration {
	ITEM_FRAME_DURATION = 500
};

class MapCanvas;
class GraphicManager;
class FileReadHandle;
class Animator;
class GameSprite;

class Sprite {
public:
	Sprite() {}
	virtual ~Sprite() {}

	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1) = 0;
	virtual void unloadDC() = 0;
private:
	Sprite(const Sprite&);
	Sprite& operator=(const Sprite&);
};

class EditorSprite : public Sprite {
public:
	EditorSprite(wxBitmap* b16x16, wxBitmap* b32x32);
	virtual ~EditorSprite();

	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1);
	virtual void unloadDC();
protected:
	wxBitmap* bm[SPRITE_SIZE_COUNT];
};

class GameSprite : public Sprite {
public:
	GameSprite();
	~GameSprite();

	int getIndex(int width, int height, int layer, int pattern_x, int pattern_y, int pattern_z, int frame) const;
	GLuint getSpriteId(int _layer, int _subtype, int _pattern_x, int _pattern_y, int _pattern_z, int _frame);
	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1);

	virtual void unloadDC();

	int getDrawHeight() const;
	std::pair<int, int> getDrawOffset();
	uint8_t getMiniMapColor() const;
	static uint8_t* invertGLColors(int spriteHeight, int spriteWidth, uint8_t* rgba);

protected:
	class Image;
	class NormalImage;
	class OutfitImage;

	wxMemoryDC* getDC(SpriteSize spriteSize);

	class Image {
	public:
		Image();
		virtual ~Image();

		bool isGLLoaded;
		int lastaccess;

		void visit();
		virtual void clean(int time);

		virtual GLuint getSpriteId() = 0;
		virtual uint8_t* getRGBAData() = 0;
	protected:
		virtual void createGLTexture(GLuint whatid);
		virtual void unloadGLTexture(GLuint whatid);
	};

	class NormalImage : public Image {
	public:
		NormalImage();
		virtual ~NormalImage();

		// We use the sprite id as GL texture id
		uint32_t id;

		// This contains the pixel data
		uint16_t size;
		uint8_t* m_cachedData = nullptr;

		virtual void clean(int time);

		virtual GLuint getSpriteId();
		virtual uint8_t* getRGBAData();
	protected:
		virtual void createGLTexture(GLuint ignored = 0);
		virtual void unloadGLTexture(GLuint ignored = 0);
	};

	class OutfitImage : public Image {
	public:
		OutfitImage(GameSprite* initParent, int initSpriteIndex, GLuint initSpriteId, const Outfit& initOutfit);
		~OutfitImage();

		virtual GLuint getSpriteId();
		virtual uint8_t* getRGBAData();

		GLuint m_spriteId = 0;
		GameSprite* m_parent = 0;
		int m_spriteIndex = 0;
		std::string m_name;
		uint8_t m_lookHead = 0;
		uint8_t m_lookBody = 0;
		uint8_t m_lookLegs = 0;
		uint8_t m_lookFeet = 0;
		bool m_isGLLoaded = false;
		uint8_t* m_cachedOutfitData = nullptr;

		void colorizePixel(uint8_t color, uint8_t &r, uint8_t &b, uint8_t &g);
		uint8_t* getOutfitData(int spriteId);

		virtual void createGLTexture(GLuint spriteId = 0);
		virtual void unloadGLTexture(GLuint ignored = 0);
	};

	uint32_t id;
	wxMemoryDC* m_wxMemoryDc[SPRITE_SIZE_COUNT];

public:
	std::shared_ptr<GameSprite::OutfitImage> getOutfitImage(int spriteId, Direction direction, const Outfit& outfit);

	uint32_t getID() const {
		return id;
	}

	bool isDrawOffsetLoaded = false;

	uint8_t getWidth();
	uint8_t getHeight();

	// GameSprite info
	uint8_t height;
	uint8_t width;
	uint8_t layers;
	uint8_t pattern_x;
	uint8_t pattern_y;
	uint8_t pattern_z;
	uint8_t sprite_phase_size;
	uint32_t numsprites;

	Animator* animator;

	uint16_t draw_height;
	uint16_t drawoffset_x;
	uint16_t drawoffset_y;

	uint16_t minimap_color;

	std::vector<NormalImage*> spriteList;
	std::list<std::shared_ptr<GameSprite::OutfitImage>> instanced_templates; // Templates that use this sprite

	friend class GraphicManager;
};

extern GameSprite g_gameSprite;

struct FrameDuration
{
	int min;
	int max;

	FrameDuration(int min, int max) : min(min), max(max)
	{
		ASSERT(min <= max);
	}

	int getDuration() const
	{
		if(min == max)
			return min;
		return uniform_random(min, max);
	};

	void setValues(int min, int max)
	{
		ASSERT(min <= max);
		this->min = min;
		this->max = max;
	}
};

class Animator
{
public:
	Animator(int frames, int start_frame, int loop_count, bool async);
	~Animator();

	int getStartFrame() const;

	FrameDuration* getFrameDuration(int frame);

	int getFrame();
	void setFrame(int frame);

	void reset();

private:
	int getDuration(int frame) const;
	int getPingPongFrame();
	int getLoopFrame();
	void calculateSynchronous();

	int frame_count;
	int start_frame;
	int loop_count;
	bool async;
	std::vector<FrameDuration*> durations;
	int current_frame;
	int current_loop;
	int current_duration;
	int total_duration;
	AnimationDirection direction;
	long last_time;
	bool is_complete;
};

class GraphicManager
{
public:
	GraphicManager();
	~GraphicManager();

	void clear();
	void cleanSoftwareSprites();

	Sprite* getSprite(int id);
	GameSprite* getCreatureSprite(int id);

	long getElapsedTime() const { return (animation_timer->TimeInMicro() / 1000).ToLong(); }

	uint16_t getItemSpriteMaxID() const;
	uint16_t getCreatureSpriteMaxID() const;

	// Get an unused texture id (this is acquired by simply increasing a value starting from 0x10000000)
	GLuint getFreeTextureID();

	// This is part of the binary
	bool loadEditorSprites();
	// Metadata should be loaded first
	// This fills the item / creature adress space
	bool loadItemSpriteMetadata(ItemType* t, wxString& error, wxArrayString& warnings);
	bool loadOutfitSpriteMetadata(canary::protobuf::appearances::Appearance outfit, wxString& error, wxArrayString& warnings);

	// Cleans old & unused textures according to config settings
	void garbageCollection();
	void addSpriteToCleanup(GameSprite* spr);

private:
	// This is used if memcaching is NOT on
	bool loadSpriteDump(uint8_t*& target, uint16_t& size, int sprite_id);

	typedef std::map<int, Sprite*> SpriteMap;
	SpriteMap sprite_space;
	typedef std::map<int, GameSprite::Image*> ImageMap;
	ImageMap image_space;
	std::deque<GameSprite*> cleanup_list;

	uint16_t item_count;
	uint16_t creature_count;
	bool otfi_found;
	bool is_extended;
	bool has_frame_durations;
	bool has_frame_groups;

	int loaded_textures;
	int lastclean;

	wxStopWatch* animation_timer;

	friend class GameSprite::Image;
	friend class GameSprite::NormalImage;
	friend class GameSprite::OutfitImage;
};

struct RGBQuad {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t reserved;

	RGBQuad(uint8_t r, uint8_t g, uint8_t b) : red(r), green (g), blue(b), reserved(0) {}

	operator uint32_t() {
		return (blue << 0) | (green << 8) | (red << 16);
	}

	operator bool() {
		//std::cout << "RGBQuad operator bool " << int(red) << " || " << int(blue) << " || " << int(green) << std::endl;
		return blue != 0 || red != 0 || green != 0;
	}
};

static RGBQuad minimap_color[256] = {
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 51),      RGBQuad(0, 0, 102),     RGBQuad(0, 0, 153),     // 0
	RGBQuad(0, 0, 204),     RGBQuad(0, 0, 255),     RGBQuad(0, 51, 0),      RGBQuad(0, 51, 51),     // 4
	RGBQuad(0, 51, 102),    RGBQuad(0, 51, 153),    RGBQuad(0, 51, 204),    RGBQuad(0, 51, 255),    // 8
	RGBQuad(0, 102, 0),     RGBQuad(0, 102, 51),    RGBQuad(0, 102, 102),   RGBQuad(0, 102, 153),   // 12
	RGBQuad(0, 102, 204),   RGBQuad(0, 102, 255),   RGBQuad(0, 153, 0),     RGBQuad(0, 153, 51),    // 16
	RGBQuad(0, 153, 102),   RGBQuad(0, 153, 153),   RGBQuad(0, 153, 204),   RGBQuad(0, 153, 255),   // 20
	RGBQuad(0, 204, 0),     RGBQuad(0, 204, 51),    RGBQuad(0, 204, 102),   RGBQuad(0, 204, 153),   // 24
	RGBQuad(0, 204, 204),   RGBQuad(0, 204, 255),   RGBQuad(0, 255, 0),     RGBQuad(0, 255, 51),    // 28
	RGBQuad(0, 255, 102),   RGBQuad(0, 255, 153),   RGBQuad(0, 255, 204),   RGBQuad(0, 255, 255),   // 32
	RGBQuad(51, 0, 0),      RGBQuad(51, 0, 51),     RGBQuad(51, 0, 102),    RGBQuad(51, 0, 153),    // 36
	RGBQuad(51, 0, 204),    RGBQuad(51, 0, 255),    RGBQuad(51, 51, 0),     RGBQuad(51, 51, 51),    // 40
	RGBQuad(51, 51, 102),   RGBQuad(51, 51, 153),   RGBQuad(51, 51, 204),   RGBQuad(51, 51, 255),   // 44
	RGBQuad(51, 102, 0),    RGBQuad(51, 102, 51),   RGBQuad(51, 102, 102),  RGBQuad(51, 102, 153),  // 48
	RGBQuad(51, 102, 204),  RGBQuad(51, 102, 255),  RGBQuad(51, 153, 0),    RGBQuad(51, 153, 51),   // 52
	RGBQuad(51, 153, 102),  RGBQuad(51, 153, 153),  RGBQuad(51, 153, 204),  RGBQuad(51, 153, 255),  // 56
	RGBQuad(51, 204, 0),    RGBQuad(51, 204, 51),   RGBQuad(51, 204, 102),  RGBQuad(51, 204, 153),  // 60
	RGBQuad(51, 204, 204),  RGBQuad(51, 204, 255),  RGBQuad(51, 255, 0),    RGBQuad(51, 255, 51),   // 64
	RGBQuad(51, 255, 102),  RGBQuad(51, 255, 153),  RGBQuad(51, 255, 204),  RGBQuad(51, 255, 255),  // 68
	RGBQuad(102, 0, 0),     RGBQuad(102, 0, 51),    RGBQuad(102, 0, 102),   RGBQuad(102, 0, 153),   // 72
	RGBQuad(102, 0, 204),   RGBQuad(102, 0, 255),   RGBQuad(102, 51, 0),    RGBQuad(102, 51, 51),   // 76
	RGBQuad(102, 51, 102),  RGBQuad(102, 51, 153),  RGBQuad(102, 51, 204),  RGBQuad(102, 51, 255),  // 80
	RGBQuad(102, 102, 0),   RGBQuad(102, 102, 51),  RGBQuad(102, 102, 102), RGBQuad(102, 102, 153), // 84
	RGBQuad(102, 102, 204), RGBQuad(102, 102, 255), RGBQuad(102, 153, 0),   RGBQuad(102, 153, 51),  // 88
	RGBQuad(102, 153, 102), RGBQuad(102, 153, 153), RGBQuad(102, 153, 204), RGBQuad(102, 153, 255), // 92
	RGBQuad(102, 204, 0),   RGBQuad(102, 204, 51),  RGBQuad(102, 204, 102), RGBQuad(102, 204, 153), // 96
	RGBQuad(102, 204, 204), RGBQuad(102, 204, 255), RGBQuad(102, 255, 0),   RGBQuad(102, 255, 51),  // 100
	RGBQuad(102, 255, 102), RGBQuad(102, 255, 153), RGBQuad(102, 255, 204), RGBQuad(102, 255, 255), // 104
	RGBQuad(153, 0, 0),     RGBQuad(153, 0, 51),    RGBQuad(153, 0, 102),   RGBQuad(153, 0, 153),   // 108
	RGBQuad(153, 0, 204),   RGBQuad(153, 0, 255),   RGBQuad(153, 51, 0),    RGBQuad(153, 51, 51),   // 112
	RGBQuad(153, 51, 102),  RGBQuad(153, 51, 153),  RGBQuad(153, 51, 204),  RGBQuad(153, 51, 255),  // 116
	RGBQuad(153, 102, 0),   RGBQuad(153, 102, 51),  RGBQuad(153, 102, 102), RGBQuad(153, 102, 153), // 120
	RGBQuad(153, 102, 204), RGBQuad(153, 102, 255), RGBQuad(153, 153, 0),   RGBQuad(153, 153, 51),  // 124
	RGBQuad(153, 153, 102), RGBQuad(153, 153, 153), RGBQuad(153, 153, 204), RGBQuad(153, 153, 255), // 128
	RGBQuad(153, 204, 0),   RGBQuad(153, 204, 51),  RGBQuad(153, 204, 102), RGBQuad(153, 204, 153), // 132
	RGBQuad(153, 204, 204), RGBQuad(153, 204, 255), RGBQuad(153, 255, 0),   RGBQuad(153, 255, 51),  // 136
	RGBQuad(153, 255, 102), RGBQuad(153, 255, 153), RGBQuad(153, 255, 204), RGBQuad(153, 255, 255), // 140
	RGBQuad(204, 0, 0),     RGBQuad(204, 0, 51),    RGBQuad(204, 0, 102),   RGBQuad(204, 0, 153),   // 144
	RGBQuad(204, 0, 204),   RGBQuad(204, 0, 255),   RGBQuad(204, 51, 0),    RGBQuad(204, 51, 51),   // 148
	RGBQuad(204, 51, 102),  RGBQuad(204, 51, 153),  RGBQuad(204, 51, 204),  RGBQuad(204, 51, 255),  // 152
	RGBQuad(204, 102, 0),   RGBQuad(204, 102, 51),  RGBQuad(204, 102, 102), RGBQuad(204, 102, 153), // 156
	RGBQuad(204, 102, 204), RGBQuad(204, 102, 255), RGBQuad(204, 153, 0),   RGBQuad(204, 153, 51),  // 160
	RGBQuad(204, 153, 102), RGBQuad(204, 153, 153), RGBQuad(204, 153, 204), RGBQuad(204, 153, 255), // 164
	RGBQuad(204, 204, 0),   RGBQuad(204, 204, 51),  RGBQuad(204, 204, 102), RGBQuad(204, 204, 153), // 168
	RGBQuad(204, 204, 204), RGBQuad(204, 204, 255), RGBQuad(204, 255, 0),   RGBQuad(204, 255, 51),  // 172
	RGBQuad(204, 255, 102), RGBQuad(204, 255, 153), RGBQuad(204, 255, 204), RGBQuad(204, 255, 255), // 176
	RGBQuad(255, 0, 0),     RGBQuad(255, 0, 51),    RGBQuad(255, 0, 102),   RGBQuad(255, 0, 153),   // 180
	RGBQuad(255, 0, 204),   RGBQuad(255, 0, 255),   RGBQuad(255, 51, 0),    RGBQuad(255, 51, 51),   // 184
	RGBQuad(255, 51, 102),  RGBQuad(255, 51, 153),  RGBQuad(255, 51, 204),  RGBQuad(255, 51, 255),  // 188
	RGBQuad(255, 102, 0),   RGBQuad(255, 102, 51),  RGBQuad(255, 102, 102), RGBQuad(255, 102, 153), // 192
	RGBQuad(255, 102, 204), RGBQuad(255, 102, 255), RGBQuad(255, 153, 0),   RGBQuad(255, 153, 51),  // 196
	RGBQuad(255, 153, 102), RGBQuad(255, 153, 153), RGBQuad(255, 153, 204), RGBQuad(255, 153, 255), // 200
	RGBQuad(255, 204, 0),   RGBQuad(255, 204, 51),  RGBQuad(255, 204, 102), RGBQuad(255, 204, 153), // 204
	RGBQuad(255, 204, 204), RGBQuad(255, 204, 255), RGBQuad(255, 255, 0),   RGBQuad(255, 255, 51),  // 208
	RGBQuad(255, 255, 102), RGBQuad(255, 255, 153), RGBQuad(255, 255, 204), RGBQuad(255, 255, 255), // 212
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 216
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 220
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 224
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 228
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 232
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 236
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 240
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 244
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       // 248
	RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0),       RGBQuad(0, 0, 0)        // 252
};

extern GraphicManager g_graphics;

#endif
