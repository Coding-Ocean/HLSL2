#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <d3d11.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <directxmath.h>
using namespace DirectX;

#define WINDOW_CLASS    L"Pixel Shader"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    1920
#define WINDOW_HEIGHT   1080

#define WARNING(msg) MessageBox(NULL, msg, WINDOW_TITLE, MB_OK | MB_ICONERROR);

// �p�C�v���C���I�u�W�F�N�g
ComPtr<ID3D11Device> Device;                             // �f�o�C�X�C���^�[�t�F�C�X
ComPtr<ID3D11DeviceContext> Context;                     // �R���e�L�X�g
ComPtr<IDXGISwapChain> SwapChain;                        // �X���b�v�`�F�C���C���^�[�t�F�C�X
ComPtr<ID3D11RenderTargetView> RenderTargetView;         // �����_�[�^�[�Q�b�g�r���[

ComPtr<ID3D11InputLayout> InputLayout;                   // �C���v�b�g���C�A�E�g
ComPtr<ID3D11VertexShader> VertexShader;                 // ���_�V�F�[�_
ComPtr<ID3D11PixelShader> PixelShader;                   // �s�N�Z���V�F�[�_

ComPtr<ID3D11Buffer> VertexBuffer;                       // ���_�o�b�t�@
ComPtr<ID3D11Buffer> ConstantBuffer;                     // �萔�o�b�t�@

//�v���O�����̊J�n����
DWORD StartTime = 0;

// ���_�\����
struct VERTEX
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
};

// �萔�\����
struct CONSTANT_BUFFER
{
    float Time;
};

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch (nMsg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) PostMessage(hWnd, WM_CLOSE, 0, 0);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // switch�����������Ȃ��������b�Z�[�W������
    return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

int InitWindow()
{
    // �E�B���h�E�N���X��������
    WNDCLASSEX	windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = WINDOW_CLASS;
    ATOM rt = RegisterClassEx(&windowClass);
    if (rt == 0)
    {
        WARNING(L"�E�B���h�E�N���X���o�^�ł��܂���ł���");
        return 1;
    }

    // �E�B���h�E�X�^�C��
    //DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    DWORD windowStyle = WS_POPUP;
    // ���ꂩ�����E�B���h�E�̃T�C�Y�����߂�
    RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&windowRect, windowStyle, FALSE);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    //�E�B���h�E�������ɕ\�������悤�ȕ\���ʒu���v�Z����B
    int windowPosX = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
    int windowPosY = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;

    // �E�B���h�E�n���h�����쐬
    HWND hWnd = CreateWindow(
        WINDOW_CLASS,
        WINDOW_TITLE,
        windowStyle,
        windowPosX,
        windowPosY,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    if (hWnd == 0)
    {
        WARNING(L"�E�B���h�E������܂���ł���");
        return 1;
    }

    return 0;
}

