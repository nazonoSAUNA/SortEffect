
#include <windows.h>
#include <filter.h>

#include <stdio.h>


FILTER_DLL filter_dll = {
    FILTER_FLAG_ALWAYS_ACTIVE | FILTER_FLAG_NO_CONFIG,
    NULL,NULL,
    const_cast<char*>("エフェクト並び替え"),
    NULL,NULL,NULL,
    NULL,NULL,
    NULL,NULL,NULL,
    NULL,
    func_init
};
EXTERN_C FILTER_DLL __declspec(dllexport)* __stdcall GetFilterTable() {
    return &filter_dll;
}


DWORD init_exeditfp(FILTER* fp) {
	SYS_INFO si;
	fp->exfunc->get_sys_info(NULL, &si);
    
	for (int i = 0; i < si.filter_n; i++) {
		FILTER* efp = (FILTER*)fp->exfunc->get_filterp(i);
        if (efp->information != NULL) {
            if (!lstrcmpA(efp->information, "拡張編集(exedit) version 0.92 by ＫＥＮくん")) {
                return (DWORD)efp->dll_hinst;
            }
        }
	}
    return 0;
}


DWORD exedit_dll_hinst;
BOOL exedit_ReplaceCall(DWORD exedit_address, void* new_address) {
    DWORD oldProtect;
    DWORD* address = (DWORD*)(exedit_address + exedit_dll_hinst);
    if (!VirtualProtect(address, 4, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }
    *address = (DWORD)new_address - (DWORD)address - 4;
    return VirtualProtect(address, 4, oldProtect, &oldProtect);
}



void fputsn(char const* _Buffer, FILE* _Stream) {
    fputs(_Buffer, _Stream);
    fputc('\n', _Stream);
}

#define GROUP_CHAR '/'
#define FILE_LINE_MAX_LEN 256

const char setting_name[] = "_Setting.txt";
const char others[] = "その他";
char path[MAX_PATH + 1];
char last_group_name[FILE_LINE_MAX_LEN + 1];

FILTER** LoadedFilterTable;
int* ExeditFilterCount_ptr;
char* add_effect_str;
char* basic_effect_str;
HMENU* hMenu_Dialog;
HMENU* hMenu_rClick;

inline static HMENU(__cdecl* AddExeditMediaMenuItem)(HMENU hmenu, LPCSTR name, BOOL flag); // 39650
inline static void(__cdecl* AddExeditMediaMenu)(HMENU menu, UINT_PTR uIDNewItem, LPCSTR lpNewItem, char* group_name); // 39970
inline static void(__cdecl* AddExeditMenu)(HMENU menu, int uIDNewItem, int flag); // 39a10
inline static BOOL(__cdecl* ReadAlias)(char* file_name, char* folder_name); // 39cf0
inline static char* (__cdecl* GetExaPath)(char* file_name, char* folder_name); // 39ed0
inline static int(__cdecl* GetExaType)(char* path); // 2a2e0
inline static int(__cdecl* GetAliasUID)(char* file_name, char* folder_name, int uIDItemOffset); // 283f0


int get_effect_idx(char* name) {
    for (int i = 0; i < *ExeditFilterCount_ptr; i++) {
        FILTER* efp = LoadedFilterTable[i];
        int efp_flag = efp->flag;
        if ((efp_flag & 0x52000a0) == 0x4000020 && (efp_flag & 0x38) || (efp_flag & 0x5200120) == 0x4000020) {
            if (lstrcmpA(efp->name, name) == 0) {
                return i;
            }
        }
    }
    return -1;
}

BOOL is_alias_effect(char* file_name, char* folder_name){
    return (GetExaType(GetExaPath (file_name, folder_name)) & 0x1e) == 2;
}

void write_default_eff_list() {
    FILE* inifile;
    if (fopen_s(&inifile, path, "w") == 0) {
        BOOL basic_eff_complete = FALSE;
        for (int i = 0; i < *ExeditFilterCount_ptr; i++) {
            FILTER* efp = LoadedFilterTable[i];
            int efp_flag = efp->flag;
            if ((efp_flag & 0x52000a0) == 0x4000020 && (efp_flag & 0x38) || (efp_flag & 0x5200120) == 0x4000020) {
                if (efp_flag & 0x8000) {
                    if (!basic_eff_complete) {
                        fputc(GROUP_CHAR, inifile);
                        fputsn(basic_effect_str, inifile);
                        for (int j = i; j < *ExeditFilterCount_ptr; j++) {
                            FILTER* efp = LoadedFilterTable[j];
                            int efp_flag = efp->flag;
                            if ((efp_flag & 0x52080a0) == 0x4008020 && (efp_flag & 0x38) || (efp_flag & 0x5208120) == 0x4008020) {
                                fputsn(efp->name, inifile);
                            }
                        }
                        fputc(GROUP_CHAR, inifile);
                        fputc('\n', inifile);
                        basic_eff_complete = TRUE;
                    }
                } else {
                    fputsn(efp->name, inifile);
                }
            }
        }

        fclose(inifile);
    }
}



void AddExeditMediaMenu_wrap39e57_0(HMENU menu, UINT_PTR uIDNewItem, LPCSTR lpNewItem, char* group_name) {
    FILE* inifile;
    if (fopen_s(&inifile, path, "a") == 0) {

        if (group_name != NULL && *group_name != '\0') {
            if (lstrcmpA(last_group_name, group_name) != 0) {
                lstrcpyA(last_group_name, group_name);
                fputc(GROUP_CHAR, inifile);
                fputsn(group_name, inifile);
            }
        } else {
            if (*last_group_name != '\0') {
                *last_group_name = '\0';
                fputc(GROUP_CHAR, inifile);
                fputc('\n', inifile);
            }
        }
        fputs(last_group_name, inifile);
        fputc('\\', inifile);
        fputsn(lpNewItem, inifile);

        fclose(inifile);
    }
    AddExeditMediaMenu(menu, uIDNewItem, lpNewItem, group_name);
}

void AddExeditMediaMenu_wrap39e57_1(HMENU menu, UINT_PTR uIDNewItem, LPCSTR lpNewItem, char* group_name) {
    FILE* inifile;
    if (fopen_s(&inifile, path, "a") == 0) {
        if (lstrcmpA(last_group_name, others) != 0) {
            lstrcpyA(last_group_name, others);
            fputc(GROUP_CHAR, inifile);
            fputsn(others, inifile);
        }

        if (group_name != NULL && *group_name != '\0') {
            fputs(group_name, inifile);
        }
        fputc('\\', inifile);
        fputsn(lpNewItem, inifile);

        fclose(inifile);
    }
    AddExeditMediaMenu(menu, uIDNewItem, lpNewItem, (char*)others);
}


void set_last_group(char* str) {
    for (int i = 0; i < FILE_LINE_MAX_LEN; i++) {
        if (IsDBCSLeadByteEx(0, str[i])) {
            last_group_name[i] = str[i];
            i++;
        } else {
            if (str[i] == '\n') {
                last_group_name[i] = '\0';
                break;
            }
        }
        last_group_name[i] = str[i];

    }
}

int split_file_folder(char* str) {
    int file_name_ptr = 0;
    char* slashptr = &last_group_name[FILE_LINE_MAX_LEN - 1];
    for (int i = 0; i < FILE_LINE_MAX_LEN; i++) {
        if (IsDBCSLeadByteEx(0, str[i])) {
            i++;
        } else if (str[i] == '\n') {
                str[i] = '\0';
                break;
        } else if (str[i] == '\\' || str[i] == '/') {
            if (file_name_ptr) {
                return -1;
            }
            str[i] = '\0';
            file_name_ptr = i + 1;

        }
    }
    return file_name_ptr;
}

void add_menu_0(HMENU menu, int menu_uid, char* group, char* name) {
    if (group[0] != '\0') {
        menu = AddExeditMediaMenuItem(menu, add_effect_str, 0);
        menu = AddExeditMediaMenuItem(menu, group, 0);
        AddExeditMediaMenu(menu, menu_uid, name, NULL);
    } else {
        AddExeditMediaMenu(menu, menu_uid, name, add_effect_str);
    }
}

void add_menu_2(HMENU menu, int menu_uid, char* group, char* name) {
    if (group[0] != '\0') {
        menu = AddExeditMediaMenuItem(menu, group, 0);
    }
    AddExeditMediaMenu(menu, menu_uid, name, NULL);
}

void __cdecl AddExeditMenu_0(HMENU menu, int uIDNewItem, int flag) {
    FILE* inifile;
    if (fopen_s(&inifile, path, "r") == 0) {

        DeleteMenu(menu, 100, 0);

        BOOL filter_called[128];
        for (int i = 0; i < 128; i++) {
            filter_called[i] = FALSE;
        }

        char str[FILE_LINE_MAX_LEN + 1];
        last_group_name[0] = last_group_name[FILE_LINE_MAX_LEN - 1] = '\0';



        while (fgets(str, FILE_LINE_MAX_LEN, inifile)) {
            if (str[0] == GROUP_CHAR) {
                set_last_group(&str[1]);
            } else if (str[0] != '\n') {
                int filename_ptr = split_file_folder(str);
                char* folder_name;
                if (filename_ptr == 0) {
                    int filter_idx = get_effect_idx(str);
                    if (0 <= filter_idx) { // エフェクトが存在する
                        FILTER* efp = LoadedFilterTable[filter_idx];
                        int efp_flag = efp->flag;

                        if ((efp_flag & 0x38) && (efp_flag & 80) == 0) {
                            add_menu_0(menu, uIDNewItem + filter_idx, last_group_name, efp->name);
                            filter_called[filter_idx] = TRUE;
                            continue;
                        }
                    }
                    folder_name = &last_group_name[FILE_LINE_MAX_LEN - 1];
                } else {
                    folder_name = str;
                }
                if (0 <= filename_ptr) { // exaフォルダは1階層まで
                    char* file_name = &str[filename_ptr];
                    if (is_alias_effect(file_name, folder_name)) {
                        int alias_menu_id = abs(GetAliasUID(file_name, folder_name, 3000));
                        if (alias_menu_id) {
                            add_menu_0(menu, alias_menu_id, last_group_name, file_name);
                        }
                    }
                }
            }
        }
        fclose(inifile);

        BOOL file_a_open = (fopen_s(&inifile, path, "a") == 0);
        char* group_name_ptr;
        for (int i = 0; i < *ExeditFilterCount_ptr; i++) {
            if (!filter_called[i]) {
                FILTER* efp = LoadedFilterTable[i];
                int efp_flag = efp->flag;

                if ((efp_flag & 0x5000080) == 0x4000000 && (efp_flag & 0x38)) {
                    if ((efp_flag & 0x20)==0) {
                        AddExeditMediaMenu(menu, uIDNewItem + i, efp->name, (char*)((int)efp + 128));
                    } else if ((efp_flag & 0x200000) == 0) {
                        if (file_a_open) {
                            if (lstrcmpA(last_group_name, others)) {
                                lstrcpyA(last_group_name, others);
                                fputc(GROUP_CHAR, inifile);
                                fputsn(others, inifile);
                            }
                            fputsn(efp->name, inifile);
                        }
                        add_menu_0(menu, uIDNewItem + i, (char*)others, efp->name);
                    }
                }
            }
        }
        if (file_a_open) {
            fclose(inifile);
        }
    } else {
        AddExeditMenu(menu, uIDNewItem, flag);
    }
}


void __cdecl AddExeditMenu_2(HMENU menu, int uIDNewItem, int flag) {
    FILE* inifile;
    if (fopen_s(&inifile, path, "r") == 0) {

        DeleteMenu(menu, 100, 0);

        BOOL filter_called[128];
        for (int i = 0; i < 128; i++) {
            filter_called[i] = FALSE;
        }

        char str[FILE_LINE_MAX_LEN + 1];
        last_group_name[0] = last_group_name[FILE_LINE_MAX_LEN - 1] = '\0';

        while (fgets(str, FILE_LINE_MAX_LEN, inifile)) {
            if (str[0] == GROUP_CHAR) {
                set_last_group(&str[1]);
            } else if (str[0] != '\n') {
                int filename_ptr = split_file_folder(str);
                char* folder_name;
                if (filename_ptr == 0) {
                    int filter_idx = get_effect_idx(str);
                    if (0 <= filter_idx) { // エフェクトが存在する
                        FILTER* efp = LoadedFilterTable[filter_idx];
                        filter_called[filter_idx] = TRUE;
                        if ((efp->flag & 0x100) == 0) {
                            add_menu_2(menu, uIDNewItem + filter_idx, last_group_name, efp->name);
                        }
                        continue;
                    }
                    folder_name = &last_group_name[FILE_LINE_MAX_LEN - 1];
                } else {
                    folder_name = str;
                }
                if (0 <= filename_ptr) { // exaフォルダは1階層まで
                    char* file_name = &str[filename_ptr];
                    if (is_alias_effect(file_name, folder_name)) {
                        int alias_menu_id = abs(GetAliasUID(file_name, folder_name, 3000));
                        if (alias_menu_id) {
                            add_menu_2(menu, alias_menu_id, last_group_name, file_name);
                        }
                    }
                }
            }
        }
        fclose(inifile);

        for (int i = 0; i < *ExeditFilterCount_ptr; i++) {
            if (!filter_called[i]) {
                FILTER* efp = LoadedFilterTable[i];
                if ((efp->flag & 0x5200120) == 0x4000020) {
                    add_menu_2(menu, uIDNewItem + i, (char*)others, efp->name);
                }
            }
        }
    } else {
        AddExeditMenu(menu, uIDNewItem, flag);
    }
}

BOOL __cdecl ReadAlias_wrap(char* file_name, char* folder_name) {
    int alias_menu_id = GetAliasUID(file_name, folder_name, 3000);
    if (alias_menu_id <= 0) {
        return FALSE;
    }
    if (is_alias_effect(file_name, folder_name)) {
        FILE* inifile;
        if (fopen_s(&inifile, path, "a") == 0) {
            if (lstrcmpA(last_group_name, others)) {
                lstrcpyA(last_group_name, others);
                fputc(GROUP_CHAR, inifile);
                fputsn(others, inifile);
            }
            if (folder_name != NULL && folder_name[0] != '\0') {
                fputs(folder_name, inifile);
            }
            fputc('\\', inifile);
            fputsn(file_name, inifile);
            fclose(inifile);
        }
        HMENU menu = GetSubMenu(GetSubMenu(*hMenu_rClick, 0), 0);
        add_menu_0(menu, alias_menu_id, (char*)others, file_name);
        menu = GetSubMenu(GetSubMenu(*hMenu_Dialog, 0), 0);
        add_menu_2(menu, alias_menu_id, (char*)others, file_name);
        return TRUE;
    }
    return ReadAlias(file_name, folder_name);
}




BOOL func_init(FILTER* fp) {

    // aufのパスを取得→設定ファイルのパスに変更　パスが250バイトを超えるプラグインは読み込まれないようなので"_Setting"を追加しても260バイトを超えない
    GetModuleFileNameA(fp->dll_hinst, path, MAX_PATH);
    lstrcpyA(&path[lstrlenA(path) - 4], setting_name);


    exedit_dll_hinst = init_exeditfp(fp);
    if (exedit_dll_hinst == 0) {
        MessageBoxA(fp->hwnd, "拡張編集0.92が見つかりませんでした", fp->name, MB_OK);
        return TRUE;
    }

    LoadedFilterTable = (FILTER**)(exedit_dll_hinst + 0x187c98);
    ExeditFilterCount_ptr = (int*)(exedit_dll_hinst + 0x146248);
    add_effect_str = (char*)(exedit_dll_hinst + 0xa4d30);
    basic_effect_str = (char*)(exedit_dll_hinst + 0xa4d44);
    hMenu_Dialog = (HMENU*)(exedit_dll_hinst + 0x158d2c);
    hMenu_rClick = (HMENU*)(exedit_dll_hinst + 0x168598);


    AddExeditMediaMenuItem = reinterpret_cast<decltype(AddExeditMediaMenuItem)>(exedit_dll_hinst + 0x39650);
    AddExeditMediaMenu = reinterpret_cast<decltype(AddExeditMediaMenu)>(exedit_dll_hinst + 0x39970);
    AddExeditMenu = reinterpret_cast<decltype(AddExeditMenu)>(exedit_dll_hinst + 0x39a10);
    ReadAlias = reinterpret_cast<decltype(ReadAlias)>(exedit_dll_hinst + 0x39cf0);
    GetExaPath = reinterpret_cast<decltype(GetExaPath )>(exedit_dll_hinst + 0x39ed0);
    GetExaType = reinterpret_cast<decltype(GetExaType)>(exedit_dll_hinst + 0x2a2e0);
    GetAliasUID  = reinterpret_cast<decltype(GetAliasUID)>(exedit_dll_hinst + 0x283f0);
    

    last_group_name[0] = '\0';
    FILE* inifile;
    if (fopen_s(&inifile, path, "r+") == 0) {

        // 最後の文字が改行でなければ追加する
        fseek(inifile, -1, SEEK_END);
        if (fgetc(inifile) != '\n') {
            fputc('\n', inifile);
        }
        fclose(inifile);

        exedit_ReplaceCall(0x4485f, &AddExeditMenu_0); // R_CLICK MENU
        exedit_ReplaceCall(0x4489f, &AddExeditMenu_2); // DIALOG MENU

        exedit_ReplaceCall(0x44a0e, &ReadAlias_wrap); // folder
        exedit_ReplaceCall(0x44a6d, &ReadAlias_wrap); // root

        exedit_ReplaceCall(0x39e57, &AddExeditMediaMenu_wrap39e57_1);
    } else {
        write_default_eff_list();

        exedit_ReplaceCall(0x39e57, &AddExeditMediaMenu_wrap39e57_0);
    }
    
	return TRUE;
}
