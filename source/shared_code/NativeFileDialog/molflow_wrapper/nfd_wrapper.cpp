#include "nfd_wrapper.h"
#include "../nfd.h"
std::string NFD_OpenFile_Cpp(const std::string& fileFilters,const std::string& path) {
	std::string resultStr;
	const char* filters = NULL; if (!fileFilters.empty()) filters = fileFilters.c_str();
	const char* defaultPath = NULL; if (!path.empty()) defaultPath = path.c_str();
	nfdchar_t* fn;
	nfdresult_t result = NFD_OpenDialog(filters, defaultPath, &fn);
	if (result == NFD_OKAY) {
		resultStr = fn;
		free(fn);
	}
	return resultStr;
}

std::string NFD_SaveFile_Cpp(const std::string& fileFilters,const std::string& path) {
	std::string resultStr;
	const char* filters = NULL; if (!fileFilters.empty()) filters = fileFilters.c_str();
	const char* defaultPath = NULL; if (!path.empty()) defaultPath = path.c_str();
	nfdchar_t* fn;
	nfdresult_t result = NFD_SaveDialog(filters, defaultPath, &fn);
	if (result == NFD_OKAY) {
		resultStr = fn;
		free(fn);
	}
	return resultStr;
}

std::vector<std::string> NFD_OpenMultiple_Cpp(const std::string& fileFilters,const std::string& path){
	std::vector<std::string> paths;
	const char* filters = NULL; if (!fileFilters.empty()) filters = fileFilters.c_str();
	const char* defaultPath = NULL; if (!path.empty()) defaultPath = path.c_str();
	nfdpathset_t *outPaths=NULL;
	nfdresult_t  result = NFD_OpenDialogMultiple(filters, defaultPath, outPaths );
	if (result == NFD_OKAY) {
		size_t nb = NFD_PathSet_GetCount(outPaths);
		for (size_t i=0;i<nb;i++) {
			paths.push_back(NFD_PathSet_GetPath( outPaths, i ));
		}
		NFD_PathSet_Free(outPaths);
	}
	return paths;	
}