// �f�o�C�X�֘A������
int InitDevice()
{
    HWND hWnd = FindWindow(WINDOW_CLASS, NULL);

    // �h���C�o�[��ʂ��`
    D3D_DRIVER_TYPE driverTypes[]
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
        D3D_DRIVER_TYPE_SOFTWARE,
    };
    size_t numDriverTypes = _countof(driverTypes);

    // �X���b�v�`�F�C���̑����ݒ�
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferDesc.Width = WINDOW_WIDTH;
    swapChainDesc.BufferDesc.Height = WINDOW_HEIGHT;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = TRUE;

    // �h���C�o�[��ʂ��ォ�猟�؂��f�o�C�X�ƃX���b�v�`�F�[��������
    HRESULT hr = E_FAIL;
    for (size_t i = 0; i < numDriverTypes; i++)
    {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            driverTypes[i],
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            SwapChain.GetAddressOf(),
            Device.GetAddressOf(),
            nullptr,
            Context.GetAddressOf()
        );
        if (SUCCEEDED(hr)) break;
    }
    if (FAILED(hr))
    {
        WARNING(L"DirectX11�ɑΉ����Ă��Ȃ��f�o�C�X�ł��B");
        return 1;
    }

    // �����_�[�^�[�Q�b�g�r���[���쐬
    ComPtr<ID3D11Texture2D> backBuffer;
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    Device->CreateRenderTargetView(backBuffer.Get(), nullptr, RenderTargetView.GetAddressOf());

    // �����_�[�^�[�Q�b�g�r���[���p�C�v���C���ɐݒ�
    Context->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), nullptr);

    // �\���̈���p�C�v���C���ɐݒ�
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(viewport));
    viewport.Width = WINDOW_WIDTH;
    viewport.Height = WINDOW_HEIGHT;
    viewport.TopLeftX = 0;// (1920 - viewport.Width) / 2;
    viewport.TopLeftY = 0;// (1080 - viewport.Height) / 2;
    viewport.MinDepth = D3D11_MIN_DEPTH;    // 0.0f
    viewport.MaxDepth = D3D11_MAX_DEPTH;    // 1.0f
    Context->RSSetViewports(1, &viewport);

    return 0;
}

// �V�F�[�_�֘A������
#include <fstream>
int InitShader()
{
    //�R���p�C���ς݃V�F�[�_��ǂݍ��ރo�b�t�@�N���X buf
    class BUFFER
    {
    public:
        void operator= (const char* fileName)
        {
            std::ifstream ifs(fileName, std::ios::binary);
            if (ifs.fail())
            {
                *(long*)(0xcbcbcbcbcbcbcbcb) = 0;//�����I��
            }
            std::istreambuf_iterator<char> it(ifs);
            std::istreambuf_iterator<char> last;
            Buffer.assign(it, last);
        }
        const char* pointer() const
        {
            return Buffer.data();
        }
        size_t size()
        {
            return Buffer.size();
        }
    private:
        std::string Buffer;
    }buf;

    // �C���v�b�g���C�A�E�g------------------------------------------------------
#ifdef _DEBUG
    buf = "x64\\Debug\\VertexShader.cso";
#else
    buf = "x64\\Release\\VertexShader.cso";
#endif
    HRESULT hr;
    // �C���v�b�g���C�A�E�g���`
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    // �C���v�b�g���C�A�E�g�̃T�C�Y
    UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);
    // �C���v�b�g���C�A�E�g���쐬
    hr = Device->CreateInputLayout(inputElementDescs, numElements, buf.pointer(), buf.size(), InputLayout.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"���_�C���v�b�g���C�A�E�g������܂���B");
        return 1;
    }

    //���_�V�F�[�_������------------------------------------------------------
    hr = Device->CreateVertexShader(buf.pointer(), buf.size(), NULL, VertexShader.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"���_�V�F�[�_�[������܂���B");
        return 1;
    }

    //�s�N�Z���V�F�[�_������-------------------------------------------------
#ifdef _DEBUG
    buf = "x64\\Debug\\PixelShader.cso";
#else
    buf = "x64\\Release\\PixelShader.cso";
#endif
    hr = Device->CreatePixelShader(buf.pointer(), buf.size(), NULL, PixelShader.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"�s�N�Z���V�F�[�_������܂���B");
        return 1;
    }

    //-----------------------------------------------------------------------
    // �C���v�b�g���C�A�E�g���p�C�v���C���ɃZ�b�g
    Context->IASetInputLayout(InputLayout.Get());
    // �V�F�[�_�I�u�W�F�N�g���p�C�v���C���ɃZ�b�g
    Context->VSSetShader(VertexShader.Get(), nullptr, 0);
    Context->PSSetShader(PixelShader.Get(), nullptr, 0);

    return 0;
}

