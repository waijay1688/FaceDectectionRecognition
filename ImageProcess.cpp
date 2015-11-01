//////////////////////////////////////////////////////////////////////
// ImageProcess.cpp: 类CImageProcess的接口
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FaceDetect.h"
#include "ImageProcess.h"
#include "KMeansCluster.h"
//////////////////////////////////////////////////////////////////////
// 构造函数/析构函数
//////////////////////////////////////////////////////////////////////

CImageProcess::CImageProcess()
{
	m_pLikeliHoodArray = NULL;
	m_pBinaryArray = NULL;
}

//////////////////////////////////////////////////////////////////////
//构造函数：
//参数：    source－输入图像数据
//          width－图像宽度
//          height－图像高度
//////////////////////////////////////////////////////////////////////
CImageProcess::CImageProcess(RGBQUAD ** source,int width,int height)
{
	int i;

	m_nWidth = width;
	m_nHeight= height;

//	m_bLikeliHoodReady = false;
	m_bLikeliHoodReady = true;
	m_bBinaryReady = false;

	//初始化各数组
	m_pSourceData = source;
	LightConpensate=new RGBQUAD*[height];
	MedianFilter=new RGBQUAD*[height];
	for(i=0;i <height; i++)
	{
		LightConpensate[i]=new RGBQUAD[width];
		MedianFilter[i]=new RGBQUAD[width];
	}
	m_pLikeliHoodArray = new  double*[height];
	for(i=0;i <height; i++)
		m_pLikeliHoodArray[i] = new double[width];	
	m_pBinaryArray = new  BYTE*[height];
	for(i=0;i <height; i++)
		m_pBinaryArray[i] = new BYTE[width];
	m_pGrayData=new int*[m_nHeight];
	for(i=0; i<m_nHeight; i++)
		m_pGrayData[i]=new int[m_nWidth];
}

////////////////////////////////////////////////////////////////////////
//析构函数
////////////////////////////////////////////////////////////////////////
CImageProcess::~CImageProcess()
{
	if(m_pLikeliHoodArray!=NULL)
	{
		for(int i=0;i<m_nHeight;i++)
			if(m_pLikeliHoodArray[i]!=NULL) delete m_pLikeliHoodArray[i];
		delete m_pLikeliHoodArray;
	}
	if(m_pBinaryArray!=NULL)
	{
		for(int i=0;i<m_nHeight;i++)
			if(m_pBinaryArray[i]!=NULL) delete m_pBinaryArray[i];
		delete m_pBinaryArray;
	}
	if(LightConpensate!=NULL)
	{
		for(int i=0;i<m_nHeight;i++)
			if(LightConpensate[i]!=NULL) delete LightConpensate[i];
		delete LightConpensate;
	}
	if(MedianFilter!=NULL)
	{
		for(int i=0;i<m_nHeight;i++)
			if(MedianFilter[i]!=NULL) delete MedianFilter[i];
		delete MedianFilter;
	}
	if(m_pGrayData!=NULL)
	{
		for(int i=0; i<m_nHeight; i++)
			delete m_pGrayData[i];
		delete[] m_pGrayData;
	}
}



//////////////////////////////////////////////////////////////////////////
//光照补偿
//////////////////////////////////////////////////////////////////////////
void CImageProcess::CalLightCompensate()
{
	const float thresholdco = 0.05;
	//象素个数的临界常数
	const int thresholdnum = 100;
	//灰度级数组
	int histogram[256];
	for(int i =0;i<256;i++)
		histogram[i] = 0;
	int colorr,colorg,colorb;
	//考察整个图片
	for( i=0;i<m_nHeight;i++)
		for(int j=0;j<m_nWidth;j++)
		{	
			colorb = m_pSourceData[i][j].rgbBlue;
			colorg = m_pSourceData[i][j].rgbGreen;
			colorr = m_pSourceData[i][j].rgbRed;
			//计算灰度值
			int gray = (colorr * 299 + colorg * 587 + colorb * 114)/1000;
			histogram[gray]++;
		}
		int calnum =0;
		int total = m_nHeight*m_nWidth;
		int num;
		//下面的循环得到满足系数thresholdco的临界灰度级
		for(i =0;i<256;i++)
		{
			if((float)calnum/total < thresholdco)
			{
				calnum+= histogram[255-i];
				num = i;
			}
			else
				break;
		}
		int averagegray = 0;
		calnum =0;
		//得到满足条件的象素总的灰度值
		for(i = 255;i>=255-num;i--)
		{
			averagegray += histogram[i]*i;
			calnum += histogram[i];
		}
		averagegray /=calnum;
		//得到光线补偿的系数
		float co = 255.0/(float)averagegray;
		//下面的循环对图象进行光线补偿
		for(i =0;i<m_nHeight;i++)
			for(int j=0;j<m_nWidth;j++)
			{	
				colorb = m_pSourceData[i][j].rgbBlue;
				//调整
				colorb *=co;
				//临界判断
				if(colorb >255)
					colorb = 255;
				//保存
				LightConpensate[i][j].rgbBlue = colorb;
				//绿色分量
				colorg = m_pSourceData[i][j].rgbGreen;
				colorg *=co;
				if(colorg >255)
					colorg = 255;
				LightConpensate[i][j].rgbGreen = colorg;
				//红色分量
				colorr = m_pSourceData[i][j].rgbRed;
				colorr *=co;
				if(colorr >255)
					colorr = 255;
				LightConpensate[i][j].rgbRed = colorr;
			}
}

