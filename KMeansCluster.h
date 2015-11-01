#ifndef KMEANS_CLUSTER_H
#define KMEANS_CLUSTER_H
#include "stdafx.h"

class KmeansCluster
{
private:
	struct Pattern
	{
		double *Data; // ��Ƕ����
		int Center;	// ���������غ�
		double *Distance; // ���ݺʹ����ĵľ���
	};
	struct Cluster
	{
		double *Center;
		int PatternNum;
	};

	bool Completed;
	int DataNum; //����������Ŀ
	int Dimension; //����ά��
	int ClusterNum; //������

	Pattern *DataMember;
	
	Cluster *ClusterMember;


public:
	KmeansCluster(int dataNum, int dimension, int clusterNum, double *DataSet);	
	
	~KmeansCluster(void);

	void AddPattern(Cluster *pData, int Idx, double *pValue, int Dimension);

	void ReducePattern(Cluster *pData, int Idx, double *pValue, int Dimension);

	void SetData(Pattern *pData, double *pValue, int DataNum, int Dimension);
	
	void CopyValue(double *pValue1, double *pValue2, int Dimension);

	int FindCenter(int nDataIndex);
	
	double SquareDistance( int nDataIndex,int nClusterIndex);

	bool GetCompleted();

	void ChangeCluster(int nDataIndex,int nClusterIndex);

	// ��ʼ����
	void StartClustering();
	bool SetupClustering();

	void PrintCenter();

	void GetCluster(int*Data);
	int GetMaxLikelyHoodCluster();
};

#endif