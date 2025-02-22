#include "stdafx.h"
#include "players.h"

#include "Blam/Engine/game/game.h"
#include "Blam/Engine/game/game_engine.h"
#include "Blam/Engine/game/game_engine_util.h"
#include "Blam/Engine/game/game_globals.h"
#include "Blam/Engine/Simulation/game_interface/simulation_game_action.h"
#include "Blam/Engine/units/units.h"
#include "Blam/Engine/saved_games/game_variant.h"
#include "Blam/Engine/scenario/scenario.h"

#include "H2MOD/Modules/Shell/Config.h"
#include "H2MOD/Modules/SpecialEvents/SpecialEvents.h"

#include "Util/Hooks/Hook.h"

/*
	- NOTES:
    - This gets the player data from the game state, thus it is available only during a match or gameplay (game life cycle is in_game or after map has been loaded)
	- If you need to do something in the pregame lobby, use the functions available in Network Session (Blam/Networking/Session)
*/

s_data_array* s_player::get_data()
{
	return *Memory::GetAddress<s_data_array**>(0x4A8260, 0x4D64C4);
}

bool s_player::is_index_valid(datum player_index)
{
    int32 player_abs_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);
	return player_abs_index >= 0 && player_abs_index < k_maximum_players;
}

s_player* s_player::get(datum player_index)
{
	if (!is_index_valid(player_index))
	{
		return nullptr;
	}
	return (s_player*)&get_data()->data[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index) * get_data()->datum_element_size];
}

e_game_team s_player::get_team(datum player_index)
{
	if (!is_index_valid(player_index))
	{
		return (e_game_team)NONE;
	}
	return (e_game_team)get(player_index)->properties[0].team_index;
}

void s_player::set_team(datum player_index, e_game_team team)
{
	if (!is_index_valid(player_index))
	{
		return;
	}
	get(player_index)->properties[0].team_index = (int8)team;
}

void s_player::set_unit_character_type(datum player_index, e_character_type character_type)
{
	if (!is_index_valid(player_index))
	{
		return;
	}
    s_player* player = get(player_index);
    player->properties[0].profile_traits.profile.player_character_type = character_type;
    player->properties[1].profile_traits.profile.player_character_type = character_type;
    return;
}

void s_player::set_unit_speed(datum player_index, float speed)
{
	if (!is_index_valid(player_index))
	{
		return;
	}
	get(player_index)->unit_speed = speed;
}

const wchar_t* s_player::get_name(datum player_index)
{
	if (!is_index_valid(player_index))
	{
		return L"";
	}
	return get(player_index)->properties[0].player_name;
}

datum s_player::get_unit_index(datum player_index)
{
    if (!is_index_valid(player_index)) 
    {
		return DATUM_INDEX_NONE;
    }

	return get(player_index)->unit_index;
}

void s_player::set_player_unit_grenade_count(datum player_index, e_unit_grenade_type type, int8 count, bool reset_equipment)
{
    int32 abs_player_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);
    datum unit_datum_index = s_player::get_unit_index(player_index);
    unit_datum* unit = (unit_datum*)object_try_and_get_and_verify_type(unit_datum_index, FLAG(_object_type_biped));

    // send simulation update for grenades if we control the simulation
    if (unit != NULL && !game_is_predicted())
    {
        // delete all weapons if required
        if (reset_equipment)
        {
            unit_delete_all_weapons(unit_datum_index);
        }

        unit->grenade_counts[type] = count;
        simulation_action_object_update(unit_datum_index, FLAG(_simulation_action_update_grenade_count_bit));

        LOG_TRACE_GAME("set_player_unit_grenade_count() - sending grenade simulation update, playerIndex={0}, peerIndex={1}", abs_player_index, NetworkSession::GetPeerIndex(abs_player_index));
    }
}

unit_datum* s_player::get_player_unit_data(datum player_index)
{
    datum unit_index = s_player::get_unit_index(player_index);
    if (DATUM_IS_NONE(unit_index))
        return nullptr;

    return (unit_datum*)object_get_fast_unsafe(unit_index);
}

real_vector3d* s_player::get_unit_coords(datum player_index)
{
    unit_datum* player_unit = get_player_unit_data(player_index);
    if (player_unit != nullptr)
        return &player_unit->object.position;

    return nullptr;
}

uint64 s_player::get_id(datum player_index)
{
	if (!is_index_valid(player_index))
	{
		return 0ull;
	}

	return get(player_index)->identifier;
}

player_iterator::player_iterator() :
    m_data_iterator(s_player::get_data())
{
}

bool player_iterator::get_next_active_player()
{
	m_current_player = m_data_iterator.get_next_datum();

	while (m_current_player)
	{
		if (!TEST_BIT(m_current_player->flags, _player_left_game_bit))
			break;

		m_current_player = m_data_iterator.get_next_datum();
	}

	return m_current_player != nullptr;
}

s_player* player_iterator::get_current_player_data()
{
	return m_current_player;
}

int player_iterator::get_current_player_index()
{
	return m_data_iterator.get_current_absolute_index();
}

wchar_t* player_iterator::get_current_player_name()
{
	return m_current_player->properties[0].player_name;
}

unsigned long long player_iterator::get_current_player_id()
{
	return s_player::get_id(this->get_current_player_index());
}

s_players_globals* get_players_globals()
{
	return *Memory::GetAddress<s_players_globals**>(0x4A825C, 0x4D64C0);
}

