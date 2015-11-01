#include "stdafx.h"
#ifndef KMEANS_CLUSTER_CPP
#define KMEANS_CLUSTER_CPP
#include "KMeansCluster.h"
#include <iostream>
#include <math.h>
// ������
KmeansCluster::KmeansCluster(int dataNum, int dimension, int clusterNum, double *DataSet)
{
	int i=0;
	Completed=false;
	DataNum = dataNum;
	Dimension = dimension;
	ClusterNum = clusterNum;

	DataMember = new Pattern[DataNum];
	ClusterMember = new Cluster[ClusterNum];

	for(i=0; i<DataNum; i++)
	{
		DataMember[i].Data = new double[Dimension];	

		DataMember[i].Distance = new double[ClusterNum];
	}	
	for(i=0; i<ClusterNum; i++)
		ClusterMember[i].Center = new double[Dimension];

	SetData(DataMember, DataSet, DataNum, Dimension);
}

// ������
KmeansCluster::~KmeansCluster(void)
{
	int i=0;
	for(i=0; i<DataNum; i++)
	{
		delete[] DataMember[i].Data;	
		delete[] DataMember[i].Distance;
	}
	delete[] DataMember;

	for(i=0; i<ClusterNum; i++)
		delete[] ClusterMember[i].Center;
	delete[] ClusterMember;
}

void KmeansCluster::AddPattern(Cluster *pData, int Idx, double *pValue, int Dimension)
{
	int i=0;
	double Center;
	for(i=0; i<Dimension; i++)
	{
		Center = pData[Idx].Center[i];
		Center = Center + (pValue[i] - Center) / (pData[Idx].PatternNum);
		pData[Idx].Center[i] = Center;
	}
}


void KmeansCluster::ReducePattern(Cluster *pData, int Idx, double *pValue, int Dimension)
{
	int i;
	double Center;
	for(i=0; i<Dimension; i++)
	{
		Center = pData[Idx].Center[i];
		Center = Center + (Center - pValue[i]) / (pData[Idx].PatternNum);
		pData[Idx].Center[i] = Center;
	}
}

void KmeansCluster::SetData(Pattern *pData, double *pValue, int DataNum, int Dimension)
{
	int i, j;
	for(i=0; i<DataNum; i++)
	{
		for(j=0; j<Dimension; j++)
			pData[i].Data[j] = pValue[i*Dimension+j];
	}
}

void KmeansCluster::CopyValue(double *pValue1, double *pValue2, int Dimension)
{
	int i;
	for(i=0; i<Dimension; i++)		
		pValue1[i] = pValue2[i];
}

int KmeansCluster::FindCenter(int nDataIndex)
{
	int i=0, Ret = 0;
	double* pValue=DataMember[nDataIndex].Distance;
	double Min = pValue[0];

	for(i=0; i<ClusterNum; i++)	
	{
		if(pValue[i]<Min)
		{
			Min = pValue[i];
			Ret = i;
		}
	}
	return Ret;
}

double KmeansCluster::SquareDistance(int nDataIndex,int nClusterIndex)
{
	int i;
	double *pValue1=DataMember[nDataIndex].Data;
	double *pValue2=ClusterMember[nClusterIndex].Center;
	double Distance = 0;
	for(i=0; i<Dimension; i++)
		Distance = Distance + (pValue2[i]-pValue1[i])*(pValue2[i]-pValue1[i]);
	return Distance;
}

// ��ʼ���࣬���ɳ�ʼ�����ģ�����ֻ�Ǹ���ʼֵ���γ����һ���������ĺ;���
void KmeansCluster::StartClustering()
{
	int i=0, j=0;
	//Ϊ�������ĸ���ʼֵ
	for(i=0; i<ClusterNum; i++)
	{
		CopyValue(ClusterMember[i].Center, DataMember[i].Data, Dimension);
		ClusterMember[i].PatternNum = 1;
	}
	
	for(i=0; i<DataNum; i++)
	{	
		//����DataMember������Center�ľ���
		for(j=0; j<ClusterNum; j++)
		{
			DataMember[i].Distance[j] = SquareDistance(i,j);
		}
		//�ҵ�����DataMember��Center���
		int ClusterIdxA = DataMember[i].Center = FindCenter(i);
		//���¾���;�������
		if(i>=ClusterNum)
		{
			ClusterMember[ClusterIdxA].PatternNum += 1;
			AddPattern(ClusterMember, ClusterIdxA, DataMember[i].Data, Dimension);
		}
		Completed=false;
	}
}

bool KmeansCluster::GetCompleted()
{
	return Completed;
}

void KmeansCluster::ChangeCluster(int nDataIndex,int nClusterIndex)
{
	//������һ�������ľ�����������ֵ
	int ClusterIdxA = DataMember[nDataIndex].Center;
	ClusterMember[ClusterIdxA].PatternNum -= 1;
	ReducePattern(ClusterMember, ClusterIdxA, DataMember[nDataIndex].Data, Dimension);
	//�����µ�����������������ֵ
	int ClusterIdxB = DataMember[nDataIndex].Center = nClusterIndex;
	ClusterMember[ClusterIdxB].PatternNum += 1;			
	AddPattern(ClusterMember, ClusterIdxB, DataMember[nDataIndex].Data, Dimension);		
}
// һ�����ಽ
bool KmeansCluster::SetupClustering()
{
	Completed=true;
	//һ�ξ���ѭ����1.���¹��ࣻ2.�޸�������
	for(int i=0; i<DataNum; i++)  
	{
		//����DataMember������Center�ľ���ľ���
		for(int j=0; j<ClusterNum ;j++)
		{
			DataMember[i].Distance[j] = SquareDistance(i,j);
		}
		//��һ���õ���DataMember�ľ������ı�ʶ	
		int Center = DataMember[i].Center;
		//�������������Ƿ�仯
		int newCenter;
		if(ClusterMember[Center].PatternNum>1&&Center!=(newCenter=FindCenter(i)))
		{
			ChangeCluster(i,newCenter);	
			Completed=false;
		}					
	} 
	//�жϾ����Ƿ���ɣ�ClusteringCompleted=true,����ֹͣ
	return GetCompleted();
}

void KmeansCluster::PrintCenter()
{
	using namespace std;
	if(!Completed)
	{
		cout<<"clustering has not completed yet."<<endl;
		return;
	}
	for(int i=0;i<ClusterNum;i++)
	{
		cout<<"cluster "<<i<<" : "<<endl;
		for(int j=0;j<DataNum;j++)
		{
			if(DataMember[j].Center==i)
			{
				for(int k=0;k<Dimension;k++)
					cout<<DataMember[j].Data[k]<<" ";
				cout<<endl;
			}
		}
		cout<<endl;
	}
}

void KmeansCluster::GetCluster(int*Data)
{
	for(int i=0;i<DataNum;i++)
		for(int j=0;j<ClusterNum;j++)
			if(DataMember[i].Center==j)
			{
				Data[i*3+0]=j;
				Data[i*3+1]=DataMember[i].Data[0];
				Data[i*3+2]=DataMember[i].Data[1];
			}
}

int KmeansCluster::GetMaxLikelyHoodCluster()
{	
	double MaxLikeHood=0; 
	int temp=0;
	for(int i=0;i<ClusterNum;i++)
		if(ClusterMember[i].Center[2]>MaxLikeHood)
		{
			MaxLikeHood=ClusterMember[i].Center[2];
			temp=i;
		}
		return temp;
}

#endif