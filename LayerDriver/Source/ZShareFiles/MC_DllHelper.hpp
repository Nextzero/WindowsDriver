//--------------------------------------------------
#ifndef MC_DLLHELPER_H
#define MC_DLLHELPER_H
//--------------------------------------------------
#include <Windows.h>
//--------------------------------------------------
#define Kinkoo_InterfaceExport(_X_)                                                         \
    extern "C" __declspec(dllexport) bool __cdecl Kinkoo_GetInterface(void** IPtr)          \
    {                                                                                       \
        if(0 == IPtr)   return false;                                                       \
        *IPtr   = new _X_;                                                                  \
        return true;                                                                        \
    }																						\
	extern "C" __declspec(dllexport) bool __cdecl Kinkoo_FreeInterface(void* IPtr)         	\
	{                                                                                       \
        if(0 == IPtr)   return false;                                                       \
        delete (_X_*)IPtr;																	\
        return true;                                                                        \
    }																						\
//--------------------------------------------------
typedef bool (__cdecl *FuncKinkoo_GetInterface)(void** IPtr);
typedef bool (__cdecl *FuncKinkoo_FreeInterface)(void* IPtr);
//--------------------------------------------------
template<class Interface_Type>
class mc_dllheper
{
public:
    mc_dllheper();
    
    bool    load    (char* vDllName);
    bool    free    (void);
    
    Interface_Type *operator->() const
    {
        return m_Interface;
    }
    
    Interface_Type*     m_Interface;
    HMODULE             m_Instance;
    bool                m_Avaliable;
};
//--------------------------------------------------
template<class Interface_Type>
mc_dllheper<Interface_Type>::mc_dllheper()
{
    m_Interface     = 0;
    m_Instance      = 0;
    m_Avaliable     = false;
}
//--------------------------------------------------
template<class Interface_Type>
bool mc_dllheper<Interface_Type>::load(char *vDllName)
{
    if(true == m_Avaliable)
        return true;        

	m_Instance = LoadLibraryA(vDllName);
	if(0 == m_Instance)
	{
		return false;
	}

	FuncKinkoo_GetInterface tFunc = (FuncKinkoo_GetInterface)GetProcAddress(m_Instance, "Kinkoo_GetInterface");
	if(0 == tFunc)
	{
		FreeLibrary(m_Instance);
		return false;
	}

	tFunc((void**)&m_Interface);
    if(0 == m_Interface)
    {
        FreeLibrary(m_Instance);
        return false;
    }

    m_Avaliable = true;
    return true;    
}
//--------------------------------------------------
template<class Interface_Type>
bool mc_dllheper<Interface_Type>::free(void)
{
    if(false == m_Avaliable)
        return true;
	
    FuncKinkoo_FreeInterface tFunc = (FuncKinkoo_FreeInterface)GetProcAddress(m_Instance, "Kinkoo_FreeInterface");
	tFunc(m_Interface);
	
    if(0 != m_Instance)
        FreeLibrary(m_Instance);
        
    m_Interface     = 0;
    m_Instance      = 0;
    m_Avaliable     = false;
    return true;
}
//--------------------------------------------------
#endif
