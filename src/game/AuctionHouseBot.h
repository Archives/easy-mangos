#ifndef AUCTION_HOUSE_BOT_H
#define AUCTION_HOUSE_BOT_H

#include "World.h"
#include "Config/Config.h"
#include "ace/Vector_T.h"
#include "ObjectGuid.h"
#include "Item.h"

#define AHB_GREY        0
#define AHB_WHITE       1
#define AHB_GREEN       2
#define AHB_BLUE        3
#define AHB_PURPLE      4
#define AHB_ORANGE      5
#define AHB_YELLOW      6
#define AHB_GREY_TG     0
#define AHB_WHITE_TG    1
#define AHB_GREEN_TG    2
#define AHB_BLUE_TG     3
#define AHB_PURPLE_TG   4
#define AHB_ORANGE_TG   5
#define AHB_YELLOW_TG   6
#define AHB_GREY_I      7
#define AHB_WHITE_I     8
#define AHB_GREEN_I     9
#define AHB_BLUE_I      10
#define AHB_PURPLE_I    11
#define AHB_ORANGE_I    12
#define AHB_YELLOW_I    13

#define AHB_QUALITY_MAX 7

enum e_ahb_quality{
    E_GREY      = 0,
    E_WHITE     = 1,
    E_GREEN     = 2,
    E_BLUE      = 3,
    E_PURPLE    = 4,
    E_ORANGE    = 5,
    E_YELLOW    = 6,
};


class c_ItemClassInfos
{
private:
    uint32 m_AmountOfItems;
    uint32 m_MissItems;
    uint32 m_Quantity;
    uint32 m_class;

    std::string m_className;

public:
    c_ItemClassInfos(uint32 indice=0)
    {
        m_class=(indice % MAX_ITEM_CLASS);

        switch (m_class)
        {
        case 0:     m_className = "Consumable"; break;
        case 1:     m_className = "Container"; break;
        case 2:     m_className = "Weapon"; break;
        case 3:     m_className = "Gem"; break;
        case 4:     m_className = "Armor"; break;
        case 5:     m_className = "Reageant"; break;
        case 6:     m_className = "Projectile"; break;
        case 7:     m_className = "Trade_Goods"; break;
        case 8:     m_className = "Generic"; break;
        case 9:     m_className = "Recipe"; break;
        case 10:    m_className = "Money"; break;
        case 11:    m_className = "Quiver"; break;
        case 12:    m_className = "Quest"; break;
        case 13:    m_className = "Key"; break;
        case 14:    m_className = "Permanant"; break;
        case 15:    m_className = "Misc"; break;
        case 16:    m_className = "Glyph"; break;
        }
        m_AmountOfItems=0;
        m_MissItems=0;
        m_Quantity=0;
    }

    const std::string GetName()
    {
        return m_className;
    }
    void SetAmountOfItems(uint32 indice)
    {
        m_AmountOfItems = indice*m_Quantity;
    }
    uint32 GetAmountOfItems()
    {
        return m_AmountOfItems;
    }
    void SetQuantityOfItems(uint32 quantity)
    {
        m_Quantity = quantity;
    }
    uint32 GetQuantityOfItems()
    {
        return m_Quantity;
    }
    void SetMissItems(uint32 value)
    {
        (m_AmountOfItems > value ) ? m_MissItems=m_AmountOfItems-value: m_MissItems = 0;
    }
    uint32 GetMissItems()
    {
        return m_MissItems;
    }
};

class c_ItemInfos
{
private:
    uint32 m_AmountOfItems;
    uint32 m_MaxItemsPrice;
    uint32 m_MinItemsPrice;
    uint32 m_MinBidPrice;
    uint32 m_MaxBidPrice;
    uint32 m_MaxStack;
    uint32 m_MissItems;
    uint32 m_buyerPrice;

    uint32 m_indice;

    std::string m_color;

public:

    std::vector<c_ItemClassInfos> ItemClassInfos;

    c_ItemInfos(uint32 indice=0)
    {
        for (int i=0; i<MAX_ITEM_CLASS; ++i) 
        {
            ItemClassInfos.push_back(c_ItemClassInfos(i));
        }

        m_indice=(indice % AHB_QUALITY_MAX);

        switch (m_indice)
        {
        case 0: m_color = "grey"; break;
        case 1: m_color = "white"; break;
        case 2: m_color = "green"; break;
        case 3: m_color = "blue"; break;
        case 4: m_color = "purple"; break;
        case 5: m_color = "orange"; break;
        case 6: m_color = "yellow"; break;
        }
    }
    const std::string GetColor()
    {
        return m_color;
    }
    void SetAmountOfItems(uint32 cnt)
    {
        m_AmountOfItems=cnt;
    }
    uint32 GetAmountOfItems()
    {
        return m_AmountOfItems;
    }
    void SetMinPrice(uint32 value)
    {
        m_MinItemsPrice=value;
    }
    uint32 GetMinPrice()
    {
        return m_MinItemsPrice;
    }

