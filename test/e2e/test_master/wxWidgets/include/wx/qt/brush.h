/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/brush.h
// Author:      Peter Most, Javier Torres, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
#ifndef _WX_QT_BRUSH_H_
#  define _WX_QT_BRUSH_H_
class QBrush;
class WXDLLIMPEXP_CORE wxBrush : public wxBrushBase
{
public:
  wxBrush();
  wxBrush(const wxColour& col, wxBrushStyle style = wxBRUSHSTYLE_SOLID);
  wxDEPRECATED_MSG("use wxBRUSHSTYLE_XXX constants") wxBrush(const wxColour& col, int style);
  wxBrush(const wxBitmap& stipple);
  void SetColour(const wxColour& col) override;
  void SetColour(unsigned char r, unsigned char g, unsigned char b) override;
  void SetStyle(wxBrushStyle style) override;
  void SetStipple(const wxBitmap& stipple) override;
  bool operator==(const wxBrush& brush) const;
  bool operator!=(const wxBrush& brush) const
  {
    return !(*this == brush);
  }
  wxColour GetColour() const override;
  wxBrushStyle GetStyle() const override;
  wxBitmap* GetStipple() const override;
  wxDEPRECATED_MSG("use wxBRUSHSTYLE_XXX constants") void SetStyle(int style)
  {
    SetStyle((wxBrushStyle) style);
  }
  QBrush GetHandle() const;
protected:
  wxGDIRefData* CreateGDIRefData() const override;
  wxGDIRefData* CloneGDIRefData(const wxGDIRefData* data) const override;
  wxDECLARE_DYNAMIC_CLASS(wxBrush);
};
#endif