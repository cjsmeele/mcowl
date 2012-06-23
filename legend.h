/*
 * legend.h
 *
 *  Created on: Jun 20, 2012
 *      Author: chris
 */

#ifndef LEGEND_H_
#define LEGEND_H_


static const char text_legend[] = {
	' ', //000 air
	'*', //001 stone
	',', //002 grass
	',', //003 dirt
	'*', //004 cobble
	'H', //005 planks
	'\'', //006 sapling
	'&', //007 bedrock
	'~', //008 water
	'~', //009 water
	'^', //010 lava
	'^', //011 lava
	'-', //012 sand
	'-', //013 gravel
	'$', //014 gold ore
	'$', //015 iron ore
	'$', //016 coal ore
	'h', //017 wood
	'%', //018 leaves
	'.', //019 sponge
	'`', //020 glass
	'$', //021 lapis ore
	'$', //022 lapis
	'@', //023 dispenser
	'*', //024 sandstone
	'@', //025 noteblock
	'|', //026 bed
	'#', //027 powered rail
	'#', //028 detector rail
	'@', //029 sticky piston
	'.', //030 cobweb
	',', //031 tall grass
	',', //032 dead bush
	'.', //033
	'.', //034
	'.', //035
	'.', //036
	';', //037 yellow flower
	';', //038 red flower
	'.', //039
	'.', //040
	'$', //041 gold block
	'$', //042 iron block
	'=', //043 double slabs
	'=', //044 slabs
	'B', //045 bricks
	'.', //046
	'.', //047
	'.', //048
	'.', //049
	'i', //050 torch
	'.', //051
	'.', //052
	'.', //053
	'[', //054 chest
	'.', //055
	'.', //056
	'.', //057
	'.', //058
	'.', //059
	'.', //060
	'.', //061
	'.', //062
	'.', //063
	'+', //064 wooden door
	'.', //065
	'.', //066
	'.', //067
	'.', //068
	'.', //069
	'.', //070
	'+', //071 iron door
	'.', //072
	'.', //073
	'.', //074
	'.', //075 redstone torch off
	'.', //076 redstone torch on
	'.', //077
	':', //078 snow
	'_', //079 ice
	'.', //080
	'c', //081 cactus
	'C', //082 clay
	'.', //083
	'.', //084
	'\\', //085 fence
	'.', //086
	'.', //087
	'.', //088
	'.', //089
	'.', //090
	'.', //091
	'.', //092
	'.', //093
	'.', //094
	'.', //095
	'.', //096
	'.', //097
	'B', //098 stonebrick
	'.', //099
	'.', //100
	'.', //101
	'.', //102
	'.', //103
	'.', //104
	'.', //105
	'.', //106
	'+', //107 fence gate
	'b', //108 brick stairs
	'b', //109 stonebrick stairs
	'.', //110
	'o', //111 lily pad
	'.', //112
	'\\', //113 nether fence
	'.', //114
	'.', //115
	'.', //116
	'.', //117
	'.', //118
	'.', //119
	'.', //120
	'.', //121
	'.', //122
	'.', //123
	'.', //124
	'.', //125
	'.', //126
	'.', //127
	'.', //128
	'.', //129
	'.', //130
	'.', //131
	'.', //132
	'.' //133
};

