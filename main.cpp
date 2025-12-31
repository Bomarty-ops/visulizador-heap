#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <windowsx.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <sstream>

#define ID_EDIT_INPUT 101
#define ID_BTN_INSERT_MAX 102
#define ID_BTN_INSERT_MIN 103
#define ID_BTN_HEAPSORT 104
#define ID_BTN_RESET 105
#define ID_STATIC_STATUS 106
#define ID_TIMER 1

const int NODE_RADIUS = 20;
const int VERTICAL_SPACING = 60;
const int ANIMATION_SPEED_MS = 16; 
const int ANIMATION_STEP_INCREMENT = 5;

struct Node {
    int value;
    float x, y;
    float targetX, targetY;
    
    Node(int v) : value(v), x(0), y(0), targetX(0), targetY(0) {}
};

enum class AppState {
    IDLE,
    INSERT_CHECK,
    INSERT_SWAP,
    SORT_EXTRACT,
    SORT_SHRINK,
    SORT_HEAPIFY_CHECK,
    SORT_HEAPIFY_SWAP,
    SORT_COMPLETED
};

class HeapVisualizer {
private:
    std::vector<Node> heap;
    
    AppState state;
    HWND hWnd;
    RECT clientRect;
    
    int animationProgress; 
    int currentIdx;
    int compareIdx;
    bool isMaxHeap;
    int heapSize;

public:
    HeapVisualizer(HWND hwnd) : hWnd(hwnd), state(AppState::IDLE), animationProgress(0), 
        currentIdx(-1), compareIdx(-1), isMaxHeap(true), heapSize(0) {}

    void SetRect(RECT r) { 
        clientRect = r; 
        RecalculateLayout();
    }

    void GetNodePos(int index, float& outX, float& outY) {
        if (index < 0) return;
        int level = (int)floor(log2(index + 1));
        
        int itemsInLevel = (int)pow(2, level);
        int posInLevel = index - (itemsInLevel - 1);
        
        float width = (float)(clientRect.right - clientRect.left);
        float slice = width / (itemsInLevel + 1);
        
        outX = slice * (posInLevel + 1);
        outY = (float)clientRect.top + 30 + (level * VERTICAL_SPACING);
    }

    void RecalculateLayout() {
        if (state == AppState::IDLE) {
            for (int i = 0; i < heap.size(); ++i) {
                GetNodePos(i, heap[i].x, heap[i].y);
                heap[i].targetX = heap[i].x;
                heap[i].targetY = heap[i].y;
            }
        }
    }

    void Insert(int value, bool max) {
        if (state != AppState::IDLE && state != AppState::SORT_COMPLETED) return;
        
        if (state == AppState::SORT_COMPLETED) {
            heapSize = (int)heap.size(); 
            state = AppState::IDLE;
        }

        isMaxHeap = max;
        heap.emplace_back(value);
        heapSize = (int)heap.size();
        
        int newIdx = heapSize - 1;
        float x, y;
        GetNodePos(newIdx, x, y);
        heap[newIdx].x = x;
        heap[newIdx].y = y;
        heap[newIdx].targetX = x;
        heap[newIdx].targetY = y;

        currentIdx = newIdx;
        state = AppState::INSERT_CHECK;
        
        SetStatus(L"Inserted " + std::to_wstring(value) + L". Analyzing...");
        InvalidateRect(hWnd, NULL, FALSE);
    }

    void StartSort() {
        if (heap.empty() || (state != AppState::IDLE && state != AppState::SORT_COMPLETED)) return;
        heapSize = (int)heap.size();
        isMaxHeap = true;
        
        state = AppState::SORT_EXTRACT;
        SetStatus(L"Starting Heapsort...");
    }

    void Reset() {
        heap.clear();
        state = AppState::IDLE;
        heapSize = 0;
        SetStatus(L"Ready");
        InvalidateRect(hWnd, NULL, FALSE);
    }

