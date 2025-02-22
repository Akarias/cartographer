#include "stdafx.h"
#include "new_hud.h"

#include "Blam/Engine/bitmaps/bitmap_group.h"
#include "Blam/Engine/camera/camera.h"
#include "Blam/Engine/game/game.h"
#include "Blam/Engine/interface/hud.h"
#include "Blam/Engine/interface/new_hud_definitions.h"
#include "Blam/Engine/main/main_screenshot.h"
#include "Blam/Engine/Networking/logic/life_cycle_manager.h"
#include "Blam/Engine/math/integer_math.h"

#include "H2MOD/Modules/Input/KeyboardInput.h"
#include "H2MOD/Modules/Shell/Config.h"
#include "H2MOD/Tags/TagInterface.h"
#include "Util/Hooks/Hook.h"

bool g_should_draw_hud_override = true;
std::vector<datum> crosshair_bitmap_datums;				// Store all the crosshair bitmap datums
std::vector<point2d> crosshair_original_bitmap_sizes;	// We use point2d struct to store the original resolutions (x as width and y as height)

void should_draw_hud_override_set(bool flag)
{
	g_should_draw_hud_override = flag;
	return;
}

s_new_hud_engine_globals* get_new_hud_engine_globals(void)
{
	return *Memory::GetAddress<s_new_hud_engine_globals**>(0x9770F4, 0x99E93C);
}

s_hud_scripted_globals* get_hud_scripted_globals(void)
{
	return *Memory::GetAddress<s_hud_scripted_globals**>(0x9765CC, 0x99FBB4);
}

s_new_hud_temporary_user_state* get_new_hud_temporary_user_state(const int local_user_index)
{
	return &Memory::GetAddress<s_new_hud_temporary_user_state*>(0x9766D0, 0)[local_user_index];
}

bool __cdecl render_ingame_chat_check() 
{
	if (H2Config_hide_ingame_chat)
	{
		datum local_player_datum_index = player_index_from_user_index(0);
		if (s_player::get(local_player_datum_index)->is_chatting == 2) 
		{
			hotkeyFuncToggleHideIngameChat();
		}
		return true;
	}

	else if (!game_is_ui_shell() && get_game_life_cycle() == _life_cycle_in_game)
	{
		//Enable chat in engine mode and game state mp.
		return false;
	}
	else {
		//original test - if is campaign
		return true;
	}
}

// Hook for ui_get_hud_elements for modifying the hud anchor for text
void __cdecl ui_get_hud_elemet_bounds_hook(e_hud_anchor anchor, real_bounds* bounds)
{
	float safe_area = *Memory::GetAddress<float*>(0x9770F0);
	s_camera* camera_data = get_global_camera();

	float scale_factor = *get_secondary_hud_scale();

	typedef void(__cdecl* ui_get_hud_elemets_anchor_t)(e_hud_anchor, real_bounds*);
	auto p_ui_get_hud_elemet_bounds = Memory::GetAddress<ui_get_hud_elemets_anchor_t>(0x223969);

	switch (anchor)
	{
	case hud_anchor_weapon_hud:
		bounds->lower = (float)camera_data->window_bounds.left + safe_area;
		bounds->upper = (float)camera_data->window_bounds.top + (safe_area / scale_factor); // (100.f * scale_factor) - 100.f;
		break;
	default:
		p_ui_get_hud_elemet_bounds(anchor, bounds);
		break;
	}
}


// We scale crosshairs by adjusting the bitmap data size in the bitmap tag
// It doesn't seem to actually downscale the bitmap since the data loaded still remains the same
void set_crosshair_scale(float scale)
{
	size_t bitmap_size_vector_index = 0;

	// Loops through every bitmap datum in the vector that's considered a crosshair
	for (size_t i = 0; i < crosshair_bitmap_datums.size(); ++i)
	{
		// Grab the bitmap definition
		bitmap_group* bitm_definition = tags::get_tag_fast<bitmap_group>(crosshair_bitmap_datums[i]);

		// Loop through every bitmap inside the bitmap tag
		for (size_t j = 0; j < bitm_definition->bitmaps.size; ++j)
		{
			bitmap_data* bitmap_data_block = bitm_definition->bitmaps[j];
			point2d original_bitmap_size = crosshair_original_bitmap_sizes[bitmap_size_vector_index];

			// Multiply bitmap size by scale and then by the upscale size
			// We do (1 /k_secondary_upscale_size) in the calculations since we need to scale down the bitmap depending on the resolution increase
			// Example: the bitmap provided is supposed to be 4 times larger in size compared to the original so we need to scale it down by (1/4) so 0.25
			bitmap_data_block->width_pixels = original_bitmap_size.x * scale * (1 / k_crosshair_upscale_size);
			bitmap_data_block->height_pixels = original_bitmap_size.y * scale * (1 / k_crosshair_upscale_size);

			++bitmap_size_vector_index;
		}
	}
}

