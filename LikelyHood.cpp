//////////////////////////////////////////////////////////////////////
// LikelyHood.cpp: 类CLikelyHood的接口
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FaceDetect.h"
#include "LikelyHood.h"
#include "KMeansCluster.h"
//////////////////////////////////////////////////////////////////////
// 构造函数/析构函数
//////////////////////////////////////////////////////////////////////

CLikelyHood::CLikelyHood()
{
	m_pLikeliHoodArray = NULL;
	m_pBinaryArray = NULL;

	bmean =  B_Mean;
	rmean =  R_Mean;
	brcov[0][0] =  Brcov00;
	brcov[0][1] =  Brcov01;
	brcov[1][0] =  Brcov10;
	brcov[1][1] =  Brcov11;
}

//////////////////////////////////////////////////////////////////////
//构造函数：
//参数：    source－输入图像数据
//          width－图像宽度
//          height－图像高度
//////////////////////////////////////////////////////////////////////
CLikelyHood::CLikelyHood(RGBQUAD ** source,int width,int height)
{
	int i;

	bmean =  B_Mean;
	rmean =  R_Mean;
	brcov[0][0] =  Brcov00;
	brcov[0][1] =  Brcov01;
	brcov[1][0] =  Brcov10;
	brcov[1][1] =  Brcov11;

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
CLikelyHood::~CLikelyHood()
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
//计算相似度
//////////////////////////////////////////////////////////////////////////
void CLikelyHood::CalLikeHood()
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


	
	for(i=0; i<m_nHeight; i++)
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

///////////////////////////////////////////////////////////////////////////////
//计算Cb，Cr
///////////////////////////////////////////////////////////////////////////////
TCbCr CLikelyHood::CalCbCr(int R, int G, int B)
{
	TCbCr res;
	res.Cb =( 128 - 37.797 * R/255 - 74.203 * G/255 +   112 * B/255);
	res.Cr =( 128 + 112    * R/255 - 93.786 * G/255 -18.214 * B/255);
	return res;
}


//////////////////////////////////////////////////////////////////////////
//二值化图像
//////////////////////////////////////////////////////////////////////////
bool CLikelyHood::CalBinary()
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
	
	BYTE **temp = new  BYTE*[m_nHeight];
	for(i=0;i <m_nHeight; i++)
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

	
	
	for(i=0; i<m_nHeight; i++)
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
		
		
		
	

	m_bBinaryReady = true;

	return true;
}

