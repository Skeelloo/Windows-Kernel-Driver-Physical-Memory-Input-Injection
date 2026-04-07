#include <ntifs.h>
#include <windef.h>
#include <ntimage.h>
#include <cstdint>
#include <intrin.h>
#include "defines.h"
#include "spoof.h"
#include <stdio.h>
#include <stdarg.h>
#include "includes.hpp"
#include "memory.hpp"
#include "utility.hpp"
#include <ntstrsafe.h>

#pragma once

#define message(...) ((void)0)

typedef struct _MOUSE_REQUEST {
    ULONG key;
    LONG x;
    LONG y;
    USHORT button_flags;
} MOUSE_REQUEST, * PMOUSE_REQUEST;

// Mouse class service callback for kernel-level mouse input injection
#ifndef MOUSE_MOVE_RELATIVE
#define MOUSE_MOVE_RELATIVE 0x00
#endif
typedef struct _MOUSE_INPUT_DATA {
    USHORT UnitId;
    USHORT Indicators;
    USHORT Flags;
    USHORT ButtonFlags;
    USHORT ButtonData;
    ULONG  RawButtons;
    LONG   LastX;
    LONG   LastY;
    ULONG  ExtraInformation;
} MOUSE_INPUT_DATA, *PMOUSE_INPUT_DATA;

typedef VOID (*MouseClassServiceCallbackFn)(PDEVICE_OBJECT, PMOUSE_INPUT_DATA, PMOUSE_INPUT_DATA, PULONG);
static MouseClassServiceCallbackFn g_mouse_callback = nullptr;
static PDEVICE_OBJECT g_mouse_device = nullptr;

NTSTATUS init_mouse_callback() {
    if (g_mouse_callback) return STATUS_SUCCESS;

    UNICODE_STRING class_name;
    RtlInitUnicodeString(&class_name, L"\\Driver\\MouClass");

    PDRIVER_OBJECT mouse_driver = nullptr;
    NTSTATUS status = ObReferenceObjectByName(&class_name, OBJ_CASE_INSENSITIVE, nullptr, 0,
        *IoDriverObjectType, KernelMode, nullptr, (PVOID*)&mouse_driver);
    if (!NT_SUCCESS(status) || !mouse_driver) return STATUS_NOT_FOUND;

    PDEVICE_OBJECT device = mouse_driver->DeviceObject;
    while (device && !device->NextDevice)
        device = device->NextDevice;
    if (!device) device = mouse_driver->DeviceObject;

    if (device && device->DeviceExtension) {
        g_mouse_device = device;
    }

    ObDereferenceObject(mouse_driver);
    return device ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

void mouse_move(LONG dx, LONG dy, USHORT button_flags) {
    if (!g_mouse_device) {
        init_mouse_callback();
        if (!g_mouse_device) return;
    }

    MOUSE_INPUT_DATA mid = {};
    mid.Flags = MOUSE_MOVE_RELATIVE;
    mid.LastX = dx;
    mid.LastY = dy;
    mid.ButtonFlags = button_flags;
    mid.UnitId = 1;

    // Direct callback via MouClass device extension — no IRP needed
    PVOID* ext = (PVOID*)g_mouse_device->DeviceExtension;
    if (ext) {
        ULONG consumed = 0;
        PMOUSE_INPUT_DATA end = &mid + 1;
        MouseClassServiceCallbackFn callback = (MouseClassServiceCallbackFn)ext[1];
        if (callback && MmIsAddressValid((PVOID)callback)) {
            callback(g_mouse_device, &mid, end, &consumed);
        }
    }
}

namespace mousemove
{
    PDEVICE_OBJECT g_device_object = nullptr;
    UNICODE_STRING g_device_name, g_symbolic_link;

    bool get_offsets()
    {
        message("Checking offsets for Windows build: %d\n", globals::u_ver_build);

        // Windows 11 23H2+ (build 22631+)
        if (globals::u_ver_build >= 22631)
        {
            globals::offsets::i_image_file_name = 0x5a8;
            globals::offsets::i_active_process_links = 0x448;
            globals::offsets::i_active_threads = 0x5f0;
            message("Using offsets for Windows 11 23H2+ (build %d)\n", globals::u_ver_build);
            return true;
        }
        // Windows 11 22H2 (build 22621)
        else if (globals::u_ver_build >= 22621)
        {
            globals::offsets::i_image_file_name = 0x5a8;
            globals::offsets::i_active_process_links = 0x448;
            globals::offsets::i_active_threads = 0x5f0;
            message("Using offsets for Windows 11 22H2\n");
            return true;
        }
        // Windows 11 21H2 (build 22000)
        else if (globals::u_ver_build >= 22000)
        {
            globals::offsets::i_image_file_name = 0x5a8;
            globals::offsets::i_active_process_links = 0x448;
            globals::offsets::i_active_threads = 0x5f0;
            message("Using offsets for Windows 11 21H2\n");
            return true;
        }
        // Windows 10 22H2 (build 19045)
        else if (globals::u_ver_build == 19045)
        {
            globals::offsets::i_image_file_name = 0x5a8;
            globals::offsets::i_active_process_links = 0x448;
            globals::offsets::i_active_threads = 0x5f0;
            message("Using offsets for Windows 10 22H2\n");
            return true;
        }
        // Windows 10 21H2/21H1 (build 19044/19043)
        else if (globals::u_ver_build >= 19043 && globals::u_ver_build <= 19044)
        {
            globals::offsets::i_image_file_name = 0x5a8;
            globals::offsets::i_active_process_links = 0x448;
            globals::offsets::i_active_threads = 0x5f0;
            message("Using offsets for Windows 10 21H1/21H2\n");
            return true;
        }
        // Windows 10 20H2 (build 19042)
        else if (globals::u_ver_build == 19042)
        {
            globals::offsets::i_image_file_name = 0x5a8;
            globals::offsets::i_active_process_links = 0x448;
            globals::offsets::i_active_threads = 0x5f0;
            message("Using offsets for Windows 10 20H2\n");
            return true;
        }
        // Windows 10 2004 (build 19041)
        else if (globals::u_ver_build == 19041)
        {
            globals::offsets::i_image_file_name = 0x5a8;
            globals::offsets::i_active_process_links = 0x448;
            globals::offsets::i_active_threads = 0x5f0;
            message("Using offsets for Windows 10 2004\n");
            return true;
        }
        // Windows 10 1909 (build 18363)
        else if (globals::u_ver_build == 18363)
        {
            globals::offsets::i_image_file_name = 0x450;
            globals::offsets::i_active_process_links = 0x2f0;
            globals::offsets::i_active_threads = 0x498;
            message("Using offsets for Windows 10 1909\n");
            return true;
        }
        // Windows 10 1903 (build 18362)
        else if (globals::u_ver_build == 18362)
        {
            globals::offsets::i_image_file_name = 0x450;
            globals::offsets::i_active_process_links = 0x2f0;
            globals::offsets::i_active_threads = 0x498;
            message("Using offsets for Windows 10 1903\n");
            return true;
        }
        // Windows 10 1809 (build 17763)
        else if (globals::u_ver_build == 17763)
        {
            globals::offsets::i_image_file_name = 0x450;
            globals::offsets::i_active_process_links = 0x2f0;
            globals::offsets::i_active_threads = 0x498;
            message("Using offsets for Windows 10 1809\n");
            return true;
        }
        // Windows 10 1803 (build 17134)
        else if (globals::u_ver_build == 17134)
        {
            globals::offsets::i_image_file_name = 0x450;
            globals::offsets::i_active_process_links = 0x2f0;
            globals::offsets::i_active_threads = 0x498;
            message("Using offsets for Windows 10 1803\n");
            return true;
        }
        // Fallback pour versions plus récentes
        else if (globals::u_ver_build > 22631)
        {
            message("WARNING: Unknown build %d, using latest known offsets\n", globals::u_ver_build);
            globals::offsets::i_image_file_name = 0x5a8;
            globals::offsets::i_active_process_links = 0x448;
            globals::offsets::i_active_threads = 0x5f0;
            return true;
        }

        message("ERROR: Unsupported Windows build number: %d\n", globals::u_ver_build);
        return false;
    }

    bool b_version_check()
    {
        OSVERSIONINFOW v_info{};
        v_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);

        NTSTATUS status = RtlGetVersion(&v_info);
        if (!NT_SUCCESS(status))
        {
            message("RtlGetVersion failed with status: 0x%X\n", status);
            return false;
        }

        message("Detected Windows version: %d.%d Build %d\n",
            v_info.dwMajorVersion,
            v_info.dwMinorVersion,
            v_info.dwBuildNumber);

        if (v_info.dwMajorVersion < 10)
        {
            message("Unsupported Windows version (needs Windows 10+)\n");
            return false;
        }

        globals::u_ver_major = v_info.dwMajorVersion;
        globals::u_ver_build = v_info.dwBuildNumber;

        if (!get_offsets())
        {
            message("No offsets available for this Windows build\n");
            return false;
        }

        return true;
    }

    NTSTATUS unsupported_dispatch(PDEVICE_OBJECT device_object, PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);

        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return irp->IoStatus.Status;
    }
}

