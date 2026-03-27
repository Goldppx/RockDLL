#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")

#define INI_NAME "config.ini"

static char g_base_dir[MAX_PATH];
static char g_ini_path[MAX_PATH];
static char g_target[MAX_PATH];
static char g_dll_path[MAX_PATH];

BOOL is_admin(void)
{
    PSID sid = NULL;
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&sia, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &sid))
    {
        BOOL result;
        CheckTokenMembership(NULL, sid, &result);
        FreeSid(sid);
        return result;
    }
    return FALSE;
}

DWORD find_process(const char* name)
{
    HANDLE s = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (s == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 p;
    p.dwSize = sizeof(p);
    DWORD pid = 0;

    if (Process32First(s, &p))
    {
        do
        {
            if (_stricmp(p.szExeFile, name) == 0)
            {
                pid = p.th32ProcessID;
                break;
            }
        } while (Process32Next(s, &p));
    }

    CloseHandle(s);
    return pid;
}

const char* get_file_name(const char* path)
{
    const char* last_slash = strrchr(path, '\\');
    if (last_slash)
        return last_slash + 1;
    return path;
}

BOOL inject(DWORD pid, const char* dll_path)
{
    DWORD dwRights = PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ;
    HANDLE p = OpenProcess(dwRights, FALSE, pid);
    if (!p)
        return FALSE;

    size_t l = strlen(dll_path) + 1;
    LPVOID m = VirtualAllocEx(p, NULL, l, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!m)
    {
        CloseHandle(p);
        return FALSE;
    }

    WriteProcessMemory(p, m, dll_path, l, NULL);

    HANDLE t = CreateRemoteThread(p, NULL, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
        m, 0, NULL);

    if (t)
    {
        WaitForSingleObject(t, 5000);
        CloseHandle(t);
    }

    VirtualFreeEx(p, m, 0, MEM_RELEASE);
    CloseHandle(p);

    return t != NULL;
}

int main(void)
{
    printf("\n[RockDLL]\n\n");

    if (!is_admin())
    {
        char szPath[MAX_PATH];
        printf("Requesting admin...\n");

        if (GetModuleFileNameA(NULL, szPath, MAX_PATH))
        {
            ShellExecuteA(NULL, "runas", szPath, NULL, NULL, SW_NORMAL);
        }
        return 0;
    }

    GetModuleFileNameA(NULL, g_base_dir, MAX_PATH);
    char* s = strrchr(g_base_dir, '\\');
    if (s)
        *s = '\0';

    snprintf(g_ini_path, MAX_PATH, "%s\\%s", g_base_dir, INI_NAME);

    GetPrivateProfileStringA("INJECT", "target", "", g_target, MAX_PATH, g_ini_path);
    GetPrivateProfileStringA("INJECT", "dlls", "", g_dll_path, MAX_PATH, g_ini_path);

    if (strlen(g_target) == 0 || strlen(g_dll_path) == 0)
    {
        printf("Error: target or dlls not found in config.ini\n");
        return 1;
    }

    printf("Waiting for %s...\n", get_file_name(g_target));

    DWORD pid = 0;
    while ((pid = find_process(get_file_name(g_target))) == 0)
    {
        Sleep(100);
    }

    printf("Found (PID: %lu)\n", pid);

    if (inject(pid, g_dll_path))
    {
        printf("OK\n");
    }
    else
    {
        printf("FAILED\n");
    }

    return 0;
}