//////////////////////////////////////////////////////////////////////////
//中值滤波
//////////////////////////////////////////////////////////////////////////
void CImageProcess::CalMedianFilter()
{
	for(int i=0; i<m_nHeight; i++)
	for(int j=0; j<m_nWidth; j++)
	{
		m_pGrayData[i][j]= (LightConpensate[i][j].rgbRed * 299 + 
							LightConpensate[i][j].rgbGreen * 587 + 
							LightConpensate[i][j].rgbBlue * 114+500)/1000;
	}

	
	for(i=0;i<m_nHeight;i++)
		for(int j=0;j<m_nWidth;j++)
		{
			if(i==0||j==0||i==m_nHeight-1||j==m_nWidth-1)
			{
				MedianFilter[i][j] = LightConpensate[i][j];
				continue;
			}
			int gray[9]={m_pGrayData[i-1][j-1],m_pGrayData[i-1][j],m_pGrayData[i-1][j+1],
			             m_pGrayData[i][j-1]  ,m_pGrayData[i][j]  ,m_pGrayData[i][j+1] ,
						 m_pGrayData[i+1][j-1],m_pGrayData[i+1][j],m_pGrayData[i+1][j+1]};
			int grayIndex[9]={0,1,2,3,4,5,6,7,8};
			for(int m=1;m<9;m++)
				for(int n=0;n<9-m;n++)
					if(gray[n]>gray[n+1])
					{
						int temp=gray[n];
						gray[n]=gray[n+1];
						gray[n+1]=temp;
						int tempIndex=grayIndex[n];
						grayIndex[n]=grayIndex[n+1];
						grayIndex[n+1]=tempIndex;
					}
					int temp=grayIndex[4];
					int tempC=temp/3-1;
					int tempR=temp%3-1;
					MedianFilter[i][j] = LightConpensate[i+tempC][j+tempR];
			//MedianFilter[i][j] = LightConpensate[i][j];						  
		}
}
//////////////////////////////////////////////////////////////////////////
//二值化图像
//////////////////////////////////////////////////////////////////////////
bool CImageProcess::CalBinary()
{	
		
	for(int i=0; i<m_nHeight; i++)
	for(int j=0; j<m_nWidth; j++)
	{
		double colorr=(double)MedianFilter[i][j].rgbRed/255,
			   colorg=(double)MedianFilter[i][j].rgbGreen/255,
			   colorb=(double)MedianFilter[i][j].rgbBlue/255;
		//int gray = (colorr * 299 + colorg * 587 + colorb * 114)/1000;
		double H=0,S=0,V=0;
		if(colorr>=colorg&&colorr>=colorb)
		{
			if(colorg>colorb)
			{
				H=(colorg-colorb)/(colorr-colorb);
				S=(colorr-colorb)/colorr;
			}
			else
			if(colorg==colorb)
			{
				H=0;
				S=0;
			}
			else
			{
				H=(colorg-colorb)/(colorr-colorg);
				S=(colorr-colorg)/colorr;
			}
			V=colorr;
		}
		else
		if(colorg>=colorr&&colorg>=colorb)
		{
			if(colorr>colorb)
			{
				H=2+(colorb-colorr)/(colorg-colorb);
				S=(colorg-colorb)/colorg;
			}
			else
			if(colorr==colorb)
			{
				H=0;
				S=0;
			}
			else
			{
				H=2+(colorb-colorr)/(colorg-colorr);
				S=(colorg-colorr)/colorg;
			}
			V=colorg;
		}
		else
		if(colorb>=colorr&&colorb>=colorg)
		{
			if(colorr>colorg)
			{
				H=4+(colorr-colorg)/(colorb-colorg);
				S=(colorb-colorg)/colorb;
			}
			else
			if(colorr==colorg)
			{
				H=0;
				S=0;
			}
			else
			{
				H=4+(colorr-colorg)/(colorb-colorr);
				S=(colorb-colorr)/colorb;
			}
			V=colorb;
		}
		H*=60;
		if(H<0)
			H+=360;
	
		if(H>10&&H<26)
		{
			m_pBinaryArray[i][j] = 1;
		}
		else
			m_pBinaryArray[i][j] = 0;

	}			

	m_bBinaryReady = true;

	return true;
}

