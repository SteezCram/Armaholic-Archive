// ArmAUnbinDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ArmAUnbin.h"
#include "ArmAUnbinDlg.h"
#include ".\armaunbindlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CArmAUnbinDlg dialog



CArmAUnbinDlg::CArmAUnbinDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CArmAUnbinDlg::IDD, pParent)
	, configEditValue(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CArmAUnbinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, configEditValue);
}

BEGIN_MESSAGE_MAP(CArmAUnbinDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTONOPEN, OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTONSAVE, OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CArmAUnbinDlg message handlers

BOOL CArmAUnbinDlg::OnInitDialog()
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
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CArmAUnbinDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CArmAUnbinDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CArmAUnbinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



int CArmAUnbinDlg::ReadCompressedInt(CFile &f) {
	int value = 0;
	int tmp;
	int i = 0;

	do {
		if (f.Read(&tmp, 1) != 1) return value;
		value += (tmp & 0x7f) << (i++ * 7);
	} while (tmp & 0x80);
	return value;
}

void CArmAUnbinDlg::ReadAsciiz(char *str, int n, CFile &f) {
	int i = 0;
	char c;

	*str = '\0';

	do {
		if (f.Read(&c, 1) != 1) return; 
		*str++ = c;
		i++;
	} while (i < n && c != '\0');
}

bool CArmAUnbinDlg::ReadArray(CFile &f, CString &cpp, bool recursive, int recursionLevel) {
	int nElements;
	unsigned char elementType;
	int valueInt;
	float valueFloat;
	char valueString[4096];
	CString errorMsg;

	nElements = ReadCompressedInt(f);

	cpp.AppendChar('{');
	for (int i = 0; i < nElements; i++) {
		f.Read(&elementType, sizeof(elementType));
		switch (elementType) {
			case 0:
				ReadAsciiz(valueString, 4095, f);
				cpp.AppendFormat("\"%s\"", valueString);
				break;
			case 1:
				f.Read(&valueFloat, sizeof(valueFloat));
				cpp.AppendFormat("%f", valueFloat);
				break;
			case 2:
				f.Read(&valueInt, sizeof(valueInt));
				cpp.AppendFormat("%d", valueInt);
				break;
			case 3:
				ReadArray(f, cpp, true, recursionLevel + 1);
				break;
			case 4:
				ReadAsciiz(valueString, 4095, f);
				cpp.AppendFormat("_%s", valueString);
				break;
			default:
				errorMsg.Format("unknown elementType %d at %08x, exiting", elementType, (int)f.GetPosition());
				MessageBox(errorMsg);
				return false;
		}
		if (i < nElements - 1) cpp.Append(", ");
	}
	cpp.AppendChar('}');
	if (!recursive) cpp.Append(";\r\n");
	return true;
}


long CArmAUnbinDlg::ReadClassBody(CFile &f, CString &cpp, int recursionLevel) {
	int nEntries;
	unsigned char entryType;
	unsigned char valueType;
	long embedClassOffset;
	long nextClassOffset;
	char stringBuffer[4096];
	int valueInt;
	float valueFloat;
	ULONGLONG currPos;
	CString errorMsg;

	// class inheritance
	ReadAsciiz(stringBuffer, 4095, f);
	if (recursionLevel > 0) {
		// root level metaclass can't have name or inhertance
		if (strlen(stringBuffer) > 0) cpp.AppendFormat(": %s", stringBuffer);
		cpp.Append(" {\r\n");
	}

	nEntries = ReadCompressedInt(f);

	for (int i = 0; i < nEntries; i++) {
		f.Read(&entryType, sizeof(entryType));
		switch (entryType) {
			// embedded class
			case 0:
				ReadAsciiz(stringBuffer, 4095, f);
				cpp.Append("\r\n");
				for (int t = 0; t < recursionLevel; t++) cpp.AppendChar('\t');
				cpp.AppendFormat("class %s", stringBuffer);
				f.Read(&embedClassOffset, sizeof(embedClassOffset));
				currPos = f.GetPosition();
				f.Seek(embedClassOffset, CFile::begin);
				if (ReadClassBody(f, cpp, recursionLevel + 1) == 0) return 0;
				f.Seek(currPos, CFile::begin);
				break;
			case 1:
				// variable
				f.Read(&valueType, sizeof(valueType));
				ReadAsciiz(stringBuffer, 4095, f);
				for (int t = 0; t < recursionLevel; t++) cpp.AppendChar('\t');
				cpp.AppendFormat("%s = ", stringBuffer);
				switch (valueType) {
					case 0:
						// string variable
						ReadAsciiz(stringBuffer, 4095, f);
						cpp.AppendFormat("\"%s\";\r\n", stringBuffer);
						break;
					case 1:
						// float
						f.Read(&valueFloat, sizeof(valueFloat));
						cpp.AppendFormat("%f;\r\n", valueFloat);
						break;
					case 2:
						// integer
						f.Read(&valueInt, sizeof(valueInt));
						cpp.AppendFormat("%d;\r\n", valueInt);
						break;
					case 4:
						// variable reference
						ReadAsciiz(stringBuffer, 4095, f);
						cpp.AppendFormat("_%s;\r\n", stringBuffer);
						break;
					default:
						errorMsg.Format("unknown valueType %d found at %08x", valueType, (long)f.GetPosition());
						MessageBox(errorMsg);
						return 0;
				}
				break;
			case 2:
				// array
				ReadAsciiz(stringBuffer, 4095, f);
				for (int t = 0; t < recursionLevel; t++) cpp.AppendChar('\t');
				cpp.AppendFormat("%s = ", stringBuffer);
				if (!ReadArray(f, cpp, false, recursionLevel + 1)) return 0;
				break;
			case 3:
				// external class reference
				ReadAsciiz(stringBuffer, 4095, f);
				for (int t = 0; t < recursionLevel; t++) cpp.AppendChar('\t');
				cpp.AppendFormat("/*extern*/ class %s;\r\n", stringBuffer);
				break;
			case 4:
				// external class reference
				ReadAsciiz(stringBuffer, 4095, f);
				for (int t = 0; t < recursionLevel; t++) cpp.AppendChar('\t');
				cpp.AppendFormat("/*extern*/ class %s;\r\n", stringBuffer);
				break;
			default:
				errorMsg.Format("unknown entryType %d found at %08x", entryType, (long)f.GetPosition());
				MessageBox(errorMsg);
				return 0;
		}
	}
	f.Read(&nextClassOffset, sizeof(nextClassOffset));
	if (recursionLevel > 0) {
		for (int t = 0; t < recursionLevel - 1; t++) cpp.AppendChar('\t');
		cpp.Append("};\r\n");
	}
	return nextClassOffset;
}

void CArmAUnbinDlg::ReadEnumList(CFile &f, CString &cpp) {
	int enumEntries;
	char enumLabel[4096];
	int enumValue;

	f.Read(&enumEntries, sizeof(enumEntries));

	if (enumEntries <= 0) return;

	cpp.Append("\r\nenum {\r\n");
	for (int i = 0; i < enumEntries; i++) {
		ReadAsciiz(enumLabel, 4095, f);
		f.Read(&enumValue, sizeof(enumValue));
		cpp.AppendFormat("\t%s = %d", enumLabel, enumValue);
		if (i < enumEntries - 1) cpp.AppendChar(',');
		cpp.Append("\r\n");
	}
	cpp.Append("}\r\n");
}


void CArmAUnbinDlg::ReadConfigBin(CString filename, CString &cpp) {
	CFile bin;
	CFileException ex;
	unsigned int rapSignature = 0x50617200;
	unsigned int standardField1 = 0x00000000;
	unsigned int standardField2 = 0x00000008;
	unsigned int tmp_uint, tmp_uint2;
	long enumListcountOffset;
	long nextClass;
	CString errorMsg;

	if (!bin.Open(filename, CFile::modeRead, &ex)) {
			ex.ReportError();
			bin.Close();
			return;
	}

	bin.Read(&tmp_uint, sizeof(tmp_uint));
	if (tmp_uint != rapSignature) {
		errorMsg.Format("This is not valid raP file.\r\nWas expecting signature %08x, got %08x\n", rapSignature, tmp_uint);
		MessageBox(errorMsg);
		bin.Close();
		return;
	}

	bin.Read(&tmp_uint, sizeof(tmp_uint));
	bin.Read(&tmp_uint2, sizeof(tmp_uint));

	if (tmp_uint != standardField1 && tmp_uint2 != standardField2) bin.Seek(16, CFile::current);

	bin.Read(&enumListcountOffset, sizeof(enumListcountOffset));

	nextClass = ReadClassBody(bin, cpp, 0);
	if (nextClass == 0) {
		// error occurred, don't go further
		//MessageBox("Error occurred during processing input file");
		bin.Close();
		return;
	}
	bin.Seek(nextClass, CFile::begin);

	ReadEnumList(bin, cpp);

	bin.Close();
}

void CArmAUnbinDlg::OnBnClickedButtonOpen()
{
	CFileDialog filedialog(TRUE, ".bin", NULL, OFN_FILEMUSTEXIST, "BIN Files(*.bin)|*.bin|All Files (*.*)|*.*||");
	if (filedialog.DoModal() == IDOK) {
		CString output = "";
		ReadConfigBin(filedialog.GetPathName(), output);
		configEditValue = output;
		UpdateData(FALSE);
	}
}

void CArmAUnbinDlg::OnBnClickedButtonSave()
{
	UpdateData(TRUE);
	CFileDialog filedialog(FALSE, ".cpp", "config.cpp", OFN_OVERWRITEPROMPT, "CPP Files(*.cpp)|*.cpp|All Files (*.*)|*.*||");
	if (filedialog.DoModal() == IDOK) {
		CFileException ex;
		CFile out;
		
		if (!out.Open(filedialog.GetPathName(), CFile::modeWrite | CFile::shareExclusive | CFile::modeCreate, &ex)) {
            ex.ReportError();
			return;
		}
		LPCTSTR outBuf = configEditValue;
		out.Write(outBuf, configEditValue.GetLength());
		out.Close();
	}
}
