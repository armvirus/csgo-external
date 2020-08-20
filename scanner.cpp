#include "stdafx.hpp"

scanner::scanner(HANDLE process_handle, std::size_t image_process_id, std::uintptr_t image_module_base)
{
	this->process_handle = process_handle;
	this->module_base = image_module_base;
	this->process_id = image_process_id;
}

std::uint32_t scanner::get_process_size()
{
	IMAGE_DOS_HEADER dos = memory::read<IMAGE_DOS_HEADER>(this->process_handle, this->module_base);
	IMAGE_NT_HEADERS nt = memory::read<IMAGE_NT_HEADERS>(this->process_handle, this->module_base + dos.e_lfanew);

	return nt.OptionalHeader.SizeOfImage;
}

void scanner::free_process(std::uintptr_t buffer_address)
{
	VirtualFree(reinterpret_cast<LPVOID>(buffer_address), this->process_size, MEM_FREE);
}

PIMAGE_NT_HEADERS scanner::get_nt_headers(std::uintptr_t buffer)
{
	return reinterpret_cast<PIMAGE_NT_HEADERS>(buffer + reinterpret_cast<PIMAGE_DOS_HEADER>(buffer)->e_lfanew);
}

std::tuple<std::uint32_t, std::uintptr_t> scanner::copy_process()
{
	this->process_size = this->get_process_size();

	BYTE* allocated_buffer = reinterpret_cast<BYTE*>(VirtualAlloc(0, this->process_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

	ReadProcessMemory(this->process_handle, reinterpret_cast<LPCVOID>(this->module_base), (LPVOID)allocated_buffer, this->process_size, 0);

	if (reinterpret_cast<PIMAGE_DOS_HEADER>(allocated_buffer)->e_magic != IMAGE_DOS_SIGNATURE)
		return { 0, 0 };

	return { this->process_size, reinterpret_cast<std::uintptr_t>(allocated_buffer) };
}