/////////////////////////////////////////////////////////////////////////
//功能：训练参数
//参数：DirectPath－Bmp文件路径
/////////////////////////////////////////////////////////////////////////
int CLikelyHood::CalParameter(CString DirectPath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	BOOL  FINDOVER=TRUE;
	hFind = FindFirstFile(DirectPath+"\\*.bmp", &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	int filenum = 0;
	while(FINDOVER)
	{
		CString m_sOpenFileName=DirectPath+FindFileData.cFileName;
		filenum ++;
		FINDOVER=FindNextFile(hFind,&FindFileData);
	}
	FindClose(hFind);

	double **CrList,**CbList;
	CrList = new double*[filenum];
	CbList = new double*[filenum];

	int* mapLength = new int[filenum];

	FINDOVER=TRUE;
	BOOL FileOK=true;
	int index = 0;
	hFind = FindFirstFile(DirectPath+"\\*.bmp", &FindFileData);
	while(FINDOVER && FileOK)
	{
		CString m_sOpenFileName=DirectPath+"\\"+FindFileData.cFileName;
		CDib * dib;
		dib = new CDib();
		if(!dib->Open(m_sOpenFileName))
		{
			AfxMessageBox(m_sOpenFileName+": Invalid File Format! It must be BMP File");
			FileOK = false;
		}
		if(FileOK)
		{
			int m_nBitMapWidth = dib->GetWidth();
			int m_nBitMapHeight= dib->GetHeight();
			CrList[index] = new double[m_nBitMapWidth*m_nBitMapHeight];
			CbList[index] = new double[m_nBitMapWidth*m_nBitMapHeight];
			mapLength[index] = m_nBitMapWidth*m_nBitMapHeight;

			BYTE	*colorTable;
			colorTable = (BYTE *)dib->m_pDibBits;
			int byteBitCount  = dib->GetBiBitCount()/8;
			int count = 0;
			double **tempCr,**tempCb;
			tempCr = new  double*[m_nBitMapHeight];
			tempCb = new  double*[m_nBitMapHeight];
			for(int i=0;i <m_nBitMapHeight; i++)
			{
				tempCr[i] = new double[m_nBitMapWidth];	
				tempCb[i] = new double[m_nBitMapWidth];	
			}

			for(i=m_nBitMapHeight-1; i>=0; i--)
			{
				for(int j=0; j<m_nBitMapWidth; j++)
				{
					TCbCr temp = CalCbCr(colorTable[count+2],colorTable[count+1],colorTable[count]);
					tempCr[i][j] = temp.Cr;
					tempCb[i][j] = temp.Cb;
					//int q = m_nBitMapWidth*(m_nBitMapHeight-1-i)+j;
					//CrList[index][q]= temp.Cr;
					//CbList[index][q]= temp.Cb;
					count += byteBitCount;
				}
				count += (4-(m_nBitMapWidth*byteBitCount)%4)%4;
			}
			filter(tempCr,m_nBitMapWidth,m_nBitMapHeight);
			filter(tempCb,m_nBitMapWidth,m_nBitMapHeight);
			for(i=0 ; i<m_nBitMapHeight; i++)
			{
				for(int j=0; j<m_nBitMapWidth; j++)
				{
					int q = m_nBitMapWidth*i+j;
					CrList[index][q]= tempCr[i][j];
					CbList[index][q]= tempCb[i][j];
				}
			}
			for(i=0;i<m_nBitMapHeight;i++)
			{
				delete tempCr[i];
				delete tempCb[i];
			}
			delete tempCr;
			delete tempCb;

			index++;
		}
		delete dib;
		FINDOVER=FindNextFile(hFind,&FindFileData);
	}
	FindClose(hFind);

	double avgCr = 0;
	double avgCb = 0;
	double cov00,cov01,cov10,cov11;
	cov00 = cov01 = cov10 = cov11 = 0.0;
	int totallen = 0;
	
	if(FileOK)
	{
		for(int i=0;i<filenum;i++)
		{
			totallen+=mapLength[i];
			for(int j=0;j<mapLength[i];j++)
			{
				avgCr += CrList[i][j];
				avgCb += CbList[i][j];
			}
		}
		avgCr /= totallen;
		avgCb /= totallen;

		for(i=0;i<filenum;i++)
		{
			for(int j=0;j<mapLength[i];j++)
			{
				cov11 += (CrList[i][j]-avgCr)*(CrList[i][j]-avgCr);
				cov00 += (CbList[i][j]-avgCb)*(CbList[i][j]-avgCb);
				cov01 += (CrList[i][j]-avgCr)*(CbList[i][j]-avgCb);
			}
		}
		cov00 /= totallen;
		cov01 /= totallen;
		cov10 = cov01;
		cov11 /= totallen;
	}

	for(int i=0;i<filenum;i++)
	{
		if(CrList[i]!=NULL) delete CrList[i];
		if(CbList[i]!=NULL) delete CbList[i];
	}
	delete mapLength;
	if(!FileOK) return 1;

	if((cov00*cov11-cov01*cov01) == 0) return 2;

	bmean =  avgCb;
	rmean =  avgCr;
	brcov[0][0] =  cov00;
	brcov[0][1] =  cov01;
	brcov[1][0] =  cov10;
	brcov[1][1] =  cov11;
	return 3;
}

int CLikelyHood::GetMaxLikelyHood(int ClusterNum,int DataNum,int *CoordinatorData)
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

void CLikelyHood::SetFakeFace(int ClusterNum,int DataNum,int *CoordinatorData,bool*Data)
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

void CLikelyHood::filter(double * *,int,int)
{
}


		