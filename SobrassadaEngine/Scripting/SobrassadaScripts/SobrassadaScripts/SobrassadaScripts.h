// The following ifdef block shows the standard way to create macros that make exporting a DLL easier. 
// All files in this DLL are compiled with the SOBRASSADASCRIPTS_EXPORTS symbol defined on the command line.
// This symbol should not be defined in any project that uses this DLL. 
// This way, other projects whose source code files include the file will interpret the SOBRASSADASCRIPTS_API functions as imported from a DLL,
// while this DLL interprets the symbols defined in this macro as exported.
#ifdef SOBRASSADASCRIPTS_EXPORTS
#define SOBRASSADA_API __declspec(dllexport)
#else
#define SOBRASSADA_API __declspec(dllimport)
#endif

// Exported class from DLL
class SOBRASSADA_API CSobrassadaScripts
{
  public:
	CSobrassadaScripts(void);
	// You can add here more functions
};

extern SOBRASSADA_API int nSobrassadaScripts;

SOBRASSADA_API int fnSobrassadaScripts(void);
