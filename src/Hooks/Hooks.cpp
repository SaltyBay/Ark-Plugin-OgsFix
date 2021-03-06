/*****************************************************************************************************************
* Copyright (C) 2019 by Matthias Birnthaler                                                                      *
*                                                                                                                *
* This file is part of the OgsFix Plugin for Ark Server API                                                      *
*                                                                                                                *
*    This program is free software : you can redistribute it and/or modify                                       *
*    it under the terms of the GNU General Public License as published by                                        *
*    the Free Software Foundation, either version 3 of the License, or                                           *
*    (at your option) any later version.                                                                         *
*                                                                                                                *
*    This program is distributed in the hope that it will be useful,                                             *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of                                              *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the                                                 *
*    GNU General Public License for more details.                                                                *
*                                                                                                                *
*    You should have received a copy of the GNU General Public License                                           *
*    along with this program.If not, see <https://www.gnu.org/licenses/>.                                        *
*                                                                                                                *
*****************************************************************************************************************/

/**
* \file Hooks.cpp
* \author Matthias Birnthaler
* \date 15 May 2019
* \brief File containing the implementation for all needed Hooks
*
*/


/* ================================================[includes]================================================ */
#include "Hooks.h"


/* ===================================== [prototype of local functions] ======================================= */

static float  Hook_APrimalStructure_TakeDamage(APrimalStructure* _this, float Damage, FDamageEvent* DamageEvent, AController* EventInstigator, AActor* DamageCauser);
static FString GetBlueprint(UObjectBase* object);


/**
* \brief GetBlueprint
*
* This function determines the blueprint of a UObjectBase
*
* \param[in] object UObjectBase the object to check
* \return FString the blueprint
*/
static FString GetBlueprint(UObjectBase* object)
{
	if (object != nullptr && object->ClassField() != nullptr)
	{
		FString path_name;
		object->ClassField()->GetDefaultObject(true)->GetFullName(&path_name, nullptr);

		if (int find_index = 0; path_name.FindChar(' ', find_index))
		{
			path_name = "Blueprint'" + path_name.Mid(find_index + 1,
				path_name.Len() - (find_index + (path_name.EndsWith(
					"_C", ESearchCase::
					CaseSensitive)
					? 3
					: 1))) + "'";
			return path_name.Replace(L"Default__", L"", ESearchCase::CaseSensitive);
		}
	}

	return FString("");
}


/**
* \brief Hook_APrimalStructure_TakeDamage
*
* This function hooks the APrimalStructure_TakeDamage Implementation. If a ogs structure takes damage from a tek sword the damage will be suppressed.
*
* \param[in] _this APrimalStructure, the structrue that takes damage
* \param[in] Damage the Damage that will be added
* \param[in] DamageEvent, variable not used 
* \param[in] EventInstigator the source of the damage 
* \param[in] DamageCauser the player that makes the damage 
* \return float 
*/
static float  Hook_APrimalStructure_TakeDamage(APrimalStructure* _this, float Damage, FDamageEvent* DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	float damage = Damage;


	if (nullptr != EventInstigator && nullptr != _this)
	{

		FString path_name = GetBlueprint(_this);

		if (path_name.Contains(L"Structure_OGS"))
		{
			APlayerController* player = EventInstigator->GetOwnerController();

			if (nullptr != player)
			{
				const auto shooter_controller = static_cast<AShooterPlayerController*>(player);

				if (nullptr != shooter_controller)
				{
					if (nullptr != shooter_controller->GetPlayerCharacter()->CurrentWeaponField())
					{
						if (nullptr != shooter_controller->GetPlayerCharacter()->CurrentWeaponField()->AssociatedPrimalItemField())
						{
							FString Weapon;
							shooter_controller->GetPlayerCharacter()->CurrentWeaponField()->AssociatedPrimalItemField()->GetItemName(&Weapon, false, true, nullptr);

							if (Weapon.Contains(L"Tek Sword"))
							{
								FString displayString ="You not allowed to damage OGS with tek sword";

								ArkApi::GetApiUtils().SendServerMessage(shooter_controller, FColorList::Red, *displayString);

								damage = 0; 
							}
							Log::GetLog()->error(Weapon.ToString());
						}
					}
				}
			}
		}
	}

	return APrimalStructure_TakeDamage_original(_this, damage, DamageEvent, EventInstigator, DamageCauser);
}




/* ===================================== [definition of global functions] ===================================== */

/**
* \brief Initialisation of needed Hooks
*
* This function initialise all needed Hooks
*
* \return void
*/
void InitHooks(void)
{
	ArkApi::GetHooks().SetHook("APrimalStructure.TakeDamage", &Hook_APrimalStructure_TakeDamage, 
		&APrimalStructure_TakeDamage_original);
}


/**
* \brief Cancellation of needed Hooks
*
* This function removes all needed Hooks.
*
* \return void
*/
void RemoveHooks(void)
{
	ArkApi::GetHooks().DisableHook("APrimalStructure.TakeDamage", 
		&Hook_APrimalStructure_TakeDamage);
}

/* =================================================[end of file]================================================= */
