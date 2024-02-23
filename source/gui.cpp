#include "../include/optalg/greedy.hpp"
#include "../include/optalg/neighborhood.hpp"
#include "../include/optalg/boxing_greedy.h"
#include "../include/optalg/boxing_neighborhood.h"

#include <wx/wx.h>
#include <wx/gdicmn.h>
#include <wx/event.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include <wx/display.h>

#include <cmath>
#include <exception>
#include <limits>
#include <stdexcept>
#include <string>
#include <memory>

namespace opt
{
    unsigned int parse_uint(const wxTextCtrl *text, const char *error_message);
    double parse_double(const wxTextCtrl *text, const char *error_message);

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
        wxStaticText *_text_time = nullptr;

        //Logic
        int _mode;
        std::unique_ptr<opt::Boxing> _boxing;
        std::vector<std::vector<Boxing::Box>> _log;
        unsigned int _iteration;

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

unsigned int opt::parse_uint(const wxTextCtrl *text, const char *error_message)
{
    std::string s = text->GetValue().ToStdString();
    if (s == "inf") return std::numeric_limits<unsigned int>::max();
    char *end;
    unsigned int result = strtoul(s.c_str(), &end, 10);
    if (*end != '\0') throw std::runtime_error(error_message);
    return result;
}

double opt::parse_double(const wxTextCtrl *text, const char *error_message)
{
    std::string s = text->GetValue().ToStdString();
    if (s == "inf") return std::numeric_limits<double>::infinity();
    char *end;
    double result = strtod(s.c_str(), &end);
    if (*end != '\0') throw std::runtime_error(error_message);
    return result;
}

opt::Frame::Frame() : wxFrame(nullptr, wxID_ANY, "Optimization algorithms")
{
    _sizer = new wxBoxSizer(wxVERTICAL);
	
    //Mode
    const wxString choices[] = { "Greedy (area)", "Greedy (largest side)", "Greedy (smallest side)",
        "Local search (geometry)", "Local search (order)", "Local search (geometry/overlaps)" };
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Mode:"), 0, wxEXPAND);
    _sizer->Add(_selector_mode = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 6, choices), 0, wxEXPAND);
    _selector_mode->SetSelection(0);

    //Problem
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Box size:"), 0, wxEXPAND);
    _sizer->Add(_edit_box_size = new wxTextCtrl(this, wxID_ANY, "10"), 0, wxEXPAND);
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Item number:"), 0, wxEXPAND);
    _sizer->Add(_edit_item_number = new wxTextCtrl(this, wxID_ANY, "100"), 0, wxEXPAND);
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Minimal item size:"), 0, wxEXPAND);
    _sizer->Add(_edit_item_size_min = new wxTextCtrl(this, wxID_ANY, "1"), 0, wxEXPAND);
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Maximal item size:"), 0, wxEXPAND);
    _sizer->Add(_edit_item_size_max = new wxTextCtrl(this, wxID_ANY, "5"), 0, wxEXPAND);
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Seed:"), 0, wxEXPAND);
    _sizer->Add(_edit_seed = new wxTextCtrl(this, wxID_ANY, "0"), 0, wxEXPAND);
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Window size:"), 0, wxEXPAND);
    _sizer->Add(_edit_window = new wxTextCtrl(this, wxID_ANY, "1"), 0, wxEXPAND);
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Vertical window size:"), 0, wxEXPAND);
    _sizer->Add(_edit_hwindow = new wxTextCtrl(this, wxID_ANY, "0"), 0, wxEXPAND);
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Desired iteration:"), 0, wxEXPAND);
    _sizer->Add(_edit_desired_iter = new wxTextCtrl(this, wxID_ANY, "100"), 0, wxEXPAND);

    //Solution
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Iteration limit:"), 0, wxEXPAND);
    _sizer->Add(_edit_iter_max = new wxTextCtrl(this, wxID_ANY, "inf"), 0, wxEXPAND);
    _sizer->Add(new wxStaticText(this, wxID_ANY, "Time limit, seconds:"), 0, wxEXPAND);
    _sizer->Add(_edit_time_max = new wxTextCtrl(this, wxID_ANY, "inf"), 0, wxEXPAND);

