#include "stdafx.hpp"

namespace memory
{
	std::uint32_t find_process_id(std::string process_name)
	{
        PROCESSENTRY32 process_info;
        process_info.dwSize = sizeof(process_info);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

        Process32First(snapshot, &process_info);
        if (!process_name.compare(process_info.szExeFile))
        {
            CloseHandle(snapshot);
            return process_info.th32ProcessID;
        }

        while (Process32Next(snapshot, &process_info))
        {
            if (!process_name.compare(process_info.szExeFile))
            {
                CloseHandle(snapshot);
                return process_info.th32ProcessID;
            }
        }

        CloseHandle(snapshot);
        return 0;
	}

	std::uintptr_t get_module_base(std::uint32_t process_id, std::string module_name)
	{
		MODULEENTRY32 module_entry;
		module_entry.dwSize = sizeof(module_entry);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);

		Module32First(snapshot, &module_entry);
		if (!module_name.compare((module_entry.szModule)))
		{
		    CloseHandle(snapshot);
		    return reinterpret_cast<std::uintptr_t>(module_entry.modBaseAddr);
		}

		while (Module32Next(snapshot, &module_entry))
		{
		    if (!module_name.compare(module_entry.szModule))
		    {
			CloseHandle(snapshot);
			return reinterpret_cast<std::uintptr_t>(module_entry.modBaseAddr);
		    }
		}

		CloseHandle(snapshot);
		return 0;
	}

	std::uintptr_t signature_scan(std::uintptr_t module_start, std::uintptr_t module_size, const char* signature)
	{
		static auto converter = [](const char* pattern)
		{
			std::vector<int>bytes = {};
			char* start = const_cast<char*>(pattern);
			char* end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current)
			{
				if (*current == '?')
				{
					++current;
					if (*current == '?')
						++current;
					bytes.push_back(-1);
				}
				else { bytes.push_back(strtoul(current, &current, 16)); }
			}
			return bytes;
		};

		std::vector<int>pattern_bytes = converter(signature);
		std::uint8_t* const scan_bytes = reinterpret_cast<std::uint8_t*>(module_start);

		std::size_t size = pattern_bytes.size();
		int* data = pattern_bytes.data();

		for (auto i = 0ul; i < module_size - size; ++i)
		{
			bool found = true;
			for (auto j = 0ul; j < size; ++j)
			{
				if (scan_bytes[i + j] != data[j] && data[j] != -1)
				{
					found = false;
					break;
				}
			}
			if (found)
			{
				return reinterpret_cast<std::uintptr_t>(&scan_bytes[i] - module_start);
			}
		}

		return 0;
	}
}
