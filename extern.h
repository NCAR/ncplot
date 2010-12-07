/*
-------------------------------------------------------------------------
OBJECT NAME:	extern.h

FULL NAME:	Global Variable External Declarations

DESCRIPTION:	

AUTHOR:		cjw@ucar.edu
-------------------------------------------------------------------------
*/

#ifndef EXTERN_H
#define EXTERN_H

extern const size_t	BUFFSIZE, MAX_DATAFILES, MAX_DATASETS, MAX_PANELS;

extern bool	Interactive, DataChanged, AsciiWinOpen, Statistics,
		ScatterPlot, WindBarbs, UTCseconds, Color, StatsWinOpen,
		LandMarks, ProjectToXY, ProjectToBack, RealTime,
		Freeze, allLabels;

extern std::string DataPath, tasVarName;
extern char	buffer[], *parmsFile, *outFile, *timeSeg;

extern const char	*statsTitle, *prelimWarning;

extern DATAFILE_INFO	dataFile[];
extern PLOT_INFO	mainPlot[], specPlot, xyzPlot, xyyPlot[], diffPlot;
extern DATASET_INFO	dataSet[], ui, vi, xyzSet[], xyXset[], xyYset[],
			diffSet;

extern int	PlotType, UserStartTime[], UserEndTime[], MinStartTime[],
                MaxEndTime[], ShowRegression;
extern size_t	CurrentDataFile, CurrentDataSet, CurrentPanel,
		NumberDataFiles, NumberDataSets, NumberXYYsets, NumberXYXsets,
		NumberOfPanels, NumberOfXYpanels, nASCIIpoints;

extern size_t	NumberSeconds, nDirectionArrows, nTimeStamps, LineThickness;

extern double	regretCo[];


/* Parameter File Variables	*/
extern char	asciiFormat[];

/* X vars	*/
extern Widget	varList;

extern instanceRec	iv;


/* Procedures	*/
char	*get_legend(), *ExtractFileDialogFilter();
void	*GetMemory(size_t nb), FreeMemory(void *p);
int	DeleteVariable(DATASET_INFO *, size_t, const char *), isAverage(),
	whichSide(), choosingXaxis(), choosingYaxis(), choosingZaxis(),
	yLegendX(PLOT_INFO *, int row), yLegendPS(PLOT_INFO *, int row),
	LoadVariable(DATASET_INFO *, std::string varName);

void	Initialize(), ProcessArgs(char **argv), ReadConfigFile();

void	CreateErrorBox(Widget w), ShowError(const char *);
void	CreateWarningBox(Widget w),
	WarnUser(const char *, XtCallbackProc, XtCallbackProc);
void	CreateQueryBox(Widget w),
	QueryUser(const char *, int, XtCallbackProc),
	ExtractAnswer(char *);
void	CreateFileSelectionBox(Widget w),
	QueryFile(const char *, const char *, XtCallbackProc),
	ExtractFileName(XmString, char **);

void	QueryCancel(Widget, XtPointer, XtPointer),
	FileCancel(Widget, XtPointer, XtPointer),
	CancelWarning(Widget, XtPointer, XtPointer),
	ErrorOK(Widget, XtPointer, XtPointer);

unsigned long	GetColor(int), NextColor(), CurrentColor();
float	*GetColorRGB_PS(int), *NextColorRGB_PS(), *CurrentColorRGB_PS();
void	PushColor(), PopColor();

void	NewDataFile(Widget, XtPointer, XtPointer),
	AddDataFile(Widget, XtPointer, XtPointer),
	SetDataFile(Widget, XtPointer, XmToggleButtonCallbackStruct *),
	SaveTemplate(Widget, XtPointer, XtPointer),
	LoadTemplate(Widget, XtPointer, XtPointer),
	AddVariable(DATASET_INFO *, const char *), SetTimeText(),
	FromSecondsSinceMidnite(int timeSeg[]),
	GetTimeInterval(int InputFile, DATAFILE_INFO *),
	ReadData(), ReduceData(int start, int newNumberSeconds);