UNICODE_STRING name, link;

typedef struct _SYSTEM_BIGPOOL_ENTRY {
    PVOID VirtualAddress;
    ULONG_PTR NonPaged : 1;
    ULONG_PTR SizeInBytes;
    UCHAR Tag[4];
} SYSTEM_BIGPOOL_ENTRY, * PSYSTEM_BIGPOOL_ENTRY;

typedef struct _SYSTEM_BIGPOOL_INFORMATION {
    ULONG Count;
    SYSTEM_BIGPOOL_ENTRY AllocatedInfo[1];
} SYSTEM_BIGPOOL_INFORMATION, * PSYSTEM_BIGPOOL_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBigPoolInformation = 0x42,
} SYSTEM_INFORMATION_CLASS;

extern "C" POBJECT_TYPE* IoDriverObjectType;
POBJECT_TYPE* IoDriverObjectType;

extern "C" NTSTATUS NTAPI IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
extern "C" PVOID NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);
extern "C" NTSTATUS NTAPI ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS systemInformationClass, PVOID systemInformation, ULONG systemInformationLength, PULONG returnLength);

#define CODE_RW CTL_CODE(FILE_DEVICE_UNKNOWN, 0x47536, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CODE_BA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x36236, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CODE_GET_GUARDED_REGION CTL_CODE(FILE_DEVICE_UNKNOWN, 0x13437, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CODE_GET_DIR_BASE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x13438, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IOCTL_MOUSE_MOVE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x27336, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define CODE_GET_SESSION_KEY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x13439, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
// Per-session security key — generated at driver init, replaces static 0x457c1d6
volatile INT32 g_session_security_key = 0;

// EPROCESS cache — avoids PsLookupProcessByProcessId on every read/write
static PEPROCESS g_cached_process = NULL;
static INT32 g_cached_pid = 0;

PEPROCESS get_cached_process(INT32 pid) {
    if (pid == g_cached_pid && g_cached_process != NULL) {
        // Validate cached process is still alive by checking ActiveThreads > 0
        ULONG active_threads = *(PULONG)((PUCHAR)g_cached_process + globals::offsets::i_active_threads);
        if (active_threads > 0) {
            return g_cached_process;
        }
        ObDereferenceObject(g_cached_process);
        g_cached_process = NULL;
        g_cached_pid = 0;
    }

    if (g_cached_process != NULL) {
        ObDereferenceObject(g_cached_process);
        g_cached_process = NULL;
        g_cached_pid = 0;
    }

    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &process);
    if (NT_SUCCESS(status) && process) {
        g_cached_process = process;
        g_cached_pid = pid;
        return process;
    }
    return NULL;
}

#define WIN_1803 17134
#define WIN_1809 17763
#define WIN_1903 18362
#define WIN_1909 18363
#define WIN_2004 19041
#define WIN_20H2 19569
#define WIN_21H1 20180
#define WIN_22H2 19045

#define PAGE_OFFSET_SIZE 12
static const UINT64 PMASK = (~0xfull << 8) & 0xfffffffffull;

typedef struct _RW {
    INT32 security;
    INT32 process_id;
    ULONGLONG address;
    ULONGLONG buffer;
    ULONGLONG size;
    BOOLEAN write;
} RW, * PRW;

