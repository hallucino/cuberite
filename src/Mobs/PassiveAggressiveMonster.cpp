
#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include "PassiveAggressiveMonster.h"

#include "../Entities/Player.h"





cPassiveAggressiveMonster::cPassiveAggressiveMonster(const AString & a_ConfigName, eMonsterType a_MobType, const AString & a_SoundHurt, const AString & a_SoundDeath, double a_Width, double a_Height) :
	super(a_ConfigName, a_MobType, a_SoundHurt, a_SoundDeath, a_Width, a_Height)
{
	m_EMPersonality = PASSIVE;
}





bool cPassiveAggressiveMonster::DoTakeDamage(TakeDamageInfo & a_TDI)
{
	if (!super::DoTakeDamage(a_TDI))
	{
		return false;
	}
	
	if ((m_Target != nullptr) && (m_Target->IsPlayer()))
	{
		if (!static_cast<cPlayer *>(m_Target)->IsGameModeCreative())
		{
			m_EMState = CHASING;
		}
	}
	return true;
}




