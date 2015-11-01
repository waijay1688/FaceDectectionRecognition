//////////////////////////////////////////////////////////////////////
// ImageProcess.h: ��ImageProcess�Ľӿ�
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

	int	m_nWidth;	//ͼ����
	int m_nHeight;	//ͼ��߶�
	RGBQUAD **m_pSourceData;	//ԭʼ����
	double  **m_pLikeliHoodArray;	//���ƶ����飨��ά��
	BYTE    **m_pBinaryArray;	//��ֵ�����飨��ά��

	bool m_bBinaryReady;	//��־λ���ж��Ƿ���ɶ�ֵ��
	RGBQUAD **LightConpensate;
	RGBQUAD **MedianFilter;
	int **m_pGrayData;
private:
	bool m_bLikeliHoodReady;	//��־λ���ж��Ƿ��������ƶ�
};

#endif // #ifndef __IMGAEPROCESS_H
