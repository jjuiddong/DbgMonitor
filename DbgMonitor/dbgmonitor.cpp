//
// Debug Monitor
//
#include "stdafx.h"

using namespace graphic;
using namespace framework;

class cViewer : public framework::cGameMain2
{
public:
	cViewer();
	virtual ~cViewer();

	virtual bool OnInit() override;
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
	virtual void OnEventProc(const sf::Event &evt) override;


public:
	cDbgMonitor m_dbgMonitor;
	sf::Vector2i m_curPos;
	Plane m_groundPlane1, m_groundPlane2;
	float m_moveLen;
	bool m_LButtonDown;
	bool m_RButtonDown;
	bool m_MButtonDown;
};

INIT_FRAMEWORK3(cViewer);


//-------------------------------------------------------------------------------------------
class cDockView1 : public framework::cDockWindow
{
public:
	cDockView1(const string &name) : framework::cDockWindow(name)
	, m_incT(0) {
	}
	virtual ~cDockView1() {
	}

	bool Init(cRenderer &renderer) {

		m_camera.SetCamera(Vector3(-10,10,-10), Vector3(0,0,0), Vector3(0,1,0));
		m_camera.SetProjection(MATH_PI / 4.f, m_rect.Width() / m_rect.Height(), 1, 10000.f);
		m_camera.SetViewPort(m_rect.Width(), m_rect.Height());

		sf::Vector2u size((u_int)m_rect.Width() - 15, (u_int)m_rect.Height() - 50);
		cViewport vp = renderer.m_viewPort;
		vp.m_vp.Width = (float)size.x;
		vp.m_vp.Height = (float)size.y;
		m_renderTarget.Create(renderer, vp, DXGI_FORMAT_R8G8B8A8_UNORM, true, true
			, DXGI_FORMAT_D24_UNORM_S8_UINT);

		return true;
	}
	
	virtual void OnUpdate(const float deltaSeconds) override {

	}

	virtual void OnRender(const float deltaSeconds) override {

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		m_incT += deltaSeconds;

		ImGui::Checkbox("LazyMode", &g_application->m_isLazyMode);
	 
		static bool animate = true;
		ImGui::Checkbox("animate", &animate);

		static float values1[1024] = { 0 };
		static float values2[1024] = { 0 };
		static int values_offset = 0;
		if (animate)
		{
			//if (m_incT > 0.033f) // 30 hz
			{
				// 누군가가 공유메모리에 쓰고 있다면, 다 쓸때까지 대기한다.
				//while (1 == g_sharedData->state)
				//	Sleep(1);

				cDbgMonitor &dbgMonitor = ((cViewer*)g_application)->m_dbgMonitor;
				dbgMonitor.m_mutex.Lock();
				const double fps = dbgMonitor.m_sharedData->fps;
				const double dt = dbgMonitor.m_sharedData->dtVal;
				dbgMonitor.m_mutex.Unlock();
				m_incVal += dt;

				m_incT = 0;
				values1[values_offset] = (float)m_incVal;
				values2[values_offset] = (float)fps;
				values_offset = (values_offset + 1) % ARRAYSIZE(values1);

				if (m_incVal > 1.0f)
					m_incVal = 0.f;
			}
		}

		ImGui::Spacing();
		ImGui::Text("Increase Seconds");
		ImGui::SameLine();
		static float graph1MinMax[2] = { 0.f, 1.2f };
		ImGui::DragFloat2("Graph1 Min-Max", graph1MinMax, 0.01f);
		ImGui::PlotLines("incT", values1, ARRAYSIZE(values1), values_offset, ""
			, graph1MinMax[0], graph1MinMax[1], ImVec2(0, 250));
		
		ImGui::Spacing();
		ImGui::Text("Delta Seconds");
		ImGui::SameLine();
		static float graph2MinMax[2] = { -1.2f, 1000.2f };
		ImGui::DragFloat2("Graph2 Min-Max", graph2MinMax, 0.01f);
		ImGui::PlotLines("fps", values2, ARRAYSIZE(values2), values_offset, ""
			, graph2MinMax[0], graph2MinMax[1], ImVec2(0, 250));
	}