    //Technical
    _sizer->Add(_button_run = new wxButton(this, wxID_ANY, "Run"), 0, wxEXPAND);
    _sizer->Add(_button_next = new wxButton(this, wxID_ANY, "Next"), 0, wxEXPAND);
    _sizer->Add(_button_previous = new wxButton(this, wxID_ANY, "Previous"), 0, wxEXPAND);
    _sizer->Add(_panel_display = new wxPanel(this, wxID_ANY), 1, wxEXPAND);
    _sizer->Add(_scroll_scroll = new wxScrollBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL), 0, wxEXPAND);
    _sizer->Add(_text_iteration = new wxStaticText(this, wxID_ANY, "Iteration: N/A"), 0, wxEXPAND);
    _sizer->Add(_text_time = new wxStaticText(this, wxID_ANY, "Time: N/A"), 0, wxEXPAND);
    Bind(wxEVT_BUTTON, &Frame::OnRun, this, _button_run->GetId());
    Bind(wxEVT_BUTTON, &Frame::OnNext, this, _button_next->GetId());
    Bind(wxEVT_BUTTON, &Frame::OnPrevious, this, _button_previous->GetId());
    _panel_display->Bind(wxEVT_PAINT, &Frame::OnPaint, this, _panel_display->GetId());
    Bind(wxEVT_SCROLL_CHANGED, &Frame::OnScroll, this, _scroll_scroll->GetId());
	
    SetSizer(_sizer);
    SetAutoLayout(true);
    wxDisplay display(0u);
    SetSize(2 * display.GetClientArea().GetSize() / 3);
    SetPosition(wxPoint(display.GetClientArea().GetSize().GetWidth() / 6, display.GetClientArea().GetSize().GetHeight() / 6));
}

void opt::Frame::OnRun(wxCommandEvent &)
{
    try
    {
        int mode = _selector_mode->GetSelection();
        if (mode < 0 || mode > 5) throw std::runtime_error("Invalid mode");

        //Problem
        const unsigned int box_size = parse_uint(_edit_box_size, "Invalid box size");
        const unsigned int item_number = parse_uint(_edit_item_number, "Invalid item number size");
        const unsigned int item_size_min = parse_uint(_edit_item_size_min, "Invalid minimal item size");
        const unsigned int item_size_max = parse_uint(_edit_item_size_max, "Invalid maximal item size");
        const unsigned int seed = parse_uint(_edit_seed, "Invalid seed");
        const unsigned int window = parse_uint(_edit_window, "Invalid window size");
        const unsigned int hwindow = parse_uint(_edit_hwindow, "Invalid vertical window size");
        const unsigned int desired_iter = parse_uint(_edit_desired_iter, "Invalid desired iteration");

        //Solution
        const unsigned int iter_max = parse_uint(_edit_iter_max, "Invalid iteration limit");
        const double time_max = parse_double(_edit_time_max, "Invalid time limit");

        double timer;
        if (mode >= 0 && mode <= 2)
        {
            BoxingGreedy::Metric metric;
            if (mode == 0) metric = BoxingGreedy::Metric::area;
            else if (mode == 1) metric = BoxingGreedy::Metric::max_size;
            else metric = BoxingGreedy::Metric::min_size;
            typedef BoxingGreedy Problem;
            Problem *problem = new Problem(box_size, item_number, item_size_min, item_size_max, seed, metric);
            _boxing.reset(problem);
            std::vector<Problem::Solution> log;
            Problem::Solution solution = greedy(*problem, &log, &timer);
            _log.resize(log.size());
            for (unsigned int i = 0; i < log.size(); i++) _log[i] = problem->get_boxes(log[i]);
        }
        else if (mode == 3)
        {
            typedef BoxingNeighborhoodGeometry Problem;
            Problem *problem = new Problem(box_size, item_number, item_size_min, item_size_max, seed, window, hwindow);
            _boxing.reset(problem);
            std::vector<Problem::Solution> log;
            Problem::Solution solution = neighborhood(*problem, iter_max, time_max, &log, &timer);
            _log.resize(log.size());
            for (unsigned int i = 0; i < log.size(); i++) _log[i] = problem->get_boxes(log[i]);
        }
        else if (mode == 4)
        {
            typedef BoxingNeighborhoodOrder Problem;
            Problem *problem = new Problem(box_size, item_number, item_size_min, item_size_max, seed, window);
            _boxing.reset(problem);
            std::vector<Problem::Solution> log;
            Problem::Solution solution = neighborhood(*problem, iter_max, time_max, &log, &timer);
            _log.resize(log.size());
            for (unsigned int i = 0; i < log.size(); i++) _log[i] = problem->get_boxes(log[i]);
        }
        else
        {
            typedef BoxingNeighborhoodGeometryOverlap Problem;
            Problem *problem = new Problem(box_size, item_number, item_size_min, item_size_max, seed, window, hwindow, desired_iter);
            _boxing.reset(problem);
            std::vector<Problem::Solution> log;
            Problem::Solution solution = neighborhood(*problem, iter_max, time_max, &log, &timer);
            _log.resize(log.size());
            for (unsigned int i = 0; i < log.size(); i++) _log[i] = problem->get_boxes(log[i]);
        }
        _mode = mode;
        _iteration = _log.size() - 1;
        _scroll_scroll->SetScrollbar(_iteration, _log.size() / 10, _log.size(), 10);
        _text_iteration->SetLabel("Iteration: " + std::to_string(_iteration) + "/" + std::to_string(_log.size()));
        _text_time->SetLabel("Time: " + std::to_string(timer));
    }
    catch (const std::exception &e)
    {
        wxMessageBox(e.what(), "Exception", wxICON_ERROR);
    }
}

