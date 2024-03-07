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
        wxButton *_button_run = nullptr;
        wxButton *_button_next = nullptr;
        wxButton *_button_previous = nullptr;
        wxPanel *_panel_display = nullptr;
        wxScrollBar *_scroll_scroll = nullptr;
        wxStaticText *_text_iteration = nullptr;
        wxStaticText *_text_time = nullptr;

        //Logic
        int _mode;                                                  //Solving mode, 0 to 5
        std::unique_ptr<opt::Boxing> _boxing;                       //Boxing problem
        std::vector<std::vector<Boxing::Box>> _log;                 //Log transformed to boxes
        std::vector<BoxingNeighborhoodOrder::Solution> _log_order;  //Log specific to order neighborhood
        unsigned int _iteration;                                    //Current iteration

        //Functions
        static unsigned int _parse_uint(const wxTextCtrl *text, const char *error_message);
        static double _parse_double(const wxTextCtrl *text, const char *error_message);
        static std::set<const Boxing::Rectangle*> _get_changes(const std::vector<Boxing::Box> &a, const std::vector<Boxing::Box> &b);
        static std::set<const Boxing::Rectangle*> _get_changes(const BoxingNeighborhoodOrder::Solution &a, const BoxingNeighborhoodOrder::Solution &b);

        //Events
        void _on_run(wxCommandEvent &event);
        void _on_next(wxCommandEvent &event);
        void _on_previous(wxCommandEvent &event);
        void _on_scroll(wxScrollEvent &event);
        void _on_paint(wxPaintEvent &event);

    public:
        Frame();
    };

    class App : public wxApp
    {
    public:
        bool OnInit() override;
    };
}

unsigned int opt::Frame::_parse_uint(const wxTextCtrl *text, const char *error_message)
{
    std::string s = text->GetValue().ToStdString();
    if (s == "inf") return std::numeric_limits<unsigned int>::max();
    char *end;
    unsigned int result = strtoul(s.c_str(), &end, 10);
    if (*end != '\0') throw std::runtime_error(error_message);
    return result;
}

double opt::Frame::_parse_double(const wxTextCtrl *text, const char *error_message)
{
    std::string s = text->GetValue().ToStdString();
    if (s == "inf") return std::numeric_limits<double>::infinity();
    char *end;
    double result = strtod(s.c_str(), &end);
    if (*end != '\0') throw std::runtime_error(error_message);
    return result;
}

