#include <windows.h>
#include <initguid.h>
#include <virtdisk.h>
#include <stdio.h>
#include <string.h>

int main() {
    VIRTUAL_STORAGE_TYPE isoImageType;
    memset(&isoImageType, 0, sizeof(isoImageType));
    isoImageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_ISO;
    isoImageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;

    ATTACH_VIRTUAL_DISK_PARAMETERS attachParameters;
    memset(&attachParameters, 0, sizeof(attachParameters));
    attachParameters.Version = ATTACH_VIRTUAL_DISK_VERSION_1;

    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_1;
    
    HANDLE h;

    FILE *file = fopen("grubenv", "r");
    if (file == NULL) {
        return 1;
    }

    char line[1024];
    wchar_t isoPath[MAX_PATH] = { 0 };
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "iso_path=", 9) == 0) {
            // Using mbstowcs_s instead of mbstowcs
            size_t len = strlen(line + 9);
            mbstowcs_s(NULL, isoPath, MAX_PATH, line + 9, len);
            break;
        }
    }
    fclose(file);

    if (wcslen(isoPath) == 0) {
        return 2;
    } else {
        for (int i = 0; isoPath[i] != L'\0'; i++) {
            if (isoPath[i] == L'/') {
                isoPath[i] = L'\\';
            }
        }
    }

    for (char driveLetter = 'A'; driveLetter <= 'Z'; ++driveLetter) {
        wchar_t drivePath[MAX_PATH];
        swprintf(drivePath, sizeof(drivePath) / sizeof(wchar_t), L"%c:\\", driveLetter);
        wchar_t fullIsoPath[MAX_PATH];
        swprintf(fullIsoPath, sizeof(fullIsoPath) / sizeof(wchar_t), L"%s%s", drivePath, isoPath);

        HANDLE fileHandle = CreateFile(fullIsoPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (fileHandle != INVALID_HANDLE_VALUE) {
            if (OpenVirtualDisk(&isoImageType, fullIsoPath, VIRTUAL_DISK_ACCESS_READ, OPEN_VIRTUAL_DISK_FLAG_NONE, &openParameters, &h) == ERROR_SUCCESS) {
                if (AttachVirtualDisk(h, NULL, ATTACH_VIRTUAL_DISK_FLAG_READ_ONLY | ATTACH_VIRTUAL_DISK_FLAG_PERMANENT_LIFETIME, 0, &attachParameters, NULL) == ERROR_SUCCESS) {
                    return 0;
                }
            }
        }
    }

    return 3;
}
