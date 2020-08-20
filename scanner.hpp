#pragma once

class scanner 
{
public:
	scanner(HANDLE process_handle, std::size_t process_id, std::uintptr_t module_base);
	void free_process(std::uintptr_t buffer_address);
	PIMAGE_NT_HEADERS get_nt_headers(std::uintptr_t buffer);
	std::tuple<std::uint32_t, std::uintptr_t>copy_process();
private:
	std::uint32_t get_process_size();

	HANDLE process_handle{};
	std::uint32_t process_size{};
	std::size_t process_id{};
	std::uintptr_t module_base{};
};