std::set<const opt::Boxing::Rectangle*> opt::Frame::_get_changes(const std::vector<Boxing::Box> &a, const std::vector<Boxing::Box> &b)
{
    typedef std::vector<Boxing::Box>::const_iterator box_iter;
    typedef std::vector<Boxing::BoxedRectangle>::const_iterator rectangle_iter;
    struct Util
    {
        static void find_initial(const std::vector<Boxing::Box> &boxes, box_iter &box, rectangle_iter &rectangle)
        {
            box = boxes.cbegin();
            while (true)
            {
                if (box == boxes.cend()) break; //Fail
                rectangle = box->rectangles.begin();
                if (rectangle != box->rectangles.cend()) break; //Success
                box++;
            }
        }
        static void find_next(const std::vector<Boxing::Box> &boxes, box_iter &box, rectangle_iter &rectangle)
        {
            rectangle++;
            while (true)
            {
                if (rectangle != box->rectangles.cend()) break; //Success
                box++;
                if (box == boxes.cend()) break; //Fail
                rectangle = box->rectangles.begin();
            }
        }
        static bool valid(const std::vector<Boxing::Box> &boxes, box_iter box, rectangle_iter rectangle)
        {
            return box != boxes.cend() && rectangle != box->rectangles.cend();
        }
    };
    box_iter a_box, b_box;
    rectangle_iter a_rectangle, b_rectangle;
    Util::find_initial(a, a_box, a_rectangle);
    Util::find_initial(b, b_box, b_rectangle);
    std::set<const opt::Boxing::Rectangle*> changes;

    while (true)
    {
        const Boxing::Rectangle *a_rectangle_p = Util::valid(a, a_box, a_rectangle) ? a_rectangle->rectangle : nullptr;
        const Boxing::Rectangle *b_rectangle_p = Util::valid(b, b_box, b_rectangle) ? b_rectangle->rectangle : nullptr;

        if (a_rectangle_p == nullptr && b_rectangle_p == nullptr)
        {
            //Reached end
            break;
        }
        else if (a_rectangle_p != b_rectangle_p)
        {
            //Different rectangles
            box_iter next_a_box = a_box, next_b_box = b_box;
            rectangle_iter next_a_rectangle = a_rectangle, next_b_rectangle = b_rectangle;
            if (a_rectangle_p != nullptr) Util::find_next(a, next_a_box, next_a_rectangle);
            if (b_rectangle_p != nullptr) Util::find_next(b, next_b_box, next_b_rectangle);
            const Boxing::Rectangle *next_a_rectangle_p = Util::valid(a, next_a_box, next_a_rectangle) ? next_a_rectangle->rectangle : nullptr;
            const Boxing::Rectangle *next_b_rectangle_p = Util::valid(b, next_b_box, next_b_rectangle) ? next_b_rectangle->rectangle : nullptr;
            
            const bool insert_a = next_a_rectangle_p == b_rectangle_p;
            const bool insert_b = next_b_rectangle_p == a_rectangle_p;
            const bool swap_ab = insert_a && insert_b;
            if (insert_a)
            {
                a_box = next_a_box;
                a_rectangle = next_a_rectangle;
                if (a_rectangle_p != nullptr) changes.insert(a_rectangle_p);
            }
            if (insert_b)
            {
                b_box = next_b_box;
                b_rectangle = next_b_rectangle;
                if (b_rectangle_p != nullptr) changes.insert(b_rectangle_p);
            }
            if (swap_ab)
            {
                if (Util::valid(a, a_box, a_rectangle)) Util::find_next(a, a_box, a_rectangle);
                if (Util::valid(b, b_box, b_rectangle)) Util::find_next(b, b_box, b_rectangle);
            }
        }
        else if (a_rectangle->x != b_rectangle->x || a_rectangle->y != b_rectangle->y || a_rectangle->transposed != b_rectangle->transposed
        || static_cast<size_t>(a_box - a.begin()) != static_cast<size_t>(b_box - b.begin()))
        {
            //Different rectangle positions
            changes.insert(a_rectangle->rectangle);
            Util::find_next(a, a_box, a_rectangle);
            Util::find_next(b, b_box, b_rectangle);
        }
        else
        {
            //All same
            Util::find_next(a, a_box, a_rectangle);
            Util::find_next(b, b_box, b_rectangle);
        }
    }
    return changes;
}

std::set<const opt::Boxing::Rectangle*> opt::Frame::_get_changes(const BoxingNeighborhoodOrder::Solution &a, const BoxingNeighborhoodOrder::Solution &b)
{
    auto a_rectangle = a.begin();
    auto b_rectangle = b.begin();
    std::set<const opt::Boxing::Rectangle*> changes;
    while (true)
    {
        const Boxing::Rectangle *a_rectangle_p = a_rectangle != a.cend() ? *a_rectangle : nullptr;
        const Boxing::Rectangle *b_rectangle_p = b_rectangle != b.cend() ? *b_rectangle : nullptr;

        if (a_rectangle_p == nullptr && b_rectangle_p == nullptr)
        {
            //Reached end
            break;
        }
        else if (a_rectangle_p != b_rectangle_p)
        {
            //Different rectangles
            auto next_a_rectangle = a_rectangle + 1;
            auto next_b_rectangle = b_rectangle + 1;
            const Boxing::Rectangle *next_a_rectangle_p = next_a_rectangle != a.cend() ? *next_a_rectangle : nullptr;
            const Boxing::Rectangle *next_b_rectangle_p = next_b_rectangle != b.cend() ? *next_b_rectangle : nullptr;
            
            const bool insert_a = next_a_rectangle_p == b_rectangle_p;
            const bool insert_b = next_b_rectangle_p == a_rectangle_p;
            const bool swap_ab = insert_a && insert_b;
            if (insert_a)
            {
                a_rectangle++;
                if (a_rectangle_p != nullptr) changes.insert(a_rectangle_p);
            }
            if (insert_b)
            {
                b_rectangle++;
                if (b_rectangle_p != nullptr) changes.insert(b_rectangle_p);
            }
            if (swap_ab)
            {
                if (a_rectangle != a.cend()) a_rectangle++;
                if (b_rectangle != b.cend()) b_rectangle++;
            }
        }
        else
        {
            //Same rectangles
            if (a_rectangle != a.cend()) a_rectangle++;
            if (b_rectangle != b.cend()) b_rectangle++;
        }
    }
    return changes;
}

