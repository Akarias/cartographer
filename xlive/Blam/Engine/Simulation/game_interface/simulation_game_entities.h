#pragma once
#include "Blam/Engine/memory/bitstream.h"

enum e_simulation_entity_type : __int16
{
	_simulation_entity_type_none = -1,
	_simulation_entity_type_slayer_engine_globals = 0x0,
	_simulation_entity_type_ctf_engine_globals = 0x1,
	_simulation_entity_type_oddball_engine_globals = 0x2,
	_simulation_entity_type_king_engine_globals = 0x3,
	_simulation_entity_type_territories_engine_globals = 0x4,
	_simulation_entity_type_juggernaut_engine_globals = 0x5,
	_simulation_entity_type_game_engine_player = 0x6,
	_simulation_entity_type_game_engine_statborg = 0x7,
	_simulation_entity_type_breakable_surface_group = 0x8,
	_simulation_entity_type_unit = 0x9,
	_simulation_entity_type_item = 0xA,
	_simulation_entity_type_generic = 0xB,
	_simulation_entity_type_vehicle = 0xC,
	_simulation_entity_type_projectile = 0xD,
	_simulation_entity_type_weapon = 0xE,
	_simulation_entity_type_turret = 0xF,
	_simulation_entity_type_device = 0x10,
	k_simulation_entity_count = 0x11,
};

struct s_simulation_game_entity
{
	datum entity_index;
	e_simulation_entity_type entity_type;
	bool exists_in_gameworld;
	char event_reference_count;
	datum object_index;
	signed int entity_update_flag;
	signed int field_10;
	int creation_data_size;
	void* creation_data;
	int state_data_size;
	void* state_data;
};
CHECK_STRUCT_SIZE(s_simulation_game_entity, 0x24);

class c_simulation_entity_definition
{
public:
	virtual e_simulation_entity_type entity_type(void) = 0;
	virtual const char* entity_type_name(void) = 0;
	virtual int32 state_data_size(void) = 0;
	virtual uint32 creation_data_size(void) = 0;
	virtual int32 update_flag_count(void) = 0;
	virtual uint32 initial_update_mask(void) = 0;
	virtual int8 entity_replication_required_for_view_activation(s_simulation_game_entity* entity) = 0;
	virtual int8 get_object_index(s_simulation_game_entity* entity) = 0;
	virtual int32  sub_A9004D(int a1, int a2, DWORD a3) = 0;;
	virtual int8 creation_minimum_required_bits(s_simulation_game_entity* entity, void* a3, int32* minimum_required_bits) = 0;
	virtual void write_creation_description_to_string(s_simulation_game_entity* entity, void* tel_data, int32 buffer_size, char* buffer) = 0;
	virtual int32 calculate_update_relevance(int a1, int a2, DWORD* a3) = 0;;
	virtual void object_creation_encode(uint32 creation_data_size, void* creation_data, void* telemetry_data, bitstream* packet) = 0;;
	virtual bool object_creation_decode(uint32 creation_data_size, void* creation_data, bitstream* packet) = 0;;
	virtual bool write_update_to_packet(bool unk, DWORD update_mask, DWORD* update_mask_written, DWORD state_data_size, void* state_data, void* telemetry_data, bitstream* packet, int required_leave_space_bits) = 0;;
	virtual bool read_update_from_packet(bool a1, DWORD* out_update_mask, DWORD state_data_size, void* state_data, bitstream* packet) = 0;;
	virtual bool sub_A90891(int a1, int a2, int a3) = 0;
	virtual int8 sub_A908DB(int a1, int a2, int a3) = 0;
	virtual uint32 build_creation_data(s_simulation_game_entity* entity, int32 creation_data_size, void* out_creation_data) = 0;
	virtual bool build_baseline_state_data(int32 creation_data_size, void* creation_data, int32 state_data_size, void* baseline_state_data) = 0;
	virtual bool build_updated_state_data(s_simulation_game_entity* entity, uint32* update_mask, int32 state_data_size, void* state_data) = 0;
	virtual uint32 rotate_entity_indices(s_simulation_game_entity* entity) = 0;
	virtual bool create_game_entity(s_simulation_game_entity* entity, int32 creation_data_size, void* creation_data, uint32 mask, int32 state_data_size, void* state_data) = 0;
	virtual bool update_game_entity(s_simulation_game_entity* entity, uint32 update_flags, int32 state_data_size, void* state_data) = 0;
	virtual bool delete_game_entity(s_simulation_game_entity* entity) = 0;
	virtual bool promote_game_entity_to_authority(s_simulation_game_entity* entity) = 0;
	virtual void* build_object_creation_data(datum object_index, int32 creation_data_size, void* creation_data) = 0;
	virtual void* handle_object_update(datum object_index, uint32 update_mask, int32 state_data_size, void* state_data) = 0;
	virtual void* object_required_to_join_game(datum object_index) = 0;
	virtual void* create_object(int32 creation_data_size, void* creation_data, uint32* flags, int32 internal_state_data_size, void* initial_state_data) = 0;
	virtual void* unk31(int32 a1) = 0;
	virtual void* apply_object_update(datum object_index, uint32 entity_sync_flag, int32 state_data_size, const void* state_data) = 0;
	virtual void* promote_object_to_authority(datum object_index) = 0;
	virtual void* ensure_object_position_update_quantization_inside_bsp(void) = 0;
	virtual void* object_interpolation_data_out(int32 a1, void* a2, void* a3) = 0;
	virtual void* object_interpolation_data(int32 a1, void* a2, void* a3) = 0;
};
