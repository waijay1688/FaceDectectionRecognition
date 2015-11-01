//////////////////////////////////////////////////////////////////////
// HairFace.cpp: ��CHairFace�ӿ�
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FaceDetect.h"
#include "HairFace.h"
#include "KMeansCluster.h"
#include "LikelyHood.h"
#include<iostream>

//////////////////////////////////////////////////////////////////////
// ���캯��/��������
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// ���캯��
// ������  source��ԭʼ����
//         width��ͼ����
//         height��ͼ��߶�
//////////////////////////////////////////////////////////////////////
CHairFace::CHairFace(RGBQUAD ** source,int width,int height)
{
	m_nWidth = width;
	m_nHeight= height;

	m_bBinaryOK = false;

	m_pSourceData = source;

	m_pBinaryArray = new  BYTE*[height];
	for(int i=0;i <height; i++)
		m_pBinaryArray[i] = new BYTE[width];
}

//////////////////////////////////////////////////////////////////////
// ��������
//////////////////////////////////////////////////////////////////////
CHairFace::~CHairFace()
{
	if(m_pBinaryArray!=NULL)
	{
		for(int i=0;i<m_nHeight;i++)
			if(m_pBinaryArray[i]!=NULL) delete m_pBinaryArray[i];
		delete m_pBinaryArray;
	}
}

//////////////////////////////////////////////////////////////////////
// �������ͷ����λ��
//////////////////////////////////////////////////////////////////////
void CHairFace::MarkHairFace()
{
	int i,j;
	int count=0;
	for(i=0;i<m_nHeight;i++)
	for(j=0;j<m_nWidth ;j++)
	{
		double r,g,Y,temp;
		temp = m_pSourceData[i][j].rgbGreen
			+m_pSourceData[i][j].rgbRed
			+m_pSourceData[i][j].rgbBlue;
		r = (double)m_pSourceData[i][j].rgbRed/temp;
		g = (double)m_pSourceData[i][j].rgbGreen/temp;
		Y = 0.30*m_pSourceData[i][j].rgbRed+0.59*m_pSourceData[i][j].rgbGreen
			+0.11*m_pSourceData[i][j].rgbBlue;
		
		if(g<0.398 && g > 0.246 && r<0.664 && r>0.333 && r>g && g>=0.5*(1-r))
		{
			m_pBinaryArray[i][j] = 0;  //����λ��
			count++;
		}
		else if(Y<40)
		{
			m_pBinaryArray[i][j] = 1;  //ͷ��λ��
		}
		else
		{
			m_pBinaryArray[i][j] = 2;  //ʲôҲ����
		}
	}
	double *PixelData=new double[count*5];
	
	count=0;
	for(i=0;i<m_nHeight;i++)
		for(j=0;j<m_nWidth ;j++)
			if(m_pBinaryArray[i][j] == 0)
			{
				PixelData[count*5+0]=i*1;
				PixelData[count*5+1]=j*1;
				PixelData[count*5+2]=m_pSourceData[i][j].rgbGreen/2;
				PixelData[count*5+3]=m_pSourceData[i][j].rgbRed/2;
				PixelData[count*5+4]=m_pSourceData[i][j].rgbBlue/2;
				count++;
			}
	CLikelyHood *likelyHood=new CLikelyHood(m_pSourceData,m_nWidth,m_nHeight);
	int* ClusterData=new int[count*3];
	KmeansCluster* m_pKCluster = new KmeansCluster(count , 5, 15, PixelData);
	delete PixelData;

	m_pKCluster->StartClustering();	
	while ( m_pKCluster->SetupClustering() != true);	
	
	m_pKCluster->GetCluster(ClusterData);
	
	likelyHood->CalLikeHood();
//	int nTemp=likelyHood->GetMaxLikelyHood(3,count,ClusterData);
	bool *fakedata=new bool[15];
	int fakecount=0;
	likelyHood->SetFakeFace(15,count,ClusterData,fakedata);
	for(i=0;i<count;i++)
		if(fakedata[ClusterData[i*3]]==false)
			m_pBinaryArray[ClusterData[i*3+1]][ClusterData[i*3+2]]=3;
	delete ClusterData;
	delete fakedata;
	//m_pKCluster->PrintCenter();
	
	m_bBinaryOK = true;
}
