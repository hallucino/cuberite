
#pragma once

#include "../Entities/EntityEffect.h"
#include "../Entities/SplashPotionEntity.h"

class cItemPotionHandler:
	public cItemHandler
{
	typedef cItemHandler super;
	
	int GetPotionName(short a_ItemDamage)
	{
		// First six bits (least significant)
		return a_ItemDamage & 0x3F;
	}
	
	cEntityEffect::eType GetEntityEffectType(short a_ItemDamage)
	{
		// First four bits (least significant)
		// Potion effect bits are different from entity effect values
		// For reference: http://minecraft.gamepedia.com/Data_values#.22Potion_effect.22_bits
		switch (a_ItemDamage & 0xF)
		{
			case 0x1: return cEntityEffect::effRegeneration;
			case 0x2: return cEntityEffect::effSpeed;
			case 0x3: return cEntityEffect::effFireResistance;
			case 0x4: return cEntityEffect::effPoison;
			case 0x5: return cEntityEffect::effInstantHealth;
			case 0x6: return cEntityEffect::effNightVision;
			case 0x8: return cEntityEffect::effWeakness;
			case 0x9: return cEntityEffect::effStrength;
			case 0xA: return cEntityEffect::effSlowness;
			case 0xC: return cEntityEffect::effInstantDamage;
			case 0xD: return cEntityEffect::effWaterBreathing;
			case 0xE: return cEntityEffect::effInvisibility;
			
			// No effect potions
			case 0x0:
			case 0x7:
			case 0xB: // Will be potion of leaping in 1.8
			case 0xF:
			{
				break;
			}
		}
		
		return cEntityEffect::effNoEffect;
	}
	
	short GetEntityEffectIntensity(short a_ItemDamage)
	{
		// Level II potion if fifth bit (from zero) is set
		return (a_ItemDamage & 0x20) ? 1 : 0;
	}
	
	int GetEntityEffectDuration(short a_ItemDamage)
	{
		// Base duration in ticks
		int base = 0;
		double TierCoeff = 1, ExtCoeff = 1, SplashCoeff = 1;
		
		switch (GetEntityEffectType(a_ItemDamage))
		{
			case cEntityEffect::effRegeneration:
			case cEntityEffect::effPoison:
			{
				base = 900;
				break;
			}
				
			case cEntityEffect::effSpeed:
			case cEntityEffect::effFireResistance:
			case cEntityEffect::effNightVision:
			case cEntityEffect::effStrength:
			case cEntityEffect::effWaterBreathing:
			case cEntityEffect::effInvisibility:
			{
				base = 3600;
				break;
			}
				
			case cEntityEffect::effWeakness:
			case cEntityEffect::effSlowness:
			{
				base = 1800;
				break;
			}
		}
		
		// If potion is level 2, half the duration. If not, stays the same
		TierCoeff = (GetEntityEffectIntensity(a_ItemDamage) > 0) ? 0.5 : 1;
		
		// If potion is extended, multiply duration by 8/3. If not, stays the same
		// Extended potion if sixth bit (from zero) is set
		ExtCoeff = (a_ItemDamage & 0x40) ? (8.0/3.0) : 1;
		
		// If potion is splash potion, multiply duration by 3/4. If not, stays the same
		SplashCoeff = IsDrinkable(a_ItemDamage) ? 1 : 0.75;
		
		// For reference: http://minecraft.gamepedia.com/Data_values#.22Tier.22_bit
		//                http://minecraft.gamepedia.com/Data_values#.22Extended_duration.22_bit
		//                http://minecraft.gamepedia.com/Data_values#.22Splash_potion.22_bit
		
		return (int)(base * TierCoeff * ExtCoeff * SplashCoeff);
	}
	
public:
	cItemPotionHandler():
	super(E_ITEM_POTIONS)
	{
	}
	
	virtual bool IsDrinkable(short a_ItemDamage) override
	{
		// Drinkable potion if 13th bit (from zero) is set
		// For reference: http://minecraft.gamepedia.com/Potions#Data_value_table
		return ((a_ItemDamage & 0x2000) != 0);
	}
	
	virtual bool OnItemUse(cWorld * a_World, cPlayer * a_Player, const cItem & a_Item, int a_BlockX, int a_BlockY, int a_BlockZ, eBlockFace a_Dir) override
	{
		short PotionDamage = a_Item.m_ItemDamage;
		
		// Only called when potion is a splash potion
		if (IsDrinkable(PotionDamage))
		{
			return false;
		}
		
		Vector3d Pos = a_Player->GetThrowStartPos();
		Vector3d Speed = a_Player->GetLookVector() * 7;
		
		cSplashPotionEntity *Projectile = new cSplashPotionEntity(a_Player, Pos.x, Pos.y, Pos.z, &Speed, GetEntityEffectType(PotionDamage), cEntityEffect(GetEntityEffectDuration(PotionDamage), GetEntityEffectIntensity(PotionDamage)), GetPotionName(PotionDamage));
		
		if (Projectile == NULL)
		{
			return false;
		}
		if (!Projectile->Initialize(*a_World))
		{
			delete Projectile;
			return false;
		}
		
		if (!a_Player->IsGameModeCreative())
		{
			a_Player->GetInventory().RemoveOneEquippedItem();
		}
		
		return true;
	}
	
	virtual bool EatItem(cPlayer * a_Player, cItem * a_Item) override
	{
		short PotionDamage = a_Item->m_ItemDamage;
		
		// Only called when potion is a drinkable potion
		if (!IsDrinkable(a_Item->m_ItemDamage))
		{
			return false;
		}
		
		a_Player->AddEntityEffect(GetEntityEffectType(PotionDamage), GetEntityEffectDuration(PotionDamage), GetEntityEffectIntensity(PotionDamage));
		a_Player->GetInventory().RemoveOneEquippedItem();
		a_Player->GetInventory().AddItem(E_ITEM_GLASS_BOTTLE);
		return true;
	}
};
