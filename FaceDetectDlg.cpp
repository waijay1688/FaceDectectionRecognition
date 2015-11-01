// FaceDetectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FaceDetect.h"
#include "FaceDetectDlg.h"
#include "ReplaceDlg.h"
#include "KMeansCluster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "AddSampleDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFaceDetectDlg dialog

CFaceDetectDlg::CFaceDetectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFaceDetectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFaceDetectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFaceDetectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFaceDetectDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFaceDetectDlg, CDialog)
	//{{AFX_MSG_MAP(CFaceDetectDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_BINARY, OnBtnBinary)
	ON_BN_CLICKED(IDC_BTN_HISTOGRAM_H, OnBtnHistogramH)
	ON_BN_CLICKED(IDC_BTN_HISTOGRAM_V, OnBtnHistogramV)
	ON_BN_CLICKED(IDC_BTN_MARK_EYE, OnBtnMarkEye)
	ON_BN_CLICKED(IDC_BTN_MARK_FACE_1, OnBtnMarkFace1)
	ON_BN_CLICKED(IDC_BTN_MARK_MOUSE, OnBtnMarkMouse)
	ON_BN_CLICKED(IDC_BTN_MARK_NOSE, OnBtnMarkNose)
	ON_BN_CLICKED(IDC_BTN_OPENFILE, OnBtnOpenfile)
	ON_BN_CLICKED(IDC_BTN_LIGHTCOMPENSATE, OnBtnLightcompensate)
	ON_BN_CLICKED(IDC_BTN_MEDIANFILTER, OnBtnMedianFilter)
	ON_BN_CLICKED(IDC_BTN_SOBEL, OnBtnSobel)
	ON_BN_CLICKED(IDC_BTN_MORPHOLOGICAL, OnBtnMorphological)
	ON_BN_CLICKED(IDC_BTN_PRECISEFACE, OnBtnPreciseface)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFaceDetectDlg message handlers

BOOL CFaceDetectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	CWnd *pWnd0= GetDlgItem(IDC_BMPSHOW);
	pDCShow = pWnd0->GetDC();

	m_pMainDib = new CDib();

	m_tOriPixelArray = NULL;
	m_tResPixelArray = NULL;

	m_pResMap = NULL;

	m_nWndWidth = 0;
	m_nWndHeight= 0;
	m_sFileName = "";

	m_bSelectByMan = false;
	m_bLBottonDown = false;

	m_ManLeft = -1;
	m_ManRight = -1;
	m_ManTop = -1;
	m_ManBottom = -1;

	m_bFaceOK = false;
	m_bShowFace = false;
	m_rFaceRegion.left = m_rFaceRegion.right = m_rFaceRegion.top = m_rFaceRegion.bottom = 0;

	m_bManualMarkFacial = false;
	m_bLeftEyeOK = m_bRightEyeOK = m_bLeftNostrilOK = m_bRightNostrilOK =
	m_bLeftEyeLeftCornerOK = m_bLeftEyeRightCornerOK = m_bRightEyeLeftCornerOK = 
	m_bRightEyeRightCornerOK = m_bLeftMouthCornerOK = m_bRightMouthCornerOK = false;

	m_bMidMouthOK = m_bMidNoseOK = false;

	m_LeftEye = m_RightEye = m_LeftEyeLeftCorner = m_LeftEyeRightCorner = 
	m_LeftNostril = m_RightNostril = m_RightEyeLeftCorner = m_RightEyeRightCorner =
	m_LeftMouthCorner = m_RightMouthCorner = m_MidMouth = m_MidNose = CPoint(-1,-1);


	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFaceDetectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFaceDetectDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);

		if(m_tResPixelArray==NULL) return;
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFaceDetectDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

////////////////////////////////////////////////////////////////////////////////
// 画十字形标记
// 参数：  pDC－CDC指针
//         point－要画的点的坐标
//         crColor－标记得颜色
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::DrawCross(CDC *pDC, CPoint point, COLORREF crColor)
{
	CPen pen,*oldPen;
	pen.CreatePen(PS_SOLID,1,crColor);
	oldPen = (CPen*)pDC->SelectObject(&pen);
	pDC->MoveTo(point.x-7,point.y);
	pDC->LineTo(point.x+7,point.y);
	pDC->MoveTo(point.x,point.y-7);
	pDC->LineTo(point.x,point.y+7);
	pDC->SelectObject(oldPen);
	pen.DeleteObject();
}

