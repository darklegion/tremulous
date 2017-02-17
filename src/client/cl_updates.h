#ifndef CL_UPDATES
#define CL_UPDATES

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#endif

void CL_GetLatestRelease();
void CL_DownloadRelease();
void CL_ExecuteInstaller();

#ifdef __cplusplus
}
#endif

#endif
