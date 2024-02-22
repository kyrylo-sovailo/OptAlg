#include <wx/wx.h>
 
namespace opt
{
    enum
    {
        ID_Hello = 1
    };

    class Frame : public wxFrame
    {
    public:
        Frame();
    
    private:
        void OnHello(wxCommandEvent& event);
        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
    };

    class App : public wxApp
    {
    public:
        bool OnInit() override;
    };
}

opt::Frame::Frame() : wxFrame(nullptr, wxID_ANY, "Optimization algorithms")
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
 
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
 
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
 
    SetMenuBar( menuBar );
 
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");
 
    Bind(wxEVT_MENU, &Frame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &Frame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &Frame::OnExit, this, wxID_EXIT);
}
 
void opt::Frame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
 
void opt::Frame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}
 
void opt::Frame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

bool opt::App::OnInit()
{
    Frame *frame = new Frame();
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(opt::App);