static const uint8_t color_legend[][3] = {
	{0x10, 0x10, 0x10}, //000 air
	{0x7f, 0x7f, 0x7f}, //001 stone
	{0x47, 0xa5, 0x39}, //002 grass
	{0x90, 0x82, 0x49}, //003 dirt
	{0x00, 0x00, 0x00}, //004 cobble
	{0xb0, 0x9a, 0x41}, //005 planks
	{0x00, 0x00, 0x00}, //006 sapling
	{0x00, 0x00, 0x00}, //007 bedrock
	{0x37, 0x57, 0xc8}, //008 water
	{0x37, 0x57, 0xc8}, //009 water
	{0xe5, 0x55, 0x11}, //010 lava
	{0xe5, 0x55, 0x11}, //011 lava
	{0xdf, 0xd7, 0x87}, //012 sand
	{0x94, 0x8f, 0xa1}, //013 gravel
	{0x00, 0x00, 0x00}, //014 gold ore
	{0x9a, 0x9a, 0x9a}, //015 iron ore
	{0x61, 0x53, 0x47}, //016 coal ore
	{0x71, 0x40, 0x00}, //017 wood
	{0x2b, 0x5e, 0x24}, //018 leaves
	{0x00, 0x00, 0x00}, //019 sponge
	{0x00, 0x00, 0x00}, //020 glass
	{0x00, 0x00, 0x00}, //021 lapis ore
	{0x00, 0x00, 0x00}, //022 lapis
	{0x00, 0x00, 0x00}, //023 dispenser
	{0xe0, 0xde, 0xc5}, //024 sandstone
	{0x00, 0x00, 0x00}, //025 noteblock
	{0x00, 0x00, 0x00}, //026 bed
	{0x00, 0x00, 0x00}, //027 powered rail
	{0x00, 0x00, 0x00}, //028 detector rail
	{0x00, 0x00, 0x00}, //029 sticky piston
	{0x00, 0x00, 0x00}, //030 cobweb
	{0x47, 0xa5, 0x39}, //031 tall grass
	{0xd2, 0xca, 0x7b}, //032 dead bush
	{0x00, 0x00, 0x00}, //033
	{0x00, 0x00, 0x00}, //034
	{0xd5, 0xd5, 0xd5}, //035 wool
	{0x00, 0x00, 0x00}, //036
	{0xff, 0xff, 0x10}, //037 yellow flower
	{0xe5, 0x23, 0x2a}, //038 red flower
	{0x00, 0x00, 0x00}, //039
	{0x00, 0x00, 0x00}, //040
	{0x00, 0x00, 0x00}, //041 gold block
	{0x00, 0x00, 0x00}, //042 iron block
	{0x7f, 0x7f, 0x7f}, //043 double slabs
	{0x7f, 0x7f, 0x7f}, //044 slabs
	{0x00, 0x00, 0x00}, //045 bricks
	{0x00, 0x00, 0x00}, //046
	{0x00, 0x00, 0x00}, //047
	{0x00, 0x00, 0x00}, //048
	{0x00, 0x00, 0x00}, //049
	{0xff, 0xea, 0x34}, //050 torch
	{0x00, 0x00, 0x00}, //051
	{0x00, 0x00, 0x00}, //052
	{0xb0, 0x9a, 0x41}, //053 wooden stairs
	{0x8e, 0x6d, 0x43}, //054 chest
	{0x00, 0x00, 0x00}, //055
	{0x00, 0x00, 0x00}, //056
	{0x00, 0x00, 0x00}, //057
	{0x00, 0x00, 0x00}, //058
	{0x00, 0x00, 0x00}, //059
	{0x00, 0x00, 0x00}, //060
	{0x00, 0x00, 0x00}, //061
	{0x00, 0x00, 0x00}, //062
	{0x00, 0x00, 0x00}, //063
	{0x00, 0x00, 0x00}, //064 wooden door
	{0x00, 0x00, 0x00}, //065
	{0x00, 0x00, 0x00}, //066
	{0x00, 0x00, 0x00}, //067
	{0x00, 0x00, 0x00}, //068
	{0x00, 0x00, 0x00}, //069
	{0x00, 0x00, 0x00}, //070
	{0x00, 0x00, 0x00}, //071 iron door
	{0x00, 0x00, 0x00}, //072
	{0x00, 0x00, 0x00}, //073
	{0x00, 0x00, 0x00}, //074
	{0x68, 0x00, 0x00}, //075 redstone torch off
	{0x98, 0x00, 0x00}, //076 redstone torch on
	{0x00, 0x00, 0x00}, //077
	{0xe0, 0xe7, 0xef}, //078 snow
	{0xab, 0xd1, 0xff}, //079 ice
	{0xe0, 0xe7, 0xef}, //080 snow block
	{0x2e, 0x72, 0x25}, //081 cactus
	{0x8d, 0xb4, 0xd8}, //082 clay
	{0x75, 0xf8, 0x38}, //083 reeds
	{0x00, 0x00, 0x00}, //084
	{0xb0, 0x9a, 0x41}, //085 fence
	{0xe1, 0x9c, 0x23}, //086 pumpkin
	{0x00, 0x00, 0x00}, //087
	{0x00, 0x00, 0x00}, //088
	{0xdd, 0xc3, 0x66}, //089 glowstone
	{0x00, 0x00, 0x00}, //090
	{0x00, 0x00, 0x00}, //091
	{0x00, 0x00, 0x00}, //092
	{0x00, 0x00, 0x00}, //093
	{0x00, 0x00, 0x00}, //094
	{0x00, 0x00, 0x00}, //095
	{0x00, 0x00, 0x00}, //096
	{0x00, 0x00, 0x00}, //097
	{0x96, 0x96, 0x96}, //098 stonebrick
	{0x00, 0x00, 0x00}, //099
	{0x00, 0x00, 0x00}, //100
	{0x00, 0x00, 0x00}, //101
	{0x00, 0x00, 0x00}, //102
	{0x00, 0x00, 0x00}, //103
	{0x00, 0x00, 0x00}, //104
	{0x00, 0x00, 0x00}, //105
	{0x4a, 0x7e, 0x43}, //106
	{0x00, 0x00, 0x00}, //107 fence gate
	{0x00, 0x00, 0x00}, //108 brick stairs
	{0x9a, 0x9a, 0x9a}, //109 stonebrick stairs
	{0x00, 0x00, 0x00}, //110
	{0x2d, 0x7f, 0x23}, //111 lily pad
	{0x4c, 0x1b, 0x18}, //112 nether brick
	{0x4c, 0x1b, 0x18}, //113 nether fence
	{0x00, 0x00, 0x00}, //114
	{0x00, 0x00, 0x00}, //115
	{0x00, 0x00, 0x00}, //116
	{0x00, 0x00, 0x00}, //117
	{0x00, 0x00, 0x00}, //118
	{0x00, 0x00, 0x00}, //119
	{0x00, 0x00, 0x00}, //120
	{0x00, 0x00, 0x00}, //121
	{0x00, 0x00, 0x00}, //122
	{0xdd, 0xd2, 0x66}, //123 redstone lamp off
	{0xf9, 0xea, 0x57}, //124 redstone lamp on
	{0x00, 0x00, 0x00}, //125
	{0x00, 0x00, 0x00}, //126
	{0x00, 0x00, 0x00}, //127
	{0x00, 0x00, 0x00}, //128
	{0x00, 0x00, 0x00}, //129
	{0x00, 0x00, 0x00}, //130
	{0x00, 0x00, 0x00}, //131
	{0x00, 0x00, 0x00}, //132
	{0x00, 0x00, 0x00} //133
};


#endif /* LEGEND_H_ */
