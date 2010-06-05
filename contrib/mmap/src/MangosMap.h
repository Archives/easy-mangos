#ifndef _MMAP_MANGOS_MAP_H
#define _MMAP_MANGOS_MAP_H

#include "MMapCommon.h"

// following is copied from src/game/Map.h (too many useless includes there to use original file)
namespace MaNGOS
{
    struct map_fileheader
    {
        uint32 mapMagic;
        uint32 versionMagic;
        uint32 buildMagic;
        uint32 areaMapOffset;
        uint32 areaMapSize;
        uint32 heightMapOffset;
        uint32 heightMapSize;
        uint32 liquidMapOffset;
        uint32 liquidMapSize;
    };

    // ==============mmaps don't use area==============
    //#define MAP_AREA_NO_AREA      0x0001

    //struct map_areaHeader
    //{
    //    uint32 fourcc;
    //    uint16 flags;
    //    uint16 gridArea;
    //};

    #define MAP_HEIGHT_NO_HEIGHT  0x0001
    #define MAP_HEIGHT_AS_INT16   0x0002
    #define MAP_HEIGHT_AS_INT8    0x0004

    struct map_heightHeader
    {
        uint32 fourcc;
        uint32 flags;
        float  gridHeight;
        float  gridMaxHeight;
    };

    // ==============mmaps don't use liquid yet==============
    //#define MAP_LIQUID_NO_TYPE    0x0001
    //#define MAP_LIQUID_NO_HEIGHT  0x0002

    //struct map_liquidHeader
    //{
    //    uint32 fourcc;
    //    uint16 flags;
    //    uint16 liquidType;
    //    uint8  offsetX;
    //    uint8  offsetY;
    //    uint8  width;
    //    uint8  height;
    //    float  liquidLevel;
    //};

    //enum ZLiquidStatus
    //{
    //    LIQUID_MAP_NO_WATER     = 0x00000000,
    //    LIQUID_MAP_ABOVE_WATER  = 0x00000001,
    //    LIQUID_MAP_WATER_WALK   = 0x00000002,
    //    LIQUID_MAP_IN_WATER     = 0x00000004,
    //    LIQUID_MAP_UNDER_WATER  = 0x00000008
    //};

    //#define MAP_LIQUID_TYPE_NO_WATER    0x00
    //#define MAP_LIQUID_TYPE_WATER       0x01
    //#define MAP_LIQUID_TYPE_OCEAN       0x02
    //#define MAP_LIQUID_TYPE_MAGMA       0x04
    //#define MAP_LIQUID_TYPE_SLIME       0x08

    //#define MAP_ALL_LIQUIDS   (MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN | MAP_LIQUID_TYPE_MAGMA | MAP_LIQUID_TYPE_SLIME)

    //#define MAP_LIQUID_TYPE_DARK_WATER  0x10
    //#define MAP_LIQUID_TYPE_WMO_WATER   0x20

    //struct LiquidData
    //{
    //    uint32 type;
    //    float  level;
    //    float  depth_level;
    //};
}

#endif