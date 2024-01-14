#pragma once

#define KBD_LONG_POINTER

struct VK_TO_BIT64
{
    BYTE Vk;
    BYTE ModBits;
};

typedef struct
{
    BYTE Vk;
    BYTE ModBits;
} VK_TO_BIT, *KBD_LONG_POINTER PVK_TO_BIT;

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4200)
#endif

struct MODIFIERS64
{
    VK_TO_BIT64 *pVkToBit;     // Virtual Keys -> Mod bits
    int _align1;
    WORD       wMaxModBits;  // max Modification bit combination value
    BYTE       ModNumber[];  // Mod bits -> Modification Number
};

typedef struct
{
    PVK_TO_BIT pVkToBit;     // Virtual Keys -> Mod bits
    WORD       wMaxModBits;  // max Modification bit combination value
    BYTE       ModNumber[];  // Mod bits -> Modification Number
} MODIFIERS, *KBD_LONG_POINTER PMODIFIERS;

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#define TYPEDEF_VK_TO_WCHARS64(n) struct VK_TO_WCHARS64_##n {  \
                                    BYTE  VirtualKey;      \
                                    BYTE  Attributes;      \
                                    WCHAR wch[n];          \
                                };

TYPEDEF_VK_TO_WCHARS64(1) // VK_TO_WCHARS64_1;
TYPEDEF_VK_TO_WCHARS64(2) // VK_TO_WCHARS64_2;
TYPEDEF_VK_TO_WCHARS64(3) // VK_TO_WCHARS64_3;
TYPEDEF_VK_TO_WCHARS64(4) // VK_TO_WCHARS64_4;
TYPEDEF_VK_TO_WCHARS64(5) // VK_TO_WCHARS64_5;
TYPEDEF_VK_TO_WCHARS64(6) // VK_TO_WCHARS64_6;
TYPEDEF_VK_TO_WCHARS64(7) // VK_TO_WCHARS64_7;
// these three (8,9,10) are for FE
TYPEDEF_VK_TO_WCHARS64(8) // VK_TO_WCHARS64_8;
TYPEDEF_VK_TO_WCHARS64(9) // VK_TO_WCHARS64_9;
TYPEDEF_VK_TO_WCHARS64(10) // VK_TO_WCHARS64_10;

#define TYPEDEF_VK_TO_WCHARS(n) typedef struct _VK_TO_WCHARS##n {  \
                                    BYTE  VirtualKey;      \
                                    BYTE  Attributes;      \
                                    WCHAR wch[n];          \
                                } VK_TO_WCHARS##n, *KBD_LONG_POINTER PVK_TO_WCHARS##n;

TYPEDEF_VK_TO_WCHARS(1) // VK_TO_WCHARS1, *PVK_TO_WCHARS1;
TYPEDEF_VK_TO_WCHARS(2) // VK_TO_WCHARS2, *PVK_TO_WCHARS2;
TYPEDEF_VK_TO_WCHARS(3) // VK_TO_WCHARS3, *PVK_TO_WCHARS3;
TYPEDEF_VK_TO_WCHARS(4) // VK_TO_WCHARS4, *PVK_TO_WCHARS4;
TYPEDEF_VK_TO_WCHARS(5) // VK_TO_WCHARS5, *PVK_TO_WCHARS5;
TYPEDEF_VK_TO_WCHARS(6) // VK_TO_WCHARS6, *PVK_TO_WCHARS5;
TYPEDEF_VK_TO_WCHARS(7) // VK_TO_WCHARS7, *PVK_TO_WCHARS7;
// these three (8,9,10) are for FE
TYPEDEF_VK_TO_WCHARS(8) // VK_TO_WCHARS8, *PVK_TO_WCHARS8;
TYPEDEF_VK_TO_WCHARS(9) // VK_TO_WCHARS9, *PVK_TO_WCHARS9;
TYPEDEF_VK_TO_WCHARS(10) // VK_TO_WCHARS10, *PVK_TO_WCHARS10;

struct VK_TO_WCHAR_TABLE64
{
    VK_TO_WCHARS64_1 *pVkToWchars;
    int              _align1;
    BYTE             nModifications;
    BYTE             cbSize;
};

typedef struct _VK_TO_WCHAR_TABLE
{
    PVK_TO_WCHARS1 pVkToWchars;
    BYTE           nModifications;
    BYTE           cbSize;
} VK_TO_WCHAR_TABLE, *KBD_LONG_POINTER PVK_TO_WCHAR_TABLE;

struct DEADKEY64
{
    DWORD  dwBoth;  // diacritic & char
    WCHAR  wchComposed;
    USHORT uFlags;
};

typedef struct
{
    DWORD  dwBoth;  // diacritic & char
    WCHAR  wchComposed;
    USHORT uFlags;
} DEADKEY, *KBD_LONG_POINTER PDEADKEY;