// �o�b�t�@�֘A������
int InitBuffer()
{
    //���_�o�b�t�@---------------------------------------------------------------
    // �l�p�`�̃W�I���g�����`
    float aspect = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    VERTEX vertices[] = {
        {{-1.0f / aspect,  1.0f, 0.0f}, { 0.0f,  0.0f}},
        {{ 1.0f / aspect,  1.0f, 0.0f}, { 1.0f,  0.0f}},
        {{-1.0f / aspect, -1.0f, 0.0f}, { 0.0f,  1.0f}},
        {{ 1.0f / aspect, -1.0f, 0.0f}, { 1.0f,  1.0f}},
    };

    // ���\�[�X�̐ݒ�
    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = vertices;

    // ���_�o�b�t�@�̑����ݒ�
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;                 // �f�t�H���g�A�N�Z�X
    bufferDesc.ByteWidth = sizeof(VERTEX) * _countof(vertices); // �T�C�Y��VERTEX�\���́~4
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // ���_�o�b�t�@���g�p
    bufferDesc.CPUAccessFlags = 0;                          // CPU�̃o�b�t�@�ւ̃A�N�Z�X����

    // ���_�o�b�t�@���쐬
    HRESULT hr;
    hr = Device->CreateBuffer(&bufferDesc, &initData, VertexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"���_�o�b�t�@���쐬�ł��܂���ł����B");
        return 1;
    }

    // �\�����钸�_�o�b�t�@���p�C�v���C���ɐݒ�
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    Context->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);

    // �g�p����v���~�e�B�u�^�C�v���p�C�v���C���ɐݒ�
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // �萔�o�b�t�@-----------------------------------------------------------------

    // �萔�o�b�t�@�̑����ݒ�
    bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(CONSTANT_BUFFER);
    bufferDesc.CPUAccessFlags = 0;
    //�s�N�Z���V�F�[�_�̎�
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    //�o�[�e�b�N�X�V�F�[�_�̎�
    //bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    // �萔�o�b�t�@���쐬
    hr = Device->CreateBuffer(&bufferDesc, nullptr, ConstantBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"�萔�o�b�t�@���쐬�ł��܂���ł����B");
        return 1;
    }

    // �萔�o�b�t�@���p�C�v���C���ɐݒ�
    Context->PSSetConstantBuffers(0, 1, ConstantBuffer.GetAddressOf());

    return 0;
}

// �`��
VOID OnRender()
{
    // �o�b�t�@�ɒl���Z�b�g
    CONSTANT_BUFFER constantBuffer{};
    constantBuffer.Time = (timeGetTime() - StartTime) / 1000.f;
    // GPU(�V�F�[�_��)�֓]������
    Context->UpdateSubresource(ConstantBuffer.Get(), 0, nullptr, &constantBuffer, 0, 0);

    // �o�b�N�o�b�t�@���w�肵���F�ŃN���A
    FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    Context->ClearRenderTargetView(RenderTargetView.Get(), clearColor);

    // ���_�o�b�t�@���o�b�N�o�b�t�@�ɕ`��
    Context->Draw(4, 0);

    // �t���[�����ŏI�o��(�t���b�v)
    SwapChain->Present(0, 0);
}

//���[�v�J�n���O�ɂ�邱��
VOID OnStart()
{
    // �E�B���h�E�̕\��
    HWND hWnd = FindWindow(WINDOW_CLASS, NULL);
    ShowWindow(hWnd, SW_SHOW);
    ShowCursor(false);
    timeBeginPeriod(1);
    StartTime = timeGetTime();
}

//��n��
VOID OnDestroy()
{
    ShowCursor(true);
    timeEndPeriod(1);
}

// ���C���֐�
INT WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ INT)
{
    //�E�B���h�E������
    if (InitWindow()) return 0;
    //�c�����������w������
    if (InitDevice()) return 0;
    if (InitShader()) return 0;
    if (InitBuffer()) return 0;
    // ���C�����[�v
    OnStart();
    MSG	msg{};
    while (msg.message != WM_QUIT)
    {
        // �L���[���̃��b�Z�[�W������
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // �`��
        else
        {
            OnRender();
        }
    }
    OnDestroy();

    return 0;
}