void	Quit(Widget, XtPointer, XtPointer),
	DismissWindow(Widget, XtPointer, XtPointer),
	ModifyActiveVars(Widget, XtPointer, XtPointer),
	ToggleColor(Widget, XtPointer, XtPointer),
	ToggleLabels(Widget, XtPointer, XtPointer),
	ToggleGrid(Widget, XtPointer, XtPointer),
	ToggleTracking(Widget, XtPointer, XtPointer),
	ToggleGeoPolMap(Widget, XtPointer, XtPointer),
	ToggleUTC(Widget, XtPointer, XtPointer),
	ToggleArrows(Widget, XtPointer, XtPointer),
	ToggleProject(Widget, XtPointer, XtPointer),
	ToggleScatter(Widget, XtPointer, XtPointer),
	ToggleWindBarbs(Widget, XtPointer, XtPointer),
	ToggleLandMarks(Widget, XtPointer, XtPointer),
	ToggleEqualScaling(Widget, XtPointer, XtPointer),
	ChangeLineThickness(Widget, XtPointer, XtPointer),
	EditPrintParms(Widget, XtPointer, XtPointer),
	EditMainParms(Widget, XtPointer, XtPointer),
	EditXYParms(Widget, XtPointer, XtPointer),
	EditDiffParms(Widget, XtPointer, XtPointer),
	EditStatsParms(Widget, XtPointer, XtPointer),
	EditTrackParms(Widget, XtPointer, XtPointer),
	EditSpecParms(Widget, XtPointer, XtPointer),
	EditPreferences(Widget, XtPointer, XtPointer),
	OpenControlWindow(Widget, XtPointer, XtPointer),
	PrintPS(Widget, XtPointer, XtPointer),
	SavePNG(Widget, XtPointer, XtPointer),
        SavePNGdiff(Widget, XtPointer, XtPointer),
	SavePNGspec(Widget, XtPointer, XtPointer),
	GetDataFileName(Widget, XtPointer, XtPointer),
	GetExpression(Widget w, XtPointer client, XtPointer),
	AcceptExpressions(Widget w, XtPointer client, XtPointer),
	ClearPlot(Widget w, XtPointer client, XtPointer),
	ClearRegression(Widget w, XtPointer client, XtPointer),
	LinearRegression(Widget w, XtPointer client, XtPointer),
	PolyRegression(Widget w, XtPointer client, XtPointer);

void	initPlotGC(PLOT_INFO *),
	copyGC(PLOT_INFO *dest, PLOT_INFO *src),
	ClearPixmap(PLOT_INFO *),
	NewPixmap(PLOT_INFO *, int width, int height, int depth),
	InitializeColors(PLOT_INFO *),
	plotTitlesX(PLOT_INFO *, int sizeOffset, bool displayWarning),
	plotLabelsX(PLOT_INFO *, int sizeOffset),
	plotWarningX(PLOT_INFO *, int sizeOffset),
	setClippingX(PLOT_INFO *),
	yTicsLabelsX(PLOT_INFO *, XFontStruct *, int, bool),
	xTicsLabelsX(PLOT_INFO *, XFontStruct *, bool),
	xLogTicsLabelsX(PLOT_INFO *, XFontStruct *, bool),
	xLogTicsLabelsPS(FILE *fp, PLOT_INFO *plot, bool labels),
	yLogTicsLabelsX(PLOT_INFO *, XFontStruct *, int, bool),
	yLogTicsLabelsPS(FILE *, PLOT_INFO *, int, bool),
	ComputeExp(DATASET_INFO *set),
	GetExpression(Widget w, XtPointer client, XtPointer call),
	AcceptExpressions(Widget w, XtPointer client, XtPointer call),
	SetColorNames(char *str), SetTemplateDirectory(char s[]),
	SetTemplateFile(char s[]), *GetPrinterList(void *arg),
	DrawMainWindow(), SetMainDefaults(), UpdateAnnotationsX(PLOT_INFO *),
	PrintTimeSeries(), PrintXY(), PrintXYZ(),
	UpdateAnnotationsPS(PLOT_INFO *, FILE *),
	PSclearClip(FILE *), PSclip(FILE *, PLOT_INFO *),
	plotTimeSeries(PLOT_INFO *, DATASET_INFO *), ClearAnnotations(),
	plotXY(PLOT_INFO *, DATASET_INFO *, DATASET_INFO *, int);

void	ApplyParms(Widget *, PLOT_INFO *),
	ApplyLogInvert(Widget *, PLOT_INFO *, int),
	SetDefaults(Widget *, PLOT_INFO *),
	SetLogInvert(Widget *, PLOT_INFO *, int);
		
void	SetYdialog(), SetYlabel(const char *s), CreateParmsWindow(Widget parent),
	SetSubtitles(), SetDiffDefaults(), SetSpecDefaults(), SetXYDefaults(),
	SetTrackDefaults(), SetPrinterShape(int), SetXlabel(const char *),
	SetPlotShape(PLOT_INFO *, int), SetActivePanels(size_t);

void	PageForward(Widget, XtPointer, XtPointer),
	PageBackward(Widget, XtPointer, XtPointer);

void	TrackOptWinControl(),
	DiffWinUp(Widget, XtPointer, XtPointer),
	DiffWinDown(Widget, XtPointer, XtPointer),
	ToggleTimeStamps(Widget, XtPointer, XtPointer),
	xyyWinUp(Widget, XtPointer, XtPointer),
	xyyWinDown(Widget, XtPointer, XtPointer),
	TrackWinUp(Widget, XtPointer, XtPointer),
	TrackWinDown(Widget, XtPointer, XtPointer),
	SpecWinDown(Widget, XtPointer, XtPointer),
	SpecWinUp(Widget, XtPointer, XtPointer);

