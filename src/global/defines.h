#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#elif __unix__
    #define EXPORT
#endif