void opt::Frame::_on_run(wxCommandEvent &)
{
    try
    {
        int mode = _selector_mode->GetSelection();
        if (mode < 0 || mode > 5) throw std::runtime_error("Invalid mode");

        //Problem
        const unsigned int box_size = _parse_uint(_edit_box_size, "Invalid box size");
        const unsigned int item_number = _parse_uint(_edit_item_number, "Invalid item number size");
        const unsigned int item_size_min = _parse_uint(_edit_item_size_min, "Invalid minimal item size");
        const unsigned int item_size_max = _parse_uint(_edit_item_size_max, "Invalid maximal item size");
        const unsigned int seed = _parse_uint(_edit_seed, "Invalid seed");
        const unsigned int window = _parse_uint(_edit_window, "Invalid window size");
        const unsigned int hwindow = _parse_uint(_edit_hwindow, "Invalid vertical window size");
        const unsigned int desired_iter = _parse_uint(_edit_desired_iter, "Invalid desired iteration");

        //Solution
        const unsigned int iter_max = _parse_uint(_edit_iter_max, "Invalid iteration limit");
        const double time_max = _parse_double(_edit_time_max, "Invalid time limit");

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
            _log_order.resize(log.size());
            for (unsigned int i = 0; i < log.size(); i++) { _log[i] = problem->get_boxes(log[i]); _log_order[i] = log[i]; }
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

void opt::Frame::_on_next(wxCommandEvent &)
{
    if (_iteration + 1 < _log.size())
    {
        _iteration++;
        _scroll_scroll->SetScrollbar(_iteration, _log.size() / 10, _log.size(), 10);
        _text_iteration->SetLabel("Iteration: " + std::to_string(_iteration) + "/" + std::to_string(_log.size()));
        _panel_display->Refresh();
    }
}

void opt::Frame::_on_previous(wxCommandEvent &)
{
    if (_iteration > 0)
    {
        _iteration--;
        _scroll_scroll->SetScrollbar(_iteration, _log.size() / 10, _log.size(), 10);
        _text_iteration->SetLabel("Iteration: " + std::to_string(_iteration) + "/" + std::to_string(_log.size()));
        _panel_display->Refresh();
    }
}

void opt::Frame::_on_scroll(wxScrollEvent &)
{
    _iteration = _scroll_scroll->GetScrollPos(wxHORIZONTAL);
    if (_iteration > _log.size() - 1) _iteration = _log.size() - 1;
    _text_iteration->SetLabel("Iteration: " + std::to_string(_iteration) + "/" + std::to_string(_log.size()));
    _panel_display->Refresh();
}

void opt::Frame::_on_paint(wxPaintEvent &)
{
    wxPaintDC dc(_panel_display);
    if (_log.empty()) return;
    const std::vector<Boxing::Box> &boxes = _log[_iteration];
    if (boxes.empty()) return;

    //Calculate sizes
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

    //Find selection
    std::set<const Boxing::Rectangle*> selection;
    if (_iteration > 0 && _mode == 4) selection = _get_changes(_log_order[_iteration - 1], _log_order[_iteration]);
    else if (_iteration > 0 && _mode != 4) selection = _get_changes(_log[_iteration - 1], _log[_iteration]);

    //Clear background
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    //Draw boxes
    wxPen pen(*wxBLACK, 2);
    dc.SetPen(pen);
    for (unsigned int box_i = 0; box_i < boxes.size(); box_i++)
    {
        const unsigned int boxes_x = box_i % boxes_width;
        const unsigned int boxes_y = box_i / boxes_width;
        const unsigned int local_x = box_margin_width * boxes_x + box_offset_x;
        const unsigned int local_y = box_margin_height * boxes_y + box_offset_y;

        //Draw rectangles
        for (auto rectangle = boxes[box_i].rectangles.cbegin(); rectangle != boxes[box_i].rectangles.cend(); rectangle++)
        {
            if (selection.count(rectangle->rectangle) == 0) dc.SetBrush(*wxGREY_BRUSH);
            else if (*selection.begin() == rectangle->rectangle) dc.SetBrush(*wxYELLOW_BRUSH);
            else dc.SetBrush(*wxBLUE_BRUSH);
            const unsigned int rectangle_x_begin = box_size * rectangle->x / _boxing->box_size();
            const unsigned int rectangle_x_end = box_size * rectangle->x_end() / _boxing->box_size();
            const unsigned int rectangle_y_begin = box_size * rectangle->y / _boxing->box_size();
            const unsigned int rectangle_y_end = box_size * rectangle->y_end() / _boxing->box_size();
            dc.DrawRectangle(
                local_x + rectangle_x_begin, local_y + box_size - rectangle_y_end,
                rectangle_x_end - rectangle_x_begin, rectangle_y_end - rectangle_y_begin);
        }

        //Draw lines
        for (auto rectangle = boxes[box_i].rectangles.cbegin(); rectangle != boxes[box_i].rectangles.cend(); rectangle++)
        {
            const unsigned int rectangle_x_begin = box_size * rectangle->x / _boxing->box_size();
            const unsigned int rectangle_x_end = box_size * rectangle->x_end() / _boxing->box_size();
            const unsigned int rectangle_y_begin = box_size * rectangle->y / _boxing->box_size();
            const unsigned int rectangle_y_end = box_size * rectangle->y_end() / _boxing->box_size();
            dc.DrawLine(
                local_x + rectangle_x_begin, local_y + box_size - rectangle_y_begin,
                local_x + rectangle_x_begin, local_y + box_size - rectangle_y_end);
            dc.DrawLine(
                local_x + rectangle_x_begin, local_y + box_size - rectangle_y_end,
                local_x + rectangle_x_end, local_y + box_size - rectangle_y_end);
            dc.DrawLine(
                local_x + rectangle_x_end, local_y + box_size - rectangle_y_end,
                local_x + rectangle_x_end, local_y + box_size - rectangle_y_begin);
            dc.DrawLine(
                local_x + rectangle_x_end, local_y + box_size - rectangle_y_begin,
                local_x + rectangle_x_begin, local_y + box_size - rectangle_y_begin);
        }

        //Draw border
        dc.DrawLine(
            local_x + 0, local_y + 0,
            local_x + 0, local_y + box_size);
        dc.DrawLine(
            local_x + 0, local_y + box_size,
            local_x + box_size, local_y + box_size);
        dc.DrawLine(
            local_x + box_size, local_y + box_size,
            local_x + box_size, local_y + 0);
        dc.DrawLine(
            local_x + box_size, local_y + 0,
            local_x + 0, local_y + 0);
    }
}

opt::Frame::Frame() : wxFrame(nullptr, wxID_ANY, "Optimization algorithms")
{
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer *vsizer = new wxFlexGridSizer(2);
    vsizer->AddGrowableCol(1);
	
    //Mode
    const wxString choices[] = { "Greedy (area)", "Greedy (largest side)", "Greedy (smallest side)",
        "Local search (geometry)", "Local search (order)", "Local search (geometry/overlaps)" };
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Mode:"), 0, wxEXPAND);
    vsizer->Add(_selector_mode = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 6, choices), 0, wxEXPAND);
    _selector_mode->SetSelection(0);

    //Problem
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Box size:"), 0);
    vsizer->Add(_edit_box_size = new wxTextCtrl(this, wxID_ANY, "10"), 1, wxEXPAND);
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Item number:"), 0);
    vsizer->Add(_edit_item_number = new wxTextCtrl(this, wxID_ANY, "100"), 1, wxEXPAND);
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Minimal item size:"), 0);
    vsizer->Add(_edit_item_size_min = new wxTextCtrl(this, wxID_ANY, "1"), 1, wxEXPAND);
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Maximal item size:"), 0);
    vsizer->Add(_edit_item_size_max = new wxTextCtrl(this, wxID_ANY, "5"), 1, wxEXPAND);
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Seed:"), 0);
    vsizer->Add(_edit_seed = new wxTextCtrl(this, wxID_ANY, "0"), 1, wxEXPAND);
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Window size:"), 0);
    vsizer->Add(_edit_window = new wxTextCtrl(this, wxID_ANY, "1"), 1, wxEXPAND);
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Vertical window size:"), 0);
    vsizer->Add(_edit_hwindow = new wxTextCtrl(this, wxID_ANY, "0"), 1, wxEXPAND);
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Desired iteration:"), 0);
    vsizer->Add(_edit_desired_iter = new wxTextCtrl(this, wxID_ANY, "100"), 1, wxEXPAND);

    //Solution
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Iteration limit:"), 0);
    vsizer->Add(_edit_iter_max = new wxTextCtrl(this, wxID_ANY, "inf"), 0, wxEXPAND);
    vsizer->Add(new wxStaticText(this, wxID_ANY, "Time limit, seconds:"), 0);
    vsizer->Add(_edit_time_max = new wxTextCtrl(this, wxID_ANY, "inf"), 0, wxEXPAND);

    //Technical
    sizer->Add(vsizer, 0, wxEXPAND);
    sizer->Add(_button_run = new wxButton(this, wxID_ANY, "Run"), 0, wxEXPAND);
    sizer->Add(_button_next = new wxButton(this, wxID_ANY, "Next"), 0, wxEXPAND);
    sizer->Add(_button_previous = new wxButton(this, wxID_ANY, "Previous"), 0, wxEXPAND);
    sizer->Add(_panel_display = new wxPanel(this, wxID_ANY), 1, wxEXPAND);
    sizer->Add(_scroll_scroll = new wxScrollBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL), 0, wxEXPAND);
    sizer->Add(_text_iteration = new wxStaticText(this, wxID_ANY, "Iteration: N/A"), 0, wxEXPAND);
    sizer->Add(_text_time = new wxStaticText(this, wxID_ANY, "Time: N/A"), 0, wxEXPAND);
    Bind(wxEVT_BUTTON, &Frame::_on_run, this, _button_run->GetId());
    Bind(wxEVT_BUTTON, &Frame::_on_next, this, _button_next->GetId());
    Bind(wxEVT_BUTTON, &Frame::_on_previous, this, _button_previous->GetId());
    _panel_display->Bind(wxEVT_PAINT, &Frame::_on_paint, this, _panel_display->GetId());
    Bind(wxEVT_SCROLL_CHANGED, &Frame::_on_scroll, this, _scroll_scroll->GetId());
	
    SetSizer(sizer);
    SetAutoLayout(true);
    wxDisplay display(0u);
    SetSize(2 * display.GetClientArea().GetSize() / 3);
    SetPosition(wxPoint(display.GetClientArea().GetSize().GetWidth() / 6, display.GetClientArea().GetSize().GetHeight() / 6));
}

bool opt::App::OnInit()
{
    Frame *frame = new Frame();
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(opt::App);