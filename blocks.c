/* Copyright (c) 2012, 2013, Chris Smeele
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *   * Neither the name of cjsx9g nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "blocks.h"
#include <stdbool.h>
#include <strings.h>

#define bPLANK0 {0xb0, 0x9a, 0x41, 0xff}
#define bPLANK1 {0x8b, 0x6e, 0x29, 0xff}
#define bPLANK2 {0xdb, 0xcb, 0x82, 0xff}
#define bPLANK3 {0xcd, 0xa5, 0x6b, 0xff}

#define bLEAF0 {0x2b, 0x5e, 0x24, 0xda}
#define bLEAF1 {0x2b, 0x5e, 0x24, 0xda}
#define bLEAF2 {0x2b, 0x5e, 0x24, 0xda}
#define bLEAF3 {0x2b, 0x5e, 0x24, 0xda}

static void getcolor_planks(uint8_t type, uint8_t data, int *color[3]){
	if(data == 1){
		int a[4] = bPLANK1;
		memcpy(color, a, sizeof(int)*3);
	}else if(data == 2){
		int a[4] = bPLANK2;
		memcpy(color, a, sizeof(int)*3);
	}else if(data == 3){
		int a[4] = bPLANK3;
		memcpy(color, a, sizeof(int)*3);
	}else{
		int a[4] = bPLANK0;
		memcpy(color, a, sizeof(int)*3);
	}
}
static void getcolor_leaves(uint8_t type, uint8_t data, int *color[4]){
}

const blockdesc_t blockdescs[256] = {
	{{0x10, 0x10, 0x10, 0x00}, 0                 }, //000 air
	{{0x7f, 0x7f, 0x7f, 0xff}, 0                 }, //001 stone
	{{0x47, 0xa5, 0x39, 0xff}, 0                 }, //002 grass
	{{0x90, 0x82, 0x49, 0xff}, 0                 }, //003 dirt
	{{0x61, 0x62, 0x61, 0xff}, 0                 }, //004 cobble
	{ bPLANK0                , getcolor_planks   }, //005 planks
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //006 sapling
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //007 bedrock
	{{0x37, 0x57, 0xc8, 0xf8}, 0                 }, //008 water
	{{0x37, 0x70, 0xc7, 0xf8}, 0                 }, //009 water
	{{0xe5, 0x55, 0x11, 0xff}, 0                 }, //010 lava
	{{0xe5, 0x55, 0x11, 0xff}, 0                 }, //011 lava
	{{0xdf, 0xd7, 0x87, 0xff}, 0                 }, //012 sand
	{{0x94, 0x8f, 0xa1, 0xff}, 0                 }, //013 gravel
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //014 gold ore
	{{0x9a, 0x9a, 0x9a, 0xff}, 0                 }, //015 iron ore
	{{0x61, 0x53, 0x47, 0xff}, 0                 }, //016 coal ore
	{{0x71, 0x40, 0x00, 0xff}, 0                 }, //017 wood
	{{0x2b, 0x5e, 0x24, 0xde}, getcolor_leaves   }, //018 leaves
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //019 sponge
	{{0x00, 0x00, 0x00, 0x50}, 0                 }, //020 glass
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //021 lapis ore
	{{0x02, 0x02, 0xf0, 0xff}, 0                 }, //022 lapis
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //023 dispenser
	{{0xe0, 0xde, 0xc5, 0xff}, 0                 }, //024 sandstone
	{{0x8e, 0x6d, 0x43, 0xff}, 0                 }, //025 noteblock
	{{0x58, 0x45, 0x41, 0xff}, 0                 }, //026 bed
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //027 powered rail
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //028 detector rail
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //029 sticky piston
	{{0x00, 0x00, 0x00, 0xf0}, 0                 }, //030 cobweb
	{{0x47, 0xa5, 0x39, 0x00}, 0                 }, //031 tall grass
	{{0xd2, 0xca, 0x7b, 0x00}, 0                 }, //032 dead bush
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //033 piston
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //034 piston extension
	{{0xd5, 0xd5, 0xd5, 0xff}, 0                 }, //035 wool
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //036 block moved by piston
	{{0xff, 0xff, 0x10, 0x00}, 0                 }, //037 yellow flower
	{{0xe5, 0x23, 0x2a, 0x00}, 0                 }, //038 red flower
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //039 brown mushroom
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //040 red mushroom
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //041 gold block
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //042 iron block
	{{0x7f, 0x7f, 0x7f, 0xff}, 0                 }, //043 double slabs
	{{0x7f, 0x7f, 0x7f, 0xff}, 0                 }, //044 slabs
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //045 bricks
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //046 tnt
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //047 bookshelf
	{{0x58, 0x63, 0x59, 0xff}, 0                 }, //048 mossy cobblestone
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //049 obsidian
	{{0xff, 0xea, 0x34, 0xf0}, 0                 }, //050 torch
	{{0xde, 0x41, 0x00, 0xf0}, 0                 }, //051 fire
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //052 monster spawner
	{ bPLANK0,                 0                 }, //053 wooden stairs
	{{0x8e, 0x6d, 0x43, 0xff}, 0                 }, //054 chest
	{{0x00, 0x00, 0x00, 0x60}, 0                 }, //055 redstone
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //056 diamond ore
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //057 diamond block
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //058 crafting table
	{{0xe2, 0xdc, 0x27, 0xd0}, 0                 }, //059 wheat
	{{0x45, 0x30, 0x11, 0xff}, 0                 }, //060 farmland
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //061 furnace
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //062 burning furnace
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //063 sign post
	{bPLANK0,                  0                 }, //064 wooden door
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //065 ladder
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //066 rail
	{{0x61, 0x62, 0x61, 0xff}, 0                 }, //067 cobblestone stairs
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //068 wall sign
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //069 lever
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //070 stone pressure plate
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //071 iron door
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //072 wooden pressure plate
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //073 redstone ore
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //074 glowing redstone ore
	{{0x68, 0x00, 0x00, 0xe0}, 0                 }, //075 redstone torch off
	{{0x98, 0x00, 0x00, 0xe0}, 0                 }, //076 redstone torch on
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //077 stone button
	{{0xe0, 0xe7, 0xef, 0xf0}, 0                 }, //078 snow
	{{0xab, 0xd1, 0xff, 0xd0}, 0                 }, //079 ice
	{{0xe0, 0xe7, 0xef, 0xff}, 0                 }, //080 snow block
	{{0x2e, 0x72, 0x25, 0xff}, 0                 }, //081 cactus
	{{0x8d, 0xb4, 0xd8, 0xff}, 0                 }, //082 clay
	{{0x75, 0xf8, 0x38, 0xf0}, 0                 }, //083 reeds
	{{0x8e, 0x6d, 0x43, 0xff}, 0                 }, //084 jukebox
	{{0xb0, 0x9a, 0x41, 0xff}, 0                 }, //085 fence
	{{0xd9, 0x9b, 0x26, 0xff}, 0                 }, //086 pumpkin
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //087 netherrack
	{{0x51, 0x47, 0x36, 0xff}, 0                 }, //088 soul sand
	{{0xdd, 0xc3, 0x66, 0xff}, 0                 }, //089 glowstone
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //090 nether portal
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //091 pumpkin lantern
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //092 cake
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //093 diode off
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //094 diode on
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //095 locked chest
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //096 trapdoor
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //097 monster egg
	{{0x96, 0x96, 0x96, 0xff}, 0                 }, //098 stonebrick
	{{0x77, 0x63, 0x42, 0xff}, 0                 }, //099 huge brown mushroom
	{{0xb3, 0x4f, 0x52, 0xff}, 0                 }, //100 huge red mushroom
	{{0x00, 0x00, 0x00, 0x80}, 0                 }, //101 iron bars
	{{0x00, 0x00, 0x00, 0x80}, 0                 }, //102 glass pane
	{{0x35, 0xbd, 0x3e, 0xff}, 0                 }, //103 melon
	{{0xf9, 0xbb, 0x46, 0xff}, 0                 }, //104 pumpkin stem
	{{0x55, 0xdb, 0x5e, 0xff}, 0                 }, //105 melon stem
	{{0x4a, 0x7e, 0x43, 0x00}, 0                 }, //106 vines
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //107 fence gate
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //108 brick stairs
	{{0x9a, 0x9a, 0x9a, 0xff}, 0                 }, //109 stonebrick stairs
	{{0x85, 0x79, 0x8c, 0xff}, 0                 }, //110 mycelium
	{{0x2d, 0x7f, 0x23, 0xff}, 0                 }, //111 lily pad
	{{0x4c, 0x1b, 0x18, 0xff}, 0                 }, //112 nether brick
	{{0x4c, 0x1b, 0x18, 0xff}, 0                 }, //113 nether fence
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //114 nether stairs
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //115 nether wart
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //116 enchantment table
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //117 brewing stand
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //118 cauldron
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //119 end portal
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //120 end portal frame
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //121 end stone
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //122 dragon egg
	{{0xdd, 0xd2, 0x66, 0xff}, 0                 }, //123 redstone lamp off
	{{0xf9, 0xea, 0x57, 0xff}, 0                 }, //124 redstone lamp on
	{bPLANK0                 , getcolor_planks   }, //125 wooden double slab
	{bPLANK0                 , getcolor_planks   }, //126 wooden slab
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //127 cocoa
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //128 sandstone stairs
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //129 emerald ore
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //130 ender chest
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //131 tripwire hook
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //132 tripwire
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //133 emerald block
	{bPLANK1                 , 0                 }, //134 spruce wood stairs
	{bPLANK2                 , 0                 }, //135 birch wood stairs
	{bPLANK3                 , 0                 }, //136 jungle wood stairs
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //137 command block
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //138 beacon
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //139 cobble wall
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //140 flower pot
	{{0xdd, 0x88, 0x00, 0xff}, 0                 }, //141 carrots
	{{0xe9, 0xd8, 0x76, 0xff}, 0                 }, //142 potatoes
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //143 wooden button
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //144 mob head
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //145 anvil
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //146 trapped chest
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //147 gold pressure plate
	{{0x00, 0x00, 0x00, 0x00}, 0                 }, //148 iron pressure plate
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //149 comparator off
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //150 comparator on
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //151 daylight sensor
	{{0xf0, 0x02, 0x02, 0xff}, 0                 }, //152 redstone block
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //153 quartz ore
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //154 hopper
	{{0xf2, 0xf2, 0xf2, 0xff}, 0                 }, //155 quartz block
	{{0xf2, 0xf2, 0xf2, 0xff}, 0                 }, //156 quartz stairs
	{{0x00, 0x00, 0x00, 0xff}, 0                 }, //157 activator rail
	{{0x00, 0x00, 0x00, 0xff}, 0                 }  //158 dropper
};