// Stores the bitmap width and height in crosshair_original_bitmap_sizes for use when scaling the crosshair bitmaps
void initialize_crosshair_bitmap_data()
{
	for (size_t i = 0; i < crosshair_bitmap_datums.size(); ++i)
	{
		bitmap_group* bitm_definition = tags::get_tag_fast<bitmap_group>(crosshair_bitmap_datums[i]);
		for (size_t j = 0; j < bitm_definition->bitmaps.size; ++j)
		{
			bitmap_data* bitmap_data_block = bitm_definition->bitmaps[j];
			point2d bitmap_size = { bitmap_data_block->width_pixels, bitmap_data_block->height_pixels };
			crosshair_original_bitmap_sizes.push_back(bitmap_size);
		}
	}
}

// Checks if the vector contains the datum provided so we don't have duplicate datums in the vector
bool crosshair_bitmap_vector_contains_datum(datum tag_datum)
{
	for (size_t i = 0; i < crosshair_bitmap_datums.size(); ++i)
	{
		if (crosshair_bitmap_datums[i] == tag_datum)
		{
			return true;
		}
	}
	
	return false;
}

// Gets all bitmap tags that are referenced as a crosshair from the new_hud_definition tags
// Intended to grab all bitmaps referenced as crosshairs rather than just the original ones
void get_crosshair_bitmap_datums()
{
	// Get all nhdt tags
	std::map<datum, std::string> new_hud_definition_tags = tags::find_tags(blam_tag::tag_group_type::newhuddefinition);
	for each (auto nhdt_tag in new_hud_definition_tags)
	{
		s_new_hud_definition* nhdt_definition = tags::get_tag_fast<s_new_hud_definition>(nhdt_tag.first);

		// Loop through every bitmap widget in the nhdt definition
		for (byte i = 0; i < nhdt_definition->bitmap_widgets.size; ++i)
		{
			s_hud_bitmap_widget_definition* bitmap_widget_definition = nhdt_definition->bitmap_widgets[i];

			if (bitmap_widget_definition->widget_inputs.input_1 == hud_input_type_unit_autoaimed && 
				bitmap_widget_definition->anchor == hud_anchor_crosshair && 
				!crosshair_bitmap_vector_contains_datum(bitmap_widget_definition->bitmap.TagIndex))
			{
				crosshair_bitmap_datums.push_back(bitmap_widget_definition->bitmap.TagIndex);
			}
		}
	}

}

void initialize_crosshair_scale(bool game_mode_ui_shell)
{
	// Clear data from previous map file
	crosshair_bitmap_datums.clear();
	crosshair_original_bitmap_sizes.clear();

	if (game_mode_ui_shell) { return; }

	get_crosshair_bitmap_datums();
	initialize_crosshair_bitmap_data();
	set_crosshair_scale(H2Config_crosshair_scale);
}

// Checks if we shouldn't draw the hud
bool new_hud_dont_draw(void)
{
	s_screenshot_globals* globals = get_screenshot_globals();

	// Added check for get_new_hud_engine_globals()->show_hud since it will still render other parts of the hud if show_hud is set to false
	// This does not match legacy behaviour, however the text for picking up weapons is a part of the hud so I assume this is a mistake from bungie
	bool dont_draw_hud = !g_should_draw_hud_override;


	bool original_check = globals->taking_screenshot && globals->resolution_multiplier > 1;

	// If original check or show_hud check are true we don't draw the hud
	// Otherwise return false so we show the hud
	return (original_check || dont_draw_hud ? true : false);
}

void new_hud_apply_patches()
{
	if (Memory::IsDedicatedServer()) { return; }

	KeyboardInput::RegisterHotkey(&H2Config_hotkeyIdToggleHideIngameChat,
		[]() {
			H2Config_hide_ingame_chat = !H2Config_hide_ingame_chat;
		}
	);

	// Redirects the is_campaign call that the in-game chat renderer makes so we can show/hide it as we like.
	PatchCall(Memory::GetAddress(0x22667B), render_ingame_chat_check);
	PatchCall(Memory::GetAddress(0x226628), render_ingame_chat_check);

	// Hook ui_get_hud_elements for modifying the hud anchor for text
	PatchCall(Memory::GetAddress(0x22D25A), ui_get_hud_elemet_bounds_hook);

	// Hook new_hud_dont_draw so we can use another bool to make sure we don't draw any parts of the hud
	PatchCall(Memory::GetAddress(0x2278B3), new_hud_dont_draw);
}

void new_hud_patches_on_map_load(bool game_mode_ui_shell)
{
	if (Memory::IsDedicatedServer()) { return; }

	initialize_crosshair_scale(game_mode_ui_shell);
}