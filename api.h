#ifndef GEXMAYA_API_H
#define GEXMAYA_API_H

#ifdef GEX_MAYA_IMPORT
#define GEX_MAYA __declspec(dllimport)
#else
#define GEX_MAYA __declspec(dllexport)
#endif


#ifndef PLUGIN_EXPORT
#define PLUGIN_EXPORT __declspec(dllexport)
#endif

#endif //GEXMAYA_API_H
