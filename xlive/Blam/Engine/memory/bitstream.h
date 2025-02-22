#pragma once
#include "Blam/Engine/math/real_math.h"

#pragma pack(push, 1)
struct bitstream
{
	//Credits : https://github.com/HaloMods/OpenSauce/blob/master/OpenSauce/Halo2/Halo2_Xbox/Networking/Networking.hpp

	enum class e_bitstream_state : DWORD
	{
		bitstream_state_none,
		bitstream_state_writing,
		bitstream_state_write_finished,
		bitstream_state_reading,
		bitstream_state_read_and_write,
		bitstream_state_5,				// haven't spent time to figure this one out
	};

	void *m_data;
	DWORD m_data_size_bytes;
	DWORD m_data_size_alignment;
	e_bitstream_state state;
	int m_current_bit_position;
	bool is_debug_stream;
	BYTE gap_14[3];
	int m_position_stack_depth;
	DWORD stack[4];
	int field_2C;
	int field_30;

	void init(void* buffer, DWORD data_length);
	void init(void* buffer, DWORD data_length, DWORD data_size_alignment);
	void begin_writing(DWORD data_size_alignment);
	void finish_writing(void*out_space_left_in_bits);
	bool overflow();
	DWORD get_space_left_in_bits();
	DWORD get_bytes_used();

	void data_encode_string_wide(const char* name, void* string, int size_in_words);
	void data_decode_string_wide(const char* name, void* string_buffer, int size_in_words);
	void data_encode_integer(const char* name, unsigned int value, unsigned int size_in_bits);
	int data_decode_integer(const char* name, unsigned int size_in_bits);
	void data_encode_bits(const char* name, void* data, unsigned int size_in_bits);
	void data_decode_bits(const char* name, void* data, int size_in_bits);
	void data_encode_bool(const char* name, bool value);
	bool data_decode_bool(const char* name);
	void data_decode_address(const char* name, void* address);
	void data_encode_quantized_real(const char* name, float value, float min_value, float max_value, int size_in_bits, char exact_midpoint);
	float data_decode_quantized_real(const char* name, float min_value, float max_value, int size_in_bits, char exact_midpoint);
	void data_encode_unit_vector(const char* name, real_vector3d* vector);
	void data_decode_unit_vector(const char* name, real_vector3d* out_vector);
	void data_encode_signed_integer(const char* name, int value, unsigned int size_in_bits);
	int data_decode_signed_integer(const char* name, unsigned int size_in_bits);
	void data_encode_axes(const char* name, real_vector3d* forward, real_vector3d* up);
	void data_decode_axes(const char* name, real_vector3d* out_forward, real_vector3d* out_up);
	void data_encode_vector(const char* name, real_vector3d* vector, float min_magnitude_value, float max_magnitude_value, int magnitude_size_in_bits);
	void data_decode_vector(const char* name, real_vector3d* out_vector, float min_magnitude_value, float max_magnitude_value, int magnitude_size_in_bits);
	void data_encode_flags(const char *name, unsigned long long value, int size_in_bits);
	unsigned long long data_decode_flags(const char *name, int size_in_bits);
};
#pragma pack(pop)
CHECK_STRUCT_SIZE(bitstream, 0x34);
