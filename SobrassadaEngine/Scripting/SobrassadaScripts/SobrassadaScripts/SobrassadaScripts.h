// El siguiente bloque ifdef muestra la forma estándar de crear macros que hacen la exportación
// de un DLL más sencillo. Todos los archivos de este archivo DLL se compilan con SOBRASSADASCRIPTS_EXPORTS
// símbolo definido en la línea de comandos. Este símbolo no debe definirse en ningún proyecto
// que use este archivo DLL. De este modo, otros proyectos cuyos archivos de código fuente incluyan el archivo verán
// interpretan que las funciones SOBRASSADASCRIPTS_API se importan de un archivo DLL, mientras que este archivo DLL interpreta los símbolos
// definidos en esta macro como si fueran exportados.
#ifdef SOBRASSADASCRIPTS_EXPORTS
#define SOBRASSADASCRIPTS_API __declspec(dllexport)
#else
#define SOBRASSADASCRIPTS_API __declspec(dllimport)
#endif

// Clase exportada del DLL
class SOBRASSADASCRIPTS_API CSobrassadaScripts {
public:
	CSobrassadaScripts(void);
	// TODO: agregar métodos aquí.
};

extern SOBRASSADASCRIPTS_API int nSobrassadaScripts;

SOBRASSADASCRIPTS_API int fnSobrassadaScripts(void);
