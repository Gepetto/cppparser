/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/logg.h
// Purpose:     Assorted wxLogXXX functions, and wxLog (sink for logs)
// Author:      Vadim Zeitlin
// Modified by:
// Created:     29/01/98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
#ifndef _WX_LOGG_H_
#  define _WX_LOGG_H_
#  if  wxUSE_GUI
class WXDLLIMPEXP_FWD_CORE wxTextCtrl;
class WXDLLIMPEXP_FWD_CORE wxLogFrame;
class WXDLLIMPEXP_FWD_CORE wxWindow;
// ----------------------------------------------------------------------------
// the following log targets are only compiled in if the we're compiling the
// GUI part (andnot just the base one) of the library, they're implemented in
// src/generic/logg.cpp *and not src/common/log.cpp unlike all the rest)
// ----------------------------------------------------------------------------
#    if  wxUSE_TEXTCTRL
// log everything to a text window (GUI only of course)
class WXDLLIMPEXP_CORE wxLogTextCtrl : public wxLog
{
public:
  wxLogTextCtrl(wxTextCtrl* pTextCtrl);
protected:
    // implement sink function
  void DoLogText(const wxString& msg) override;
private:
    // the control we use
  wxTextCtrl* m_pTextCtrl;
  wxDECLARE_NO_COPY_CLASS(wxLogTextCtrl);
};
#    endif
// ----------------------------------------------------------------------------
// GUI log target, the default one for wxWidgets programs
// ----------------------------------------------------------------------------
#    if  wxUSE_LOGGUI
class WXDLLIMPEXP_CORE wxLogGui : public wxLog
{
public:
    // ctor
  wxLogGui();
    // show all messages that were logged since the last Flush()
  void Flush() override;
protected:
  void DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo& info) override;
    // return the title to be used for the log dialog, depending on m_bErrors
    // and m_bWarnings values
  wxString GetTitle() const;
    // return the icon (one of wxICON_XXX constants) to be used for the dialog
    // depending on m_bErrors/m_bWarnings
  int GetSeverityIcon() const;
    // empty everything
  void Clear();
  wxArrayString m_aMessages;
  wxArrayInt m_aSeverity;
  wxArrayLong m_aTimes;
  bool m_bErrors, m_bWarnings, m_bHasMessages;
private:
    // this method is called to show a single log message, it uses
    // wxMessageBox() by default
  virtual void DoShowSingleLogMessage(const wxString& message, const wxString& title, int style);
    // this method is called to show multiple log messages, it uses wxLogDialog
  virtual void DoShowMultipleLogMessages(const wxArrayString& messages, const wxArrayInt& severities, const wxArrayLong& times, const wxString& title, int style);
};
#    endif
// ----------------------------------------------------------------------------
// (background) log window: this class forwards all log messages to the log
// target which was active when it was instantiated, but also collects them
// to the log window. This window has its own menu which allows the user to
// close it, clear the log contents or save it to the file.
// ----------------------------------------------------------------------------
#    if  wxUSE_LOGWINDOW
class WXDLLIMPEXP_CORE wxLogWindow : public wxLogPassThrough
{
public:
  wxLogWindow(wxWindow* pParent, const wxString& szTitle, bool bShow = true, bool bPassToOld = true);
  virtual ~wxLogWindow();
    // window operations
        // show/hide the log window
  void Show(bool bShow = true);
        // retrieve the pointer to the frame
  wxFrame* GetFrame() const;
    // overridables
        // called if the user closes the window interactively, will not be
        // called if it is destroyed for another reason (such as when program
        // exits) - return true from here to allow the frame to close, false
        // to prevent this from happening
  virtual bool OnFrameClose(wxFrame* frame);
        // called right before the log frame is going to be deleted: will
        // always be called unlike OnFrameClose()
  virtual void OnFrameDelete(wxFrame* frame);
protected:
  void DoLogTextAtLevel(wxLogLevel level, const wxString& msg) override;
private:
  wxLogFrame* m_pLogFrame;
  wxDECLARE_NO_COPY_CLASS(wxLogWindow);
};
#    endif
#  endif
#endif