void	specPostScript(Widget, XtPointer, XtPointer),
	diffPostScript(Widget, XtPointer, XtPointer);

void	Zoom(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	UnZoom(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	CanvasInput(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	CanvasMotion(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	SetCursorXY(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	ProcessText(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	ExposeMainWindow(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	ResizeMainWindow(Widget, XtPointer, XtPointer),
	ResizeSpecWindow(Widget, XtPointer, XtPointer),
	ResizeDiffWindow(Widget, XtPointer, XtPointer),
	ForkNetscape(Widget, XtPointer, XtPointer),
	DrawGeoPolMapXY(PLOT_INFO *, FILE *),
	DrawGeoPolMapXYZ(PLOT_INFO *, int, float, float, FILE *),
	PlotSpectrum(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	PlotDifference(Widget, XtPointer, XmDrawingAreaCallbackStruct *),
	PlotWindBarbs(PLOT_INFO *, FILE *),
	PlotLandMarks3D(PLOT_INFO *, int, float, float, FILE *),
	PlotDirectionArrow(PLOT_INFO *, int, int, int, int, FILE *),
	PlotTimeStamps(PLOT_INFO *, int, int, int, FILE *),
	PlotVarianceX(PLOT_INFO *plot, XFontStruct *fontInfo),
	PlotVariancePS(PLOT_INFO *plot, FILE *fp),
	PlotLandMarksXY(PLOT_INFO *, FILE *);

void	SetCurrentPanel(Widget, XtPointer, XmToggleButtonCallbackStruct *),
	ClearPanel(Widget, XtPointer, XtPointer),
	AddPanel(Widget, XtPointer, XtPointer),
	DeletePanel(Widget, XtPointer, XtPointer);

void	ViewASCII(Widget w, XtPointer client, XtPointer call),
	ViewTitles(Widget w, XtPointer client, XtPointer call),
	ViewHeader(Widget w, XtPointer client, XtPointer call),
	ViewStats(Widget w, XtPointer client, XtPointer call),
	SetASCIIdata(Widget w, XtPointer client, XtPointer call);

void	ComputeStats(DATASET_INFO *), SetStatsData(), ComputeDiff(),
	SetList(Widget w, XtPointer client, XtPointer call),
	SetSegLen(Widget w, XtPointer client, XtPointer call),
	SetWindow(Widget w, XtPointer client, XtPointer call),
	SetDetrend(Widget w, XtPointer client, XtPointer call);

void	AutoScale(), AutoScaleSpec(), AutoScaleXY(), AutoScaleDiff(),
	AutoScaleXYZ();

void	ResizeTimeSeries(), ResizeXY(), ResizeXYZ(), DrawTimeSeries(),
	DrawXY(), DrawXYZ(), ResetColors(), WaitCursorAll(),
	PointerCursorAll();;

void	WaitCursor(Widget), TextCursor(Widget), CrossHairCursor(Widget),
	GrabCursor(Widget), PointerCursor(Widget);

void	HandleError(const char s[], bool interactiv, char status);
void	FreeDataSets(void);
void	SortTable(char **table, int beg, int end);

void	ValidateTime(Widget w, XtPointer client, XtPointer call),
	ValidateFloat(Widget w, XtPointer client, XtPointer call),
	ValidateInteger(Widget w, XtPointer client, XtPointer call);

void	SetYlabels(PLOT_INFO *plot, DATASET_INFO *set, size_t nSets),
	SetXlabels(PLOT_INFO *plot, DATASET_INFO *set, size_t nSets);

int	MakeTicLabel(char buffer[], float diff, int majorTics, double value),
	MakeLogTicLabel(char buffer[], int value),
	MakeTimeTicLabel(char buffer[], int indx, int nTics),
	MakeXYlegendLabel(char buf[], DATASET_INFO *set);

int	SearchTable(std::vector<VARTBL *>& table, std::string target);

void	createPanelButts(Widget, std::vector<Widget>&, XtCallbackProc);
Widget	createParamsTitles(Widget, Widget *),
	createParamsLabels(Widget, Widget *, PLOT_INFO *),
	createParamsMinMax(Widget, Widget *, PLOT_INFO *, Widget *),
	createParamsTics(Widget, Widget *, PLOT_INFO *, Widget *),
	createLogInvert(Widget, Widget *, XtCallbackProc, PLOT_INFO *, int),
	createARDbuttons(Widget);

bool	isMissingValue(float, float);

#endif

/* END EXTERN.H */
