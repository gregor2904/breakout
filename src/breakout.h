#pragma once

#include "resource.h"
#include "elements.h"

// test mode - ball moves with max speed and ricochet from playground bottom
// useful to verify that game can be won
constexpr bool c_TestMode = false;

struct GameSettings
{
    size_t livesStart = 3;
    float gameInformationHeight = 60.f; // pixels    
    std::set <size_t> hitsForSpeedUp{ 4, 12 };
    std::set <size_t> linesForSpeedUp{ 4, 6 };

    float playerHeight = 10.f; // pixels
    size_t playerStartPosition = 5;
    size_t playerPositionsCount = 10;
    size_t playerSplitOnHitTop = 2;

    float ballRadius = 7.f; // pixels
    float ballSpeedUpKoeff = 1.2f;
    float ballSpeedBase = c_TestMode ? 0.5f : 0.005f; // relative to rect
    PointF ballStartDirection{ 0.5f, 1.0f }; // x and y of vector (negative is up/left)
    PointF ballStartPosition{ 0.5f, 0.5f }; // relative to rect, center

    Targets::TLines targetLines = {
        {0,{Color::Yellow,1}},
        {1,{Color::Yellow,1}},
        {2,{Color::Green,3}},
        {3,{Color::Green,3}},
        {4,{Color::Orange,5}},
        {5,{Color::Orange,5}},
        {6,{Color::Red,7}},
        {7,{Color::Red,7}}
    };

    size_t targetsInLine = 13;
    float targetsMargin = 5.f; // pixels
    float targetsTopMargin = 30.f; // pixels
    float targetHeight = 10.f; // pixels
};

class GameMainWindow
{
public:

    bool Init(HINSTANCE hInstance, int nCmdShow)
    {
        RegisterMainWindowClass(L"BREAKOUT", hInstance);

        hWnd_ = CreateWindowW(L"BREAKOUT", L"Breakout Game", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
            0, 0, 500, 600, nullptr, nullptr, hInstance, this);

        if (!hWnd_)
        {
            return false;
        }

        RECT rc;
        GetWindowRect(hWnd_, &rc);

        const auto xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
        const auto yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;

        SetWindowPos(hWnd_, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        ShowWindow(hWnd_, nCmdShow);
        UpdateWindow(hWnd_);

        return true;
    }

    int Run()
    {
        CreateGameElements();

        gameInfo_->SetPaused(false);

        running_.store(true);
        workingThread_ = std::thread(&GameMainWindow::ProcessGameLogicAsync, this);

        MSG msg;

        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        running_.store(false);
        workingThread_.join();

        return (int)msg.wParam;
    }

private:

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        GameMainWindow* game = nullptr;
        if (message == WM_CREATE)
        {
            game = (GameMainWindow*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)game);

        }
        else
        {
            game = (GameMainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        }

        if (nullptr != game)
            return game->ProcessWindowMessage(message, wParam, lParam);

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    LRESULT CALLBACK ProcessWindowMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_PAINT:
        {
            std::lock_guard<std::mutex> lock{ lock_ };
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd_, &ps);
            Paint(hdc);
            EndPaint(hWnd_, &ps);
        }
        break;

