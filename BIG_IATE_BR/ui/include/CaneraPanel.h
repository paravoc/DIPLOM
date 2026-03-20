#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include <vector>

class CameraPanel : public wxPanel {
public:
    CameraPanel(wxWindow* parent);
    virtual ~CameraPanel();

    void UpdateCameraStatus(int id, bool online);

private:
    void CreateCameraGrid();

    std::vector<wxPanel*> m_cameras;

    wxDECLARE_EVENT_TABLE();
};