    void Update() {
        if (state == AppState::IDLE || state == AppState::SORT_COMPLETED) return;

        if (state == AppState::INSERT_SWAP || state == AppState::SORT_HEAPIFY_SWAP || state == AppState::SORT_EXTRACT) {
            animationProgress += ANIMATION_STEP_INCREMENT;
            
            Node& n1 = heap[currentIdx];
            Node& n2 = heap[compareIdx];
            
            auto moveTowards = [&](float &curr, float target) {
                float diff = target - curr;
                if (abs(diff) < 2.0f) curr = target;
                else curr += diff * 0.2f; 
            };
            
            moveTowards(n1.x, n1.targetX);
            moveTowards(n1.y, n1.targetY);
            moveTowards(n2.x, n2.targetX);
            moveTowards(n2.y, n2.targetY);

             if (abs(n1.x - n1.targetX) < 1.0f && abs(n1.y - n1.targetY) < 1.0f &&
                abs(n2.x - n2.targetX) < 1.0f && abs(n2.y - n2.targetY) < 1.0f) {
                
                n1.x = n1.targetX; n1.y = n1.targetY;
                n2.x = n2.targetX; n2.y = n2.targetY;
                
                std::swap(heap[currentIdx], heap[compareIdx]);
                std::swap(currentIdx, compareIdx); 
                
                if (state == AppState::INSERT_SWAP) {
                    state = AppState::INSERT_CHECK; 
                } else if (state == AppState::SORT_EXTRACT) {
                    state = AppState::SORT_SHRINK;
                } else if (state == AppState::SORT_HEAPIFY_SWAP) {
                    state = AppState::SORT_HEAPIFY_CHECK;
                }
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return;
        }

        if (state == AppState::INSERT_CHECK) {
            if (currentIdx == 0) {
                state = AppState::IDLE;
                SetStatus(L"Insertion Complete.");
                RecalculateLayout();
                return;
            }
            
            int parentIdx = (currentIdx - 1) / 2;
            bool swapNeeded = false;
            
            if (isMaxHeap && heap[currentIdx].value > heap[parentIdx].value) swapNeeded = true;
            if (!isMaxHeap && heap[currentIdx].value < heap[parentIdx].value) swapNeeded = true;
            
            if (swapNeeded) {
                compareIdx = parentIdx;
                PrepareSwap(currentIdx, compareIdx);
                state = AppState::INSERT_SWAP;
                SetStatus(L"Swapping with parent...");
            } else {
                state = AppState::IDLE;
                SetStatus(L"Insertion Complete.");
            }
        }
        else if (state == AppState::SORT_EXTRACT) {
            if (heapSize <= 1) {
                state = AppState::SORT_COMPLETED;
                SetStatus(L"Sort Complete.");
                return;
            }
            currentIdx = 0;
            compareIdx = heapSize - 1;
            PrepareSwap(currentIdx, compareIdx); 
            SetStatus(L"Extracting Root...");
        }
        else if (state == AppState::SORT_SHRINK) {
            heapSize--;
            currentIdx = 0;
            state = AppState::SORT_HEAPIFY_CHECK;
        }
        else if (state == AppState::SORT_HEAPIFY_CHECK) {
            int left = 2 * currentIdx + 1;
            int right = 2 * currentIdx + 2;
            int largest = currentIdx;
            
            if (left < heapSize && heap[left].value > heap[largest].value) largest = left;
            if (right < heapSize && heap[right].value > heap[largest].value) largest = right;
            
            if (largest != currentIdx) {
                compareIdx = largest;
                PrepareSwap(currentIdx, compareIdx);
                state = AppState::SORT_HEAPIFY_SWAP;
                SetStatus(L"Heapifying Down...");
            } else {
                state = AppState::SORT_EXTRACT;
            }
        }
    }

    void PrepareSwap(int i, int j) {
        float ix, iy, jx, jy;
        GetNodePos(i, ix, iy); 
        GetNodePos(j, jx, jy);
        
        heap[i].targetX = jx;
        heap[i].targetY = jy;
        heap[j].targetX = ix;
        heap[j].targetY = iy;
        
        animationProgress = 0;
    }

    void Draw(HDC hdc) {
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(245, 245, 245));
        FillRect(memDC, &clientRect, bgBrush);
        DeleteObject(bgBrush);

        SetBkMode(memDC, TRANSPARENT);
        
        HPEN edgePen = CreatePen(PS_SOLID, 2, RGB(80, 80, 80));
        HPEN oldPen = (HPEN)SelectObject(memDC, edgePen);
        
        for (int i = 0; i < heapSize; ++i) {
            if (i > 0) {
                int parent = (i - 1) / 2;
                MoveToEx(memDC, (int)heap[parent].x, (int)heap[parent].y, NULL);
                LineTo(memDC, (int)heap[i].x, (int)heap[i].y);
            }
        }
        SelectObject(memDC, oldPen);
        DeleteObject(edgePen);

        HBRUSH nodeBrush = CreateSolidBrush(RGB(135, 206, 250));
        HBRUSH sortedBrush = CreateSolidBrush(RGB(144, 238, 144));
        HBRUSH activeBrush = CreateSolidBrush(RGB(255, 160, 122));
        HPEN nodeBorder = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        SelectObject(memDC, nodeBorder);

        for (int i = 0; i < heap.size(); ++i) {
            bool isSorted = (i >= heapSize);
            bool isActive = (i == currentIdx || i == compareIdx) && state != AppState::IDLE && state != AppState::SORT_COMPLETED;
            
            HBRUSH chkBrush = isSorted ? sortedBrush : (isActive ? activeBrush : nodeBrush);
            SelectObject(memDC, chkBrush);
            
            int x = (int)heap[i].x;
            int y = (int)heap[i].y;
            Ellipse(memDC, x - NODE_RADIUS, y - NODE_RADIUS, x + NODE_RADIUS, y + NODE_RADIUS);
            
            std::wstring s = std::to_wstring(heap[i].value);
            SIZE sz;
            GetTextExtentPoint32(memDC, s.c_str(), (int)s.length(), &sz);
            TextOut(memDC, x - sz.cx/2, y - sz.cy/2, s.c_str(), (int)s.length());
        }

        DeleteObject(nodeBrush);
        DeleteObject(sortedBrush);
        DeleteObject(activeBrush);
        DeleteObject(nodeBorder);

        BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
    }
    