datum __cdecl player_index_from_user_index(int32 user_index)
{
	return INVOKE(0x5141D, 0x5992B, player_index_from_user_index, user_index);
}

bool __cdecl players_user_is_active(int32 user_index)
{
	return INVOKE(0x5139B, 0x598BE, players_user_is_active, user_index);
}

uint32 player_appearance_required_bits()
{
	return 39;
}

void __cdecl player_configuration_validate_character_type(s_player_properties* configuration_data)
{
    // Campaign verification
    if (game_is_campaign())
    {
        scenario* scnr = get_global_scenario();
        uint32 block_size = scnr->player_starting_locations.size;
        if (block_size > 0)
        {
            // Go through every player starting location and check if there's a campaign player type set
            // Set the appropriate data in the player to the values of the tagblock
            bool found = false;
            for (uint32 i = 0; i < block_size; ++i)
            {
                scenario_player* player_starting_location = scnr->player_starting_locations[i];
                if (player_starting_location->campaign_player_type != NONE)
                {
                    configuration_data->team_index = _game_team_player;
                    configuration_data->profile_traits.profile.player_character_type = (e_character_type)player_starting_location->campaign_player_type;
                    found = true;
                    break;
                 }
            }

            // If a campaign_player_type type wasn't found in any of the starting locations set default values
            if (!found)
            {
                configuration_data->team_index = _game_team_player;
                configuration_data->profile_traits.profile.player_character_type = _character_type_masterchief;
            }
        }
        else
        {
            configuration_data->team_index = _game_team_player;
            configuration_data->profile_traits.profile.player_character_type = _character_type_masterchief;
        }
    }
    // Multiplayer verification
    else if (game_is_multiplayer())
    {
        // If the character is mastechief set him to a spartan in multiplayer
        if (configuration_data->profile_traits.profile.player_character_type == _character_type_masterchief)
        {
            configuration_data->profile_traits.profile.player_character_type = _character_type_spartan;
        }

        // Don't allow dervish since he's not loaded properly in shared
        if (configuration_data->profile_traits.profile.player_character_type == _character_type_dervish)
        {
            configuration_data->profile_traits.profile.player_character_type = _character_type_elite;
        }


        // Force skeletons in mp during the halloween event
        // Carto addition
        if (e_character_type character = configuration_data->profile_traits.profile.player_character_type;
            character != _character_type_flood && H2Config_spooky_boy && get_current_special_event() == _special_event_halloween)
        {
            configuration_data->profile_traits.profile.player_character_type = _character_type_skeleton;
            if (!Memory::IsDedicatedServer())
            {
                for (uint32 i = 0; i < k_number_of_users; i++)
                {
                    network_session_interface_set_local_user_character_type(i, _character_type_skeleton);
                }
            }
        }
    }
}

void __cdecl player_validate_configuration(datum player_index, s_player_properties* configuration_data)
{
    player_configuration_validate_character_type(configuration_data);
    
    // General character verification
    e_character_type character = configuration_data->profile_traits.profile.player_character_type;
    if (character != NONE)
    {
        if (character >= _character_type_masterchief)
        {
            s_game_globals* globals = scenario_get_game_globals();
            if (character > (e_character_type)globals->player_representation.size - 1)
            {
                character = (e_character_type)(globals->player_representation.size - 1);
            }
        }
        else
        {
            character = _character_type_masterchief;
        }
        configuration_data->profile_traits.profile.player_character_type = character;
    }

    // Skill verification
    int8 player_displayed_skill = configuration_data->player_displayed_skill;
    if (player_displayed_skill != NONE)
    {
        if (player_displayed_skill < 0)
            player_displayed_skill = 0;
        configuration_data->player_displayed_skill = player_displayed_skill;
    }
    int8 player_overall_skill = configuration_data->player_overall_skill;
    if (player_overall_skill != NONE)
    {
        if (player_overall_skill < 0)
        {
            player_overall_skill = 0;
        }
        configuration_data->player_overall_skill = player_overall_skill;
    }

    // Handicap verification
    e_handicap player_handicap_level = configuration_data->player_handicap_level;
    if (player_handicap_level >= _handicap_none)
    {
        if (player_handicap_level > _handicap_severe)
        {
            player_handicap_level = _handicap_severe;
        }
    }
    else
    {
        player_handicap_level = _handicap_none;
    }
    configuration_data->player_handicap_level = player_handicap_level;
    
    // User role verification
    int8 bungie_user_role = configuration_data->bungie_user_role;
    if (bungie_user_role < 0 || bungie_user_role > 7)
    {
        configuration_data->bungie_user_role = 0;
    }

    if (current_game_engine())
    {
        if (TEST_BIT(get_game_variant()->game_engine_flags, _game_engine_teams_bit))
        {
            if (configuration_data->team_index != NONE && !TEST_BIT(game_engine_globals_get()->team_bitmask, configuration_data->team_index))
            {
                configuration_data->team_index = NONE;
            }
        }
    }

    return;
}

void players_apply_patches(void)
{
    // Change the validation for player_appearance_valid to use the updated k_player_character_type_count constant
    WriteValue<BYTE>(Memory::GetAddress(0x54fb3, 0x5D4AB), k_player_character_type_count);

    // Replace the player profile validation function with our own
    PatchCall(Memory::GetAddress(0x5509E, 0x5D596), player_validate_configuration);
    return;
}
