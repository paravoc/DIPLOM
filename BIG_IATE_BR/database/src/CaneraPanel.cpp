
#include"../include/CaneraPanel.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

#define _T(str) wxString::FromUTF8(str)

class ModernCamera : public wxPanel {
public:
    ModernCamera(wxWindow* parent, int id, const wxString& name)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(300, 220), wxBORDER_NONE),
        m_id(id), m_name(name), m_online(true) {

        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_PAINT, &ModernCamera::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &ModernCamera::OnClick, this);
    }

    void SetOnline(bool online) { m_online = online; Refresh(); }

private:
    int m_id;
    wxString m_name;
    bool m_online;

    void OnPaint(wxPaintEvent& event) {
        wxAutoBufferedPaintDC dc(this);
        wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
        if (!gc) return;

        wxSize size = GetClientSize();

        gc->SetBrush(wxBrush(wxColour(45, 45, 55)));
        gc->SetPen(wxPen(wxColour(70, 70, 80), 1));
        gc->DrawRoundedRectangle(0, 0, size.x, size.y, 8);

        gc->SetBrush(wxBrush(wxColour(0, 120, 215)));
        gc->DrawRectangle(0, 0, size.x, 4);

        gc->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD), *wxWHITE);
        gc->DrawText(m_name, 12, 12);

        gc->SetBrush(wxBrush(wxColour(30, 30, 40)));
        gc->SetPen(wxPen(wxColour(90, 90, 100), 1));
        gc->DrawRoundedRectangle(12, 45, size.x - 24, size.y - 85, 5);

        wxColour statusColor = m_online ? wxColour(0, 180, 0) : wxColour(180, 0, 0);
        gc->SetBrush(wxBrush(statusColor));
        gc->DrawEllipse(12, size.y - 28, 8, 8);

        gc->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), wxColour(180, 180, 190));
        gc->DrawText(m_online ? _T("Online") : _T("Offline"), 28, size.y - 32);

        delete gc;
    }

    void OnClick(wxMouseEvent& event) {
        wxMessageBox(wxString::Format(_T("Камера %d: %s"), m_id, m_name),
            _T("Просмотр"), wxOK | wxICON_INFORMATION);
    }
};

wxBEGIN_EVENT_TABLE(CameraPanel, wxPanel)
wxEND_EVENT_TABLE()

CameraPanel::CameraPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    SetBackgroundColour(wxColour(25, 25, 35));
    CreateCameraGrid();
}

CameraPanel::~CameraPanel() {}

void CameraPanel::CreateCameraGrid() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* title = new wxStaticText(this, wxID_ANY, _T("ВИДЕОНАБЛЮДЕНИЕ"));
    title->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    title->SetForegroundColour(*wxWHITE);
    mainSizer->Add(title, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 15);

    wxGridSizer* grid = new wxGridSizer(2, 2, 10, 10);
    wxString names[] = { _T("ГЛАВНЫЙ ВХОД"), _T("ЗАПАСНОЙ ВЫХОД"), _T("ПАРКОВКА"), _T("ВЕСТИБЮЛЬ") };

    for (int i = 0; i < 4; i++) {
        ModernCamera* cam = new ModernCamera(this, i + 1, names[i]);
        grid->Add(cam, 1, wxEXPAND);
        m_cameras.push_back(cam);
    }

    mainSizer->Add(grid, 1, wxEXPAND | wxALL, 10);
    SetSizer(mainSizer);
}

void CameraPanel::UpdateCameraStatus(int id, bool online) {
    if (id >= 1 && id <= (int)m_cameras.size()) {
        ((ModernCamera*)m_cameras[id - 1])->SetOnline(online);
    }
}