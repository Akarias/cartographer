#pragma once

#include "Blam/Engine/objects/objects.h"

enum e_game_engine_types
{
	invalid = -1,

	_none = 0,
	_ctf = 1,
	_slayer = 2,
	_oddball = 3,
	_king_of_the_hill = 4,
	_unknown5 = 5,
	_unknown6 = 6,
	_juggernaut = 7,
	_territories = 8,
	_assualt = 9,
	_unknown10 = 10,

	_game_engine_type_enum_size,
};

struct c_engine_internal;
class c_game_engine_base
{
public:
	virtual e_game_engine_types get_type() const { return type; }
	virtual e_game_engine_types get_base_type() const { return base_type; }
	virtual c_engine_internal *get_base_engine() const;
	/* 
		Called on scenario load, returns success
		game engine will be disabled on failure
	*/
	virtual bool setup();

	/* Called on scenario cleanup/exit */
	virtual void cleanup();
	virtual int unk_function_4();
	virtual int unk_function_5(int arg1);
	virtual void unk_function_players_1(int arg1);
	virtual void update_player__maybe__maybe_spawn(int arg1);
	virtual void unk_function_8(int arg1);
	virtual void unk_function_9(int arg1);
	virtual void unk_function_players_2(int arg1, int arg2);
	virtual void unk_function_11(int arg1);
	virtual void unk_function_12();
	virtual void unk_function_13();

	/* Render 3d objects (e.g hologram around hill in KotH) */
	virtual void render_in_world(size_t arg1);
	virtual void update_hud(int hud_owner);
	virtual void unk_function_16(int arg1);
	virtual int unk_function_17(int arg1, int arg2);
	virtual int unk_function_18(int arg1, int arg2);
	virtual void unk_function_19();
	virtual float player_speed_multiplier(int arg1);
	virtual int unk_function_21(int arg1);
	virtual void unk_function_22(int arg1);
	virtual void unk_function_23(int arg1);
	virtual void unk_function_24(int arg1, int arg2);
	virtual int unk_function_25(int arg1, int arg2);
	virtual int unk_function_26(int arg1, int arg2, int arg3);
	virtual int unk_function_27();
	virtual bool is_team_enemy(e_game_team team_a, e_game_team team_b);
	virtual void unk_function_29(int arg1);
	virtual void unk_function_30(int arg1, int arg2, int arg3);
	virtual void unk_function_31(int arg1, int arg2, char arg3, int arg4);
	virtual int unk_function_32(int arg1, int arg2, int arg3);
	virtual void unk_function_33(int arg1, int arg2);
	virtual int unk_function_34(int arg1, int arg2);
	virtual bool unk_function_35(int arg1);
	virtual int unk_function_36(int arg1, int arg2);
	virtual void unk_function_37(int arg1);
	virtual int unk_function_38(int arg1, int arg2);
	virtual bool should_garbage_collect(int arg1);
	virtual void unk_function_40();
	virtual void unk_function_41();
	virtual void* unk_function_42(void* arg1);
	virtual void* unk_function_43(char a1, size_t *a2, void *a3);
	virtual bool unk_function_44(char a1, void *a2);
	virtual int unk_function_45(char a1, int a2);
	virtual int unk_function_46(char a1, int a2, int a3);
	virtual bool unk_function_47(char a1, int a2, int a3);
	virtual void* unk_function_48(int a1, int a2, void *a3);
	virtual void unk_function_49(int arg1, int arg2, int arg3, int arg4);
	virtual bool unk_function_50(__int16 a1, __int16 a2, int a3, int a4);
	virtual int unk_function_51(int arg1, int arg2, int arg3, int arg4, int arg5);

	void set_type(e_game_engine_types new_type)
	{
		type = new_type;
	}
	void set_base_type(e_game_engine_types new_type)
	{
		base_type = new_type;
	}
private:
	e_game_engine_types type = e_game_engine_types::_none;
	e_game_engine_types base_type = e_game_engine_types::_slayer;
};

namespace custom_game_engines
{
	/* Init needed patches */
	void init();

	/* Register a custom game engine */
	bool register_engine(e_game_engine_types id, c_game_engine_base *engine, e_game_engine_types base_id = e_game_engine_types::_slayer);
}