////////////////////////////////////////////////////////////////////////////////
// 拷贝位图
// 参数：  dest－目标位图指针
//         source－源位图指针
////////////////////////////////////////////////////////////////////////////////
bool CFaceDetectDlg::CopyBitMap(RGBQUAD **dest, RGBQUAD **source)
{
	if(source==NULL || dest==NULL) 
		return false;
	for(int i=0; i<m_nWndHeight; i++)
		for(int j=0; j<m_nWndWidth; j++)
			dest[i][j]=source[i][j];
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// 生成新的位图
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::MakeBitMap()
{
	CClientDC ClientDC(pDCShow->GetWindow());
	if(m_pResMap!=NULL) delete m_pResMap;

	m_pResMap=new CBitmap();
	m_pResMap->CreateCompatibleBitmap(&ClientDC,m_nWndWidth,m_nWndHeight);

	CDC  dc;
	dc.CreateCompatibleDC(&ClientDC);
	dc.SelectObject(m_pResMap);

	for(int i=0; i<m_nWndHeight; i++)
	for(int j=0; j<m_nWndWidth; j++)
	dc.SetPixelV(j,i,RGB(m_tResPixelArray[i][j].rgbRed,m_tResPixelArray[i][j].rgbGreen,m_tResPixelArray[i][j].rgbBlue));

	if(m_bFaceOK && m_bShowFace)
	{
		CBrush Pen;
		Pen.CreateSolidBrush(RGB(255,0,0));
		dc.FrameRect(m_rFaceRegion,&Pen);
	 	Pen.DeleteObject();
	}

	if(m_bLeftEyeOK)					DrawCross(&dc,m_LeftEye,RGB(255,0,0));
	if(m_bRightEyeOK)					DrawCross(&dc,m_RightEye,RGB(255,0,0));
	if(m_bLeftEyeLeftCornerOK)			DrawCross(&dc,m_LeftEyeLeftCorner,RGB(255,0,255));
	if(m_bLeftEyeRightCornerOK)			DrawCross(&dc,m_LeftEyeRightCorner,RGB(255,255,0));
	if(m_bRightEyeLeftCornerOK)			DrawCross(&dc,m_RightEyeLeftCorner,RGB(255,0,255));
	if(m_bRightEyeRightCornerOK)		DrawCross(&dc,m_RightEyeRightCorner,RGB(255,255,0));
	if(m_bLeftNostrilOK)				DrawCross(&dc,m_LeftNostril,RGB(0,255,0));
	if(m_bRightNostrilOK)				DrawCross(&dc,m_RightNostril,RGB(0,255,0));
	if(m_bMidNoseOK)					DrawCross(&dc,m_MidNose,RGB(0,255,0));
	if(m_bLeftMouthCornerOK)			DrawCross(&dc,m_LeftMouthCorner,RGB(0,0,255));
	if(m_bRightMouthCornerOK)			DrawCross(&dc,m_RightMouthCorner,RGB(0,0,255));
	if(m_bMidMouthOK)					DrawCross(&dc,m_MidMouth,RGB(0,0,255));

	dc.DeleteDC();

	MyDraw();
}

////////////////////////////////////////////////////////////////////////////////////
//读原图的数据
////////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::LoadOriPixel(CDib *pDib)
{
	BYTE	*colorTable;
	colorTable = (BYTE *)pDib->m_pDibBits;
	int byteBitCount  = pDib->GetBiBitCount()/8;

	m_tOriPixelArray  = new RGBQUAD*[m_nWndHeight];
	m_tResPixelArray  = new RGBQUAD*[m_nWndHeight];
	for(int l=0 ; l<m_nWndHeight; l++)
	{
		m_tOriPixelArray[l] = new RGBQUAD[m_nWndWidth];
		m_tResPixelArray[l] = new RGBQUAD[m_nWndWidth];
	}

	int count = 0;
	for(int i=m_nWndHeight-1; i>=0; i--)
	{
		for(int j=0; j<m_nWndWidth; j++)
		{
			m_tOriPixelArray[i][j].rgbBlue =colorTable[count++];
			m_tOriPixelArray[i][j].rgbGreen=colorTable[count++];
			m_tOriPixelArray[i][j].rgbRed  =colorTable[count++];
			m_tOriPixelArray[i][j].rgbReserved = 0;
			m_tResPixelArray[i][j]=m_tOriPixelArray[i][j];
			count += byteBitCount-3;
		}
		count += (4-(m_nWndWidth*byteBitCount)%4)%4;
	}

	method1 = new CImageProcess(m_tOriPixelArray,m_nWndWidth,m_nWndHeight);
}

////////////////////////////////////////////////////////////////////////////////
// 给位图赋值
// 参数：  target－目标位图指针
//         Val－要赋予的值
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::SetPixelArray(RGBQUAD **target, int Val)
{
	for(int i=0; i<m_nWndHeight; i++)
	for(int j=0; j<m_nWndWidth; j++)
	{
		target[i][j].rgbRed =  target[i][j].rgbBlue = target[i][j].rgbGreen = Val;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 粗略二值化
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::OnBtnBinary() 
{
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	
	if(!method1->CalBinary())
	{
		AfxMessageBox("请先计算相似度！");
		SetCursor(LoadCursor(NULL,IDC_ARROW));	
		return;
	}
	
	m_bShowFace = false;
	for(int i=0; i<m_nWndHeight; i++)
	for(int j=0; j<m_nWndWidth;  j++)
	{
		m_tResPixelArray[i][j].rgbBlue = m_tResPixelArray[i][j].rgbGreen =	
		m_tResPixelArray[i][j].rgbRed  = (int)(method1->m_pBinaryArray[i][j]*255);
	}
	MakeBitMap();
	SetCursor(LoadCursor(NULL,IDC_ARROW));	
	MyDraw();
}


////////////////////////////////////////////////////////////////////////////////
// 水平方向直方图
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::OnBtnHistogramH() 
{
	if(!method1->m_bBinaryReady)
	{
		AfxMessageBox("请先计算二值图");
		return;
	}
	m_bShowFace = false;
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	if(BtnHistogramH==NULL)
		BtnHistogramH=new int[m_nWndWidth];
	else
	{
		BtnHistogramH=NULL;
		BtnHistogramH=new int[m_nWndWidth];
	}
	for(int j=0; j<m_nWndWidth;  j++)	
	{
		int count = 0;
		for(int i=0; i<m_nWndHeight; i++)
		{
			if(method1->m_pBinaryArray[i][j] == 1) count++;
			m_tResPixelArray[i][j].rgbBlue = m_tResPixelArray[i][j].rgbGreen =	
			m_tResPixelArray[i][j].rgbRed  = 255;
		}
		BtnHistogramH[j]=count;
		for(i=m_nWndHeight-1; i>=m_nWndHeight-count;i--)
		{
			m_tResPixelArray[i][j].rgbBlue = m_tResPixelArray[i][j].rgbGreen =	
			m_tResPixelArray[i][j].rgbRed  = 0;
		}
	}
	MakeBitMap();
	SetCursor(LoadCursor(NULL,IDC_ARROW));		
}


////////////////////////////////////////////////////////////////////////////////
// 垂直方向的直方图
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::OnBtnHistogramV() 
{
	if(!method1->m_bBinaryReady)
	{
		AfxMessageBox("请先计算二值图");
		return;
	}

	m_bShowFace = false;
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	if(BtnHistogramV!=NULL)
	{
		BtnHistogramV=NULL;
		BtnHistogramV=new int[m_nWndHeight];
	}
	else
		BtnHistogramV=new int[m_nWndHeight];
	for(int i=0; i<m_nWndHeight; i++)		
	{
		int count = 0;
		for(int j=0; j<m_nWndWidth;  j++)
		{
			if(method1->m_pBinaryArray[i][j] == 1) 
				count++;
			m_tResPixelArray[i][j].rgbBlue = m_tResPixelArray[i][j].rgbGreen =	
			m_tResPixelArray[i][j].rgbRed  = 255;
		}
		BtnHistogramV[i]=count;
		for(j=0; j<count;  j++)
		{
			m_tResPixelArray[i][j].rgbBlue = m_tResPixelArray[i][j].rgbGreen =	
			m_tResPixelArray[i][j].rgbRed  = 0;
		}
	}
	MakeBitMap();
	SetCursor(LoadCursor(NULL,IDC_ARROW));			
}


////////////////////////////////////////////////////////////////////////////////
// 第一种方法标记脸部区域
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::OnBtnMarkFace1() 
{
	if(!method1->m_bBinaryReady)
	{
		AfxMessageBox("请先计算二值化图！");
		return;
	}
	m_bShowFace = true;
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	int top=0,bottom=m_nWndHeight-1;
	int left=0,right=m_nWndWidth-1;
	int i,j;
	int maxV=0,maxIndexV=0;
	int maxH=0,maxIndexH=0;
	int oldtop=0,oldbottom=m_nWndHeight-1;
	int oldleft=0,oldright=m_nWndWidth-1;
	bool tempV=false,tempH=false;
	int count=0;
	do
	{			
		maxV=0;
		for(i=oldtop;i<oldbottom;i++)
		if(BtnHistogramV[i]>maxV)
		{
			maxV=BtnHistogramV[i];
			maxIndexV=i;
		}
		for(i=oldtop;i<oldbottom;i++)
		if(BtnHistogramV[i+1]-BtnHistogramV[i]>maxV/15)
		{
			top=i;
			break;
		}
		for(i=oldbottom;i>=top;i--)
		if(BtnHistogramV[i-1]-BtnHistogramV[i]>maxV/15&&(double)(i-maxIndexV)/(maxIndexV-top)<4)
		{			
			bottom=i;
			break;
		}
 		tempV=(double)(top-oldtop)/(top+1)<0.05&&(double)(oldbottom-bottom)/(bottom+1)<0.05;
		oldtop=top;
		oldbottom=bottom;
		//重新计算水平投影
		for(j=oldleft; j<oldright;  j++)	
		{
			int count = 0;
			for( i=top; i<bottom; i++)
			{
				if(method1->m_pBinaryArray[i][j] == 1) 
					count++;
			}
			BtnHistogramH[j]=count;
		}	
		
		//根据得到的水平投影确定left right
		maxH=0;
		for(i=oldleft;i<oldright;i++)
			if(BtnHistogramH[i]>maxH)
			{
				maxH=BtnHistogramH[i];
				maxIndexH=i;
			}
		for(i=oldleft;i<oldright;i++)
			if(BtnHistogramH[i+1]-BtnHistogramH[i]>maxH/7)
			{
				left=i;
				break;
			}
		for(i=oldright;i>=left;i--)
			if(BtnHistogramH[i-1]-BtnHistogramH[i]>maxH/7&&double(i-maxIndexH)/double(maxIndexH-left)<2)			
			{
				right=i;
				break;
			}
		//循环退出条件
		tempH=(double)(left-oldleft)/(left+1)<0.05&&(double)(oldright-right)/(right+1)<0.05;
		oldleft=left;
		oldright=right;
		
		//重新计算垂直投影
		for(i=oldtop; i<oldbottom; i++)		
		{
			int count = 0;
			for(j=left; j<right;  j++)
			{
				if(method1->m_pBinaryArray[i][j] == 1) 
					count++;
			}
			BtnHistogramV[i]=count;
		}		
		
		count++;
		if(count==8)
			break;
	}while((!tempV&&!tempH)||count<3);
	
	//	保存maxH和maxV
	MaxH=maxH;
	MaxV=maxV;
	CopyBitMap(m_tResPixelArray,m_tOriPixelArray);

	for(i=top;i<=bottom;i++)
	{
		m_tResPixelArray[i][left].rgbBlue=255;
		m_tResPixelArray[i][left].rgbGreen = m_tResPixelArray[i][left].rgbRed = 0;
		m_tResPixelArray[i][right].rgbBlue=255;
		m_tResPixelArray[i][right].rgbGreen = m_tResPixelArray[i][right].rgbRed = 0;
	}
	for(j=left;j<=right;j++)
	{
		m_tResPixelArray[top][j].rgbBlue=255;
		m_tResPixelArray[top][j].rgbGreen = m_tResPixelArray[top][j].rgbRed = 0;
		m_tResPixelArray[bottom][j].rgbBlue=255;
		m_tResPixelArray[bottom][j].rgbGreen = m_tResPixelArray[bottom][j].rgbRed = 0;
	}
	MakeBitMap();
	SetCursor(LoadCursor(NULL,IDC_ARROW));			

	if(m_bFaceOK)
	{
		ReplaceDlg dlg;
		if(dlg.DoModal()==IDOK)
		{
			CopyBitMap(m_tResPixelArray,m_tOriPixelArray);
			CRect rect(left,top,right,bottom);
			m_rFaceRegion = rect;
			MakeBitMap();
		}
	}
	else
	{
		m_bFaceOK = true;
		CopyBitMap(m_tResPixelArray,m_tOriPixelArray);
		CRect rect(left,top,right,bottom);
		m_rFaceRegion = rect;
		MakeBitMap();
	}
}
/*
////////////////////////////////////////////////////////////////////////////////
// 标记嘴巴
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::OnBtnMarkMouse() 
{
	int i,j;
	if(!(m_bLeftEyeOK&&m_bRightEyeOK))
	{
		AfxMessageBox("请先确定眼睛");
		return;
	}
	int mouthtop=m_rNoseRegion.bottom+5;
	int mouthbottom=m_rFaceRegion.bottom-30;
	int mouthleft=m_rEyeRegion.left+15;
	int mouthright=m_rEyeRegion.right-15;
	int height=mouthbottom-mouthtop+1;
	int width=mouthright-mouthleft+1;
	double *PixelData=new double[height*width*5];
	
	for(i=0;i<height;i++)
		for(j=0;j<width ;j++)
		{
			PixelData[(i*width+j)*5+0]=i;
			PixelData[(i*width+j)*5+1]=j;
			PixelData[(i*width+j)*5+2]=method1->MedianFilter[i+mouthtop][j+mouthleft].rgbGreen;
			PixelData[(i*width+j)*5+3]=method1->MedianFilter[i+mouthtop][j+mouthleft].rgbRed;
			PixelData[(i*width+j)*5+4]=method1->MedianFilter[i+mouthtop][j+mouthleft].rgbBlue;
		}
		
	int* ClusterData=new int[height*width*3];
	KmeansCluster* m_pKCluster = new KmeansCluster(width*height , 5, 5, PixelData);
	delete PixelData;

	m_pKCluster->StartClustering();	
	while ( m_pKCluster->SetupClustering() != true);	
	
	m_pKCluster->GetCluster(ClusterData);
	
	CopyBitMap(m_tResPixelArray,m_tOriPixelArray);
	for(i=0;i<height;i++)
		for(j=0;j<width ;j++)
		{
			if(ClusterData[3*(i*width+j)]==0)
			{
				int tempx=i+mouthtop;
				int tempy=j+mouthleft;
				m_tResPixelArray[tempx][tempy].rgbGreen=255;
				m_tResPixelArray[tempx][tempy].rgbBlue = m_tResPixelArray[tempx][tempy].rgbRed = 0;
			}
			if(ClusterData[3*(i*width+j)]==1)
			{
				int tempx=i+mouthtop;
				int tempy=j+mouthleft;
				m_tResPixelArray[tempx][tempy].rgbGreen=0;
				m_tResPixelArray[tempx][tempy].rgbBlue = m_tResPixelArray[tempx][tempy].rgbRed = 0;
			}
			if(ClusterData[3*(i*width+j)]==2)
			{
				int tempx=i+mouthtop;
				int tempy=j+mouthleft;
				m_tResPixelArray[tempx][tempy].rgbGreen=0;
				m_tResPixelArray[tempx][tempy].rgbBlue = 0;
				m_tResPixelArray[tempx][tempy].rgbRed = 255;
			}
		}
	MakeBitMap();
}
*/
void CFaceDetectDlg::OnBtnMarkMouse() 
{
	if(!(m_bMidNoseOK))
	{
		AfxMessageBox("请先确定鼻子");
		return;
	}
	int i,j;
	if(!(m_bLeftEyeOK&&m_bRightEyeOK))
	{
		AfxMessageBox("请先确定眼睛");
		return;
	}
	int mouthtop=m_rNoseRegion.bottom+5;
	int mouthbottom=m_rFaceRegion.bottom-30;
	int mouthleft=m_LeftEye.x;
	int mouthright=m_RightEye.x;
	int height=mouthbottom-mouthtop+1;
	int width=mouthright-mouthleft+1;

	BYTE **Mouth=new BYTE*[height];
	for(i=0;i<height;i++)
		Mouth[i]=new BYTE[width];
	//唇色判定
	for(i=mouthtop;i<=mouthbottom;i++)
		for(j=mouthleft;j<=mouthright;j++)
		{
			BYTE R,G,B;
			double temp,dlta;
			R = method1->MedianFilter[i][j].rgbRed;
			G = method1->MedianFilter[i][j].rgbGreen;
			B = method1->MedianFilter[i][j].rgbBlue;
			if((R==G) && (G==B)) 
				temp = 0;
			else
				temp = 0.5*(2*R-G-B)/sqrt((R-G)*(R-G)+(R-B)*(G-B));
			dlta = acos(temp);
			if(dlta < 0.2)
				Mouth[i-mouthtop][j-mouthleft]=1;
			else 
				Mouth[i-mouthtop][j-mouthleft]=0;
		}
	int *MouthHistogramV=new int[height];
	int *MouthHistogramH=new int[width];
	int maxMouthV=0,maxMouthIndexV=0;
	int maxMouthH=0,maxMouthIndexH=0;
	//建立嘴唇垂直直方图
	for(i=0;i<height;i++)
	{
		int count=0;
		for(j=0;j<width;j++)
			if(Mouth[i][j]==1)
				count++;
		MouthHistogramV[i]=count;
		if(count>maxMouthV)
		{
			maxMouthV=count;
			maxMouthIndexV=i;
		}
	}
	//建立嘴唇水平直方图
	for(i=0;i<width;i++)
	{
		int count=0;
		for(j=0;j<height;j++)
			if(Mouth[j][i]==1)
				count++;
		MouthHistogramH[i]=count;
		if(count>maxMouthH)
		{
			maxMouthH=count;
			maxMouthIndexH=i;
		}
	}
	
	for(i=0;i<height;i++)
	{
		if(MouthHistogramV[i]>(double)maxMouthV/5)
		{
			mouthtop+=i;
			break;
		}
	}
	for(i=height-1;i>mouthtop;i--)
	{
		if(MouthHistogramV[i]>(double)maxMouthV/5)
		{
			mouthbottom-=(height-i);
			break;
		}
	}
	
	for(i=0;i<width;i++)
	{
		if(MouthHistogramH[i]>(double)maxMouthH/5)
		{
			mouthleft+=i;
			break;
		}
	}
	for(i=width-1;i>mouthleft;i--)
	{
		if(MouthHistogramH[i]>(double)maxMouthH/5)
		{
			mouthright-=(width-i);
			break;
		}
	}
	m_bMidMouthOK=TRUE;
	m_MidMouth.y=(mouthtop+mouthbottom)/2;
	m_MidMouth.x=(mouthleft+mouthright)/2;

	for(i=mouthtop;i<=mouthbottom;i++)
	{
		m_tResPixelArray[i][mouthleft].rgbGreen=255;
		m_tResPixelArray[i][mouthleft].rgbBlue = m_tResPixelArray[i][mouthleft].rgbRed = 0;
		m_tResPixelArray[i][mouthright].rgbGreen=255;
		m_tResPixelArray[i][mouthright].rgbBlue = m_tResPixelArray[i][mouthright].rgbRed = 0;
	}
	for(j=mouthleft;j<=mouthright;j++)
	{
		m_tResPixelArray[mouthtop][j].rgbGreen=255;
		m_tResPixelArray[mouthtop][j].rgbBlue = m_tResPixelArray[mouthtop][j].rgbRed = 0;
		m_tResPixelArray[mouthbottom][j].rgbGreen=255;
		m_tResPixelArray[mouthbottom][j].rgbBlue = m_tResPixelArray[mouthbottom][j].rgbRed = 0;
	}

	MakeBitMap();	
}

////////////////////////////////////////////////////////////////////////////////
// 打开文件
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::OnBtnOpenfile() 
{
	CAddSampleDlg FileDlg(TRUE, "", NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, 
		"BMP人脸图像(*.bmp)|*.bmp|所有文件(*.*)|*.*||",
		AfxGetMainWnd());

	CString strFile;
	if (FileDlg.DoModal () != IDOK)
		return;

	POSITION pos = FileDlg.GetStartPosition();
	strFile = FileDlg.GetNextPathName(pos);
	m_pMainDib->Open(strFile);
	
	m_nWndWidth = m_pMainDib->GetWidth();
	m_nWndHeight= m_pMainDib->GetHeight();
	m_sFileName = strFile;

	m_rFaceRegion.left = m_rFaceRegion.right = m_rFaceRegion.top = m_rFaceRegion.bottom = 0;

	m_bLeftEyeOK = m_bRightEyeOK = m_bLeftNostrilOK = m_bRightNostrilOK =
	m_bLeftEyeLeftCornerOK = m_bLeftEyeRightCornerOK = m_bRightEyeLeftCornerOK = 
	m_bRightEyeRightCornerOK = m_bLeftMouthCornerOK = m_bRightMouthCornerOK = false;

	m_bMidMouthOK = m_bMidNoseOK = false;

	m_LeftEye = m_RightEye = m_LeftEyeLeftCorner = m_LeftEyeRightCorner = 
	m_LeftNostril = m_RightNostril = m_RightEyeLeftCorner = m_RightEyeRightCorner =
	m_LeftMouthCorner = m_RightMouthCorner = m_MidMouth = m_MidNose = CPoint(-1,-1);

	SetCursor(LoadCursor(NULL,IDC_WAIT));
	//获取像素的值
	LoadOriPixel(m_pMainDib);
	MakeBitMap();
	SetCursor(LoadCursor(NULL,IDC_ARROW));	
}

////////////////////////////////////////////////////////////////////////////////
// 画图
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::MyDraw()
{
	CRect rc;
	pDCShow->GetWindow()->GetClientRect(&rc);
	pDCShow->Rectangle(&rc);
	CDC dc;
	CBitmap *pOldBitmap;
	dc.CreateCompatibleDC(pDCShow);
	pOldBitmap=dc.SelectObject(m_pResMap);
	pDCShow->StretchBlt(0,0,m_nWndWidth,m_nWndHeight,&dc,0,0,m_nWndWidth,m_nWndHeight,SRCCOPY);
	dc.SelectObject(pOldBitmap);
	dc.DeleteDC();
}

////////////////////////////////////////////////////////////////////////////////
// 标记眼睛区域
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::OnBtnMarkEye() 
{
	int i,j;
	if(!m_bFaceOK)
	{
		AfxMessageBox("请先确定脸部区域");
		return;
	}
	CopyBitMap(m_tResPixelArray,m_tOriPixelArray);
	//确定眼睛大概区域
	int eyetop=m_rFaceRegion.top,eyebottom=m_rFaceRegion.bottom,
		eyeleft=m_rFaceRegion.left,eyeright=m_rFaceRegion.right;
	int maxV=MaxV;
	int maxH=MaxH;
	//进一步确定眼睛区域
	for(i=eyetop;i<eyebottom;i++)
		if(BtnHistogramV[i]>maxV*7/9)
		{
			eyetop=i+5;
			break;
		}
	//根据本方法特点，将所取top bottom向下移动几个单位
	for(i=eyebottom;i>=eyetop;i--)
		if(BtnHistogramV[i]>maxV*11/18)
		{
			eyebottom=i+15;
			break;
		}
	//确定左右
	int templeft=eyeleft-10;
	if(templeft<0)
		templeft=0;	
	for(i=templeft;i>=eyeright;i--)
		if(BtnHistogramV[i]>maxV*1/5)
		{
			eyeleft=i;
			break;
		}
	int tempright=eyeright+10;
	if(tempright>m_nWndWidth-1)
		tempright=m_nWndWidth-1;
	for(i=tempright;i>=eyeleft;i--)
		if(BtnHistogramV[i]>maxV*1/5)
		{
			eyeright=i;
			break;
		}
	//保存眼睛大概区域
	CRect rect(eyeleft,eyetop,eyeright,eyebottom);
	m_rEyeRegion=rect;
	//对眼睛区域提取角点
	int nLeft,nRight,nTop,nBottom;	
	nLeft=m_rEyeRegion.left;
	nRight=m_rEyeRegion.right;
	nTop=m_rEyeRegion.top;
	nBottom=m_rEyeRegion.bottom;
	if(nLeft<3)
		nLeft=3;
	if(nRight>m_nWndHeight-3)
		nRight=m_nWndHeight-3;
	if(nTop<3)
		nTop=3;
	if(nBottom>m_nWndHeight-3)
		nBottom=m_nWndHeight-3;
	//模板范围
	int OffSetX[37] = 
			 { -1, 0, 1,
			-2,-1, 0, 1, 2,
		-3,-2,-1, 0, 1, 2, 3,
		-3,-2,-1, 0, 1, 2, 3,
    	-3,-2,-1, 0, 1, 2, 3,
			-2,-1, 0, 1, 2,
			-1, 0, 1 };
	int OffSetY[37] = 
			{ -3,-3,-3,
	       -2,-2,-2,-2,-2,
		-1,-1,-1,-1,-1,-1,-1,
		 0, 0, 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1, 1, 1,
			2, 2, 2, 2, 2,
			   3, 3, 3 };
	int NearPoint[37];
	int thre, same, max, min;	
		
	BYTE **Eye=new BYTE*[nBottom-nTop+1];
	for(i=0;i<nBottom-nTop+1;i++)
		Eye[i]=new BYTE[nRight-nLeft+1];
		
	//统计象素亮度最大值和最小值
	max = min = 0; 
	for(i=nTop;i<nBottom;i++)
		for(j=nLeft;j<nRight;j++)
		{
			if(max<method1->m_pGrayData[i][j]) 
				max=method1->m_pGrayData[i][j];
			if(min>method1->m_pGrayData[i][j])
				min=method1->m_pGrayData[i][j];
		}
	//相似度阈值为最大值和最小值差的1/10
	thre = (max-min)/10;
	for(i=nTop;i<nBottom;i++)
		for(j=nLeft;j<nRight;j++)
		{
		    //统计圆形邻域内相似的点的个数
			same = 0; 
			for(int h=0;h<37;h++)
				NearPoint[h]=method1->m_pGrayData[i+OffSetY[h]][j+OffSetX[h]];
			for(h=0;h<37;h++)
				if((int)labs(NearPoint[h]-NearPoint[18])<= thre) 
					same ++;							
			if(same<5&&same>2)
			{
				Eye[i-nTop][j-nLeft]=1;		//二值化提取角点
			}	
			else
				Eye[i-nTop][j-nLeft]=0;
		}
	//取角点中灰度最大的为眼睛精确位置
	int gray=0;
	int tempx=-1,tempy=-1;
	int middle=(nLeft+nRight)/2;
	for(i=nTop;i<nBottom;i++)
		for(j=nLeft;j<middle;j++)
		{
			if(Eye[i-nTop][j-nLeft])
			{
				if(method1->m_pGrayData[i][j]>=gray)
				{
					gray=method1->m_pGrayData[i][j];
					tempx=i;
					tempy=j;
				}
			}
		}

	if(tempx!=-1&&tempy!=-1)
	{
		m_LeftEye.y=tempx;
		m_LeftEye.x=tempy;
		m_bLeftEyeOK=TRUE;
	}

	gray=0;
	tempx=-1;tempy=-1;
	for(i=nTop;i<nBottom;i++)
		for(j=middle;j<nRight;j++)
		{
			if(Eye[i-nTop][j-nLeft])
			{
				if(method1->m_pGrayData[i][j]>=gray)
				{
					gray=method1->m_pGrayData[i][j];
					tempx=i;
					tempy=j;
				}
			}
		}

	if(tempx!=-1&&tempy!=-1)
	{
  		m_RightEye.y=tempx;
		m_RightEye.x=tempy;
		m_bRightEyeOK=TRUE;
	}

	for(i=0;i<nBottom-nTop+1;i++)
		delete Eye[i];
	delete[] Eye;	
	
	MaxV=maxV;

	for(i=nTop;i<=nBottom;i++)
	{
		m_tResPixelArray[i][nLeft].rgbBlue=255;
		m_tResPixelArray[i][nLeft].rgbGreen = m_tResPixelArray[i][nLeft].rgbRed = 0;
		m_tResPixelArray[i][nRight].rgbBlue=255;
		m_tResPixelArray[i][nRight].rgbGreen = m_tResPixelArray[i][nRight].rgbRed = 0;
	}
	for(j=nLeft;j<=nRight;j++)
	{
		m_tResPixelArray[nTop][j].rgbBlue=255;
		m_tResPixelArray[nTop][j].rgbGreen = m_tResPixelArray[nTop][j].rgbRed = 0;
		m_tResPixelArray[nBottom][j].rgbBlue=255;
		m_tResPixelArray[nBottom][j].rgbGreen = m_tResPixelArray[nBottom][j].rgbRed = 0;
	}
	
	MakeBitMap();		
}
////////////////////////////////////////////////////////////////////////////////
// 标记鼻子
////////////////////////////////////////////////////////////////////////////////
void CFaceDetectDlg::OnBtnMarkNose() 
{
	int i,j;
	if(!(m_bLeftEyeOK&&m_bRightEyeOK))
	{
		AfxMessageBox("请先确定眼睛");
		return;
	}
	//左右眼的水平区域	

	int nNoseTop=m_rEyeRegion.bottom+10;
	int nNoseBottom=m_rFaceRegion.bottom;
	int nNoseLeft=m_LeftEye.x;
	int nNoseRight=m_RightEye.x;
	int maxV=MaxV;
	for(i=nNoseTop;i<nNoseBottom;i++)
		if(BtnHistogramV[i]>maxV/3)
		{
			nNoseBottom=i+10;
			break;
		}
	int gray=0;
	int tempx,tempy;
	for(i=nNoseTop;i<=nNoseBottom;i++)
		for(j=nNoseLeft;j<=nNoseRight;j++)
		{
			if(method1->m_pGrayData[i][j]>gray)
			{
				gray=method1->m_pGrayData[i][j];
				tempy=i;
				tempx=j;
			}
		}
	m_bMidNoseOK=TRUE;
	m_MidNose.x=tempx;
	m_MidNose.y=tempy;
	CRect rectnose(nNoseLeft,nNoseTop,nNoseRight,nNoseBottom);
	m_rNoseRegion=rectnose;
	CopyBitMap(m_tResPixelArray,m_tOriPixelArray);
	for(i=nNoseTop;i<=nNoseBottom;i++)
	{
		m_tResPixelArray[i][nNoseLeft].rgbGreen=255;
		m_tResPixelArray[i][nNoseLeft].rgbBlue = m_tResPixelArray[i][nNoseLeft].rgbRed = 0;
		m_tResPixelArray[i][nNoseRight].rgbGreen=255;
		m_tResPixelArray[i][nNoseRight].rgbBlue = m_tResPixelArray[i][nNoseRight].rgbRed = 0;
	}
	for(j=nNoseLeft;j<=nNoseRight;j++)
	{
		m_tResPixelArray[nNoseTop][j].rgbGreen=255;
		m_tResPixelArray[nNoseTop][j].rgbBlue = m_tResPixelArray[nNoseTop][j].rgbRed = 0;
		m_tResPixelArray[nNoseBottom][j].rgbGreen=255;
		m_tResPixelArray[nNoseBottom][j].rgbBlue = m_tResPixelArray[nNoseBottom][j].rgbRed = 0;
	}
	MakeBitMap();	
}


void CFaceDetectDlg::OnBtnLightcompensate() 
{
	// TODO: Add your control notification handler code here
	m_bShowFace = false;
	SetCursor(LoadCursor(NULL,IDC_WAIT));

	method1->CalLightCompensate();
	
	for(int i=0; i<m_nWndHeight; i++)
	for(int j=0; j<m_nWndWidth;  j++)
	{
		m_tResPixelArray[i][j].rgbBlue=method1->LightConpensate[i][j].rgbBlue;
		m_tResPixelArray[i][j].rgbRed=method1->LightConpensate[i][j].rgbRed;
		m_tResPixelArray[i][j].rgbGreen=method1->LightConpensate[i][j].rgbGreen;
	}
	MakeBitMap();

	SetCursor(LoadCursor(NULL,IDC_ARROW));	
}

void CFaceDetectDlg::OnBtnMedianFilter() 
{
	// TODO: Add your control notification handler code here
	m_bShowFace = false;
	SetCursor(LoadCursor(NULL,IDC_WAIT));

	method1->CalMedianFilter();
	
	for(int i=0; i<m_nWndHeight; i++)
	for(int j=0; j<m_nWndWidth;  j++)
	{
		m_tResPixelArray[i][j].rgbBlue=method1->MedianFilter[i][j].rgbBlue;
		m_tResPixelArray[i][j].rgbRed=method1->MedianFilter[i][j].rgbRed;
		m_tResPixelArray[i][j].rgbGreen=method1->MedianFilter[i][j].rgbGreen;
	}
	MakeBitMap();

	SetCursor(LoadCursor(NULL,IDC_ARROW));	
}

void CFaceDetectDlg::OnBtnSobel() 
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	
	if(!method1->CalSobel())
	{
		AfxMessageBox("请先计算相似度！");
		SetCursor(LoadCursor(NULL,IDC_ARROW));	
		return;
	}
	
	m_bShowFace = false;
	for(int i=0; i<m_nWndHeight; i++)
	for(int j=0; j<m_nWndWidth;  j++)
	{
		m_tResPixelArray[i][j].rgbBlue = m_tResPixelArray[i][j].rgbGreen =	
		m_tResPixelArray[i][j].rgbRed  = (int)(method1->m_pBinaryArray[i][j]*255);
	}
	MakeBitMap();
	SetCursor(LoadCursor(NULL,IDC_ARROW));	
	MyDraw();
}

void CFaceDetectDlg::OnBtnMorphological() 
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	
	if(!method1->CalMorphologicalFilter())
	{
		AfxMessageBox("请先计算相似度！");
		SetCursor(LoadCursor(NULL,IDC_ARROW));	
		return;
	}
	
	m_bShowFace = false;
	for(int i=0; i<m_nWndHeight; i++)
	for(int j=0; j<m_nWndWidth;  j++)
	{
		m_tResPixelArray[i][j].rgbBlue = m_tResPixelArray[i][j].rgbGreen =	
		m_tResPixelArray[i][j].rgbRed  = (int)(method1->m_pBinaryArray[i][j]*255);
	}
	MakeBitMap();
	SetCursor(LoadCursor(NULL,IDC_ARROW));	
	MyDraw();
}

void CFaceDetectDlg::OnBtnPreciseface() 
{
	// TODO: Add your control notification handler code here
	if(!(m_bLeftEyeOK&&m_bRightEyeOK))
	{
		AfxMessageBox("请先确定眼睛");
		return;
	}
	int i,j;
	int facetop=m_rFaceRegion.top;
	int facebottom=m_rFaceRegion.bottom;
	int faceleft=m_rFaceRegion.left;
	int faceright=m_rFaceRegion.right;
	if(  (double)(facebottom-facetop)/(faceright-faceleft)<1.5 &&m_bLeftEyeOK  &&m_bRightEyeOK )
		return;
	int maxV=0;
	int maxIndexV=0;
	int middle=(m_LeftEye.y+m_RightEye.y)/2;
	for(i=facetop;i<facebottom;i++)
		if(BtnHistogramV[i]>maxV)
		{
			maxV=BtnHistogramV[i];
			maxIndexV=i;
		}
	for(i=facebottom;i>=facetop;i--)
		if(BtnHistogramV[i-1]-BtnHistogramV[i]>maxV/10&&(double)(i-middle)/(middle-facetop)<1.5)
		{
			facebottom=i;
			break;
		}
	//重新确定脸部区域
	CRect facerect(faceleft,facetop,faceright,facebottom);
	m_rFaceRegion=facerect;
	MaxV=maxV;

	CopyBitMap(m_tResPixelArray,m_tOriPixelArray);
	for(i=facetop;i<=facebottom;i++)
	{
		m_tResPixelArray[i][faceleft].rgbGreen=255;
		m_tResPixelArray[i][faceleft].rgbBlue = m_tResPixelArray[i][faceleft].rgbRed = 0;
		m_tResPixelArray[i][faceright].rgbGreen=255;
		m_tResPixelArray[i][faceright].rgbBlue = m_tResPixelArray[i][faceright].rgbRed = 0;
	}
	for(j=faceleft;j<=faceright;j++)
	{
		m_tResPixelArray[facetop][j].rgbGreen=255;
		m_tResPixelArray[facetop][j].rgbBlue = m_tResPixelArray[facetop][j].rgbRed = 0;
		m_tResPixelArray[facebottom][j].rgbGreen=255;
		m_tResPixelArray[facebottom][j].rgbBlue = m_tResPixelArray[facebottom][j].rgbRed = 0;
	}
	MakeBitMap();
	SetCursor(LoadCursor(NULL,IDC_ARROW));	
	MyDraw();
}