//struct DEADKEY_WSTR64
//{
//    WCHAR *str;
//    int _align1;
//};
//typedef DEADKEY_WSTR64* DEADKEY_LPWSTR64;

struct VSC_LPWSTR64
{
    BYTE   vsc;
    int _align1;
    WCHAR *pwsz;
    int _align2;
};

typedef struct
{
    BYTE   vsc;
    WCHAR *KBD_LONG_POINTER pwsz;
} VSC_LPWSTR, *KBD_LONG_POINTER PVSC_LPWSTR;

typedef WCHAR *KBD_LONG_POINTER DEADKEY_LPWSTR;

struct WCHARARRAY64
{
    WCHAR *str;
    int _align1;
};

struct VSC_VK64
{
    BYTE Vsc;
    USHORT Vk;
};

typedef struct _VSC_VK
{
    BYTE Vsc;
    USHORT Vk;
} VSC_VK, *KBD_LONG_POINTER PVSC_VK;

#define TYPEDEF_LIGATURE64(n) struct LIGATURE64_##n {     \
                                    BYTE  VirtualKey;         \
                                    WORD  ModificationNumber; \
                                    WCHAR wch[n];             \
                                };

TYPEDEF_LIGATURE64(1) // LIGATURE64_1;
TYPEDEF_LIGATURE64(2) // LIGATURE64_2;
TYPEDEF_LIGATURE64(3) // LIGATURE64_3;
TYPEDEF_LIGATURE64(4) // LIGATURE64_4;
TYPEDEF_LIGATURE64(5) // LIGATURE64_5;

#define TYPEDEF_LIGATURE(n) typedef struct _LIGATURE##n {     \
                                    BYTE  VirtualKey;         \
                                    WORD  ModificationNumber; \
                                    WCHAR wch[n];             \
                                } LIGATURE##n, *KBD_LONG_POINTER PLIGATURE##n;

TYPEDEF_LIGATURE(1) // LIGATURE1, *PLIGATURE1;
TYPEDEF_LIGATURE(2) // LIGATURE2, *PLIGATURE2;
TYPEDEF_LIGATURE(3) // LIGATURE3, *PLIGATURE3;
TYPEDEF_LIGATURE(4) // LIGATURE4, *PLIGATURE4;
TYPEDEF_LIGATURE(5) // LIGATURE5, *PLIGATURE5;

struct KBDTABLES64
{
    MODIFIERS64 *pCharModifiers;
    int _align1;
    VK_TO_WCHAR_TABLE64 *pVkToWcharTable;
    int _align2;
    DEADKEY64 *pDeadKey;
    int _align3;
    VSC_LPWSTR64 *pKeyNames;
    int _align4;
    VSC_LPWSTR64 *pKeyNamesExt;
    int _align5;
    /* WCHAR *KBD_LONG_POINTER *KBD_LONG_POINTER */ WCHARARRAY64 *pKeyNamesDead;
    int _align6;
    USHORT *pusVSCtoVK;
    int _align7;
    BYTE bMaxVSCtoVK;
    int _align8;
    VSC_VK64 *pVSCtoVK_E0;
    int _align9;
    VSC_VK64 *pVSCtoVK_E1;
    int _align10;
    DWORD fLocaleFlags;
    byte nLgMax;
    byte cbLgEntry;
    LIGATURE64_1 *pLigature;
    int _align11;

#if (NTDDI_VERSION >= NTDDI_WINXP)
    DWORD      dwType;     // Keyboard Type
    DWORD      dwSubType;  // Keyboard SubType: may contain OemId
#endif
};

struct KBDTABLES
{
    PMODIFIERS pCharModifiers;
    PVK_TO_WCHAR_TABLE pVkToWcharTable;  // ptr to tbl of ptrs to tbl
    PDEADKEY pDeadKey;
    PVSC_LPWSTR pKeyNames;
    PVSC_LPWSTR pKeyNamesExt;
    WCHAR *KBD_LONG_POINTER *KBD_LONG_POINTER pKeyNamesDead;
    USHORT  *KBD_LONG_POINTER pusVSCtoVK;
    BYTE    bMaxVSCtoVK;
    PVSC_VK pVSCtoVK_E0;  // Scancode has E0 prefix
    PVSC_VK pVSCtoVK_E1;  // Scancode has E1 prefix
    DWORD fLocaleFlags;
    BYTE       nLgMax;
    BYTE       cbLgEntry;
    PLIGATURE1 pLigature;

#if (NTDDI_VERSION >= NTDDI_WINXP)
    DWORD      dwType;     // Keyboard Type
    DWORD      dwSubType;  // Keyboard SubType: may contain OemId
#endif
};

typedef __int64 (CALLBACK *LayerDescriptor64)(); // Should be cast to KBDTABLES64.
typedef KBDTABLES* (CALLBACK *LayerDescriptor)();
