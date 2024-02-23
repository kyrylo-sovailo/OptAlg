#include <wx/wx.h>
#include <wx/event.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>

namespace opt
{
    class Frame : public wxFrame
    {
    private:
        //Mode
        wxChoice *_selector_mode;

        //Problem
        wxTextCtrl *_edit_box_size = nullptr;
        wxTextCtrl *_edit_item_number = nullptr;
        wxTextCtrl *_edit_item_size_min = nullptr;
        wxTextCtrl *_edit_item_size_max = nullptr;
        wxTextCtrl *_edit_seed = nullptr;
        wxTextCtrl *_edit_window = nullptr;
        wxTextCtrl *_edit_hwindow = nullptr;
        wxTextCtrl *_edit_desired_iter = nullptr;

        //Solution
        wxTextCtrl *_edit_iter_max = nullptr;
        wxTextCtrl *_edit_time_max = nullptr;

        //Technical
        wxBoxSizer *_sizer = nullptr;
        wxButton *_button_run = nullptr;
        wxButton *_button_next = nullptr;
        wxButton *_button_previous = nullptr;
        wxPanel *_panel_display = nullptr;
        wxScrollBar *_scroll_scroll = nullptr;
        wxStaticText *_text_iteration = nullptr;

        void OnRun(wxCommandEvent &event);
        void OnNext(wxCommandEvent &event);
        void OnPrevious(wxCommandEvent &event);
        void OnScroll(wxScrollEvent &event);
        void OnPaint(wxPaintEvent &event);

    public:
        Frame();
    };

    class App : public wxApp
    {
    public:
        bool OnInit() override;
    };
}

opt::Frame::Frame() : wxFrame(nullptr, wxID_ANY, "Optimization algorithms")
{
    _sizer = new wxBoxSizer(wxVERTICAL);
	
    //Mode
    const wxString choices[] = { "Greedy (area)", "Greedy (largest side)", "Greedy (smallest side)",
        "Local search (geometry)", "Local search (order)", "Local search (geometry/overlaps)" };
    _sizer->Add(_selector_mode = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 6, choices), 0, wxEXPAND);

    //Problem
    _sizer->Add(_edit_box_size = new wxTextCtrl(this, wxID_ANY, "10"), 0, wxEXPAND);
    _sizer->Add(_edit_item_number = new wxTextCtrl(this, wxID_ANY, "100"), 0, wxEXPAND);
    _sizer->Add(_edit_item_size_min = new wxTextCtrl(this, wxID_ANY, "1"), 0, wxEXPAND);
    _sizer->Add(_edit_item_size_max = new wxTextCtrl(this, wxID_ANY, "7"), 0, wxEXPAND);
    _sizer->Add(_edit_seed = new wxTextCtrl(this, wxID_ANY, "0"), 0, wxEXPAND);
    _sizer->Add(_edit_window = new wxTextCtrl(this, wxID_ANY, "1"), 0, wxEXPAND);
    _sizer->Add(_edit_hwindow = new wxTextCtrl(this, wxID_ANY, "0"), 0, wxEXPAND);
    _sizer->Add(_edit_desired_iter = new wxTextCtrl(this, wxID_ANY, "100"), 0, wxEXPAND);

    //Solution
    _sizer->Add(_edit_iter_max = new wxTextCtrl(this, wxID_ANY, "inf"), 0, wxEXPAND);
    _sizer->Add(_edit_time_max = new wxTextCtrl(this, wxID_ANY, "inf"), 0, wxEXPAND);

    //Technical
    _sizer->Add(_button_run = new wxButton(this, wxID_ANY, "Run"), 0, wxEXPAND);
    _sizer->Add(_button_next = new wxButton(this, wxID_ANY, "Next"), 0, wxEXPAND);
    _sizer->Add(_button_previous = new wxButton(this, wxID_ANY, "Previous"), 0, wxEXPAND);
    _sizer->Add(_panel_display = new wxPanel(this, wxID_ANY), 1, wxEXPAND);
    _sizer->Add(_scroll_scroll = new wxScrollBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL), 0, wxEXPAND);
    _sizer->Add(_text_iteration = new wxStaticText(this, wxID_ANY, "Iteration: N/A"), 0, wxEXPAND);
    Bind(wxEVT_BUTTON, &Frame::OnRun, this, _button_run->GetId());
    Bind(wxEVT_BUTTON, &Frame::OnNext, this, _button_next->GetId());
    Bind(wxEVT_BUTTON, &Frame::OnPrevious, this, _button_previous->GetId());
    _panel_display->Bind(wxEVT_PAINT, &Frame::OnPaint, this, _panel_display->GetId());
    Bind(wxEVT_SCROLL_CHANGED, &Frame::OnScroll, this, _scroll_scroll->GetId());
    _scroll_scroll->SetScrollbar(0, 10, 20, 10);
	
    SetSizer(_sizer);
    SetAutoLayout(true);
}

void opt::Frame::OnRun(wxCommandEvent &)
{
    wxLogMessage("OnRun");
}

void opt::Frame::OnNext(wxCommandEvent &)
{
    wxLogMessage("OnNext");
}

void opt::Frame::OnPrevious(wxCommandEvent &)
{
    wxLogMessage("OnPrevious");
}

void opt::Frame::OnScroll(wxScrollEvent &)
{
    wxLogMessage("OnScroll");
}

void opt::Frame::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(_panel_display);

    // draw some text
    dc.DrawText(wxT("Testing"), 40, 60); 
    
    // draw a circle
    dc.SetBrush(*wxGREEN_BRUSH); // green filling
    dc.SetPen( wxPen( wxColor(255,0,0), 5 ) ); // 5-pixels-thick red outline
    dc.DrawCircle( wxPoint(200,100), 25 /* radius */ );
    
    // draw a rectangle
    dc.SetBrush(*wxBLUE_BRUSH); // blue filling
    dc.SetPen( wxPen( wxColor(255,175,175), 10 ) ); // 10-pixels-thick pink outline
    dc.DrawRectangle( 300, 100, 400, 200 );
    
    // draw a line
    dc.SetPen( wxPen( wxColor(0,0,0), 3 ) ); // black line, 3 pixels thick
    dc.DrawLine( 300, 100, 700, 300 ); // draw line across the rectangle
}

bool opt::App::OnInit()
{
    Frame *frame = new Frame();
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(opt::App);