typedef struct _BA {
    INT32 security;
    INT32 process_id;
    ULONGLONG* address;
} BA, * PBA;

typedef struct _GA {
    INT32 security;
    ULONGLONG* address;
} GA, * PGA;

typedef struct _MEMORY_OPERATION_DATA {
    uint32_t pid;
    uintptr_t cr3;
} MEMORY_OPERATION_DATA, * PMEMORY_OPERATION_DATA;

NTSTATUS(*DynamicMmCopyMemory)(PVOID, MM_COPY_ADDRESS, SIZE_T, ULONG, PSIZE_T) = NULL;
PVOID(*DynamicMmMapIoSpaceEx)(PHYSICAL_ADDRESS, SIZE_T, ULONG) = NULL;
VOID(*DynamicMmUnmapIoSpace)(PVOID, SIZE_T) = NULL;

__forceinline ULONG RandomTag() {
    static const ULONG tags[] = {
        'FMic', 'CcPf', 'MmLd', 'NtfF', 'IoSb',
        'CmKb', 'AlPC', 'ObNm', 'SeLs', 'ExWn',
        'SmTr', 'VfDv', 'WmIp', 'PfTl', 'KsAu'
    };
    return tags[(__rdtsc() >> 4) % (sizeof(tags) / sizeof(tags[0]))];
}

NTSTATUS load_dynamic_functions() {
    message("Loading dynamic functions...\n");

    UNICODE_STRING funcName;

    RtlInitUnicodeString(&funcName, L"MmCopyMemory");
    DynamicMmCopyMemory = (PVOID(*)(PVOID, MM_COPY_ADDRESS, SIZE_T, ULONG, PSIZE_T))MmGetSystemRoutineAddress(&funcName);
    if (!DynamicMmCopyMemory) {
        message("ERROR: Failed to get MmCopyMemory\n");
        return STATUS_UNSUCCESSFUL;
    }

    RtlInitUnicodeString(&funcName, L"MmMapIoSpaceEx");
    DynamicMmMapIoSpaceEx = (PVOID(*)(PHYSICAL_ADDRESS, SIZE_T, ULONG))MmGetSystemRoutineAddress(&funcName);
    if (!DynamicMmMapIoSpaceEx) {
        message("ERROR: Failed to get MmMapIoSpaceEx\n");
        return STATUS_UNSUCCESSFUL;
    }

    RtlInitUnicodeString(&funcName, L"MmUnmapIoSpace");
    DynamicMmUnmapIoSpace = (VOID(*)(PVOID, SIZE_T))MmGetSystemRoutineAddress(&funcName);
    if (!DynamicMmUnmapIoSpace) {
        message("ERROR: Failed to get MmUnmapIoSpace\n");
        return STATUS_UNSUCCESSFUL;
    }

    message("Dynamic functions loaded successfully\n");
    return STATUS_SUCCESS;
}