void opt::Frame::OnNext(wxCommandEvent &)
{
    if (_iteration + 1 < _log.size())
    {
        _iteration++;
        _scroll_scroll->SetScrollbar(_iteration, _log.size() / 10, _log.size(), 10);
        _text_iteration->SetLabel("Iteration: " + std::to_string(_iteration) + "/" + std::to_string(_log.size()));
    }
}

void opt::Frame::OnPrevious(wxCommandEvent &)
{
    if (_iteration > 0)
    {
        _iteration--;
        _scroll_scroll->SetScrollbar(_iteration, _log.size() / 10, _log.size(), 10);
        _text_iteration->SetLabel("Iteration: " + std::to_string(_iteration) + "/" + std::to_string(_log.size()));
    }
}

void opt::Frame::OnScroll(wxScrollEvent &)
{
    _iteration = _scroll_scroll->GetScrollPos(wxHORIZONTAL);
    if (_iteration > _log.size() - 1) _iteration = _log.size() - 1;
    _text_iteration->SetLabel("Iteration: " + std::to_string(_iteration) + "/" + std::to_string(_log.size()));
}

void opt::Frame::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(_panel_display);
    if (_log.empty()) return;

    //Calculate sizes
    const std::vector<Boxing::Box> &boxes = _log[_iteration];
    const wxSize panel_size = _panel_display->GetSize();
    const unsigned int boxes_width = static_cast<unsigned int>(
        std::ceil(std::sqrt(1.0 * boxes.size() * panel_size.GetWidth() / panel_size.GetHeight())));
    const unsigned int boxes_height = static_cast<unsigned int>(
        std::ceil(1.0 * boxes.size() / boxes_width));
    const unsigned int box_margin_width = panel_size.GetWidth() / boxes_width;
    const unsigned int box_margin_height = panel_size.GetHeight() / boxes_height;
    const unsigned int box_size = static_cast<unsigned int>(std::min(box_margin_width / 1.1, box_margin_height / 1.1));
    const unsigned int box_offset_x = (box_margin_width - box_size) / 2;
    const unsigned int box_offset_y = (box_margin_height - box_size) / 2;

    //Clear background
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    //Draw boxes
    dc.SetBrush(*wxBLACK_BRUSH);
    for (unsigned int box_i = 0; box_i < boxes.size(); box_i++)
    {
        const unsigned int boxes_x = box_i % boxes_width;
        const unsigned int boxes_y = box_i / boxes_width;
        dc.DrawRectangle(box_margin_width * boxes_x + box_offset_x, box_margin_height * boxes_y + box_offset_y, box_size, box_size);
    }
}

bool opt::App::OnInit()
{
    Frame *frame = new Frame();
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(opt::App);