    void SetMaxPrice(uint32 value)
    {
        m_MaxItemsPrice=value;
    }
    uint32 GetMaxPrice()
    {
        return m_MaxItemsPrice;
    }
    void SetMinBidPrice(uint32 value)
    {
        m_MinBidPrice=value;
    }
    uint32 GetMinBidPrice()
    {
        return m_MinBidPrice;
    }
    void SetMaxBidPrice(uint32 value)
    {
        m_MaxBidPrice=value;
    }
    uint32 GetMaxBidPrice()
    {
        return m_MaxBidPrice;
    }
    void SetMaxStack(uint32 value)
    {
        m_MaxStack=value;
    }
    uint32 GetMaxStack()
    {
        return m_MaxStack;
    }
    void SetBuyerPrice(uint32 value)
    {
        m_buyerPrice = value;
     }
    uint32 GetBuyerPrice()
    {
        return m_buyerPrice;
    }
};

class AHBConfig
{
private:
    uint32 AHID;
    uint32 AHFID;
    uint32 minTime;
    uint32 maxTime;

    uint32 buyerPriceGrey;
    uint32 buyerPriceWhite;
    uint32 buyerPriceGreen;
    uint32 buyerPriceBlue;
    uint32 buyerPricePurple;
    uint32 buyerPriceOrange;
    uint32 buyerPriceYellow;
    uint32 buyerBiddingInterval;
    uint32 buyerBidsPerInterval;

public:

    std::vector<c_ItemInfos> ItemInfos;

    AHBConfig(uint32 ahid=120)
    {
        AHID = ahid;
        switch(ahid)
        {
        case 2:
            AHFID = 55;
            break;
        case 6:
            AHFID = 29;
            break;
        case 7:
            AHFID = 120;
            break;
        default:
            AHFID = 120;
            break;
        }

        for (int i=0; i<AHB_QUALITY_MAX; ++i) 
        {
            ItemInfos.push_back(c_ItemInfos(i));
        }
    }


    uint32 GetAHID()
    {
        return AHID;
    }
    uint32 GetAHFID()
    {
        return AHFID;
    }
        void SetMinTime(uint32 value)
    {
        minTime = value;
    }
    uint32 GetMinTime()
    {
        if (minTime < 1)
            return 1;
        else if ((maxTime) && (minTime > maxTime))
            return maxTime;
        else
            return minTime;
    }
    void SetMaxTime(uint32 value)
    {
        maxTime = value;
    }
    uint32 GetMaxTime()
    {
        return maxTime;
    }
    void SetBiddingInterval(uint32 value)
    {
        buyerBiddingInterval = value;
    }
    uint32 GetBiddingInterval()
    {
        return buyerBiddingInterval;
    }
    void SetBidsPerInterval(uint32 value)
    {
        buyerBidsPerInterval = value;
    }
    uint32 GetBidsPerInterval()
    {
        return buyerBidsPerInterval;
    }

    ~AHBConfig()
    {
    }
};

struct s_randomArray
{
    uint32 color;
    uint32 itemclass;
};

class AuctionHouseBot
{
private:
    ACE_Vector<uint32> npcItems;
    ACE_Vector<uint32> lootItems;
    
    std::vector<std::vector<std::vector<uint32> > > ItemPool;
    
    bool debug_Out;

    AHBConfig AllianceConfig;
    AHBConfig HordeConfig;
    AHBConfig NeutralConfig;

    time_t _lastrun_a;
    time_t _lastrun_h;
    time_t _lastrun_n;

    uint32 ItemsPerCycleBoost;
    uint32 ItemsPerCycleNormal;

    ObjectGuid m_FakeGuid;

    inline uint32 minValue(uint32 a, uint32 b) { return a <= b ? a : b; };
    void    addNewAuctions(AHBConfig& config);
    void    addNewAuctionBuyerBotBid(AHBConfig *config, WorldSession *session);
    uint32  SetStat(AHBConfig& config);
    bool    getRandomArray( AHBConfig& config, std::vector<s_randomArray>& ra, const std::vector<std::vector<uint32> >& addedItem  );
    void    SetPricesOfItem(const Item *item, AHBConfig& config, uint32& buyp, uint32& bidp, uint32& stackcnt, e_ahb_quality AHB_ITEMS);
public:
    AuctionHouseBot();
    ~AuctionHouseBot();
    void Update();
    void Initialize();
    void LoadConfig();
    void LoadSellerValues(AHBConfig& config);
    void LoadBuyerValues(AHBConfig& config);
    void Commands(uint32, uint32, uint32, char*);
    ObjectGuid GetAHBObjectGuid() { return m_FakeGuid; };
};

#define auctionbot MaNGOS::Singleton<AuctionHouseBot>::Instance()

#endif
