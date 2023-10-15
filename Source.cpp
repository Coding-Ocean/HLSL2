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

// パイプラインオブジェクト
ComPtr<ID3D11Device> Device;                             // デバイスインターフェイス
ComPtr<ID3D11DeviceContext> Context;                     // コンテキスト
ComPtr<IDXGISwapChain> SwapChain;                        // スワップチェインインターフェイス
ComPtr<ID3D11RenderTargetView> RenderTargetView;         // レンダーターゲットビュー

ComPtr<ID3D11InputLayout> InputLayout;                   // インプットレイアウト
ComPtr<ID3D11VertexShader> VertexShader;                 // 頂点シェーダ
ComPtr<ID3D11PixelShader> PixelShader;                   // ピクセルシェーダ

ComPtr<ID3D11Buffer> VertexBuffer;                       // 頂点バッファ
ComPtr<ID3D11Buffer> ConstantBuffer;                     // 定数バッファ

//プログラムの開始時間
DWORD StartTime = 0;

// 頂点構造体
struct VERTEX
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
};

// 定数構造体
struct CONSTANT_BUFFER
{
    float Time;
};

// ウィンドウプロシージャ
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

    // switch文が処理しなかったメッセージを処理
    return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

int InitWindow()
{
    // ウィンドウクラスを初期化
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
        WARNING(L"ウィンドウクラスが登録できませんでした");
        return 1;
    }

    // ウィンドウスタイル
    //DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    DWORD windowStyle = WS_POPUP;
    // これからつくるウィンドウのサイズを求める
    RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&windowRect, windowStyle, FALSE);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    //ウィンドウが中央に表示されるような表示位置を計算する。
    int windowPosX = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
    int windowPosY = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;

    // ウィンドウハンドルを作成
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
        WARNING(L"ウィンドウがつくれませんでした");
        return 1;
    }

    return 0;
}

// デバイス関連初期化
int InitDevice()
{
    HWND hWnd = FindWindow(WINDOW_CLASS, NULL);

    // ドライバー種別を定義
    D3D_DRIVER_TYPE driverTypes[]
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
        D3D_DRIVER_TYPE_SOFTWARE,
    };
    size_t numDriverTypes = _countof(driverTypes);

    // スワップチェインの属性設定
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

    // ドライバー種別を上から検証しデバイスとスワップチェーンをつくる
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
        WARNING(L"DirectX11に対応していないデバイスです。");
        return 1;
    }

    // レンダーターゲットビューを作成
    ComPtr<ID3D11Texture2D> backBuffer;
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    Device->CreateRenderTargetView(backBuffer.Get(), nullptr, RenderTargetView.GetAddressOf());

    // レンダーターゲットビューをパイプラインに設定
    Context->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), nullptr);

    // 表示領域をパイプラインに設定
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

// シェーダ関連初期化
#include <fstream>
int InitShader()
{
    //コンパイル済みシェーダを読み込むバッファクラス buf
    class BUFFER
    {
    public:
        void operator= (const char* fileName)
        {
            std::ifstream ifs(fileName, std::ios::binary);
            if (ifs.fail())
            {
                *(long*)(0xcbcbcbcbcbcbcbcb) = 0;//強制終了
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

    // インプットレイアウト------------------------------------------------------
#ifdef _DEBUG
    buf = "x64\\Debug\\VertexShader.cso";
#else
    buf = "x64\\Release\\VertexShader.cso";
#endif
    HRESULT hr;
    // インプットレイアウトを定義
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    // インプットレイアウトのサイズ
    UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);
    // インプットレイアウトを作成
    hr = Device->CreateInputLayout(inputElementDescs, numElements, buf.pointer(), buf.size(), InputLayout.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"頂点インプットレイアウトがつくれません。");
        return 1;
    }

    //頂点シェーダをつくる------------------------------------------------------
    hr = Device->CreateVertexShader(buf.pointer(), buf.size(), NULL, VertexShader.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"頂点シェーダーがつくれません。");
        return 1;
    }

    //ピクセルシェーダをつくる-------------------------------------------------
#ifdef _DEBUG
    buf = "x64\\Debug\\PixelShader.cso";
#else
    buf = "x64\\Release\\PixelShader.cso";
#endif
    hr = Device->CreatePixelShader(buf.pointer(), buf.size(), NULL, PixelShader.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"ピクセルシェーダがつくれません。");
        return 1;
    }

    //-----------------------------------------------------------------------
    // インプットレイアウトをパイプラインにセット
    Context->IASetInputLayout(InputLayout.Get());
    // シェーダオブジェクトをパイプラインにセット
    Context->VSSetShader(VertexShader.Get(), nullptr, 0);
    Context->PSSetShader(PixelShader.Get(), nullptr, 0);

    return 0;
}

