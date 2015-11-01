//////////////////////////////////////////////////////////////////////
// ImageProcess.h: 类ImageProcess的接口
//////////////////////////////////////////////////////////////////////

#ifndef __CImageProcess_H
#define __CImageProcess_H

#include <math.h>
#include "Dib.h"


class CImageProcess : public CObject  
{
public:
	CImageProcess();
	CImageProcess(RGBQUAD **source,int width,int height);
	virtual ~CImageProcess();
public:
	void CalLightCompensate();
	void CalMedianFilter();
	bool CalBinary();
	bool CalMorphologicalFilter();
	bool CalSobel();
	int GetMaxLikelyHood(int ClusterNum,int DataNum,int *CoordinatorData);
	void SetFakeFace(int ClusterNUm,int DataNum,int *CoordinatorData,bool*Data);
public:

	int	m_nWidth;	//图像宽度
	int m_nHeight;	//图像高度
	RGBQUAD **m_pSourceData;	//原始数据
	double  **m_pLikeliHoodArray;	//相似度数组（二维）
	BYTE    **m_pBinaryArray;	//二值化数组（二维）

	bool m_bBinaryReady;	//标志位，判断是否完成二值化
	RGBQUAD **LightConpensate;
	RGBQUAD **MedianFilter;
	int **m_pGrayData;
private:
	bool m_bLikeliHoodReady;	//标志位，判断是否计算好相似度
};

#endif // #ifndef __IMGAEPROCESS_H