	virtual void OnPreRender(const float deltaSeconds) override {
		//cAutoCam cam(&m_camera);
		//cRenderer &renderer = GetRenderer();
		//if (m_renderTarget.Begin(renderer))
		//{
		//	//RenderScene(renderer, deltaSeconds, "ShadowMap", m_isShadow, parentTm);
		//	renderer.RenderAxis();
		//}
		//m_renderTarget.End(renderer);
	}
	virtual void OnPostRender(const float deltaSeconds) override {

	}
	virtual void OnResizeEnd(const framework::eDockResize::Enum type, const sRectf &rect) override {

	}

	virtual void OnResetDevice() override
	{
		cRenderer &renderer = GetRenderer();

		// update viewport
		sRectf viewRect = { 0, 0, m_rect.Width() - 15, m_rect.Height() - 50 };
		m_camera.SetViewPort(viewRect.Width(), viewRect.Height());

		cViewport vp = GetRenderer().m_viewPort;
		vp.m_vp.Width = viewRect.Width();
		vp.m_vp.Height = viewRect.Height();
		m_renderTarget.Create(renderer, vp, DXGI_FORMAT_R8G8B8A8_UNORM, true, true, DXGI_FORMAT_D24_UNORM_S8_UINT);
	}

	float m_incT = 0;
	double m_incVal = 0;
	graphic::cRenderTarget m_renderTarget;
};



//-------------------------------------------------------------------------------------------
class cDockView2 : public framework::cDockWindow
{
public:
	cDockView2(const string &name) : framework::cDockWindow(name) {}
	virtual ~cDockView2() {}

	virtual void OnRender(const float deltaSeconds) override {

		cDbgMonitor &dbgMonitor = ((cViewer*)g_application)->m_dbgMonitor;
		dbgMonitor.m_mutex.Lock();
		ImGui::Checkbox("Debug Render", &dbgMonitor.m_sharedData->isDbgRender);
		char *styles = "Sphere\0Box\0None\0";
		ImGui::Combo("Debug Render Style", &dbgMonitor.m_sharedData->dbgRenderStyle, styles);
		dbgMonitor.m_mutex.Unlock();
	}
};


//-------------------------------------------------------------------------------------------
class cDockView3 : public framework::cDockWindow
{
public:
	cDockView3(const string &name) : framework::cDockWindow(name) {}
	virtual ~cDockView3() {}

	virtual void OnRender(const float deltaSeconds) override {

		cDbgMonitor &dbgMonitor = ((cViewer*)g_application)->m_dbgMonitor;
		dbgMonitor.m_mutex.Lock();
		ImGui::InputFloat3("eyePos", (float*)&dbgMonitor.m_sharedData->eyePos, -1, ImGuiInputTextFlags_ReadOnly);
		ImGui::InputFloat3("lookPos", (float*)&dbgMonitor.m_sharedData->lookPos, -1, ImGuiInputTextFlags_ReadOnly);
		Vector3 dir = (dbgMonitor.m_sharedData->lookPos - dbgMonitor.m_sharedData->eyePos).Normal();
		ImGui::InputFloat3("dir", (float*)&dir, -1, ImGuiInputTextFlags_ReadOnly);

		ImGui::Spacing();
		ImGui::InputFloat3("eyePos2D", (float*)&dbgMonitor.m_sharedData->eyePos2D, -1, ImGuiInputTextFlags_ReadOnly);
		ImGui::InputFloat3("lookPos2D", (float*)&dbgMonitor.m_sharedData->lookPos2D, -1, ImGuiInputTextFlags_ReadOnly);
		ImGui::Spacing();
		ImGui::InputFloat3("mousePos", (float*)&dbgMonitor.m_sharedData->mousePos, -1, ImGuiInputTextFlags_ReadOnly);
		dbgMonitor.m_mutex.Unlock();
	}
};


//-------------------------------------------------------------------------------------------
class cDockView4 : public framework::cDockWindow
{
public:
	cDockView4(const string &name) : framework::cDockWindow(name) {}
	virtual ~cDockView4() {}