NTSTATUS read(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_read) {
    SPOOF_FUNC;
    MM_COPY_ADDRESS to_read = { 0 };
    to_read.PhysicalAddress.QuadPart = (LONGLONG)target_address;

    SIZE_T bytes_copied = 0;
    NTSTATUS result = DynamicMmCopyMemory(buffer, to_read, size, MM_COPY_MEMORY_PHYSICAL, &bytes_copied);

    if (bytes_read) {
        *bytes_read = bytes_copied;
    }

    return (NT_SUCCESS(result) && bytes_copied == size) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS write(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_written) {
    SPOOF_FUNC;
    if (!target_address || !buffer || !bytes_written)
        return STATUS_INVALID_PARAMETER;

    PHYSICAL_ADDRESS AddrToWrite = { 0 };
    AddrToWrite.QuadPart = (LONGLONG)(target_address);

    PVOID pmapped_mem = DynamicMmMapIoSpaceEx(AddrToWrite, size, PAGE_READWRITE);

    if (!pmapped_mem)
        return STATUS_UNSUCCESSFUL;
    unsigned char* dest = (unsigned char*)pmapped_mem;
    unsigned char* src = (unsigned char*)buffer;

    for (SIZE_T i = 0; i < size; i++) {
        dest[i] = src[i];
    }

    *bytes_written = size;

    DynamicMmUnmapIoSpace(pmapped_mem, size);

    return STATUS_SUCCESS;
}

INT32 get_winver() {
    RTL_OSVERSIONINFOW ver = { 0 };
    RtlGetVersion(&ver);
    switch (ver.dwBuildNumber)
    {
    case WIN_1803:
        return 0x0278;
    case WIN_1809:
        return 0x0278;
    case WIN_1903:
        return 0x0280;
    case WIN_1909:
        return 0x0280;
    case WIN_2004:
        return 0x0388;
    case WIN_20H2:
        return 0x0388;
    case WIN_21H1:
        return 0x0388;
    case WIN_22H2:
        return 0x0388;
    default:
        return 0x0388;
    }
}

volatile uint64_t g_MmPfnDatabase = 0;
volatile uint64_t g_PXE_BASE = 0;
volatile uint64_t g_idx = 0;
uintptr_t dirBase = 0;

void initDefinesCR3() {
    KDDEBUGGER_DATA64 kdBlock = { 0 };
    CONTEXT context = { 0 };
    context.ContextFlags = CONTEXT_FULL;
    (RtlCaptureContext)(&context);

    PDUMP_HEADER dumpHeader = (PDUMP_HEADER)ExAllocatePoolWithTag(NonPagedPool, DUMP_BLOCK_SIZE, RandomTag());
    if (dumpHeader) {
        (KeCapturePersistentThreadState)(&context, NULL, 0, 0, 0, 0, 0, dumpHeader);
        RtlCopyMemory(&kdBlock, (PUCHAR)dumpHeader + KDDEBUGGER_DATA_OFFSET, sizeof(kdBlock));

        ExFreePool(dumpHeader);

        g_MmPfnDatabase = *(ULONG64*)(kdBlock.MmPfnDatabase);

        ULONG64 g_PTE_BASE = kdBlock.PteBase;
        ULONG64 g_PDE_BASE = g_PTE_BASE + ((g_PTE_BASE & 0xffffffffffff) >> 9);
        ULONG64 g_PPE_BASE = g_PTE_BASE + ((g_PDE_BASE & 0xffffffffffff) >> 9);
        g_PXE_BASE = g_PTE_BASE + ((g_PPE_BASE & 0xffffffffffff) >> 9);
        g_idx = (g_PTE_BASE >> 39) - 0x1FFFE00;
    }
}

uintptr_t get_kernel_base() {
    const auto idtbase = *reinterpret_cast<uint64_t*>(__readgsqword(0x18) + 0x38);
    const auto descriptor_0 = *reinterpret_cast<uint64_t*>(idtbase);
    const auto descriptor_1 = *reinterpret_cast<uint64_t*>(idtbase + 8);
    const auto isr_base = ((descriptor_0 >> 32) & 0xFFFF0000) + (descriptor_0 & 0xFFFF) + (descriptor_1 << 32);
    auto align_base = isr_base & 0xFFFFFFFFFFFFF000;

    for (; ; align_base -= 0x1000) {
        for (auto* search_base = reinterpret_cast<uint8_t*>(align_base); search_base < reinterpret_cast<uint8_t*>(align_base) + 0xFF9; search_base++) {
            if (search_base[0] == 0x48 &&
                search_base[1] == 0x8D &&
                search_base[2] == 0x1D &&
                search_base[6] == 0xFF) {
                const auto relative_offset = *reinterpret_cast<int*>(&search_base[3]);
                const auto address = reinterpret_cast<uint64_t>(search_base + relative_offset + 7);
                if ((address & 0xFFF) == 0) {
                    if (*reinterpret_cast<uint16_t*>(address) == 0x5A4D) {
                        return address;
                    }
                }
            }
        }
    }
}

intptr_t search_pattern(void* module_handle, const char* section, const char* signature_value) {
    static auto in_range = [](auto x, auto a, auto b) { return (x >= a && x <= b); };
    static auto get_bits = [](auto  x) { return (in_range((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xa) : (in_range(x, '0', '9') ? x - '0' : 0)); };
    static auto get_byte = [](auto  x) { return (get_bits(x[0]) << 4 | get_bits(x[1])); };

    const auto dos_headers = reinterpret_cast<PIMAGE_DOS_HEADER>(module_handle);
    const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(module_handle) + dos_headers->e_lfanew);
    const auto section_headers = reinterpret_cast<PIMAGE_SECTION_HEADER>(nt_headers + 1);

    auto range_start = 0ui64;
    auto range_end = 0ui64;
    for (auto cur_section = section_headers; cur_section < section_headers + nt_headers->FileHeader.NumberOfSections; cur_section++) {
        if (strcmp(reinterpret_cast<const char*>(cur_section->Name), section) == 0) {
            range_start = reinterpret_cast<uintptr_t>(module_handle) + cur_section->VirtualAddress;
            range_end = range_start + cur_section->Misc.VirtualSize;
        }
    }

    if (range_start == 0)
        return 0u;

    auto first_match = 0ui64;
    auto pat = signature_value;
    for (uintptr_t cur = range_start; cur < range_end; cur++) {
        if (*pat == '\0') {
            return first_match;
        }
        if (*(uint8_t*)pat == '\?' || *reinterpret_cast<uint8_t*>(cur) == get_byte(pat)) {
            if (!first_match)
                first_match = cur;

            if (!pat[2])
                return first_match;

            if (*(uint16_t*)pat == 16191 || *(uint8_t*)pat != '\?') {
                pat += 3;
            }
            else {
                pat += 2;
            }
        }
        else {
            pat = signature_value;
            first_match = 0;
        }
    }
    return 0u;
}

#pragma warning(push)
#pragma warning(disable:4201)

typedef union {
    struct {
        uint64_t reserved1 : 3;
        uint64_t page_level_write_through : 1;
        uint64_t page_level_cache_disable : 1;
        uint64_t reserved2 : 7;
        uint64_t address_of_page_directory : 36;
        uint64_t reserved3 : 16;
    };
    uint64_t flags;
} cr3;
static_assert(sizeof(cr3) == 0x8);

typedef union {
    struct {
        uint64_t present : 1;
        uint64_t write : 1;
        uint64_t supervisor : 1;
        uint64_t page_level_write_through : 1;
        uint64_t page_level_cache_disable : 1;
        uint64_t accessed : 1;
        uint64_t dirty : 1;
        uint64_t large_page : 1;
        uint64_t global : 1;
        uint64_t ignored_1 : 2;
        uint64_t restart : 1;
        uint64_t page_frame_number : 36;
        uint64_t reserved1 : 4;
        uint64_t ignored_2 : 7;
        uint64_t protection_key : 4;
        uint64_t execute_disable : 1;
    };

    uint64_t flags;
} pt_entry_64;
static_assert(sizeof(pt_entry_64) == 0x8);
#pragma warning(pop)

static uint64_t pte_base = 0;
static uint64_t pde_base = 0;
static uint64_t ppe_base = 0;
static uint64_t pxe_base = 0;
static uint64_t self_mapidx = 0;
static uint64_t mm_pfn_database = 0;

uint64_t get_dirbase() {
    return __readcr3() & 0xFFFFFFFFFFFFF000;
}

void* phys_to_virt(uint64_t phys) {
    PHYSICAL_ADDRESS phys_addr = { .QuadPart = (int64_t)(phys) };
    return reinterpret_cast<void*>(MmGetVirtualForPhysical(phys_addr));
}

void init_pte_base() {
    cr3 system_cr3 = { .flags = get_dirbase() };
    uint64_t dirbase_phys = system_cr3.address_of_page_directory << 12;
    pt_entry_64* pt_entry = reinterpret_cast<pt_entry_64*>(phys_to_virt(dirbase_phys));
    for (uint64_t idx = 0; idx < 0x200; idx++) {
        if (pt_entry[idx].page_frame_number == system_cr3.address_of_page_directory) {
            pte_base = (idx + 0x1FFFE00ui64) << 39ui64;
            pde_base = (idx << 30ui64) + pte_base;
            ppe_base = (idx << 30ui64) + pte_base + (idx << 21ui64);
            pxe_base = (idx << 12ui64) + ppe_base;
            self_mapidx = idx;
            break;
        }
    }
}

uintptr_t init_mmpfn_database() {
    auto search = search_pattern(reinterpret_cast<void*>(get_kernel_base()), ".text", "B9 ? ? ? ? 48 8B 05 ? ? ? ? 48 89 43 18") + 5;
    auto resolved_base = search + *reinterpret_cast<int32_t*>(search + 3) + 7;
    mm_pfn_database = *reinterpret_cast<uintptr_t*>(resolved_base);
    return mm_pfn_database;
}

NTSTATUS get_process_cr3(PMEMORY_OPERATION_DATA x) {
    message("get_process_cr3: Looking for CR3 of process %d\n", x->pid);

    if (!pte_base) {
        message("get_process_cr3: Initializing PTE base\n");
        init_pte_base();
    }

    if (!mm_pfn_database) {
        message("get_process_cr3: Initializing MmPfn database\n");
        init_mmpfn_database();
    }

    PEPROCESS target_process = get_cached_process(x->pid);
    if (!target_process) {
        message("get_process_cr3: Failed to lookup process\n");
        return STATUS_UNSUCCESSFUL;
    }

    auto mem_range = MmGetPhysicalMemoryRanges();
    auto cr3_ptebase = self_mapidx * 8 + pxe_base;

    for (int mem_range_count = 0; mem_range_count < 200; ++mem_range_count)
    {
        if (mem_range[mem_range_count].BaseAddress.QuadPart == 0 &&
            mem_range[mem_range_count].NumberOfBytes.QuadPart == 0)
            break;

        auto start_pfn = mem_range[mem_range_count].BaseAddress.QuadPart >> 12;
        auto end_pfn = start_pfn + (mem_range[mem_range_count].NumberOfBytes.QuadPart >> 12);

        for (auto i = start_pfn; i < end_pfn; ++i)
        {
            auto cur_mmpfn = reinterpret_cast<_MMPFN*>(mm_pfn_database + 0x30 * i);

            if (cur_mmpfn->flags)
            {
                if (cur_mmpfn->flags == 1)
                    continue;

                if (cur_mmpfn->pte_address != cr3_ptebase)
                    continue;

                auto decrypted_eprocess = ((cur_mmpfn->flags | 0xF000000000000000) >> 0xd) | 0xFFFF000000000000;

                if (MmIsAddressValid(reinterpret_cast<void*>(decrypted_eprocess)) &&
                    reinterpret_cast<PEPROCESS>(decrypted_eprocess) == target_process)
                {
                    dirBase = i << 12;

                    if (dirBase) {
                        message("get_process_cr3: Found CR3 = 0x%llx for process %d\n", dirBase, x->pid);
                        RtlCopyMemory((void*)x->cr3, &dirBase, sizeof(dirBase));
                        return STATUS_SUCCESS;
                    }
                }
            }
        }
    }

    message("get_process_cr3: Using fallback method\n");
    dirBase = *(PULONGLONG)((PUCHAR)target_process + 0x28); // DirectoryTableBase offset

    if (dirBase) {
        message("get_process_cr3: Fallback found CR3 = 0x%llx\n", dirBase);
        RtlCopyMemory((void*)x->cr3, &dirBase, sizeof(dirBase));
        return STATUS_SUCCESS;
    }

    message("get_process_cr3: Failed to find CR3\n");
    return STATUS_UNSUCCESSFUL;
}

struct cache {
    uintptr_t Address;
    UINT64 Value;
};

static cache cached_pml4e[512];

UINT64 read_cached(UINT64 address, cache* cached_entry, SIZE_T* readsize) {
    if (cached_entry->Address == address) {
        return cached_entry->Value;
    }

    read(PVOID(address), &cached_entry->Value, sizeof(cached_entry->Value), readsize);

    cached_entry->Address = address;

    return cached_entry->Value;
}

UINT64 translate_linear(UINT64 directoryTableBase, UINT64 virtualAddress) {
    directoryTableBase &= ~0xf;

    UINT64 pageOffset = virtualAddress & ((1ULL << PAGE_OFFSET_SIZE) - 1);
    UINT64 pte = (virtualAddress >> 12) & 0x1ff;
    UINT64 pt = (virtualAddress >> 21) & 0x1ff;
    UINT64 pd = (virtualAddress >> 30) & 0x1ff;
    UINT64 pdp = (virtualAddress >> 39) & 0x1ff;

    SIZE_T readsize = 0;
    UINT64 pdpe = 0;

    pdpe = read_cached(directoryTableBase + 8 * pdp, &cached_pml4e[pdp], &readsize);
    if ((pdpe & 1) == 0)
        return 0;

    UINT64 pde = 0;

    read(PVOID((pdpe & PMASK) + 8 * pd), &pde, sizeof(pde), &readsize);
    if ((pde & 1) == 0)
        return 0;

    if (pde & 0x80) {
        return (pde & PMASK) + (virtualAddress & ((1ULL << 30) - 1));
    }

    UINT64 pteAddr = 0;

    read(PVOID((pde & PMASK) + 8 * pt), &pteAddr, sizeof(pteAddr), &readsize);
    if ((pteAddr & 1) == 0)
        return 0;

    if (pteAddr & 0x80) {
        return (pteAddr & PMASK) + (virtualAddress & ((1ULL << 21) - 1));
    }

    UINT64 finalAddr = 0;

    read(PVOID((pteAddr & PMASK) + 8 * pte), &finalAddr, sizeof(finalAddr), &readsize);
    finalAddr &= PMASK;

    if (finalAddr == 0)
        return 0;

    return finalAddr + pageOffset;
}

ULONG64 find_min(INT32 g, SIZE_T f) {
    INT32 h = (INT32)f;
    ULONG64 result = 0;

    result = (((g) < (h)) ? (g) : (h));

    return result;
}

NTSTATUS frw(PRW x) {
    NTSTATUS status = STATUS_SUCCESS;
    if (x->security != g_session_security_key) {
        message("frw: Invalid security code\n");
        return STATUS_UNSUCCESSFUL;
    }

    if (!x->process_id) {
        message("frw: Invalid process ID\n");
        return STATUS_UNSUCCESSFUL;
    }

    PEPROCESS process = get_cached_process(x->process_id);
    if (!process) {
        message("frw: Failed to lookup process %d\n", x->process_id);
        return STATUS_UNSUCCESSFUL;
    }

    ULONGLONG process_dirbase = 0;

    if (dirBase != 0) {
        process_dirbase = dirBase;
    }
    else {
        process_dirbase = *(PULONGLONG)((PUCHAR)process + 0x28);
    }

    if (!process_dirbase) {
        message("frw: Failed to get process dirbase\n");
        return STATUS_UNSUCCESSFUL;
    }

    SIZE_T total_size = x->size;
    SIZE_T bytes_processed = 0;

    while (bytes_processed < total_size) {
        ULONG64 current_address = (ULONG64)x->address + bytes_processed;

        INT64 physical_address = translate_linear(process_dirbase, current_address);

        if (!physical_address) {
            message("frw: Failed to translate address 0x%llx\n", current_address);
            return STATUS_UNSUCCESSFUL;
        }

        ULONG64 bytes_in_page = PAGE_SIZE - (physical_address & 0xFFF);
        ULONG64 bytes_to_process = min(bytes_in_page, total_size - bytes_processed);

        SIZE_T bytes_transferred = 0;

        if (x->write) {
            status = write(
                PVOID(physical_address),
                PVOID((ULONG64)x->buffer + bytes_processed),
                bytes_to_process,
                &bytes_transferred
            );
        }
        else {
            status = read(
                PVOID(physical_address),
                PVOID((ULONG64)x->buffer + bytes_processed),
                bytes_to_process,
                &bytes_transferred
            );
        }

        if (!NT_SUCCESS(status)) {
            message("frw: Read/Write failed at 0x%llx\n", current_address);
            return status;
        }

        bytes_processed += bytes_transferred;

        if (bytes_transferred < bytes_to_process) {
            break;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS fba(PBA x) {
    ULONGLONG image_base = NULL;
    if (x->security != g_session_security_key)
        return STATUS_UNSUCCESSFUL;

    if (!x->process_id)
        return STATUS_UNSUCCESSFUL;

    PEPROCESS process = get_cached_process(x->process_id);
    if (!process) {
        return STATUS_UNSUCCESSFUL;
    }

    image_base = (ULONGLONG)PsGetProcessSectionBaseAddress(process);

    if (!image_base) {
        return STATUS_UNSUCCESSFUL;
    }

    RtlCopyMemory(x->address, &image_base, sizeof(image_base));

    return STATUS_SUCCESS;
}

NTSTATUS fget_guarded_region(PGA x) {
    if (x->security != g_session_security_key)
        return STATUS_UNSUCCESSFUL;

    ULONG infoLen = 0;
    NTSTATUS status = ZwQuerySystemInformation(SystemBigPoolInformation, &infoLen, 0, &infoLen);
    PSYSTEM_BIGPOOL_INFORMATION pPoolInfo = 0;

    while (status == STATUS_INFO_LENGTH_MISMATCH)
    {
        if (pPoolInfo)
            ExFreePool(pPoolInfo);

        pPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, infoLen, RandomTag());
        status = ZwQuerySystemInformation(SystemBigPoolInformation, pPoolInfo, infoLen, &infoLen);
    }

    if (pPoolInfo)
    {
        for (unsigned int i = 0; i < pPoolInfo->Count; i++)
        {
            SYSTEM_BIGPOOL_ENTRY* Entry = &pPoolInfo->AllocatedInfo[i];
            PVOID VirtualAddress;
            VirtualAddress = (PVOID)((uintptr_t)Entry->VirtualAddress & ~1ull);
            SIZE_T SizeInBytes = Entry->SizeInBytes;
            BOOLEAN NonPaged = Entry->NonPaged;

            if (Entry->NonPaged && Entry->SizeInBytes == 0x200000) {
                UCHAR expectedTag[] = "TnoC";
                if (memcmp(Entry->Tag, expectedTag, sizeof(expectedTag)) == 0) {
                    RtlCopyMemory((void*)x->address, &Entry->VirtualAddress, sizeof(Entry->VirtualAddress));
                    return STATUS_SUCCESS;
                }
            }

        }

        ExFreePool(pPoolInfo);
    }

    return STATUS_SUCCESS;
}

NTSTATUS io_controller(PDEVICE_OBJECT device_obj, PIRP irp) {
    message("IRP received in io_controller\n");
    UNREFERENCED_PARAMETER(device_obj);

    NTSTATUS status = STATUS_SUCCESS;
    ULONG bytes = 0;

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
    ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
    ULONG size = stack->Parameters.DeviceIoControl.InputBufferLength;

    message("IOCTL Code: 0x%X, Size: %d\n", code, size);

    if (code == CODE_RW) {
        if (size == sizeof(RW)) {
            PRW req = (PRW)(irp->AssociatedIrp.SystemBuffer);
            status = frw(req);
            bytes = sizeof(RW);
        }
        else {
            status = STATUS_INFO_LENGTH_MISMATCH;
            bytes = 0;
        }
    }
    else if (code == CODE_BA) {
        if (size == sizeof(BA)) {
            PBA req = (PBA)(irp->AssociatedIrp.SystemBuffer);
            status = fba(req);
            bytes = sizeof(BA);
        }
        else {
            status = STATUS_INFO_LENGTH_MISMATCH;
            bytes = 0;
        }
    }
    else if (code == CODE_GET_GUARDED_REGION) {
        if (size == sizeof(GA)) {
            PGA req = (PGA)(irp->AssociatedIrp.SystemBuffer);
            status = fget_guarded_region(req);
            bytes = sizeof(GA);
        }
        else {
            status = STATUS_INFO_LENGTH_MISMATCH;
            bytes = 0;
        }
    }
    else if (code == CODE_GET_DIR_BASE) {
        if (size == sizeof(MEMORY_OPERATION_DATA)) {
            PMEMORY_OPERATION_DATA req = (PMEMORY_OPERATION_DATA)(irp->AssociatedIrp.SystemBuffer);
            status = get_process_cr3(req);
            bytes = sizeof(MEMORY_OPERATION_DATA);
        }
        else {
            status = STATUS_INFO_LENGTH_MISMATCH;
            bytes = 0;
        }
    }
    else if (code == IOCTL_MOUSE_MOVE) {
        if (size == sizeof(MOUSE_REQUEST)) {
            PMOUSE_REQUEST req = (PMOUSE_REQUEST)(irp->AssociatedIrp.SystemBuffer);
            if (req->key == g_session_security_key) {
                mouse_move(req->x, req->y, req->button_flags);
                status = STATUS_SUCCESS;
                bytes = 0;
            }
            else {
                status = STATUS_ACCESS_DENIED;
                bytes = 0;
            }
        }
        else {
            status = STATUS_INFO_LENGTH_MISMATCH;
            bytes = 0;
        }
    }
    else if (code == CODE_GET_SESSION_KEY) {
        // Return the per-session security key to usermode
        if (irp->AssociatedIrp.SystemBuffer && stack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(INT32)) {
            RtlCopyMemory(irp->AssociatedIrp.SystemBuffer, (void*)&g_session_security_key, sizeof(INT32));
            status = STATUS_SUCCESS;
            bytes = sizeof(INT32);
        }
        else {
            status = STATUS_BUFFER_TOO_SMALL;
            bytes = 0;
        }
    }


    irp->IoStatus.Status = status;        // ← irp
    irp->IoStatus.Information = bytes;    // ← irp
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS unsupported_dispatch(PDEVICE_OBJECT device_obj, PIRP irp) {
    UNREFERENCED_PARAMETER(device_obj);

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    IoCompleteRequest(irp, IO_NO_INCREMENT);

    return irp->IoStatus.Status;
}

NTSTATUS dispatch_handler(PDEVICE_OBJECT device_obj, PIRP irp) {
    UNREFERENCED_PARAMETER(device_obj);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);

    switch (stack->MajorFunction) {
    case IRP_MJ_CREATE:
        message("IRP_MJ_CREATE received\n");
        break;
    case IRP_MJ_CLOSE:
        message("IRP_MJ_CLOSE received\n");
        break;
    default:
        break;
    }

    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return irp->IoStatus.Status;
}

void unload_drv(PDRIVER_OBJECT drv_obj) {
    message("Unloading driver...\n");
    NTSTATUS status = { };

    status = IoDeleteSymbolicLink(&link);

    if (!NT_SUCCESS(status))
        return;

    IoDeleteDevice(drv_obj->DeviceObject);
    message("Driver unloaded successfully\n");
}

NTSTATUS NTAPI IopInvalidDeviceRequest(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_DEVICE_REQUEST;
}

void InitializeIoDriverObjectType() {
    UNICODE_STRING driverTypeName;
    RtlInitUnicodeString(&driverTypeName, L"Driver");
    ObReferenceObjectByName(&driverTypeName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&IoDriverObjectType);
}

NTSTATUS LoadDriverIntoSignedMemory(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING signedDriverName;
    RtlInitUnicodeString(&signedDriverName, L"\\Driver\\IDE");

    PDRIVER_OBJECT signedDriverObject;
    NTSTATUS status = ObReferenceObjectByName(&signedDriverName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&signedDriverObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (signedDriverObject->DriverSection) {
        DriverObject->DriverSection = signedDriverObject->DriverSection;
    }
    else {
        ObDereferenceObject(signedDriverObject);
        return STATUS_UNSUCCESSFUL;
    }

    ObDereferenceObject(signedDriverObject);
    return STATUS_SUCCESS;
}

typedef struct _PIDDB_CACHE_ENTRY {
    LIST_ENTRY List;
    UNICODE_STRING DriverName;
    ULONG TimeDateStamp;
    NTSTATUS LoadStatus;
    char _pad0[16];
} PIDDB_CACHE_ENTRY, *PPIDDB_CACHE_ENTRY;

typedef struct _MM_UNLOADED_DRIVER {
    UNICODE_STRING Name;
    PVOID ModuleStart;
    PVOID ModuleEnd;
    LARGE_INTEGER UnloadTime;
} MM_UNLOADED_DRIVER, *PMM_UNLOADED_DRIVER;

typedef struct _RTL_BALANCED_NODE {
    union {
        struct _RTL_BALANCED_NODE* Children[2];
        struct {
            struct _RTL_BALANCED_NODE* Left;
            struct _RTL_BALANCED_NODE* Right;
        };
    };
    union {
        UCHAR Red : 1;
        UCHAR Balance : 2;
        ULONG_PTR ParentValue;
    };
} RTL_BALANCED_NODE, *PRTL_BALANCED_NODE;

void CleanMmUnloadedDrivers(PDRIVER_OBJECT DriverObject) {
    PLDR_DATA_TABLE_ENTRY ldrEntry = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
    if (!ldrEntry) return;

    ULONG64 ntBase = (ULONG64)get_kernel_base();
    if (!ntBase) return;

    auto mmUnloadedSig = search_pattern((void*)ntBase, "PAGE",
        "4C 8B ? ? ? ? ? 4C 8B C9 4D 85 ? 74");
    if (!mmUnloadedSig) return;

    PMM_UNLOADED_DRIVER* pMmUnloadedDrivers = (PMM_UNLOADED_DRIVER*)(
        mmUnloadedSig + *(int*)(mmUnloadedSig + 3) + 7);

    if (!MmIsAddressValid(pMmUnloadedDrivers) || !*pMmUnloadedDrivers) return;

    PMM_UNLOADED_DRIVER drivers = *pMmUnloadedDrivers;
    for (ULONG i = 0; i < 50; i++) {
        if (drivers[i].Name.Buffer &&
            MmIsAddressValid(drivers[i].Name.Buffer)) {
            RtlZeroMemory(drivers[i].Name.Buffer, drivers[i].Name.MaximumLength);
            RtlZeroMemory(&drivers[i], sizeof(MM_UNLOADED_DRIVER));
        }
    }
}

void CleanPiDDBCacheTable(PDRIVER_OBJECT DriverObject) {
    PLDR_DATA_TABLE_ENTRY ldrEntry = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
    if (!ldrEntry) return;

    ULONG64 ntBase = (ULONG64)get_kernel_base();
    if (!ntBase) return;

    auto piDDBSig = search_pattern((void*)ntBase, "PAGE",
        "48 8D 0D ? ? ? ? E8 ? ? ? ? 3D ? ? ? ? 0F 83");
    if (!piDDBSig) return;

    PRTL_AVL_TABLE piDDBTable = (PRTL_AVL_TABLE)(
        piDDBSig + *(int*)(piDDBSig + 3) + 7);

    if (!MmIsAddressValid(piDDBTable)) return;

    PPIDDB_CACHE_ENTRY cacheEntry = (PPIDDB_CACHE_ENTRY)piDDBTable->BalancedRoot.RightChild;
    if (!cacheEntry || !MmIsAddressValid(cacheEntry)) return;

    PLIST_ENTRY listHead = &cacheEntry->List;
    if (!MmIsAddressValid(listHead)) return;

    for (PLIST_ENTRY entry = listHead->Flink;
         entry && entry != listHead && MmIsAddressValid(entry);
         entry = entry->Flink) {
        PPIDDB_CACHE_ENTRY item = CONTAINING_RECORD(entry, PIDDB_CACHE_ENTRY, List);
        if (MmIsAddressValid(item) && item->DriverName.Buffer &&
            MmIsAddressValid(item->DriverName.Buffer)) {
            if (_wcsicmp(item->DriverName.Buffer, ldrEntry->BaseDllName.Buffer) == 0) {
                PLIST_ENTRY prev = entry->Blink;
                RemoveEntryList(entry);
                entry = prev;
            }
        }
    }
}

void HideDriver(PDRIVER_OBJECT DriverObject) {
    PLDR_DATA_TABLE_ENTRY entry = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
    if (!entry) {
        return;
    }

    CleanMmUnloadedDrivers(DriverObject);
    CleanPiDDBCacheTable(DriverObject);

    RemoveEntryList(&entry->InLoadOrderLinks);
    RemoveEntryList(&entry->InMemoryOrderLinks);
    RemoveEntryList(&entry->InInitializationOrderLinks);
    InitializeListHead(&entry->InLoadOrderLinks);
    InitializeListHead(&entry->InMemoryOrderLinks);
    InitializeListHead(&entry->InInitializationOrderLinks);

    entry->BaseDllName.Length = 0;
    entry->BaseDllName.MaximumLength = 0;
    if (entry->BaseDllName.Buffer) {
        RtlZeroMemory(entry->BaseDllName.Buffer, 64);
        entry->BaseDllName.Buffer = NULL;
    }
    entry->FullDllName.Length = 0;
    entry->FullDllName.MaximumLength = 0;
    if (entry->FullDllName.Buffer) {
        RtlZeroMemory(entry->FullDllName.Buffer, 256);
        entry->FullDllName.Buffer = NULL;
    }
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

__forceinline void xor_decrypt_wide(wchar_t* buf, size_t len, UCHAR key) {
    for (size_t i = 0; i < len; i++) {
        buf[i] ^= (wchar_t)(key ^ (UCHAR)(i & 0xFF));
    }
}

__forceinline void xor_encrypt_wide(wchar_t* buf, size_t len, UCHAR key) {
    xor_decrypt_wide(buf, len, key);
}

NTSTATUS initialize_driver(_In_ PDRIVER_OBJECT drv_obj, _In_ PUNICODE_STRING path) {
    UNREFERENCED_PARAMETER(path);
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT device_obj = NULL;

    static wchar_t dev_name[] = L"\\Device\\{d6579ab0-c95b-4463-9135-41gbcf16e4eg}";
    static wchar_t dev_link[] = L"\\DosDevices\\{d6579ab0-c95b-4463-9135-41gbcf16e4eg}";
    RtlInitUnicodeString(&name, dev_name);
    RtlInitUnicodeString(&link, dev_link);

    status = IoCreateDevice(drv_obj, 0, &name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_obj);
    if (!NT_SUCCESS(status)) {
        message("[-] IoCreateDevice failed: 0x%X\n", status);
        return status;
    }

    HideDriver(drv_obj);

    status = IoCreateSymbolicLink(&link, &name);
    if (!NT_SUCCESS(status)) {
        message("[-] IoCreateSymbolicLink failed: 0x%X\n", status);
        IoDeleteDevice(device_obj);
        return status;
    }

    for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        drv_obj->MajorFunction[i] = IopInvalidDeviceRequest;
    }

    drv_obj->MajorFunction[IRP_MJ_CREATE] = dispatch_handler;
    drv_obj->MajorFunction[IRP_MJ_CLOSE] = dispatch_handler;
    drv_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = io_controller;
    drv_obj->DriverUnload = unload_drv;

    device_obj->Flags |= DO_BUFFERED_IO;
    device_obj->Flags &= ~DO_DEVICE_INITIALIZING;

    status = load_dynamic_functions();
    if (!NT_SUCCESS(status)) {
        IoDeleteSymbolicLink(&link);
        IoDeleteDevice(device_obj);
        return status;
    }

    if (!mousemove::b_version_check()) {
        IoDeleteSymbolicLink(&link);
        IoDeleteDevice(device_obj);
        return STATUS_NOT_SUPPORTED;
    }

    // Generate per-session security key (unique each boot, never static in binary)
    g_session_security_key = (INT32)((__rdtsc() >> 4) ^ 0x7A3F9C1Dui64 ^ (__rdtsc() << 7));
    if (g_session_security_key == 0) g_session_security_key = (INT32)(__rdtsc() | 1);

    // Wipe PE header from driver memory to defeat memory scanners
    // EAC scans BigPool for MZ/PE signatures and compiler-generated patterns
    PVOID driverBase = drv_obj->DriverStart;
    if (driverBase && MmIsAddressValid(driverBase)) {
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)driverBase;
        ULONG headerSize = 0;
        __try {
            PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)driverBase + dosHeader->e_lfanew);
            if (MmIsAddressValid(ntHeaders)) {
                headerSize = ntHeaders->OptionalHeader.SizeOfHeaders;
                if (headerSize > 0 && headerSize < 0x2000) {
                    RtlZeroMemory(driverBase, headerSize);
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) { }
    }

    drv_obj->DriverSection = NULL;
    return STATUS_SUCCESS;
}


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    message("[+] DriverEntry called\n");

    NTSTATUS status = IoCreateDriver(NULL, initialize_driver);
    if (!NT_SUCCESS(status)) {
        message("[-] IoCreateDriver failed: 0x%X\n", status);
    }

    return status;
}