bool CImageProcess::CalMorphologicalFilter()
{
	BYTE **temp = new  BYTE*[m_nHeight];
	for(int i=0;i <m_nHeight; i++)
	{
		temp[i] = new BYTE[m_nWidth];	
		for(int j=0; j<m_nWidth; j++) 
			temp[i][j] = 0;
	}
	for(i=0; i<m_nHeight; i++)
	for(int j=0; j<m_nWidth; j++)
	{
		if(i==0||j==0||i==m_nHeight-1||j==m_nWidth-1)
			{
				temp[i][j] = 0;
				continue;
			}
		int nTemp=(m_pBinaryArray[i-1][j-1]==0)+
			      (m_pBinaryArray[i-1][j]==0)  +
				  (m_pBinaryArray[i-1][j+1]==0)+
				  (m_pBinaryArray[i][j-1]==0)  +
				  (m_pBinaryArray[i][j+1]==0)  +
				  (m_pBinaryArray[i+1][j-1]==0)+
				  (m_pBinaryArray[i+1][j]==0)  +
				  (m_pBinaryArray[i+1][j+1]==0);
		if(nTemp>3)
			temp[i][j] = 0;
		if(nTemp<2)
			temp[i][j] = 1;
	}
	for(i=0; i<m_nHeight; i++)
	for(int j=0; j<m_nWidth; j++)
	{
		m_pBinaryArray[i][j]=temp[i][j];
	}
	
	for(i=0; i<m_nHeight; i++)
		delete temp[i];
	delete[] temp;	

	return true;
}


bool CImageProcess::CalSobel()
{
	for(int i=0; i<m_nHeight; i++)
	for(int j=0; j<m_nWidth; j++)
	{
		m_pGrayData[i][j]= (MedianFilter[i][j].rgbRed * 299 + 
							MedianFilter[i][j].rgbGreen * 587 + 
							MedianFilter[i][j].rgbBlue * 114+500)/1000;
	}

	
	for(i=0; i<m_nHeight; i++)
	for(int j=0; j<m_nWidth; j++)
	{
		if(m_pBinaryArray[i][j] == 1)
		{
			if(i==0||j==0||i==m_nHeight-1||j==m_nWidth-1)
			{
				m_pBinaryArray[i][j] = 0;
				continue;
			}
			int temp=m_pGrayData[i-1][j-1]*-1  +
					 m_pGrayData[i-1][j]*-2    +
					 m_pGrayData[i-1][j+1]*-1  +
					 m_pGrayData[i+1][j-1]*1   +
					 m_pGrayData[i+1][j]*2     +
					 m_pGrayData[i+1][j+1]*1;
			if(temp<70&&temp>-70)
			{
				
				temp=m_pGrayData[i-1][j-1]*1   +
					 m_pGrayData[i-1][j]*0     +
					 m_pGrayData[i-1][j+1]*-1  +
					 m_pGrayData[i][j-1]*2     +
					 m_pGrayData[i][j]*0       +
					 m_pGrayData[i][j+1]*-2    +
					 m_pGrayData[i+1][j-1]*1   +
					 m_pGrayData[i+1][j]*0     +
					 m_pGrayData[i+1][j+1]*-1;
				if(temp<70&&temp>-70)			
					m_pBinaryArray[i][j] = 0;
			}
		}
	}
	return true;
}
int CImageProcess::GetMaxLikelyHood(int ClusterNum,int DataNum,int *CoordinatorData)
{
	if(	!m_bLikeliHoodReady)
		return -1;
	double *pLikelyHood=new double[ClusterNum];
	int *pCount=new int[ClusterNum];
	for(int i=0;i<ClusterNum;i++)
	{
		pLikelyHood[i]=0;
		pCount[i]=0;
	}

	for(i=0;i<DataNum;i++)
	{
		for(int j=0;j<ClusterNum;j++)
		{
			if(CoordinatorData[i]==j)
			{
				pLikelyHood[j]+=m_pLikeliHoodArray[CoordinatorData[i+1]][CoordinatorData[i+2]];
				pCount[j]++;
			}
		}
	}
	double MaxLikeHood=0;
	int nTemp=0;
	for(i=0;i<ClusterNum;i++)
		if(pLikelyHood[i]/(double)pCount[i]>MaxLikeHood)
		{
			MaxLikeHood=pLikelyHood[i]/pCount[i];
			nTemp=i;
		}
	return nTemp;
}

void CImageProcess::SetFakeFace(int ClusterNum,int DataNum,int *CoordinatorData,bool*Data)
{
	if(	!m_bLikeliHoodReady)
		return;
	double *pLikelyHood=new double[ClusterNum];
	int *pCount=new int[ClusterNum];
	for(int i=0;i<ClusterNum;i++)
	{
		pLikelyHood[i]=0;
		pCount[i]=0;
	}

	for(i=0;i<DataNum;i++)
	{
		for(int j=0;j<ClusterNum;j++)
		{
			if(CoordinatorData[3*i]==j)
			{
				pLikelyHood[j]+=m_pLikeliHoodArray[CoordinatorData[3*i+1]][CoordinatorData[3*i+2]];
				pCount[j]++;
			}
		}
	}
	for(i=0;i<ClusterNum;i++)
	{
		pLikelyHood[i]=pLikelyHood[i]/pCount[i];
		if(pLikelyHood[i]<0.58)
			Data[i]=false;
		else
			Data[i]=true;
	}
	delete pLikelyHood;
	delete pCount;
}