	virtual void OnRender(const float deltaSeconds) override {

		cDbgMonitor &dbgMonitor = ((cViewer*)g_application)->m_dbgMonitor;
		dbgMonitor.m_mutex.Lock();
		//ImGui::LabelText("label", "Value");

		int actualDrawCall = 0;
		ImGui::InputInt("Draw Call", &dbgMonitor.m_sharedData->drawCallCount, -1, ImGuiInputTextFlags_ReadOnly);
		for (int i = 0; i < 10; ++i)
		{
			const int vtxType = dbgMonitor.m_sharedData->shaderDrawCall[0][i];
			const int drawCallCnt = dbgMonitor.m_sharedData->shaderDrawCall[1][i];
			if (0 == drawCallCnt)
				continue;

			if (0 != vtxType)
				actualDrawCall += drawCallCnt;

			Str128 strVtxType;
			GetVtxTypeStr(vtxType, strVtxType);
			ImGui::Text("%s : %d", strVtxType.c_str(), drawCallCnt);
		}
		ImGui::InputInt("Actual Draw Call", &actualDrawCall, -1, ImGuiInputTextFlags_ReadOnly);

		dbgMonitor.m_mutex.Unlock();
	}

	void GetVtxTypeStr(const int vtxType, OUT Str128 &out)
	{
		if (vtxType & eVertexType::POSITION)
			out += "Pos ";
		if (vtxType & eVertexType::POSITION_RHW)
			out += "Pos_Rhw ";
		if (vtxType & eVertexType::NORMAL)
			out += "Norm ";
		if (vtxType & eVertexType::TEXTURE)
			out += "Tex ";
		if (vtxType & eVertexType::COLOR)
			out += "Color ";
		if (vtxType & eVertexType::TANGENT)
			out += "Tangent ";
		if (vtxType & eVertexType::BINORMAL)
			out += "Binorm ";
		if (vtxType & eVertexType::BLENDINDICES)
			out += "BlendIndices ";
		if (vtxType & eVertexType::BLENDWEIGHT)
			out += "BlendWeight ";
		if (out.empty())
			out.Format("None %d", vtxType);
	}

};




cViewer::cViewer()
	: m_groundPlane1(Vector3(0, 1, 0), 0)
{
	m_windowName = L"Debug Monitor";
	m_isLazyMode = true;

	const RECT r = { 0, 0, 1024, 768 };
	//const RECT r = { 0, 0, 1280, 1024 };
	m_windowRect = r;
	m_moveLen = 0;
	m_LButtonDown = false;
	m_RButtonDown = false;
	m_MButtonDown = false;
}

cViewer::~cViewer()
{
}