        case WM_KEYDOWN:
            ProcessUserInput(wParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd_, message, wParam, lParam);
        }
        return 0;
    }

    void ProcessUserInput(WPARAM wParam)
    {
        bool bCanRedraw = false;
        {
            std::lock_guard<std::mutex> lock{ lock_ };

            switch (wParam)
            {
            case VK_LEFT:
                if (!gameInfo_->IsPaused())
                {
                    player_->MoveLeft();
                    bCanRedraw = true;
                }
                break;
            case VK_RIGHT:
                if (!gameInfo_->IsPaused())
                {
                    player_->MoveRight();
                    bCanRedraw = true;
                }
                break;
            case VK_SPACE:
                if (!gameInfo_->IsOver())
                {
                    gameInfo_->SetPaused(!gameInfo_->IsPaused());
                    bCanRedraw = true;
                }
                break;
            case VK_RETURN:
                CreateGameElements();
                gameInfo_->SetPaused(false);
                bCanRedraw = true;
                break;
            case VK_ESCAPE:
                DestroyWindow(hWnd_);
                break;
            default:
                break;
            }
        }

        if (bCanRedraw)
            RedrawWindow(hWnd_, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);
    }

    ATOM RegisterMainWindowClass(LPCWSTR className, HINSTANCE hInst)
    {
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);

        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInst;
        wcex.hIcon = LoadIconW(hInst, MAKEINTRESOURCE(IDI_BREAKOUT));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = 0;
        wcex.lpszClassName = className;
        wcex.hIconSm = LoadIconW(hInst, MAKEINTRESOURCE(IDI_SMALL));

        return RegisterClassExW(&wcex);
    }

    void Paint(HDC hdc)
    {
        RECT rect;
        ::GetClientRect(hWnd_, &rect);

        const auto width = rect.right - rect.left;
        const auto height = rect.bottom - rect.top;

        // prepare memory context
        Bitmap bitmap(width, height, PixelFormat32bppARGB);
        Graphics bGraphics(&bitmap);

        const RectF rectF(0.f, 0.f, REAL(width), REAL(height));

        DrawGameElements(&bGraphics, &rectF);

        // draw from memory to paint context
        Graphics graphics(hdc);
        graphics.DrawImage(&bitmap, 0.f, 0.f);
    }

    void DrawGameElements(Graphics* graphics, const RectF* rect)
    {
        if (!running_.load())
            return;

        playground_->Draw(graphics, rect);
        gameInfo_->Draw(graphics, rect);

        auto playgroundRect = playground_->GetBounds();

        player_->Draw(graphics, playgroundRect);
        ball_->Draw(graphics, playgroundRect);
        targets_->Draw(graphics, playgroundRect);
    }

    void ProcessGameLogicAsync()
    {
        while (running_.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            if (!IsWindow(hWnd_))
                continue;

            RedrawWindow(hWnd_, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);

            ProcessGameLogic();
        }
    }

    void CreateGameElements()
    {
        playground_ = std::make_unique< Playgroud>(
            Color::Black,
            settings_.gameInformationHeight);

        gameInfo_ = std::make_unique< GameInformation>(
            Color::DarkBlue,
            settings_.gameInformationHeight,
            settings_.livesStart);

        player_ = std::make_unique<Player>(
            Color::White,
            settings_.playerHeight,
            settings_.playerStartPosition,
            settings_.playerPositionsCount);

        ball_ = std::make_unique<Ball>(
            Color::White,
            settings_.ballRadius,
            settings_.ballSpeedBase,
            settings_.ballStartDirection,
            settings_.ballStartPosition,
            settings_.targetHeight + settings_.ballRadius * 1.5f);

        targets_ = std::make_unique<Targets>(
            settings_.targetLines,
            settings_.targetsInLine,
            settings_.targetsMargin,
            settings_.targetsTopMargin,
            settings_.targetHeight);
    }

    void ProcessGameLogic()
    {
        std::lock_guard<std::mutex> lock{ lock_ };

        if (gameInfo_->IsOver())
            return;

        if (gameInfo_->IsPaused())
            return;

        if (targets_->IsEmpty())
        {
            gameInfo_->SetPaused(true);
            gameInfo_->SetVictory();
            return;
        }

        ProcessBallHits();

        if (!gameInfo_->IsOver())
            ball_->CalcNextPosition();
    }

    void ProcessBallHits()
    {
        if (ball_->HitWithTop(player_.get(), Ball::eHitType::hitOutside)
            || ball_->HitWithBottom(player_.get(), Ball::eHitType::hitOutside))
        {
            return;
        }

        if (ball_->HitWithBottom(playground_.get(), Ball::eHitType::hitInside))
        {
            if (c_TestMode)
                return;

            gameInfo_->RemoveLife();

            if (gameInfo_->NoMoreLives())
            {
                ball_->SetColor(Color::Red);
                gameInfo_->SetPaused(true);
                gameInfo_->SetFail();
            }
        }

        if (ball_->HitWithTop(playground_.get(), Ball::eHitType::hitInside))
        {
            if (!gameInfo_->IsHitTop())
            {
                gameInfo_->SetHitTop();
                player_->SplitBy(settings_.playerSplitOnHitTop);
            }
        }

        {
            auto target = targets_->GetTargetHitWithBall(ball_.get());
            if (nullptr != target)
            {
                ProcessHitTarget(target);
                targets_->RemoveTarget(target);
                //if (c_TestMode)
                //    gameInfo_->SetPaused(true);

                return;
            }
        }

        ball_->HitWithLeft(playground_.get(), Ball::eHitType::hitInside);
        ball_->HitWithRight(playground_.get(), Ball::eHitType::hitInside);
    }

    void ProcessHitTarget(const Target* target)
    {
        gameInfo_->AddToScore(target->GetCost());

        gameInfo_->IncrementHits();

        const auto hits = gameInfo_->GetHits();

        const auto line = target->GetLine();

        const auto newHitLine = !gameInfo_->IsHitLine(line);
        if (newHitLine)
            gameInfo_->SetHitLine(line);

        const bool speedUpBall = (settings_.hitsForSpeedUp.count(hits) > 0) || (newHitLine && settings_.linesForSpeedUp.count(line) > 0);

        if (speedUpBall)
        {
            ball_->SpeedUp(settings_.ballSpeedUpKoeff);
        }
    }

private:
    GameSettings settings_;

    HWND hWnd_ = nullptr;

    std::unique_ptr<Playgroud> playground_;
    std::unique_ptr<GameInformation> gameInfo_;
    std::unique_ptr<Player> player_;
    std::unique_ptr<Ball> ball_;
    std::unique_ptr<Targets> targets_;

    std::thread workingThread_;
    std::atomic_bool running_ = false;
    std::mutex lock_;
};