/*-----------------------------------------------------------------------------
 *
 *  spwmath.h: SpaceWare Math Library public definitions 
 *
 *  $Id: spwmath.h 9452 2013-10-07 10:20:47Z jwick $
 *
 *-----------------------------------------------------------------------------
 *
 */

#ifndef SPW_MATH_H
#define SPW_MATH_H

#define SPW_MATH_MAJOR    3
#define SPW_MATH_MINOR    0
#define SPW_MATH_UPDATE   0
#define SPW_MATH_BUILD    7
#define SPW_MATH_VERSION  "MATH version 3.0"
#define SPW_MATH_DATE     "March 27, 1998"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EXPORT_FUNCTIONS
#define DLL_EXPORT __declspec(dllexport)
#define CALL_TYPE __stdcall
#else
#define DLL_EXPORT
#define CALL_TYPE __stdcall
#endif

/* PUBLIC VARIABLES & DEFINES */

#if !defined(__GL_GL_H__) && !defined(SPW_NO_MATRIX)
typedef float Matrix[4][4];
#endif

DLL_EXPORT void SPW_ArbitraryAxisToMatrix (Matrix, const float[3], float);
DLL_EXPORT int SPW_CompareMatrices (const Matrix, const Matrix);
DLL_EXPORT float SPW_DotProduct (const float [3], const float [3]);
DLL_EXPORT float SPW_GetMatrixScale (const Matrix);
DLL_EXPORT void SPW_InvertMatrix (Matrix, const Matrix);      
DLL_EXPORT void SPW_LookAtUpVectorToMatrix (Matrix, const float [3], const float [3], 
                                            const float [3]);
DLL_EXPORT void SPW_MatrixToArbitraryAxis (float [3], float *, const Matrix);
DLL_EXPORT float SPW_MatrixDeterminant (const Matrix);
DLL_EXPORT void SPW_MatrixToLookAtUpVector (float [3], float [3], float [3], 
                                            const Matrix);
DLL_EXPORT void SPW_Mult44x44 (Matrix, const Matrix, const Matrix);
DLL_EXPORT void SPW_MultFull14x44 (float [3], const float [3], const Matrix);
DLL_EXPORT void SPW_Mult14x44 (float [3], const float [3], const Matrix);
DLL_EXPORT void SPW_Mult33x33 (Matrix, const Matrix, const Matrix);
DLL_EXPORT void SPW_Mult13x33 (float [3], const float [3], const Matrix);
DLL_EXPORT int SPW_OrthoNormMatrix (Matrix);
DLL_EXPORT float SPW_VectorLength (const float [3]);


DLL_EXPORT void SPW_TransposeMatrix (Matrix, const Matrix);
DLL_EXPORT void SPW_CopyMatrix (Matrix, const Matrix);
DLL_EXPORT void SPW_ScaleMatrix (Matrix, const Matrix, float);
DLL_EXPORT void SPW_GetTranslationMatrix (Matrix, const Matrix);
DLL_EXPORT void SPW_InitializeMatrix (Matrix, float, float, float, float,
                                      float, float, float, float,
                                      float, float, float, float,
                                      float, float, float, float);
DLL_EXPORT void SPW_MakeIdentityMatrix (Matrix);
DLL_EXPORT void SPW_NormalizeVector (float [3], float [3]);
DLL_EXPORT void SPW_CrossProduct (float [3], const float [3], const float [3]);
DLL_EXPORT void SPW_PrintMatrix (const char *, const Matrix);
DLL_EXPORT void SPW_PrintVector (const char *, const float [3]);
DLL_EXPORT void SPW_PrintSpaceballData (const char *, const float [7]);
DLL_EXPORT void SPW_HighValuePassFilter (float *, int);

#ifdef __cplusplus
}
#endif

#endif /* SPW_MATH_H */