bool cViewer::OnInit()
{
	const float WINSIZE_X = float(m_windowRect.right - m_windowRect.left);
	const float WINSIZE_Y = float(m_windowRect.bottom - m_windowRect.top);
	GetMainCamera().SetCamera(Vector3(30, 30, -30), Vector3(0, 0, 0), Vector3(0, 1, 0));
	GetMainCamera().SetProjection(MATH_PI / 4.f, (float)WINSIZE_X / (float)WINSIZE_Y, 0.1f, 10000.0f);
	GetMainCamera().SetViewPort(WINSIZE_X, WINSIZE_Y);

	m_camera.SetCamera(Vector3(-3, 10, -10), Vector3(0, 0, 0), Vector3(0, 1, 0));
	m_camera.SetProjection(MATH_PI / 4.f, (float)WINSIZE_X / (float)WINSIZE_Y, 1.0f, 10000.f);
	m_camera.SetViewPort(WINSIZE_X, WINSIZE_Y);

	GetMainLight().Init(cLight::LIGHT_DIRECTIONAL,
		Vector4(0.2f, 0.2f, 0.2f, 1), Vector4(0.9f, 0.9f, 0.9f, 1),
		Vector4(0.2f, 0.2f, 0.2f, 1));
	const Vector3 lightPos(-300, 300, -300);
	const Vector3 lightLookat(0, 0, 0);
	GetMainLight().SetPosition(lightPos);
	GetMainLight().SetDirection((lightLookat - lightPos).Normal());

	if (!m_dbgMonitor.Create(m_renderer, "DebugMonitorShmem::jjuiddong"))
		assert(0);
	//g_sharedData = (sSharedData*)m_dbgShmem.m_memPtr;

	m_gui.SetContext();

	cDockView1 *view1 = new cDockView1("Graph View");
	view1->Create(eDockState::DOCKWINDOW, eDockSlot::TAB, this, NULL);
	view1->Init(m_renderer);

	cDockView2 *view2 = new cDockView2("Flags");
	view2->Create(eDockState::DOCKWINDOW, eDockSlot::BOTTOM, this, view1, 0.4f);
	cDockView3 *view3 = new cDockView3("Variable");
	view3->Create(eDockState::DOCKWINDOW, eDockSlot::RIGHT, this, view2);
	cDockView4 *view4 = new cDockView4("Graphic");
	view4->Create(eDockState::DOCKWINDOW, eDockSlot::TAB, this, view3);

	const int cx = GetSystemMetrics(SM_CXSCREEN);
	const int cy = GetSystemMetrics(SM_CYSCREEN);

	m_gui.SetContext();

	float col_main_hue = 0.0f / 255.0f;
	float col_main_sat = 0.0f / 255.0f;
	float col_main_val = 80.0f / 255.0f;

	//float col_area_hue = 0.0f / 255.0f;
	//float col_area_sat = 0.0f / 255.0f;
	//float col_area_val = 50.0f / 255.0f;
	float col_area_hue = 0.0f / 255.0f;
	float col_area_sat = 30.0f / 255.0f;
	float col_area_val = 45.0f / 255.0f;

	//float col_back_hue = 0.0f / 255.0f;
	//float col_back_sat = 0.0f / 255.0f;
	//float col_back_val = 35.0f / 255.0f;
	float col_back_hue = 0.0f / 255.0f;
	float col_back_sat = 30.0f / 255.0f;
	float col_back_val = 25.0f / 255.0f;

	float col_text_hue = 0.0f / 255.0f;
	float col_text_sat = 0.0f / 255.0f;
	float col_text_val = 255.0f / 255.0f;
	float frameRounding = 0.0f;

	ImVec4 col_text = ImColor::HSV(col_text_hue, col_text_sat, col_text_val);
	ImVec4 col_main = ImColor::HSV(col_main_hue, col_main_sat, col_main_val);
	ImVec4 col_area = ImColor::HSV(col_area_hue, col_area_sat, col_area_val);
	ImVec4 col_back = ImColor::HSV(col_back_hue, col_back_sat, col_back_val);
	float rounding = frameRounding;

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = rounding;
	style.WindowRounding = rounding;
	style.Colors[ImGuiCol_Text] = ImVec4(col_text.x, col_text.y, col_text.z, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(col_text.x, col_text.y, col_text.z, 0.58f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(col_back.x, col_back.y, col_back.z, 0.80f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(col_text.x, col_text.y, col_text.z, 0.30f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(col_back.x, col_back.y, col_back.z, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.68f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(col_main.x, col_main.y, col_main.z, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(col_main.x, col_main.y, col_main.z, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(col_main.x, col_main.y, col_main.z, 0.31f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.78f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(col_text.x, col_text.y, col_text.z, 0.80f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(col_main.x, col_main.y, col_main.z, 0.54f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(col_main.x, col_main.y, col_main.z, 0.44f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.86f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(col_main.x, col_main.y, col_main.z, 0.76f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.86f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(col_text.x, col_text.y, col_text.z, 0.32f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(col_text.x, col_text.y, col_text.z, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(col_text.x, col_text.y, col_text.z, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(col_main.x, col_main.y, col_main.z, 0.20f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.78f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(col_text.x, col_text.y, col_text.z, 0.16f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(col_text.x, col_text.y, col_text.z, 0.39f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(col_text.x, col_text.y, col_text.z, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(col_text.x, col_text.y, col_text.z, 0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(col_text.x, col_text.y, col_text.z, 0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(col_main.x, col_main.y, col_main.z, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.10f, 0.10f, 0.10f, 0.55f);

	return true;
}


void cViewer::OnUpdate(const float deltaSeconds)
{
	__super::OnUpdate(deltaSeconds);

	cAutoCam cam(&m_camera);

	GetMainCamera().Update(deltaSeconds);
}


void cViewer::OnRender(const float deltaSeconds)
{
}


void cViewer::OnEventProc(const sf::Event &evt)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (evt.type)
	{
	case sf::Event::KeyPressed:
		switch (evt.key.code)
		{
		case sf::Keyboard::Escape: close(); break;
		}
		break;
	}
}
