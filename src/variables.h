#pragma once

class ConditionFlag;

class TfrmVariables : public nlib::Form
{
	friend class TfrmMain;
public:
	TfrmVariables();
	virtual void Destroy();

	FalconCalc::WindowSide GetSnapSide();		// returns area index: 0: none 1 top, 2 right, 3 bottom, 4 left	
	void Snap();				// snap this window to the side of the main window usin'_snappedSide' and '_snapDist'
	constexpr bool Snapped() const { return _snappedToSide != FalconCalc::wsNone; }
	constexpr FalconCalc::WindowSide SnappedSide() const { return _snappedToSide; }

N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::TabControl *tcVars;
	nlib::StringGrid *sgUser;
	nlib::Panel *pBuiltin;
	nlib::StringGrid *sgBuiltin;
	nlib::FlatButton *btnDelVar;
	nlib::FlatButton *btnClear;
	nlib::Button *btnCancel;
	nlib::Button *btnSave;

	void btnSaveClick(void *sender, nlib::EventParameters param);
	void btnCancelClick(void *sender, nlib::EventParameters param);
	void btnClearClick(void *sender, nlib::EventParameters param);
	void btnDelVarClick(void *sender, nlib::EventParameters param);
	void BuiltinMouseUp(void *sender, nlib::MouseButtonParameters param);
	void BuiltinMouseDown(void *sender, nlib::MouseButtonParameters param);
	void BuiltinMouseMove(void *sender, nlib::MouseMoveParameters param);
	void FormClose(void *sender, nlib::FormCloseParameters param);
	void FormSizeMoveEnded(void* sender, nlib::SizePositionChangedParameters param);
	void sgBuiltinColumnSizing(void *sender, nlib::ColumnRowSizeParameters param);
	void sgUserEditorKeyDown(void *sender, nlib::KeyParameters param);
	void sgUserKeyPress(void *sender, nlib::KeyPressParameters param);
	void sgUserDoubleClick(void* sender, nlib::MouseButtonParameters param);
	void sgBuiltinDoubleClick(void* sender, nlib::MouseButtonParameters param);
	void sgUserColumnSizing(void *sender, nlib::ColumnRowSizeParameters param);
	void tcVarsTabChange(void *sender, nlib::TabChangeParameters param);
	void VariablesKeyDown(void* sender, nlib::KeyParameters param);
protected:
	virtual ~TfrmVariables(); /* Don't make public. Call Destroy() to delete the object. */ 
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
	bool _changed[2] = { false, false };	// so that at creation data would be stored in the string vectors	
	ConditionFlag _busy;					// e.g. don't handle snap
	bool _underResize = false;	// user/builtin function pane
	int  _gridH;				// size of user var/function grid during resize
	int _activeTab = -1;		// not set yet

	struct _RowData
	{
		SmartString cols[4];	// name, body, descr, unit

		_RowData() {}
		_RowData(const _RowData& o) { *this = o; }
		_RowData(SmartString name, SmartString body, SmartString descr, SmartString unit)
		{
			cols[0] = name;
			cols[1] = body;
			cols[2] = descr;
			cols[3] = unit;
		}
		void clear() { cols[0] = cols[1] = cols[2] = cols[3] = u""; }
		_RowData& operator=(const _RowData& o)
		{
			cols[0] = o.cols[0];
			cols[1] = o.cols[1];
			cols[2] = o.cols[2];
			cols[3] = o.cols[3];
			return *this;
		}
		bool operator!=(const _RowData &rd)
		{
			for (int i = 0; i < 4; ++i)
				if (cols[i] != rd.cols[i])
					return true;
			return false;
		}
		bool operator==(const _RowData &rd)
		{
			return !(*this != rd);
		}
		SmartString Serialize();
	};
	typedef DataMap<_RowData, SmartString> _RowDataMap;

	_RowDataMap _rvUserVars, _rvUserFuncs, _rvUserVarsIn, _rvUserFuncsIn;

	FalconCalc::WindowSide _snappedToSide = FalconCalc::wsNone;
	int _snapPixelLimit = 30;	// pixels snap if inside this distance from, main window
	int _snapDist = 0;			// when '_snapped' is true: distance from '_snappedToSide'
	static int _colW[2][4];		// column widths

	void _CollectFrom(int index);	// from index-th grid to RowData vector
	void _SetupGridLayout(int tabIndex = 0);	// 0: functions, 1: variables
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmVariables *frmVariables;