// バッファ関連初期化
int InitBuffer()
{
    //頂点バッファ---------------------------------------------------------------
    // 四角形のジオメトリを定義
    float aspect = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    VERTEX vertices[] = {
        {{-1.0f / aspect,  1.0f, 0.0f}, { 0.0f,  0.0f}},
        {{ 1.0f / aspect,  1.0f, 0.0f}, { 1.0f,  0.0f}},
        {{-1.0f / aspect, -1.0f, 0.0f}, { 0.0f,  1.0f}},
        {{ 1.0f / aspect, -1.0f, 0.0f}, { 1.0f,  1.0f}},
    };

    // リソースの設定
    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = vertices;

    // 頂点バッファの属性設定
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;                 // デフォルトアクセス
    bufferDesc.ByteWidth = sizeof(VERTEX) * _countof(vertices); // サイズはVERTEX構造体×4
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // 頂点バッファを使用
    bufferDesc.CPUAccessFlags = 0;                          // CPUのバッファへのアクセス拒否

    // 頂点バッファを作成
    HRESULT hr;
    hr = Device->CreateBuffer(&bufferDesc, &initData, VertexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"頂点バッファを作成できませんでした。");
        return 1;
    }

    // 表示する頂点バッファをパイプラインに設定
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    Context->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);

    // 使用するプリミティブタイプをパイプラインに設定
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // 定数バッファ-----------------------------------------------------------------

    // 定数バッファの属性設定
    bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(CONSTANT_BUFFER);
    bufferDesc.CPUAccessFlags = 0;
    //ピクセルシェーダの時
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    //バーテックスシェーダの時
    //bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    // 定数バッファを作成
    hr = Device->CreateBuffer(&bufferDesc, nullptr, ConstantBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        WARNING(L"定数バッファを作成できませんでした。");
        return 1;
    }

    // 定数バッファをパイプラインに設定
    Context->PSSetConstantBuffers(0, 1, ConstantBuffer.GetAddressOf());

    return 0;
}

// 描画
VOID OnRender()
{
    // バッファに値をセット
    CONSTANT_BUFFER constantBuffer{};
    constantBuffer.Time = (timeGetTime() - StartTime) / 1000.f;
    // GPU(シェーダ側)へ転送する
    Context->UpdateSubresource(ConstantBuffer.Get(), 0, nullptr, &constantBuffer, 0, 0);

    // バックバッファを指定した色でクリア
    FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    Context->ClearRenderTargetView(RenderTargetView.Get(), clearColor);

    // 頂点バッファをバックバッファに描画
    Context->Draw(4, 0);

    // フレームを最終出力(フリップ)
    SwapChain->Present(0, 0);
}

//ループ開始直前にやること
VOID OnStart()
{
    // ウィンドウの表示
    HWND hWnd = FindWindow(WINDOW_CLASS, NULL);
    ShowWindow(hWnd, SW_SHOW);
    ShowCursor(false);
    timeBeginPeriod(1);
    StartTime = timeGetTime();
}

//後始末
VOID OnDestroy()
{
    ShowCursor(true);
    timeEndPeriod(1);
}

// メイン関数
INT WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ INT)
{
    //ウィンドウ初期化
    if (InitWindow()) return 0;
    //ＤｉｒｅｃｔＸ初期化
    if (InitDevice()) return 0;
    if (InitShader()) return 0;
    if (InitBuffer()) return 0;
    // メインループ
    OnStart();
    MSG	msg{};
    while (msg.message != WM_QUIT)
    {
        // キュー内のメッセージを処理
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // 描画
        else
        {
            OnRender();
        }
    }
    OnDestroy();

    return 0;
}
