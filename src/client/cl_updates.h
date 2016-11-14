#ifndef CL_UPDATES
#define CL_UPDATES

#ifdef __cplusplus
extern "C" {
#endif

void CL_GetLatestRelease();
void ExecuteInstaller(const char* path);

#ifdef __cplusplus
}
#endif

#endif
