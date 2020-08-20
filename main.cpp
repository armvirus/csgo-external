#include "stdafx.hpp"

#define team_offset 0xF4
#define glow_index 0xA438

namespace global_info
{
	HANDLE process_handle{};

	std::uint32_t process_id{};
	std::uintptr_t module_base{};

	std::uint32_t process_size{};
	std::uintptr_t copied_buffer{};
}

std::uint32_t dump_sig(const char* signature, int extra, int offset)
{
	std::uintptr_t instruction_offset = memory::signature_scan(global_info::copied_buffer, global_info::process_size, signature);

	return memory::read<std::uint32_t>(global_info::process_handle, global_info::module_base + instruction_offset + offset) + extra - global_info::module_base;
}

int main()
{
	std::printf("[+] starting [armvirus]\n\n");

	global_info::process_id = memory::find_process_id("csgo.exe");

	if (!global_info::process_id)
	{
		printf("[-] failed to find process id\n");
		return -1;
	}

	std::printf("[+] process id [%i]\n", global_info::process_id);

	global_info::module_base = memory::get_module_base(global_info::process_id, "client.dll");

	if (!global_info::module_base)
	{
		printf("[-] failed to find module base\n");
		return -1;
	}

	std::printf("[+] module base [%p]\n", global_info::module_base);

	global_info::process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, global_info::process_id);

	if (!global_info::process_handle)
	{
		printf("[-] failed to open handle to process [run as admin]\n");
		return -1;
	}

	std::printf("[+] handle to process [%p]\n\n", global_info::process_handle);

	scanner process_buffer(global_info::process_handle, global_info::process_id, global_info::module_base);

	std::tie(global_info::process_size, global_info::copied_buffer) = process_buffer.copy_process();

	std::printf("[+] process size [%i] > [%p]\n\n", global_info::process_size, global_info::copied_buffer);

	std::uint32_t glow_manager_offset = dump_sig("A1 ? ? ? ? A8 01 75 4B", 4, 1);
	std::uint32_t local_player_offset = dump_sig("8D 34 85 ? ? ? ? 89 15 ? ? ? ? 8B 41 08 8B 48 04 83 F9 FF", 4, 3);
	std::uint32_t entity_list_offset = dump_sig("BB ? ? ? ? 83 FF 01 0F 8C ? ? ? ? 3B F8", 0, 1);

	printf("[+] dumped glow manager offset [0x%x]\n", glow_manager_offset);
	printf("[+] dumped local player offset [0x%x]\n", local_player_offset);
	printf("[+] dumped entity list offset [0x%x]\n", entity_list_offset);

	printf("\n[+] press [F1] to close\n");

	while (!GetAsyncKeyState(VK_F1))
	{
		std::uint32_t glow_manager = memory::read<std::uint32_t>(global_info::process_handle, global_info::module_base + glow_manager_offset);
		std::uint32_t local_player = memory::read<std::uint32_t>(global_info::process_handle, global_info::module_base + local_player_offset);

		int local_team = memory::read<int>(global_info::process_handle, local_player + team_offset);

		for (int i = 0; i < 64; i++)
		{
			std::uint32_t entity = memory::read<std::uint32_t>(global_info::process_handle, global_info::module_base + entity_list_offset + i * 0x10);
			if (entity == 0 || entity == local_player) continue;

			int entity_team = memory::read<int>(global_info::process_handle, entity + team_offset);

			std::uint32_t entity_glow_index = memory::read<std::uint32_t>(global_info::process_handle, entity + glow_index) * 0x38;

			if (local_team == entity_team)
			{
				memory::write<float>(global_info::process_handle, glow_manager + entity_glow_index + 0x4, 0);
				memory::write<float>(global_info::process_handle, glow_manager + entity_glow_index + 0x8, 2.0f);
				memory::write<float>(global_info::process_handle, glow_manager + entity_glow_index + 0xc, 0);
				memory::write<float>(global_info::process_handle, glow_manager + entity_glow_index + 0x10, 1.0f);
			}
			else
			{
				memory::write<float>(global_info::process_handle, glow_manager + entity_glow_index + 0x4, 2.0f);
				memory::write<float>(global_info::process_handle, glow_manager + entity_glow_index + 0x8, 0);
				memory::write<float>(global_info::process_handle, glow_manager + entity_glow_index + 0xc, 0);
				memory::write<float>(global_info::process_handle, glow_manager + entity_glow_index + 0x10, 1.0f);
			}

			memory::write<bool>(global_info::process_handle, glow_manager + entity_glow_index + 0x24, true);
			memory::write<bool>(global_info::process_handle, glow_manager + entity_glow_index + 0x25, false);
		}
	}

	return 0;
}