    void SetStatus(std::wstring text) {
        SetWindowText(GetDlgItem(hWnd, ID_STATIC_STATUS), text.c_str());
    }
};

HeapVisualizer* g_Visualizer = nullptr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"HeapVisualizerClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW; 

    if (!RegisterClass(&wc)) return 0;

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Heap Visualizer",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    g_Visualizer = new HeapVisualizer(hwnd);
    
    SetTimer(hwnd, ID_TIMER, ANIMATION_SPEED_MS, NULL);

    ShowWindow(hwnd, nCmdShow);

    CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        15, 15, 80, 25, hwnd, (HMENU)ID_EDIT_INPUT, hInstance, NULL);

    CreateWindow(L"BUTTON", L"Insert Max", WS_CHILD | WS_VISIBLE,
        105, 15, 120, 25, hwnd, (HMENU)ID_BTN_INSERT_MAX, hInstance, NULL);
    
    CreateWindow(L"BUTTON", L"Insert Min", WS_CHILD | WS_VISIBLE,
        235, 15, 120, 25, hwnd, (HMENU)ID_BTN_INSERT_MIN, hInstance, NULL);

    CreateWindow(L"BUTTON", L"Heapsort", WS_CHILD | WS_VISIBLE,
        365, 15, 120, 25, hwnd, (HMENU)ID_BTN_HEAPSORT, hInstance, NULL);

    CreateWindow(L"BUTTON", L"Reset", WS_CHILD | WS_VISIBLE,
        495, 15, 80, 25, hwnd, (HMENU)ID_BTN_RESET, hInstance, NULL);
        
    CreateWindow(L"STATIC", L"Ready", WS_CHILD | WS_VISIBLE,
        15, 700, 800, 25, hwnd, (HMENU)ID_STATIC_STATUS, hInstance, NULL);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete g_Visualizer;
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static wchar_t buf[32];
    
    switch (uMsg) {
    case WM_COMMAND:
        if (g_Visualizer) {
            int id = LOWORD(wParam);
            if (id == ID_BTN_INSERT_MAX || id == ID_BTN_INSERT_MIN) {
                GetWindowText(GetDlgItem(hwnd, ID_EDIT_INPUT), buf, 32);
                if (wcslen(buf) > 0) {
                    try {
                        int val = std::stoi(buf); 
                        g_Visualizer->Insert(val, (id == ID_BTN_INSERT_MAX));
                        SetWindowText(GetDlgItem(hwnd, ID_EDIT_INPUT), L"");
                        SetFocus(GetDlgItem(hwnd, ID_EDIT_INPUT));
                    } catch(...) {}
                }
            } else if (id == ID_BTN_HEAPSORT) {
                g_Visualizer->StartSort();
            } else if (id == ID_BTN_RESET) {
                g_Visualizer->Reset();
            }
        }
        return 0;

    case WM_TIMER:
        if (g_Visualizer) g_Visualizer->Update();
        return 0;

    case WM_SIZE:
        if (g_Visualizer) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            SetWindowPos(GetDlgItem(hwnd, ID_STATIC_STATUS), NULL, 15, rc.bottom - 35, rc.right - 30, 25, SWP_NOZORDER);
            rc.top += 55;
            rc.bottom -= 45; 
            g_Visualizer->SetRect(rc);
        }
        return 0;

    case WM_ERASEBKGND:
        return 1; 

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (g_Visualizer) g_Visualizer->Draw(hdc);
            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
