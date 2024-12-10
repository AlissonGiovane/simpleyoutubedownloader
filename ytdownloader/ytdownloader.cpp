#include <windows.h>
#include <string>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

// Global handles
HWND hCancelButton, hUrlEdit, hDownloadButton, hUpdateButton;
bool cancelDownload = false;

// Função para baixar a última versão do yt-dlp.exe
void UpdateYtDlp() {
    std::wstring url = L"https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe";
    std::wstring dest = L"yt-dlp.exe";

    if (MessageBox(NULL, L"Deseja baixar a última versão do yt-dlp.exe?", L"baixar yt-dlp", MB_YESNO) == IDYES) {
        HRESULT hr = URLDownloadToFile(NULL, url.c_str(), dest.c_str(), 0, NULL);
        if (SUCCEEDED(hr)) {
            MessageBox(NULL, L"yt-dlp baixado com sucesso!", L"Sucesso", MB_OK);
        }
        else {
            MessageBox(NULL, L"Falha ao baixar o yt-dlp.", L"Erro", MB_OK);
        }
    }
}

// Função que simula a leitura do progresso e chama yt-dlp
void DownloadFile(const std::wstring& url) {
    std::wstring command = L"yt-dlp.exe -f bestvideo[ext=mp4]+bestaudio[ext=m4a]/mp4 " + url;

    FILE* pipe = _wpopen(command.c_str(), L"r");
    if (!pipe) {
        MessageBox(NULL, L"Falha ao iniciar o download.", L"Erro", MB_OK);
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (cancelDownload) {
            fclose(pipe);
            return;
        }
    }

    fclose(pipe);

    MessageBox(NULL, L"Download Concluído! Salvo na pasta do programa.", L"Sucesso", MB_OK);

    EnableWindow(hUrlEdit, TRUE);
    EnableWindow(hDownloadButton, TRUE);
    ShowWindow(hCancelButton, SW_HIDE);
}

// Função de callback para a janela principal
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hUrlEdit = CreateWindowEx(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            50, 50, 300, 30, hwnd, (HMENU)3, GetModuleHandle(NULL), NULL);

        hDownloadButton = CreateWindow(L"BUTTON", L"Iniciar Download",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 100, 120, 30, hwnd, (HMENU)4, GetModuleHandle(NULL), NULL);

        hCancelButton = CreateWindow(L"BUTTON", L"Cancelar",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            200, 100, 100, 30, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);
        ShowWindow(hCancelButton, SW_HIDE);

        hUpdateButton = CreateWindow(L"BUTTON", L"Atualizar yt-dlp",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 150, 120, 30, hwnd, (HMENU)5, GetModuleHandle(NULL), NULL);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == 2) {
            cancelDownload = true;
        }
        if (LOWORD(wParam) == 4) {
            wchar_t url[1024];
            GetWindowTextW(hUrlEdit, url, 1024);
            std::wstring videoUrl(url);

            if (!videoUrl.empty()) {
                EnableWindow(hUrlEdit, FALSE);
                EnableWindow(hDownloadButton, FALSE);
                ShowWindow(hCancelButton, SW_SHOW);

                cancelDownload = false;
                std::thread downloadThread(DownloadFile, videoUrl);
                downloadThread.detach();
            }
            else {
                MessageBox(hwnd, L"Por favor, insira uma URL válida.", L"Erro", MB_OK);
            }
        }
        if (LOWORD(wParam) == 5) {
            UpdateYtDlp();
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Função principal para criar a janela
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"Downloader",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 250, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
